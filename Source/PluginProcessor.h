#pragma once

#include "DSP/BandSplitter.h"
#include "DSP/DynamicReducer.h"
#include "DSP/InputTrim.h"
#include "DSP/ReapMixer.h"
#include "DSP/TransientDetector.h"
#include "DSP/Tuning.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>
#include <vector>

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

    /** Peak reduction on one of the four processed bands, in dB. Phase 6
        reads this for the Wasp Meter; nothing draws it yet. */
    float getBandReductionDb (int processedBand) const noexcept;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();

    void updateDspParameters() noexcept;
    void processChunk (juce::AudioBuffer<float>& block, int numChannels, int numSamples) noexcept;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts { *this, &undoManager,
                                               juce::Identifier ("CABROT"),
                                               buildParameterLayout() };

    // ------------------------------------------------------------------
    // Phase 4 DSP
    // ------------------------------------------------------------------
    static constexpr int numProcessedBands = dsp::tuning::kNumProcessedBands;

    dsp::InputTrim    inputStage, outputStage;
    dsp::BandSplitter splitter;
    dsp::ReapMixer    mixer;

    std::array<dsp::TransientDetector, numProcessedBands> detectors;
    std::array<dsp::DynamicReducer,    numProcessedBands> reducers;

    std::array<juce::AudioBuffer<float>, dsp::BandSplitter::numBands> bandBuffers;
    juce::AudioBuffer<float> deltaBuffer;
    std::vector<float>       gateScratch;

    std::array<std::atomic<float>, numProcessedBands> bandReductionDb { { {}, {}, {}, {} } };

    // Resolved once in the constructor. Reading these per block avoids a
    // string lookup on the audio thread.
    struct ParamPointers
    {
        std::atomic<float>* fizzHunt      {};
        std::atomic<float>* edgePreserve  {};
        std::atomic<float>* cabSmooth     {};
        std::atomic<float>* digitalSand   {};
        std::atomic<float>* airRot        {};
        std::atomic<float>* reapMix       {};
        std::atomic<float>* inputGain     {};
        std::atomic<float>* outputGain    {};
        std::atomic<float>* stereoLink    {};
        std::atomic<float>* clampSpeed    {};
        std::atomic<float>* maxReapDb     {};
        std::atomic<float>* pickWindow    {};
        std::atomic<float>* autoGain      {};
    } p;

    int  preparedChannels { 2 };
    int  preparedBlockSize { 512 };
    bool isPrepared { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotProcessor)
};
} // namespace cabrot
