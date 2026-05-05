#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace cabrot
{
// Stable parameter identifiers. Keep these in one place so attachments,
// state restoration, and host automation all agree.
namespace params
{
inline const juce::String fizzHunt     { "fizzHunt" };
inline const juce::String edgePreserve { "edgePreserve" };
inline const juce::String cabSmooth    { "cabSmooth" };
inline const juce::String digitalSand  { "digitalSand" };
inline const juce::String airRot       { "airRot" };
inline const juce::String reapMix      { "reapMix" };

inline const juce::String mode         { "mode" };
inline const juce::String oversampling { "oversampling" };
inline const juce::String deltaListen  { "deltaListen" };
inline const juce::String aOrB         { "aOrB" };
inline const juce::String inputGain    { "inputGain" };
inline const juce::String outputGain   { "outputGain" };

// Crypt advanced - UI lands in Phase 8; parameters declared here so any
// future automation lane can reach them without a state migration.
inline const juce::String stereoLink    { "stereoLink" };
inline const juce::String detectorFocus { "detectorFocus" };
inline const juce::String clampSpeed    { "clampSpeed" };
inline const juce::String maxReapDb     { "maxReapDb" };
inline const juce::String pickWindow    { "pickWindow" };
inline const juce::String quality       { "quality" };
inline const juce::String autoGain      { "autoGain" };
inline const juce::String uiAnimation   { "uiAnimation" };
} // namespace params

class CabRotProcessor final : public juce::AudioProcessor
{
public:
    CabRotProcessor();
    ~CabRotProcessor() override = default;

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                       { return true; }

    const juce::String getName() const override           { return "Cab Rot"; }
    bool acceptsMidi()  const override                    { return false; }
    bool producesMidi() const override                    { return false; }
    bool isMidiEffect() const override                    { return false; }
    double getTailLengthSeconds() const override          { return 0.0; }

    int  getNumPrograms() override                        { return 1; }
    int  getCurrentProgram() override                     { return 0; }
    void setCurrentProgram (int) override                 {}
    const juce::String getProgramName (int) override      { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getApvts() noexcept { return apvts; }
    juce::UndoManager&                  getUndoManager() noexcept { return undoManager; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts { *this, &undoManager,
                                               juce::Identifier ("CABROT"),
                                               buildParameterLayout() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotProcessor)
};
} // namespace cabrot
