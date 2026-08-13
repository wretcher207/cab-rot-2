#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace cabrot
{
namespace
{
constexpr float kEngineLiveThresholdDb = -72.0f;
constexpr double kEngineLiveHoldSeconds = 1.0;
constexpr double kModeRampSeconds = 0.300;

const juce::Identifier kPluginStateType { "CABROT_PLUGIN_STATE" };
const juce::Identifier kSlotAType       { "CABROT_SLOT_A" };
const juce::Identifier kSlotBType       { "CABROT_SLOT_B" };
const juce::Identifier kFormatVersion   { "formatVersion" };
const juce::Identifier kActiveSlot      { "activeSlot" };
constexpr int kPluginStateFormatVersion = 2;

bool readWrappedSlot (const juce::ValueTree& root,
                      const juce::Identifier& wrapperType,
                      const juce::Identifier& payloadType,
                      juce::ValueTree& result)
{
    juce::ValueTree wrapper;
    int wrapperCount = 0;

    for (const auto& child : root)
    {
        if (child.hasType (wrapperType))
        {
            wrapper = child;
            ++wrapperCount;
        }
    }

    if (wrapperCount != 1 || wrapper.getNumChildren() > 1)
        return false;

    if (wrapper.getNumChildren() == 0)
    {
        result = {};
        return true;
    }

    const auto payload = wrapper.getChild (0);
    if (! payload.hasType (payloadType))
        return false;

    result = payload.createCopy();
    return true;
}

void appendWrappedSlot (juce::ValueTree& root,
                        const juce::Identifier& wrapperType,
                        const juce::ValueTree& payload)
{
    juce::ValueTree wrapper (wrapperType);
    if (payload.isValid())
        wrapper.appendChild (payload.createCopy(), nullptr);

    root.appendChild (wrapper, nullptr);
}

void publishMaximum (std::atomic<float>& mailbox, float value) noexcept
{
    auto current = mailbox.load (std::memory_order_relaxed);
    while (value > current
           && ! mailbox.compare_exchange_weak (current, value,
                                                std::memory_order_relaxed,
                                                std::memory_order_relaxed))
    {
    }
}

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
    const auto abAttributes = juce::AudioParameterBoolAttributes()
        .withAutomatable (false)
        .withMeta (true);
    layout.add (std::make_unique<BoolParam> (juce::ParameterID (params::aOrB, 1),
                                              "A/B Slot", false, abAttributes));

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
    p.fizzHunt     = apvts.getRawParameterValue (params::fizzHunt);
    p.edgePreserve = apvts.getRawParameterValue (params::edgePreserve);
    p.cabSmooth    = apvts.getRawParameterValue (params::cabSmooth);
    p.digitalSand  = apvts.getRawParameterValue (params::digitalSand);
    p.airRot       = apvts.getRawParameterValue (params::airRot);
    p.reapMix      = apvts.getRawParameterValue (params::reapMix);
    p.inputGain    = apvts.getRawParameterValue (params::inputGain);
    p.outputGain   = apvts.getRawParameterValue (params::outputGain);
    p.deltaListen  = apvts.getRawParameterValue (params::deltaListen);
    p.mode         = apvts.getRawParameterValue (params::mode);
    p.oversampling = apvts.getRawParameterValue (params::oversampling);
    p.stereoLink   = apvts.getRawParameterValue (params::stereoLink);
    p.detectorFocus = apvts.getRawParameterValue (params::detectorFocus);
    p.clampSpeed   = apvts.getRawParameterValue (params::clampSpeed);
    p.maxReapDb    = apvts.getRawParameterValue (params::maxReapDb);
    p.pickWindow   = apvts.getRawParameterValue (params::pickWindow);
    p.autoGain     = apvts.getRawParameterValue (params::autoGain);

    for (auto& value : bandReductionDb)
        value.store (0.0f, std::memory_order_relaxed);

    for (auto& value : uiBandReductionMax)
        value.store (0.0f, std::memory_order_relaxed);

    apvts.addParameterListener (params::aOrB, this);
    startTimerHz (60);
}

CabRotProcessor::~CabRotProcessor()
{
    stopTimer();
    apvts.removeParameterListener (params::aOrB, this);
}

void CabRotProcessor::parameterChanged (const juce::String& parameterId, float newValue)
{
    if (parameterId != params::aOrB)
        return;

    const int requested = newValue >= 0.5f ? 1 : 0;
    if (requested == applyingAbSlot.load (std::memory_order_acquire))
        return;

    // APVTS listeners may run on the audio thread. The timer performs the
    // locking and ValueTree work later on the message thread.
    pendingAbSlot.store (requested, std::memory_order_release);
}

void CabRotProcessor::timerCallback()
{
    if (latencyReportDirty.exchange (false, std::memory_order_acq_rel))
        setLatencySamples (pendingLatencySamples.load (std::memory_order_relaxed));

    const juce::ScopedLock lock (abStateLock);
    const int requested = pendingAbSlot.exchange (-1, std::memory_order_acq_rel);

    if (requested == 0 || requested == 1)
        switchToSlotLocked (requested == 0 ? AbSlot::a : AbSlot::b);
}

void CabRotProcessor::selectAbSlotFromUi (int slotIndex)
{
    jassert (juce::MessageManager::existsAndIsCurrentThread());

    const int requested = juce::jlimit (0, 1, slotIndex);
    auto* selector = apvts.getParameter (params::aOrB);
    if (selector == nullptr)
        return;

    selector->beginChangeGesture();
    selector->setValueNotifyingHost (selector->convertTo0to1 (
        static_cast<float> (requested)));
    selector->endChangeGesture();

    // Button clicks already run on the message thread, so complete their
    // state transition synchronously. The timer remains only as a safe
    // fallback for non-automated external parameter writes.
    const juce::ScopedLock lock (abStateLock);
    pendingAbSlot.exchange (-1, std::memory_order_acq_rel);
    switchToSlotLocked (requested == 0 ? AbSlot::a : AbSlot::b);
}

juce::ValueTree& CabRotProcessor::slotTree (AbSlot slot) noexcept
{
    return slot == AbSlot::a ? slotStateA : slotStateB;
}

void CabRotProcessor::forceSlotMarker (juce::ValueTree& state, AbSlot slot)
{
    if (! state.isValid())
        return;

    const juce::Identifier paramType { "PARAM" };
    const juce::Identifier idProperty { "id" };
    const juce::Identifier valueProperty { "value" };
    const float marker = slot == AbSlot::b ? 1.0f : 0.0f;

    for (auto child : state)
    {
        if (child.hasType (paramType)
            && child.getProperty (idProperty).toString() == params::aOrB)
        {
            child.setProperty (valueProperty, marker, nullptr);
            return;
        }
    }

    juce::ValueTree markerTree (paramType);
    markerTree.setProperty (idProperty, params::aOrB, nullptr);
    markerTree.setProperty (valueProperty, marker, nullptr);
    state.appendChild (markerTree, nullptr);
}

CabRotProcessor::AbSlot CabRotProcessor::readSlotMarker (const juce::ValueTree& state)
{
    const juce::Identifier paramType { "PARAM" };
    const juce::Identifier idProperty { "id" };
    const juce::Identifier valueProperty { "value" };

    for (const auto& child : state)
    {
        if (child.hasType (paramType)
            && child.getProperty (idProperty).toString() == params::aOrB)
        {
            return static_cast<float> (child.getProperty (valueProperty, 0.0f)) >= 0.5f
                ? AbSlot::b : AbSlot::a;
        }
    }

    return AbSlot::a;
}

void CabRotProcessor::switchToSlotLocked (AbSlot target)
{
    const auto active = activeAbSlot.load (std::memory_order_acquire) == 1
        ? AbSlot::b : AbSlot::a;

    if (target == active)
        return;

    auto departing = apvts.copyState();
    if (! departing.isValid())
        return;

    // The selector parameter changes before this callback. Put the departing
    // slot's own marker back into its detached snapshot before storing it.
    forceSlotMarker (departing, active);
    slotTree (active) = departing.createCopy();

    auto& arriving = slotTree (target);
    if (! arriving.isValid())
        arriving = departing.createCopy();

    forceSlotMarker (arriving, target);

    const int targetIndex = static_cast<int> (target);
    applyingAbSlot.store (targetIndex, std::memory_order_release);
    abApplyGeneration.fetch_add (1, std::memory_order_acq_rel); // odd: replacing
    apvts.replaceState (arriving.createCopy());
    activeAbSlot.store (targetIndex, std::memory_order_release);
    abApplyGeneration.fetch_add (1, std::memory_order_release); // even: complete
    applyingAbSlot.store (-1, std::memory_order_release);
}

void CabRotProcessor::prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock)
{
    preparedSampleRate = sampleRate;
    preparedChannels  = juce::jmax (1, getTotalNumInputChannels());
    preparedBlockSize = juce::jmax (32, maximumExpectedSamplesPerBlock);

    inputStage .prepare (sampleRate, preparedChannels);
    outputStage.prepare (sampleRate, preparedChannels);

    // The reduction core may run at up to 4x, so every scratch buffer it
    // touches is sized for the largest oversampled chunk up front. Factor
    // switches then never need an audio-thread allocation.
    const int maxCoreBlock = preparedBlockSize * kMaxOsFactor;

    for (auto& b : bandBuffers)
    {
        b.setSize (preparedChannels, maxCoreBlock, false, false, true);
        b.clear();
    }

    deltaBuffer.setSize (preparedChannels, maxCoreBlock, false, false, true);
    deltaBuffer.clear();

    gateScratch.assign ((size_t) maxCoreBlock, 0.0f);
    osChannelPointers.assign ((size_t) preparedChannels, nullptr);

    // Hand-built stages instead of JUCE's stock quality presets. The first
    // stage is the only one whose transition band can touch the audible
    // range, so it gets the tight -90 dB linear-phase filter. The second
    // stage's transition sits above 40 kHz at any supported host rate, so a
    // wide, short filter protects the audio band just as completely at a
    // fraction of the stock preset's cost (measured: 14.8% -> ~10% at 4x).
    using Oversampler = juce::dsp::Oversampling<float>;

    for (size_t stage = 0; stage < oversamplers.size(); ++stage)
    {
        auto os = std::make_unique<Oversampler> ((size_t) preparedChannels);
        os->clearOversamplingStages();
        os->addOversamplingStage (Oversampler::filterHalfBandFIREquiripple,
                                  0.05f, -90.0f, 0.06f, -80.0f);

        if (stage == 1)
            os->addOversamplingStage (Oversampler::filterHalfBandFIREquiripple,
                                      0.25f, -80.0f, 0.26f, -75.0f);

        os->setUsingIntegerLatency (true);
        os->initProcessing ((size_t) preparedBlockSize);
        oversamplers[stage] = std::move (os);
    }

    // Prepares splitter, detectors, reducers, and mixer at the effective rate.
    currentOsChoice = -1;
    applyOversamplingConfig (juce::jlimit (0, 2,
        juce::roundToInt (p.oversampling->load (std::memory_order_relaxed))));

    // prepareToPlay runs off the audio thread, so report directly.
    latencyReportDirty.store (false, std::memory_order_release);
    setLatencySamples (pendingLatencySamples.load (std::memory_order_relaxed));

    initialiseModeSmoothing (sampleRate);
    updateDspParameters (0);
    inputStage .snapToTarget();
    outputStage.snapToTarget();
    mixer.reset();

    for (auto& value : bandReductionDb)
        value.store (0.0f, std::memory_order_relaxed);
    discardUiPeakTelemetry();
    cpuPercent.store (-1.0f, std::memory_order_relaxed);
    lastInputActivityTicks.store (0, std::memory_order_relaxed);
    cpuLoadEma = 0.0;
    cpuLoadEmaSeeded = false;

    isPrepared = true;
}

juce::dsp::Oversampling<float>* CabRotProcessor::activeOversampler() const noexcept
{
    if (currentOsChoice <= 0 || currentOsChoice > (int) oversamplers.size())
        return nullptr;

    return oversamplers[(size_t) (currentOsChoice - 1)].get();
}

void CabRotProcessor::applyOversamplingConfig (int osChoice)
{
    osChoice = juce::jlimit (0, (int) oversamplers.size(), osChoice);

    const int    factor        = 1 << osChoice;
    const double effectiveRate = preparedSampleRate * factor;
    const int    effectiveBlock = preparedBlockSize * factor;

    // These prepares only recompute coefficients and reset state; every
    // buffer they rely on was sized for 4x in prepareToPlay, so a factor
    // switch mid-stream performs no allocation on the audio thread.
    splitter.prepare (effectiveRate, preparedChannels, effectiveBlock);
    mixer   .prepare (effectiveRate, preparedChannels);

    for (auto& d : detectors)
        d.prepare (effectiveRate);

    for (auto& r : reducers)
        r.prepare (effectiveRate, preparedChannels);

    if (osChoice > 0 && oversamplers[(size_t) (osChoice - 1)] != nullptr)
        oversamplers[(size_t) (osChoice - 1)]->reset();

    const int latency = osChoice == 0
        ? 0
        : juce::roundToInt (oversamplers[(size_t) (osChoice - 1)]->getLatencyInSamples());

    pendingLatencySamples.store (latency, std::memory_order_relaxed);
    latencyReportDirty.store (true, std::memory_order_release);
    currentOsChoice = osChoice;
}

void CabRotProcessor::releaseResources()
{
    isPrepared = false;

    discardUiPeakTelemetry();
    for (auto& value : bandReductionDb)
        value.store (0.0f, std::memory_order_relaxed);
    cpuPercent.store (-1.0f, std::memory_order_relaxed);
    lastInputActivityTicks.store (0, std::memory_order_relaxed);
    cpuLoadEma = 0.0;
    cpuLoadEmaSeeded = false;

    splitter.reset();
    mixer.reset();
    inputStage.reset();
    outputStage.reset();

    for (auto& d : detectors)
        d.reset();

    for (auto& r : reducers)
        r.reset();

    for (auto& b : bandBuffers)
        b.setSize (0, 0);

    deltaBuffer.setSize (0, 0);
    gateScratch.clear();
    osChannelPointers.clear();

    for (auto& os : oversamplers)
        os.reset();
}

float CabRotProcessor::getBandReductionDb (int processedBand) const noexcept
{
    if (! juce::isPositiveAndBelow (processedBand, numProcessedBands))
        return 0.0f;

    return bandReductionDb[(size_t) processedBand].load (std::memory_order_relaxed);
}

CabRotProcessor::UiTelemetry CabRotProcessor::consumeUiTelemetry() noexcept
{
    UiTelemetry result;

    for (size_t i = 0; i < result.bandReductionDb.size(); ++i)
        result.bandReductionDb[i] = uiBandReductionMax[i].exchange (0.0f,
                                                                    std::memory_order_relaxed);

    result.inputPeak = uiInputPeak.exchange (0.0f, std::memory_order_relaxed);
    result.outputPeak = uiOutputPeak.exchange (0.0f, std::memory_order_relaxed);
    result.cpuPercent = cpuPercent.load (std::memory_order_relaxed);

    const auto lastLive = lastInputActivityTicks.load (std::memory_order_relaxed);
    if (lastLive > 0)
    {
        const auto ageTicks = juce::Time::getHighResolutionTicks() - lastLive;
        result.engineLive = ageTicks >= 0
            && juce::Time::highResolutionTicksToSeconds (ageTicks) <= kEngineLiveHoldSeconds;
    }

    return result;
}

void CabRotProcessor::discardUiPeakTelemetry() noexcept
{
    for (auto& value : uiBandReductionMax)
        value.exchange (0.0f, std::memory_order_relaxed);

    uiInputPeak.exchange (0.0f, std::memory_order_relaxed);
    uiOutputPeak.exchange (0.0f, std::memory_order_relaxed);
}

void CabRotProcessor::initialiseModeSmoothing (double sampleRate) noexcept
{
    const int modeIndex = juce::jlimit (0, dsp::kNumModeConfigs - 1,
                                        juce::roundToInt (p.mode->load (std::memory_order_relaxed)));
    const auto& mode = dsp::kModes[modeIndex];

    const auto initialise = [sampleRate] (juce::SmoothedValue<float>& value,
                                           float initialValue) noexcept
    {
        value.reset (sampleRate, kModeRampSeconds);
        value.setCurrentAndTargetValue (initialValue);
    };

    initialise (modeThresholdOffsetDb, mode.thresholdOffsetDb);
    initialise (modeAttackScale, mode.attackScale);
    initialise (modeEdgeBias, mode.edgeBias);
    initialise (modeShelfStart, mode.shelfStart);

    for (int i = 0; i < numProcessedBands; ++i)
    {
        const float scale = modeIndex == dsp::kHm2ModeIndex
                         && i == dsp::kWaspProcessedBandIndex
            ? dsp::kHm2WaspCeilingScale
            : mode.ceilingScale;
        initialise (modeCeilingScale[(size_t) i], scale);
    }
}

void CabRotProcessor::updateDspParameters (int numSamples) noexcept
{
    using namespace dsp::tuning;

    // replaceState redirects APVTS parameters one child at a time. Snapshot
    // every raw value between two matching even generations so a host block
    // sees all of A or all of B, never a transient hybrid.
    const auto generationBefore = abApplyGeneration.load (std::memory_order_acquire);
    if ((generationBefore & 1u) != 0u)
        return;

    const float fizzValue       = p.fizzHunt    ->load (std::memory_order_relaxed);
    const float edgeValue       = p.edgePreserve->load (std::memory_order_relaxed);
    const float smoothValue     = p.cabSmooth   ->load (std::memory_order_relaxed);
    const float sandValue       = p.digitalSand ->load (std::memory_order_relaxed);
    const float airValue        = p.airRot      ->load (std::memory_order_relaxed);
    const float mixValue        = p.reapMix     ->load (std::memory_order_relaxed);
    const float modeValue       = p.mode        ->load (std::memory_order_relaxed);
    const float ceilingDb       = p.maxReapDb   ->load (std::memory_order_relaxed);
    const float attackMs        = p.clampSpeed  ->load (std::memory_order_relaxed);
    const float windowMs        = p.pickWindow  ->load (std::memory_order_relaxed);
    const float stereoChoice    = p.stereoLink  ->load (std::memory_order_relaxed);
    const float focusValue      = p.detectorFocus->load (std::memory_order_relaxed);
    const float inputGainDb     = p.inputGain   ->load (std::memory_order_relaxed);
    const float outputGainDb    = p.outputGain  ->load (std::memory_order_relaxed);
    const bool wantsAuto        = p.autoGain    ->load (std::memory_order_relaxed) > 0.5f;
    const bool listenToRemoved  = p.deltaListen ->load (std::memory_order_relaxed) >= 0.5f;

    const auto generationAfter = abApplyGeneration.load (std::memory_order_acquire);
    if (generationBefore != generationAfter || (generationAfter & 1u) != 0u)
        return;

    const auto norm = [] (float value) noexcept
    {
        return juce::jlimit (0.0f, 1.0f, value * 0.01f);
    };

    const auto lift = [] (float value) noexcept
    {
        return value + kMainControlLift * value * (1.0f - value);
    };

    const float fizz    = lift (norm (fizzValue));
    const float edge    = lift (norm (edgeValue));
    const float smooth  = lift (norm (smoothValue));
    const float sand    = lift (norm (sandValue));
    const float air     = lift (norm (airValue));
    const float mixAmt  = lift (norm (mixValue));

    const int modeIndex = juce::jlimit (0, dsp::kNumModeConfigs - 1,
                                        juce::roundToInt (modeValue));
    const auto& mode = dsp::kModes[modeIndex];

    modeThresholdOffsetDb.setTargetValue (mode.thresholdOffsetDb);
    modeAttackScale.setTargetValue (mode.attackScale);
    modeEdgeBias.setTargetValue (mode.edgeBias);
    modeShelfStart.setTargetValue (mode.shelfStart);

    for (int i = 0; i < numProcessedBands; ++i)
    {
        const float scale = modeIndex == dsp::kHm2ModeIndex
                         && i == dsp::kWaspProcessedBandIndex
            ? dsp::kHm2WaspCeilingScale
            : mode.ceilingScale;
        modeCeilingScale[(size_t) i].setTargetValue (scale);
    }

    const auto advance = [numSamples] (juce::SmoothedValue<float>& value) noexcept
    {
        return numSamples > 0 ? value.skip (numSamples) : value.getCurrentValue();
    };

    const float thresholdOffsetDb = advance (modeThresholdOffsetDb);
    const float attackScale = advance (modeAttackScale);
    const float edgeBias = advance (modeEdgeBias);
    const float shelfStart = advance (modeShelfStart);

    // Stereo Behavior: 0 Linked, 1 Partial, 2 Dual Mono.
    const int linkChoice = juce::roundToInt (stereoChoice);
    const float link = (linkChoice == 0) ? 1.0f : (linkChoice == 1 ? 0.5f : 0.0f);

    const float thresholdDb = kThresholdAtZeroDb
                            + fizz * (kThresholdAtHundredDb - kThresholdAtZeroDb)
                            + thresholdOffsetDb;

    // Knob to band, per PLAN.md's mapping. Order is BITE, PLASTIC, WASP, ICE.
    const float bandAmount[numProcessedBands] = { smooth, sand, sand, air };

    // Detector Focus aims that threshold rather than moving it. Focus at 50
    // gives tilt == 0, so every band keeps the untilted threshold exactly.
    const float focusTilt = (juce::jlimit (0.0f, 1.0f, focusValue * 0.01f) - 0.5f) * 2.0f;

    for (int i = 0; i < numProcessedBands; ++i)
    {
        // -1 at BITE through +1 at ICE.
        const float bandPosition = numProcessedBands > 1
            ? ((float) i / (float) (numProcessedBands - 1) - 0.5f) * 2.0f
            : 0.0f;
        const float focusOffsetDb = -focusTilt * bandPosition * kDetectorFocusMaxTiltDb;

        auto& reducer = reducers[(size_t) i];
        reducer.setThresholdDb (thresholdDb + focusOffsetDb);
        reducer.setMaxReductionDb (bandAmount[i] * ceilingDb
                                   * advance (modeCeilingScale[(size_t) i]));
        reducer.setStereoLink (link);
        reducer.setAttackMs (attackMs * attackScale);
        reducer.setStaticTrimDb (0.0f);

        auto& detector = detectors[(size_t) i];
        detector.setEdgePreserve (juce::jlimit (0.0f, 1.0f, edge + edgeBias));
        detector.setPickWindowMs (windowMs);
    }

    // Air Rot's shelf, on the ICE band only, engaging over the top half of
    // the knob so the midpoint stays honest.
    const float shelfDrive = juce::jmax (0.0f, air - shelfStart)
                           / juce::jmax (1.0e-6f, 1.0f - shelfStart);
    reducers[numProcessedBands - 1].setStaticTrimDb (-shelfDrive * kAirRotShelfMaxCutDb);

    inputStage .setGainDb (inputGainDb);
    outputStage.setGainDb (outputGainDb);

    mixer.setMix (mixAmt);
    mixer.setAutoGain (wantsAuto);
    listenToRemovedSignal = listenToRemoved;
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
    const auto numSamples             = buffer.getNumSamples();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, numSamples);

    if (! isPrepared || numSamples <= 0)
        return;

    const int numChannels = juce::jmin (totalNumInputChannels, preparedChannels);

    if (numChannels <= 0)
        return;

    const auto startedAt = juce::Time::getHighResolutionTicks();

    const int osChoice = juce::jlimit (0, (int) oversamplers.size(),
        juce::roundToInt (p.oversampling->load (std::memory_order_relaxed)));
    if (osChoice != currentOsChoice)
        applyOversamplingConfig (osChoice);

    updateDspParameters (numSamples);
    const bool listenToRemoved = listenToRemovedSignal;
    BlockTelemetry telemetry;

    // Hosts are supposed to honour the block size they announced, but a
    // longer block should thin the sound rather than run off the end of the
    // scratch buffers, so slice it.
    for (int offset = 0; offset < numSamples; offset += preparedBlockSize)
    {
        const int chunk = juce::jmin (preparedBlockSize, numSamples - offset);

        juce::AudioBuffer<float> slice (buffer.getArrayOfWritePointers(),
                                        numChannels,
                                        offset,
                                        chunk);

        processChunk (slice, numChannels, chunk, listenToRemoved, telemetry);
    }

    const auto finishedAt = juce::Time::getHighResolutionTicks();

    for (size_t i = 0; i < telemetry.bandReductionDb.size(); ++i)
    {
        bandReductionDb[i].store (telemetry.bandReductionDb[i], std::memory_order_relaxed);
        publishMaximum (uiBandReductionMax[i], telemetry.bandReductionDb[i]);
    }

    publishMaximum (uiInputPeak, telemetry.inputPeak);
    publishMaximum (uiOutputPeak, telemetry.outputPeak);

    const float liveThreshold = juce::Decibels::decibelsToGain (kEngineLiveThresholdDb);
    if (telemetry.inputPeak >= liveThreshold)
        lastInputActivityTicks.store (finishedAt, std::memory_order_relaxed);

    const double blockSeconds = static_cast<double> (numSamples) / preparedSampleRate;
    if (blockSeconds > 0.0)
    {
        const double elapsed = juce::Time::highResolutionTicksToSeconds (finishedAt - startedAt);
        const double instantLoad = elapsed / blockSeconds;

        if (std::isfinite (instantLoad))
        {
            if (! cpuLoadEmaSeeded)
            {
                cpuLoadEma = instantLoad;
                cpuLoadEmaSeeded = true;
            }
            else
            {
                cpuLoadEma += (instantLoad - cpuLoadEma) / 32.0;
            }

            cpuPercent.store (static_cast<float> (cpuLoadEma * 100.0),
                              std::memory_order_relaxed);
        }
    }
}

void CabRotProcessor::processChunk (juce::AudioBuffer<float>& block, int numChannels,
                                    int numSamples, bool listenToRemoved,
                                    BlockTelemetry& telemetry) noexcept
{
    inputStage.process (block, numChannels, numSamples);
    telemetry.inputPeak = juce::jmax (telemetry.inputPeak,
                                      block.getMagnitude (0, numSamples));

    if (auto* os = activeOversampler())
    {
        juce::dsp::AudioBlock<float> baseBlock (block.getArrayOfWritePointers(),
                                                (size_t) numChannels,
                                                (size_t) numSamples);
        auto upBlock = os->processSamplesUp (baseBlock);

        for (int ch = 0; ch < numChannels; ++ch)
            osChannelPointers[(size_t) ch] = upBlock.getChannelPointer ((size_t) ch);

        juce::AudioBuffer<float> upView (osChannelPointers.data(), numChannels,
                                         (int) upBlock.getNumSamples());
        processCore (upView, numChannels, (int) upBlock.getNumSamples(),
                     listenToRemoved, telemetry);

        os->processSamplesDown (baseBlock);
    }
    else
    {
        processCore (block, numChannels, numSamples, listenToRemoved, telemetry);
    }

    outputStage.process (block, numChannels, numSamples);
    telemetry.outputPeak = juce::jmax (telemetry.outputPeak,
                                       block.getMagnitude (0, numSamples));
}

void CabRotProcessor::processCore (juce::AudioBuffer<float>& block, int numChannels,
                                   int numSamples, bool listenToRemoved,
                                   BlockTelemetry& telemetry) noexcept
{
    splitter.process (block, bandBuffers, numChannels, numSamples);

    // The band sum is the reference everything downstream measures against.
    // It is an allpassed copy of the input, not a bit-identical one, which is
    // inherent to a Linkwitz-Riley split and is exactly why the dry side of
    // the mix is taken from here rather than from the raw input.
    block.clear (0, numSamples);
    for (auto& band : bandBuffers)
        for (int ch = 0; ch < numChannels; ++ch)
            block.addFrom (ch, 0, band, ch, 0, numSamples);

    deltaBuffer.clear (0, numSamples);

    for (int i = 0; i < numProcessedBands; ++i)
    {
        auto& band = bandBuffers[(size_t) (i + dsp::tuning::kFirstProcessedBand)];

        detectors[(size_t) i].process (band.getArrayOfReadPointers(),
                                       numChannels,
                                       numSamples,
                                       gateScratch.data());

        const float reduced = reducers[(size_t) i].processToDelta (band.getArrayOfWritePointers(),
                                                                   gateScratch.data(),
                                                                   numChannels,
                                                                   numSamples);

        telemetry.bandReductionDb[(size_t) i] = juce::jmax (
            telemetry.bandReductionDb[(size_t) i], reduced);

        for (int ch = 0; ch < numChannels; ++ch)
            deltaBuffer.addFrom (ch, 0, band, ch, 0, numSamples);
    }

    if (listenToRemoved)
        mixer.processRemovedSignal (block, deltaBuffer, numChannels, numSamples);
    else
        mixer.process (block, deltaBuffer, numChannels, numSamples);
}

juce::AudioProcessorEditor* CabRotProcessor::createEditor()
{
    return new CabRotEditor (*this);
}

int CabRotProcessor::getNumPrograms()
{
    return presets::PresetManager::numFactoryPresets();
}

int CabRotProcessor::getCurrentProgram()
{
    // A host program index only means anything while the state still matches
    // the preset it came from. Once the user has moved a control the name is
    // cleared, and there is no honest index to report, so report the first.
    const auto name = presetManager.currentPresetName();

    for (int i = 0; i < presets::PresetManager::numFactoryPresets(); ++i)
        if (presets::PresetManager::factoryName (i) == name)
            return i;

    return 0;
}

void CabRotProcessor::setCurrentProgram (int index)
{
    presetManager.loadFactory (index);
}

const juce::String CabRotProcessor::getProgramName (int index)
{
    const auto name = presets::PresetManager::factoryName (index);
    return name.isNotEmpty() ? name : juce::String ("Default");
}

void CabRotProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    const juce::ScopedLock lock (abStateLock);
    destData.reset();

    const auto active = activeAbSlot.load (std::memory_order_acquire) == 1
        ? AbSlot::b : AbSlot::a;
    const int pending = pendingAbSlot.load (std::memory_order_acquire);
    const auto selected = pending == 0 ? AbSlot::a
                        : pending == 1 ? AbSlot::b
                                       : active;

    auto live = apvts.copyState();
    if (! live.isValid())
        return;

    // If a selector write is awaiting the message-thread handoff, the live
    // APVTS marker already names the destination even though the remaining
    // values still belong to the departing slot.
    forceSlotMarker (live, active);

    auto aState = slotStateA.isValid() ? slotStateA.createCopy()
                                       : juce::ValueTree {};
    auto bState = slotStateB.isValid() ? slotStateB.createCopy()
                                       : juce::ValueTree {};

    auto& activeState = active == AbSlot::a ? aState : bState;
    activeState = live.createCopy();

    auto& selectedState = selected == AbSlot::a ? aState : bState;
    if (! selectedState.isValid())
        selectedState = live.createCopy();

    if (aState.isValid())
        forceSlotMarker (aState, AbSlot::a);
    if (bState.isValid())
        forceSlotMarker (bState, AbSlot::b);

    juce::ValueTree state (kPluginStateType);
    state.setProperty (kFormatVersion, kPluginStateFormatVersion, nullptr);
    state.setProperty (kActiveSlot, static_cast<int> (selected), nullptr);
    appendWrappedSlot (state, kSlotAType, aState);
    appendWrappedSlot (state, kSlotBType, bState);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void CabRotProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr)
        return;

    const auto decoded = juce::ValueTree::fromXml (*xml);
    if (! decoded.isValid())
        return;

    juce::ValueTree loadedA, loadedB;
    AbSlot selected = AbSlot::a;

    if (decoded.hasType (apvts.state.getType()))
    {
        // Pre-v2 presets contained one raw APVTS tree. Keep that tree in its
        // selected slot; the other slot will clone it on first entry.
        selected = readSlotMarker (decoded);
        auto legacy = decoded.createCopy();
        forceSlotMarker (legacy, selected);

        if (selected == AbSlot::a)
            loadedA = std::move (legacy);
        else
            loadedB = std::move (legacy);
    }
    else if (decoded.hasType (kPluginStateType))
    {
        if (static_cast<int> (decoded.getProperty (kFormatVersion, 0))
                != kPluginStateFormatVersion
            || ! decoded.hasProperty (kActiveSlot)
            || decoded.getNumChildren() != 2)
        {
            return;
        }

        const int selectedIndex = static_cast<int> (decoded.getProperty (kActiveSlot));
        if (selectedIndex != 0 && selectedIndex != 1)
            return;

        if (! readWrappedSlot (decoded, kSlotAType, apvts.state.getType(), loadedA)
            || ! readWrappedSlot (decoded, kSlotBType, apvts.state.getType(), loadedB))
        {
            return;
        }

        selected = selectedIndex == 0 ? AbSlot::a : AbSlot::b;
    }
    else
    {
        return;
    }

    auto& selectedState = selected == AbSlot::a ? loadedA : loadedB;
    auto& otherState = selected == AbSlot::a ? loadedB : loadedA;

    if (! selectedState.isValid() && otherState.isValid())
        selectedState = otherState.createCopy();

    if (! selectedState.isValid())
        return;

    if (loadedA.isValid())
        forceSlotMarker (loadedA, AbSlot::a);
    if (loadedB.isValid())
        forceSlotMarker (loadedB, AbSlot::b);

    const juce::ScopedLock lock (abStateLock);
    const int selectedIndex = static_cast<int> (selected);

    applyingAbSlot.store (selectedIndex, std::memory_order_release);
    pendingAbSlot.store (-1, std::memory_order_release);
    slotStateA = loadedA.isValid() ? loadedA.createCopy() : juce::ValueTree {};
    slotStateB = loadedB.isValid() ? loadedB.createCopy() : juce::ValueTree {};
    abApplyGeneration.fetch_add (1, std::memory_order_acq_rel); // odd: replacing
    apvts.replaceState (selectedState.createCopy());
    activeAbSlot.store (selectedIndex, std::memory_order_release);
    abApplyGeneration.fetch_add (1, std::memory_order_release); // even: complete
    applyingAbSlot.store (-1, std::memory_order_release);
}
} // namespace cabrot

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new cabrot::CabRotProcessor();
}
