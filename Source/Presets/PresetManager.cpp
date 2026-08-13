#include "PresetManager.h"

#include "../PluginProcessor.h"

namespace cabrot::presets
{
namespace
{
const juce::Identifier kPresetType   { "CABROT_PRESET" };
const juce::Identifier kParamType    { "PARAM" };
const juce::Identifier kNameProperty { "name" };
const juce::Identifier kIdProperty   { "id" };
const juce::Identifier kValueProperty{ "value" };

constexpr const char* kPresetExtension = ".cabrot";
} // namespace

const juce::StringArray& PresetManager::presetScope()
{
    static const juce::StringArray scope {
        params::mode,
        params::fizzHunt,
        params::edgePreserve,
        params::cabSmooth,
        params::digitalSand,
        params::airRot,
        params::reapMix,
        params::detectorFocus,
        params::clampSpeed,
        params::maxReapDb,
        params::pickWindow,
        params::stereoLink,
        params::autoGain
    };

    return scope;
}

PresetManager::PresetManager (juce::AudioProcessorValueTreeState& state)
    : apvts (state)
{
}

//==============================================================================
juce::String PresetManager::factoryName (int index)
{
    if (! juce::isPositiveAndBelow (index, kNumFactoryPresets))
        return {};

    return juce::String (kFactoryPresets[index].name);
}

bool PresetManager::isFactoryName (const juce::String& name)
{
    for (const auto& preset : kFactoryPresets)
        if (name == preset.name)
            return true;

    return false;
}

juce::File PresetManager::userPresetDirectory()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
              .getChildFile ("Dead Pixel Harmonix")
              .getChildFile ("Cab Rot")
              .getChildFile ("Presets");
}

juce::File PresetManager::fileForUserPreset (const juce::String& name)
{
    // The display name is stored inside the file, so the filename only has to
    // be legal and stable. Hashing the trimmed name keeps two presets that
    // sanitise to the same string ("Bob's" and "Bobs") in separate files.
    const auto trimmed = name.trim();
    const auto safe = juce::File::createLegalFileName (trimmed).substring (0, 48);
    const auto suffix = juce::String::toHexString ((juce::int64) trimmed.hashCode64());

    return userPresetDirectory().getChildFile (safe + "-" + suffix + kPresetExtension);
}

juce::StringArray PresetManager::userPresetNames() const
{
    juce::StringArray names;

    const auto dir = userPresetDirectory();
    if (! dir.isDirectory())
        return names;

    for (const auto& entry : juce::RangedDirectoryIterator (dir, false, juce::String ("*") + kPresetExtension))
    {
        if (auto xml = juce::XmlDocument::parse (entry.getFile()))
        {
            const auto tree = juce::ValueTree::fromXml (*xml);
            if (tree.hasType (kPresetType))
            {
                const auto name = tree.getProperty (kNameProperty).toString();
                if (name.isNotEmpty())
                    names.addIfNotAlreadyThere (name);
            }
        }
    }

    names.sortNatural();
    return names;
}

//==============================================================================
juce::ValueTree PresetManager::captureScope (const juce::String& name) const
{
    juce::ValueTree preset (kPresetType);
    preset.setProperty (kNameProperty, name, nullptr);

    for (const auto& id : presetScope())
    {
        if (auto* param = apvts.getParameter (id))
        {
            juce::ValueTree entry (kParamType);
            entry.setProperty (kIdProperty, id, nullptr);
            // Stored denormalised so a future range change stays readable
            // rather than silently remapping every saved preset.
            entry.setProperty (kValueProperty, param->convertFrom0to1 (param->getValue()), nullptr);
            preset.appendChild (entry, nullptr);
        }
    }

    return preset;
}

void PresetManager::applyScope (const juce::ValueTree& preset)
{
    for (const auto& entry : preset)
    {
        if (! entry.hasType (kParamType))
            continue;

        const auto id = entry.getProperty (kIdProperty).toString();
        if (! presetScope().contains (id))
            continue;

        if (auto* param = apvts.getParameter (id))
        {
            const auto value = static_cast<float> (entry.getProperty (kValueProperty));
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (value));
            param->endChangeGesture();
        }
    }
}

//==============================================================================
bool PresetManager::loadFactory (int index)
{
    if (! juce::isPositiveAndBelow (index, kNumFactoryPresets))
        return false;

    const auto& preset = kFactoryPresets[index];

    const auto set = [this] (const juce::String& id, float value)
    {
        if (auto* param = apvts.getParameter (id))
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->convertTo0to1 (value));
            param->endChangeGesture();
        }
    };

    set (params::mode,          (float) preset.mode);
    set (params::fizzHunt,      preset.fizzHunt);
    set (params::edgePreserve,  preset.edgePreserve);
    set (params::cabSmooth,     preset.cabSmooth);
    set (params::digitalSand,   preset.digitalSand);
    set (params::airRot,        preset.airRot);
    set (params::reapMix,       preset.reapMix);
    set (params::detectorFocus, preset.detectorFocus);
    set (params::clampSpeed,    preset.clampSpeed);
    set (params::maxReapDb,     preset.maxReapDb);
    set (params::pickWindow,    preset.pickWindow);
    set (params::stereoLink,    (float) preset.stereoLink);
    set (params::autoGain,      preset.autoGain ? 1.0f : 0.0f);

    currentName = preset.name;
    return true;
}

bool PresetManager::loadUser (const juce::String& name)
{
    const auto file = fileForUserPreset (name);
    if (! file.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return false;

    const auto tree = juce::ValueTree::fromXml (*xml);
    if (! tree.hasType (kPresetType))
        return false;

    applyScope (tree);
    currentName = name;
    return true;
}

bool PresetManager::saveUser (const juce::String& name)
{
    const auto trimmed = name.trim();
    if (trimmed.isEmpty() || isFactoryName (trimmed))
        return false;

    const auto dir = userPresetDirectory();
    if (! dir.isDirectory() && ! dir.createDirectory().wasOk())
        return false;

    const auto preset = captureScope (trimmed);
    auto xml = preset.createXml();
    if (xml == nullptr)
        return false;

    if (! xml->writeTo (fileForUserPreset (trimmed)))
        return false;

    currentName = trimmed;
    return true;
}

bool PresetManager::deleteUser (const juce::String& name)
{
    const auto file = fileForUserPreset (name);
    if (! file.existsAsFile())
        return false;

    if (! file.deleteFile())
        return false;

    if (currentName == name)
        currentName = {};

    return true;
}
} // namespace cabrot::presets
