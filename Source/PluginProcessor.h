#pragma once

#include "DSP/BandSplitter.h"
#include "DSP/DynamicReducer.h"
#include "DSP/InputTrim.h"
#include "DSP/ModeConfig.h"
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

class CabRotProcessor final : public juce::AudioProcessor,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::Timer
{
public:
    CabRotProcessor();
    ~CabRotProcessor() override;

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
    int getActiveAbSlot() const noexcept
    {
        return activeAbSlot.load (std::memory_order_acquire);
    }

    struct UiTelemetry
    {
        std::array<float, dsp::tuning::kNumProcessedBands> bandReductionDb {};
        float inputPeak { 0.0f };
        float outputPeak { 0.0f };
        float cpuPercent { -1.0f };
        bool engineLive { false };
    };

    /** Latest complete host-block reduction for diagnostics. */
    float getBandReductionDb (int processedBand) const noexcept;

    /** Consumes maxima accumulated since the previous UI poll. */
    UiTelemetry consumeUiTelemetry() noexcept;
    void discardUiPeakTelemetry() noexcept;

private:
    enum class AbSlot : int { a = 0, b = 1 };

    static juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();

    void parameterChanged (const juce::String& parameterId, float newValue) override;
    void timerCallback() override;
    void switchToSlotLocked (AbSlot target);

    static void forceSlotMarker (juce::ValueTree& state, AbSlot slot);
    static AbSlot readSlotMarker (const juce::ValueTree& state);
    juce::ValueTree& slotTree (AbSlot slot) noexcept;

    struct BlockTelemetry
    {
        std::array<float, dsp::tuning::kNumProcessedBands> bandReductionDb {};
        float inputPeak { 0.0f };
        float outputPeak { 0.0f };
    };

    void initialiseModeSmoothing (double sampleRate) noexcept;
    void updateDspParameters (int numSamples) noexcept;
    void processChunk (juce::AudioBuffer<float>& block, int numChannels,
                       int numSamples, bool listenToRemoved,
                       BlockTelemetry& telemetry) noexcept;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts { *this, &undoManager,
                                               juce::Identifier ("CABROT"),
                                               buildParameterLayout() };

    // A/B snapshots are detached APVTS trees. Parameter listeners can run on
    // the audio thread, so they only publish a requested slot; the processor
    // timer performs the non-realtime copy/replace work on the message thread.
    juce::ValueTree slotStateA, slotStateB;
    juce::CriticalSection abStateLock;
    std::atomic<int> pendingAbSlot  { -1 };
    std::atomic<int> applyingAbSlot { -1 };
    std::atomic<int> activeAbSlot   { 0 };

    // ------------------------------------------------------------------
    // Phase 4 DSP
    // ------------------------------------------------------------------
    static constexpr int numProcessedBands = dsp::tuning::kNumProcessedBands;

    dsp::InputTrim    inputStage, outputStage;
    dsp::BandSplitter splitter;
    dsp::ReapMixer    mixer;

    std::array<dsp::TransientDetector, numProcessedBands> detectors;
    std::array<dsp::DynamicReducer,    numProcessedBands> reducers;

    // The mode table changes derivative targets, not user parameters. A slow
    // 300 ms ramp keeps profile automation free of block-edge discontinuities.
    juce::SmoothedValue<float> modeThresholdOffsetDb;
    std::array<juce::SmoothedValue<float>, numProcessedBands> modeCeilingScale;
    juce::SmoothedValue<float> modeAttackScale;
    juce::SmoothedValue<float> modeEdgeBias;
    juce::SmoothedValue<float> modeShelfStart;

    std::array<juce::AudioBuffer<float>, dsp::BandSplitter::numBands> bandBuffers;
    juce::AudioBuffer<float> deltaBuffer;
    std::vector<float>       gateScratch;

    std::array<std::atomic<float>, numProcessedBands> bandReductionDb;
    std::array<std::atomic<float>, numProcessedBands> uiBandReductionMax;
    std::atomic<float> uiInputPeak { 0.0f };
    std::atomic<float> uiOutputPeak { 0.0f };
    std::atomic<float> cpuPercent { -1.0f };
    std::atomic<juce::int64> lastInputActivityTicks { 0 };

    double cpuLoadEma { 0.0 };
    double preparedSampleRate { 48000.0 };
    bool cpuLoadEmaSeeded { false };

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
        std::atomic<float>* deltaListen   {};
        std::atomic<float>* mode          {};
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
