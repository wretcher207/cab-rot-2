#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace cabrot
{
namespace
{
constexpr float kEngineLiveThresholdDb = -72.0f;
constexpr double kEngineLiveHoldSeconds = 1.0;

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
    p.fizzHunt     = apvts.getRawParameterValue (params::fizzHunt);
    p.edgePreserve = apvts.getRawParameterValue (params::edgePreserve);
    p.cabSmooth    = apvts.getRawParameterValue (params::cabSmooth);
    p.digitalSand  = apvts.getRawParameterValue (params::digitalSand);
    p.airRot       = apvts.getRawParameterValue (params::airRot);
    p.reapMix      = apvts.getRawParameterValue (params::reapMix);
    p.inputGain    = apvts.getRawParameterValue (params::inputGain);
    p.outputGain   = apvts.getRawParameterValue (params::outputGain);
    p.deltaListen  = apvts.getRawParameterValue (params::deltaListen);
    p.stereoLink   = apvts.getRawParameterValue (params::stereoLink);
    p.clampSpeed   = apvts.getRawParameterValue (params::clampSpeed);
    p.maxReapDb    = apvts.getRawParameterValue (params::maxReapDb);
    p.pickWindow   = apvts.getRawParameterValue (params::pickWindow);
    p.autoGain     = apvts.getRawParameterValue (params::autoGain);

    for (auto& value : bandReductionDb)
        value.store (0.0f, std::memory_order_relaxed);

    for (auto& value : uiBandReductionMax)
        value.store (0.0f, std::memory_order_relaxed);
}

void CabRotProcessor::prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock)
{
    preparedSampleRate = sampleRate;
    preparedChannels  = juce::jmax (1, getTotalNumInputChannels());
    preparedBlockSize = juce::jmax (32, maximumExpectedSamplesPerBlock);

    inputStage .prepare (sampleRate, preparedChannels);
    outputStage.prepare (sampleRate, preparedChannels);
    splitter   .prepare (sampleRate, preparedChannels, preparedBlockSize);
    mixer      .prepare (sampleRate, preparedChannels);

    for (auto& d : detectors)
        d.prepare (sampleRate);

    for (auto& r : reducers)
        r.prepare (sampleRate, preparedChannels);

    for (auto& b : bandBuffers)
    {
        b.setSize (preparedChannels, preparedBlockSize, false, false, true);
        b.clear();
    }

    deltaBuffer.setSize (preparedChannels, preparedBlockSize, false, false, true);
    deltaBuffer.clear();

    gateScratch.assign ((size_t) preparedBlockSize, 0.0f);

    updateDspParameters();
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

void CabRotProcessor::updateDspParameters() noexcept
{
    using namespace dsp::tuning;

    const auto norm = [] (const std::atomic<float>* v) noexcept
    {
        return juce::jlimit (0.0f, 1.0f, v->load (std::memory_order_relaxed) * 0.01f);
    };

    const auto lift = [] (float value) noexcept
    {
        return value + kMainControlLift * value * (1.0f - value);
    };

    const float fizz    = lift (norm (p.fizzHunt));
    const float edge    = lift (norm (p.edgePreserve));
    const float smooth  = lift (norm (p.cabSmooth));
    const float sand    = lift (norm (p.digitalSand));
    const float air     = lift (norm (p.airRot));
    const float mixAmt  = lift (norm (p.reapMix));

    const float ceilingDb  = p.maxReapDb ->load (std::memory_order_relaxed);
    const float attackMs   = p.clampSpeed->load (std::memory_order_relaxed);
    const float windowMs   = p.pickWindow->load (std::memory_order_relaxed);
    const bool  wantsAuto  = p.autoGain  ->load (std::memory_order_relaxed) > 0.5f;

    // Stereo Behavior: 0 Linked, 1 Partial, 2 Dual Mono.
    const int linkChoice = juce::roundToInt (p.stereoLink->load (std::memory_order_relaxed));
    const float link = (linkChoice == 0) ? 1.0f : (linkChoice == 1 ? 0.5f : 0.0f);

    const float thresholdDb = kThresholdAtZeroDb
                            + fizz * (kThresholdAtHundredDb - kThresholdAtZeroDb);

    // Knob to band, per PLAN.md's mapping. Order is BITE, PLASTIC, WASP, ICE.
    const float bandAmount[numProcessedBands] = { smooth, sand, sand, air };

    for (int i = 0; i < numProcessedBands; ++i)
    {
        auto& reducer = reducers[(size_t) i];
        reducer.setThresholdDb (thresholdDb);
        reducer.setMaxReductionDb (bandAmount[i] * ceilingDb);
        reducer.setStereoLink (link);
        reducer.setAttackMs (attackMs);
        reducer.setStaticTrimDb (0.0f);

        auto& detector = detectors[(size_t) i];
        detector.setEdgePreserve (edge);
        detector.setPickWindowMs (windowMs);
    }

    // Air Rot's shelf, on the ICE band only, engaging over the top half of
    // the knob so the midpoint stays honest.
    const float shelfDrive = juce::jmax (0.0f, (air - kAirRotShelfStart))
                           / juce::jmax (1.0e-6f, 1.0f - kAirRotShelfStart);
    reducers[numProcessedBands - 1].setStaticTrimDb (-shelfDrive * kAirRotShelfMaxCutDb);

    inputStage .setGainDb (p.inputGain ->load (std::memory_order_relaxed));
    outputStage.setGainDb (p.outputGain->load (std::memory_order_relaxed));

    mixer.setMix (mixAmt);
    mixer.setAutoGain (wantsAuto);
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
    updateDspParameters();
    const bool listenToRemoved = p.deltaListen->load (std::memory_order_relaxed) >= 0.5f;
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

    outputStage.process (block, numChannels, numSamples);
    telemetry.outputPeak = juce::jmax (telemetry.outputPeak,
                                       block.getMagnitude (0, numSamples));
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
