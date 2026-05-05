#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace cabrot
{
namespace
{
juce::AudioParameterFloatAttributes percentAttrs()
{
    return juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 0) + "%"; })
        .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); });
}

juce::AudioParameterFloatAttributes dbAttrs()
{
    return juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " dB"; })
        .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); });
}

juce::AudioParameterFloatAttributes msAttrs()
{
    return juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return juce::String (v, 1) + " ms"; })
        .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); });
}

juce::AudioParameterFloatAttributes hzAttrs()
{
    return juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v)) + " Hz"; })
        .withValueFromStringFunction ([] (const juce::String& s) { return s.getFloatValue(); });
}
}

juce::AudioProcessorValueTreeState::ParameterLayout CabRotProcessor::buildParameterLayout()
{
    using FloatParam  = juce::AudioParameterFloat;
    using ChoiceParam = juce::AudioParameterChoice;
    using BoolParam   = juce::AudioParameterBool;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Six main knobs: 0..100 with one-decimal step
    auto pct = juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f);
    auto a   = percentAttrs();
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::fizzHunt,     1), "Fizz Hunt",     pct, 50.0f, a));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::edgePreserve, 1), "Edge Preserve", pct, 50.0f, a));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::cabSmooth,    1), "Cab Smooth",    pct, 50.0f, a));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::digitalSand,  1), "Digital Sand",  pct, 50.0f, a));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::airRot,       1), "Air Rot",       pct, 50.0f, a));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::reapMix,      1), "Reap Mix",      pct, 100.0f, a));

    // Mode (locked decision: 6 distinct amp profiles)
    juce::StringArray modeNames { "5150", "Recto", "HM-2", "Djent", "Blackened", "Sludge" };
    layout.add (std::make_unique<ChoiceParam> (juce::ParameterID (params::mode, 1), "Mode", modeNames, 0));

    // Oversampling (locked decision #2: Off / 2x / 4x)
    juce::StringArray osNames { "Off", "2x", "4x" };
    layout.add (std::make_unique<ChoiceParam> (juce::ParameterID (params::oversampling, 1), "Oversampling", osNames, 0));

    // Booleans
    layout.add (std::make_unique<BoolParam> (juce::ParameterID (params::deltaListen, 1), "Delta Listen", false));
    layout.add (std::make_unique<BoolParam> (juce::ParameterID (params::aOrB,        1), "A/B Slot",     false));

    // Input/Output trim (dB)
    auto db = juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f);
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::inputGain,  1), "Input Gain",  db, 0.0f, dbAttrs()));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::outputGain, 1), "Output Gain", db, 0.0f, dbAttrs()));

    // Crypt advanced (Phase 8 wires UI). Declared now so future automation
    // lanes resolve correctly.
    juce::StringArray stereoNames { "Linked", "Partial", "Dual Mono" };
    layout.add (std::make_unique<ChoiceParam> (juce::ParameterID (params::stereoLink, 1), "Stereo Behavior", stereoNames, 0));

    juce::StringArray qualityNames { "Eco", "Normal", "Ritual" };
    layout.add (std::make_unique<ChoiceParam> (juce::ParameterID (params::quality, 1), "Quality", qualityNames, 1));

    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::detectorFocus, 1), "Detector Focus",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f), 50.0f, percentAttrs()));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::clampSpeed, 1), "Clamp Speed",
        juce::NormalisableRange<float> (0.5f, 50.0f, 0.1f, 0.4f), 8.0f, msAttrs()));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::maxReapDb, 1), "Max Reap dB",
        juce::NormalisableRange<float> (1.0f, 12.0f, 0.1f), 6.0f, dbAttrs()));
    layout.add (std::make_unique<FloatParam> (juce::ParameterID (params::pickWindow, 1), "Pick Window",
        juce::NormalisableRange<float> (1.0f, 30.0f, 0.1f), 5.0f, msAttrs()));

    layout.add (std::make_unique<BoolParam>  (juce::ParameterID (params::autoGain,    1), "Auto Gain",    true));
    layout.add (std::make_unique<BoolParam>  (juce::ParameterID (params::uiAnimation, 1), "UI Animation", true));

    juce::ignoreUnused (hzAttrs);
    return layout;
}

CabRotProcessor::CabRotProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void CabRotProcessor::prepareToPlay (double, int)
{
}

void CabRotProcessor::releaseResources()
{
}

bool CabRotProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainIn == mainOut;
}

void CabRotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    // Phase 3: APVTS exists but no DSP yet. processBlock remains a pure
    // passthrough so the null test continues at 16384/16384. Phase 4 wires
    // the band splitter and dynamic reducer.
    juce::ignoreUnused (totalNumInputChannels);
}

juce::AudioProcessorEditor* CabRotProcessor::createEditor()
{
    return new CabRotEditor (*this);
}

void CabRotProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
    }
}

void CabRotProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}
} // namespace cabrot

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new cabrot::CabRotProcessor();
}
