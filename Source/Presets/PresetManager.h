#pragma once

#include "FactoryPresets.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace cabrot::presets
{
/**
    Loads, saves and deletes presets against a live APVTS.

    A preset covers voicing only. `kPresetScope` is the whole list, and it is
    the single place that decides what a preset is allowed to touch: adding a
    parameter to the plugin does not silently add it to every preset.

    Factory presets are compiled in and read-only. User presets are XML files
    under the user's application data folder, one per preset. The display name
    lives in a ValueTree property rather than in the filename, so a name with
    an apostrophe in it survives the round trip and the file on disk still has
    a legal name.
*/
class PresetManager
{
public:
    /** Every parameter a preset carries. Nothing outside this list is
        written when a preset loads, so trims, oversampling, Delta Listen,
        the A/B slot and UI Animation all survive a preset change. */
    static const juce::StringArray& presetScope();

    explicit PresetManager (juce::AudioProcessorValueTreeState& state);

    //==============================================================================
    static int         numFactoryPresets() noexcept { return kNumFactoryPresets; }
    static juce::String factoryName (int index);

    /** Names of the user presets currently on disk, sorted. */
    juce::StringArray userPresetNames() const;

    //==============================================================================
    /** Applies a compiled-in preset. Returns false on a bad index. */
    bool loadFactory (int index);

    /** Applies a user preset by display name. Returns false if it is missing
        or unreadable. */
    bool loadUser (const juce::String& name);

    /** Writes the current in-scope parameter values as a user preset,
        overwriting any existing one with the same display name. */
    bool saveUser (const juce::String& name);

    /** Deletes a user preset by display name. */
    bool deleteUser (const juce::String& name);

    //==============================================================================
    /** The last preset loaded or saved, for display. Empty once the user has
        edited a control, since the state no longer matches the preset. */
    juce::String currentPresetName() const { return currentName; }
    void clearCurrentPresetName() { currentName = {}; }

    /** True when `name` is one of the compiled-in twelve. */
    static bool isFactoryName (const juce::String& name);

    static juce::File userPresetDirectory();

private:
    /** Turns a display name into a legal, collision-resistant filename. */
    static juce::File fileForUserPreset (const juce::String& name);

    juce::ValueTree captureScope (const juce::String& name) const;
    void applyScope (const juce::ValueTree& preset);

    juce::AudioProcessorValueTreeState& apvts;
    juce::String currentName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
} // namespace cabrot::presets
