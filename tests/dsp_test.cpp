// Phase 4 self-review gate, measured rather than eyeballed.
//
// Covers the boxes in PLAN.md's "Self-Review Gate 4" that a machine can
// answer. The two it cannot answer, whether the fizz actually goes away and
// whether the guitar still sounds like a guitar, are David's.
//
// Returns 0 when every check passes.

#include "TestSupport.h"

#include <chrono>

using namespace cabrot;
using namespace cabrot::test;

namespace
{
constexpr int kFftOrder = 13;               // 8192
constexpr int kFftSize  = 1 << kFftOrder;
constexpr int kAnalysisLength = kFftSize * 8;

float getParamValue (CabRotProcessor& processor, const juce::String& id)
{
    if (const auto* value = processor.getApvts().getRawParameterValue (id))
        return value->load (std::memory_order_relaxed);

    return 0.0f;
}

bool pumpUntilSlot (CabRotProcessor& processor, int expectedSlot,
                    double timeoutMs = 250.0)
{
    const double deadline = juce::Time::getMillisecondCounterHiRes() + timeoutMs;
    auto* messages = juce::MessageManager::getInstance();

    while (juce::Time::getMillisecondCounterHiRes() < deadline)
    {
        if (processor.getActiveAbSlot() == expectedSlot)
            return true;

        messages->runDispatchLoopUntil (5);
    }

    return processor.getActiveAbSlot() == expectedSlot;
}

// --------------------------------------------------------------------------
// 1. Reconstruction. The band split must not colour the signal.
// --------------------------------------------------------------------------
void testReconstruction (Report& report)
{
    constexpr double sr = 48000.0;

    CabRotProcessor processor;
    prepareStereo (processor, sr, kDefaultBlockSize);
    setNeutral (processor);
    warmUp (processor, sr, kDefaultBlockSize);

    juce::AudioBuffer<float> signal (2, kAnalysisLength);
    PinkNoise noise;
    for (int n = 0; n < kAnalysisLength; ++n)
    {
        const float sample = noise.next() * 0.35f;
        signal.setSample (0, n, sample);
        signal.setSample (1, n, noise.next() * 0.35f);
    }

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf (signal);

    runInPlace (processor, signal, kDefaultBlockSize);

    report.check (allFinite (signal), "reconstruction output is finite");

    // Skip the head so the filters are settled before we measure.
    const int skip = kFftSize;
    const auto measured = averageSpectrum (signal   .getReadPointer (0) + skip, kAnalysisLength - skip, kFftOrder);
    const auto original = averageSpectrum (reference.getReadPointer (0) + skip, kAnalysisLength - skip, kFftOrder);

    const float worst = worstDeltaDb (measured, original, 30.0, 20000.0, sr, kFftSize);
    const float mean  = bandDeltaDb  (measured, original, 30.0, 20000.0, sr, kFftSize);

    report.check (worst < 0.5f, "band sum is magnitude-flat within 0.5 dB, 30 Hz to 20 kHz");
    report.note ("worst bin deviation " + juce::String (worst, 3) + " dB, mean "
                 + juce::String (mean, 4) + " dB");

    // The time-domain residual is reported, not asserted. A Linkwitz-Riley
    // pair sums to an allpass, so this number is large by construction and
    // says nothing about whether the crossover is correct.
    float peakResidual = 0.0f;
    for (int n = skip; n < kAnalysisLength; ++n)
        peakResidual = juce::jmax (peakResidual,
                                   std::abs (signal.getSample (0, n) - reference.getSample (0, n)));

    report.note ("time-domain residual vs raw input "
                 + juce::String (juce::Decibels::gainToDecibels (peakResidual), 1)
                 + " dB (allpass phase, expected)");
}

// --------------------------------------------------------------------------
// 2. Knobs at zero must be exactly no-op, whatever Reap Mix says.
// --------------------------------------------------------------------------
void testZeroKnobsAreExact (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int length = 48000;

    const auto render = [&] (float mix)
    {
        CabRotProcessor processor;
        prepareStereo (processor, sr, kDefaultBlockSize);
        setNeutral (processor);
        setParam (processor, params::reapMix, mix);
        warmUp (processor, sr, kDefaultBlockSize);

        juce::AudioBuffer<float> buffer (2, length);
        PinkNoise noise;
        for (int ch = 0; ch < 2; ++ch)
            for (int n = 0; n < length; ++n)
                buffer.setSample (ch, n, noise.next() * 0.4f);

        runInPlace (processor, buffer, kDefaultBlockSize);
        return buffer;
    };

    const auto dry = render (0.0f);
    const auto wet = render (100.0f);

    float worst = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < length; ++n)
            worst = juce::jmax (worst, std::abs (dry.getSample (ch, n) - wet.getSample (ch, n)));

    report.check (worst == 0.0f, "every reduction knob at zero: Reap Mix changes nothing at all");
    report.note ("max difference " + juce::String (worst, 12));
}

// --------------------------------------------------------------------------
// 3. It has to actually remove fizz, and only where it was asked to.
// --------------------------------------------------------------------------
void testFizzAttenuation (Report& report)
{
    constexpr double sr = 48000.0;

    CabRotProcessor processor;
    prepareStereo (processor, sr, kDefaultBlockSize);
    setNeutral (processor);
    setParam (processor, params::fizzHunt,     100.0f);
    setParam (processor, params::edgePreserve,   0.0f);
    setParam (processor, params::cabSmooth,     50.0f);
    setParam (processor, params::digitalSand,  100.0f);
    setParam (processor, params::airRot,        50.0f);
    setParam (processor, params::reapMix,      100.0f);
    setParam (processor, params::maxReapDb,     12.0f);
    warmUp (processor, sr, kDefaultBlockSize);

    juce::AudioBuffer<float> signal (2, kAnalysisLength);
    PinkNoise noise;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < kAnalysisLength; ++n)
            signal.setSample (ch, n, noise.next() * 0.35f);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf (signal);

    runInPlace (processor, signal, kDefaultBlockSize);

    report.check (allFinite (signal), "attenuation output is finite");

    const int skip = kFftSize;
    const auto measured = averageSpectrum (signal   .getReadPointer (0) + skip, kAnalysisLength - skip, kFftOrder);
    const auto original = averageSpectrum (reference.getReadPointer (0) + skip, kAnalysisLength - skip, kFftOrder);

    const float wasp = bandDeltaDb (measured, original, 4000.0, 8000.0, sr, kFftSize);
    const float low  = bandDeltaDb (measured, original,  100.0,  900.0, sr, kFftSize);

    report.check (wasp < -1.0f, "4 to 8 kHz is attenuated with Fizz Hunt wide open");
    report.note ("4-8 kHz " + juce::String (wasp, 2) + " dB, below 900 Hz "
                 + juce::String (low, 2) + " dB");

    report.check (std::abs (low) < 0.5f, "below 900 Hz is left alone");
}

// --------------------------------------------------------------------------
// 4. The differentiator: Edge Preserve must measurably spare pick attacks.
// --------------------------------------------------------------------------
void testTransientPreservation (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int    noteSpacing = 12000; // 250 ms
    constexpr int    numNotes    = 16;
    constexpr int    length      = noteSpacing * numNotes;
    constexpr int    attackLen   = 144;   // 3 ms

    // A steady band of fizz with a hard leading edge every 250 ms. The
    // sustain is what the reducer should clamp; the edge is what it should
    // let past.
    juce::AudioBuffer<float> source (2, length);
    {
        PinkNoise noise;
        for (int n = 0; n < length; ++n)
        {
            const int posInNote = n % noteSpacing;
            const float carrier = std::sin (juce::MathConstants<float>::twoPi * 5500.0f * (float) n / (float) sr);
            const float fizz = 0.5f * carrier + 0.5f * noise.next();
            const float envelope = (posInNote < attackLen) ? 4.0f : 1.0f;

            const float sample = 0.18f * fizz * envelope;
            source.setSample (0, n, sample);
            source.setSample (1, n, sample);
        }
    }

    const auto attackPeak = [&] (float edgePreserve)
    {
        CabRotProcessor processor;
        prepareStereo (processor, sr, kDefaultBlockSize);
        setNeutral (processor);
        setParam (processor, params::fizzHunt,     100.0f);
        setParam (processor, params::edgePreserve, edgePreserve);
        setParam (processor, params::cabSmooth,    100.0f);
        setParam (processor, params::digitalSand,  100.0f);
        setParam (processor, params::airRot,       100.0f);
        setParam (processor, params::reapMix,      100.0f);
        setParam (processor, params::maxReapDb,     12.0f);
        setParam (processor, params::pickWindow,     5.0f);
        warmUp (processor, sr, kDefaultBlockSize);

        juce::AudioBuffer<float> buffer;
        buffer.makeCopyOf (source);
        runInPlace (processor, buffer, kDefaultBlockSize);

        // Ignore the first two notes while the slow envelope settles.
        double sum = 0.0;
        int counted = 0;

        for (int note = 2; note < numNotes; ++note)
        {
            const int start = note * noteSpacing;
            float peak = 0.0f;

            for (int n = start; n < start + attackLen; ++n)
                peak = juce::jmax (peak, std::abs (buffer.getSample (0, n)));

            sum += peak;
            ++counted;
        }

        return (float) (sum / juce::jmax (1, counted));
    };

    const float guarded = attackPeak (100.0f);
    const float exposed = attackPeak (0.0f);

    const float advantageDb = juce::Decibels::gainToDecibels (guarded)
                            - juce::Decibels::gainToDecibels (exposed);

    report.check (advantageDb > 1.0f, "Edge Preserve at 100 keeps more attack than at 0");
    report.note ("attack survives " + juce::String (advantageDb, 2)
                 + " dB louder with Edge Preserve wide open");
}

// --------------------------------------------------------------------------
// 5. No clicks or stepping when a knob is thrown across its range.
// --------------------------------------------------------------------------
void testNoZipperNoise (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int length = 48000 * 2;

    CabRotProcessor processor;
    prepareStereo (processor, sr, kDefaultBlockSize);
    setNeutral (processor);
    setParam (processor, params::digitalSand, 100.0f);
    setParam (processor, params::cabSmooth,   100.0f);
    setParam (processor, params::reapMix,     100.0f);
    setParam (processor, params::maxReapDb,    12.0f);
    warmUp (processor, sr, kDefaultBlockSize);

    juce::AudioBuffer<float> buffer (2, length);
    PinkNoise noise;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < length; ++n)
            buffer.setSample (ch, n, noise.next() * 0.3f);

    float inputSlew = 0.0f;
    for (int n = 1; n < length; ++n)
        inputSlew = juce::jmax (inputSlew, std::abs (buffer.getSample (0, n) - buffer.getSample (0, n - 1)));

    // Throw Fizz Hunt and Reap Mix end to end every block while it runs.
    juce::MidiBuffer midi;
    int blockIndex = 0;

    for (int offset = 0; offset < length; offset += kDefaultBlockSize)
    {
        const int chunk = juce::jmin (kDefaultBlockSize, length - offset);
        const float phase = (blockIndex % 2 == 0) ? 100.0f : 0.0f;

        setParam (processor, params::fizzHunt, phase);
        setParam (processor, params::reapMix,  phase);

        juce::AudioBuffer<float> slice (buffer.getArrayOfWritePointers(), 2, offset, chunk);
        processor.processBlock (slice, midi);
        ++blockIndex;
    }

    report.check (allFinite (buffer), "knob-sweep output is finite");

    float outputSlew = 0.0f;
    for (int n = 1; n < length; ++n)
        outputSlew = juce::jmax (outputSlew, std::abs (buffer.getSample (0, n) - buffer.getSample (0, n - 1)));

    const float ratio = outputSlew / juce::jmax (1.0e-9f, inputSlew);

    report.check (ratio < 2.0f, "fast knob sweeps add no step discontinuity");
    report.note ("worst output slew is " + juce::String (ratio, 3) + "x the input's");
}

// --------------------------------------------------------------------------
// 6. Every sample rate the plan names.
// --------------------------------------------------------------------------
void testSampleRates (Report& report)
{
    const double rates[] = { 44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0 };
    bool allGood = true;

    for (const auto rate : rates)
    {
        CabRotProcessor processor;
        prepareStereo (processor, rate, kDefaultBlockSize);
        setNeutral (processor);
        setParam (processor, params::fizzHunt,    100.0f);
        setParam (processor, params::cabSmooth,   100.0f);
        setParam (processor, params::digitalSand, 100.0f);
        setParam (processor, params::airRot,      100.0f);
        setParam (processor, params::reapMix,     100.0f);

        juce::AudioBuffer<float> buffer (2, (int) rate / 4);
        PinkNoise noise;
        for (int ch = 0; ch < 2; ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                buffer.setSample (ch, n, noise.next() * 0.4f);

        runInPlace (processor, buffer, kDefaultBlockSize);

        const bool ok = allFinite (buffer) && buffer.getMagnitude (0, buffer.getNumSamples()) < 4.0f;
        if (! ok)
            report.note ("sample rate " + juce::String (rate, 0) + " produced garbage");

        allGood = allGood && ok;
    }

    report.check (allGood, "44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz all stay sane");
}

// --------------------------------------------------------------------------
// 7. Odd block sizes, including one larger than the prepared size.
// --------------------------------------------------------------------------
void testBlockSizes (Report& report)
{
    constexpr double sr = 48000.0;
    const int blockSizes[] = { 1, 7, 64, 512, 2048 };
    bool allGood = true;

    for (const auto block : blockSizes)
    {
        CabRotProcessor processor;
        prepareStereo (processor, sr, 512);
        setNeutral (processor);
        setParam (processor, params::fizzHunt,    100.0f);
        setParam (processor, params::digitalSand, 100.0f);
        setParam (processor, params::reapMix,     100.0f);

        juce::AudioBuffer<float> buffer (2, 24000);
        PinkNoise noise;
        for (int ch = 0; ch < 2; ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                buffer.setSample (ch, n, noise.next() * 0.3f);

        runInPlace (processor, buffer, block);

        const bool ok = allFinite (buffer);
        if (! ok)
            report.note ("block size " + juce::String (block) + " produced garbage");

        allGood = allGood && ok;
    }

    report.check (allGood, "block sizes 1 / 7 / 64 / 512 / 2048 all survive");
}

// --------------------------------------------------------------------------
// 8. Denormals must flush, not stall.
// --------------------------------------------------------------------------
void testDenormalsFlush (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int burst = 4800;
    constexpr int tail  = 48000;

    CabRotProcessor processor;
    prepareStereo (processor, sr, kDefaultBlockSize);
    setNeutral (processor);
    setParam (processor, params::fizzHunt,    100.0f);
    setParam (processor, params::digitalSand, 100.0f);
    setParam (processor, params::reapMix,     100.0f);

    juce::AudioBuffer<float> buffer (2, burst + tail);
    buffer.clear();

    PinkNoise noise;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < burst; ++n)
            buffer.setSample (ch, n, noise.next() * 0.5f);

    runInPlace (processor, buffer, kDefaultBlockSize);

    float tailPeak = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = burst + tail / 2; n < burst + tail; ++n)
            tailPeak = juce::jmax (tailPeak, std::abs (buffer.getSample (ch, n)));

    report.check (tailPeak == 0.0f, "silence after a burst decays to true zero");
    report.note ("tail peak " + juce::String (tailPeak, 12));
}

// --------------------------------------------------------------------------
// 9. UI telemetry must aggregate a full host block, not only its last slice.
// --------------------------------------------------------------------------
void testUiTelemetry (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int preparedBlock = 512;
    constexpr int oversizedHostBlock = 1536;

    CabRotProcessor processor;
    prepareStereo (processor, sr, preparedBlock);
    setNeutral (processor);
    processor.discardUiPeakTelemetry();

    juce::AudioBuffer<float> buffer (2, oversizedHostBlock);
    buffer.clear();
    buffer.setSample (0, 0, 0.75f);
    buffer.setSample (1, 0, -0.75f);

    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);

    const auto first = processor.consumeUiTelemetry();
    report.check (first.inputPeak > 0.70f,
                  "UI telemetry keeps a first-slice peak across an oversized host block");
    report.check (first.engineLive, "UI telemetry reports recent input as live");
    report.check (std::isfinite (first.cpuPercent) && first.cpuPercent >= 0.0f,
                  "UI telemetry publishes a finite measured CPU value");

    const auto consumed = processor.consumeUiTelemetry();
    report.check (consumed.inputPeak == 0.0f,
                  "UI peak telemetry is consumed exactly once");
}

// --------------------------------------------------------------------------
// 10. Delta Listen must be the removed signal, with the correct polarity.
// --------------------------------------------------------------------------
void testDeltaListen (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int length = 48000;

    juce::AudioBuffer<float> source (2, length);
    PinkNoise noise;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < length; ++n)
            source.setSample (ch, n, noise.next() * 0.35f);

    const auto render = [&] (float mix, bool deltaListen)
    {
        CabRotProcessor processor;
        prepareStereo (processor, sr, kDefaultBlockSize);
        setNeutral (processor);
        setParam (processor, params::fizzHunt,    100.0f);
        setParam (processor, params::edgePreserve,  0.0f);
        setParam (processor, params::cabSmooth,   100.0f);
        setParam (processor, params::digitalSand, 100.0f);
        setParam (processor, params::airRot,      100.0f);
        setParam (processor, params::reapMix,       mix);
        setParam (processor, params::maxReapDb,    12.0f);
        setParam (processor, params::deltaListen, deltaListen ? 1.0f : 0.0f);
        warmUp (processor, sr, kDefaultBlockSize);

        juce::AudioBuffer<float> result;
        result.makeCopyOf (source);
        runInPlace (processor, result, kDefaultBlockSize);
        return result;
    };

    const auto dry = render (0.0f, false);
    const auto wet = render (100.0f, false);
    const auto removed = render (100.0f, true);

    float reconstructionResidual = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < length; ++n)
            reconstructionResidual = juce::jmax (
                reconstructionResidual,
                std::abs (wet.getSample (ch, n) + removed.getSample (ch, n)
                          - dry.getSample (ch, n)));

    report.check (reconstructionResidual < 1.0e-4f,
                  "Delta Listen plus processed output reconstructs the dry path below -80 dB");

    const auto zeroMix = render (0.0f, true);
    report.check (zeroMix.getMagnitude (0, length) == 0.0f,
                  "Delta Listen at Reap Mix zero is exact silence");
}

// --------------------------------------------------------------------------
// 11. A/B must snapshot both parameter state and the processing it controls.
// --------------------------------------------------------------------------
void testAbSnapshots (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr int renderLength = 24000;

    CabRotProcessor processor;
    prepareStereo (processor, sr, kDefaultBlockSize);
    setNeutral (processor);
    setParam (processor, params::fizzHunt, 100.0f);
    setParam (processor, params::reapMix, 100.0f);
    setParam (processor, params::maxReapDb, 12.0f);

    setParam (processor, params::aOrB, 1.0f);
    report.check (pumpUntilSlot (processor, 1), "A/B enters slot B on the message thread");
    report.check (getParamValue (processor, params::digitalSand) == 0.0f,
                  "first entry to B clones the current A values");

    setParam (processor, params::digitalSand, 100.0f);
    setParam (processor, params::airRot, 75.0f);

    setParam (processor, params::aOrB, 0.0f);
    report.check (pumpUntilSlot (processor, 0), "A/B returns to slot A");
    report.check (getParamValue (processor, params::digitalSand) == 0.0f
                  && getParamValue (processor, params::airRot) == 0.0f,
                  "slot A restores its own knob values");

    setParam (processor, params::cabSmooth, 25.0f);
    setParam (processor, params::aOrB, 1.0f);
    report.check (pumpUntilSlot (processor, 1), "A/B re-enters slot B");
    report.check (getParamValue (processor, params::digitalSand) == 100.0f
                  && getParamValue (processor, params::airRot) == 75.0f
                  && getParamValue (processor, params::cabSmooth) == 0.0f,
                  "slot B retains values distinct from A");

    const auto renderCurrent = [&]
    {
        warmUp (processor, sr, kDefaultBlockSize, 350.0f);

        juce::AudioBuffer<float> buffer (2, renderLength);
        PinkNoise noise (0xAB5107u);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                buffer.setSample (ch, n, noise.next() * 0.35f);

        runInPlace (processor, buffer, kDefaultBlockSize);
        return buffer;
    };

    const auto bAudio = renderCurrent();
    setParam (processor, params::aOrB, 0.0f);
    report.check (pumpUntilSlot (processor, 0), "A/B switches to A before audio comparison");
    const auto aAudio = renderCurrent();

    float processingDifference = 0.0f;
    for (int ch = 0; ch < aAudio.getNumChannels(); ++ch)
        for (int n = 0; n < renderLength; ++n)
            processingDifference = juce::jmax (
                processingDifference,
                std::abs (aAudio.getSample (ch, n) - bAudio.getSample (ch, n)));

    report.check (processingDifference > 1.0e-3f,
                  "A/B restores processing behavior, not only displayed values");

    setParam (processor, params::aOrB, 1.0f);
    report.check (pumpUntilSlot (processor, 1), "A/B selects B before saving");
    setParam (processor, params::digitalSand, 91.0f);
    setParam (processor, params::airRot, 73.0f);

    juce::MemoryBlock saved;
    processor.getStateInformation (saved);

    CabRotProcessor restored;
    restored.setStateInformation (saved.getData(), static_cast<int> (saved.getSize()));
    report.check (restored.getActiveAbSlot() == 1
                  && getParamValue (restored, params::digitalSand) == 91.0f
                  && getParamValue (restored, params::airRot) == 73.0f,
                  "preset state restores active B including its latest edits");

    setParam (restored, params::aOrB, 0.0f);
    report.check (pumpUntilSlot (restored, 0)
                  && getParamValue (restored, params::digitalSand) == 0.0f
                  && getParamValue (restored, params::cabSmooth) == 25.0f,
                  "preset state preserves inactive A");

    setParam (restored, params::aOrB, 1.0f);
    report.check (pumpUntilSlot (restored, 1)
                  && getParamValue (restored, params::digitalSand) == 91.0f,
                  "preset state preserves B across a post-load round trip");

    // Legacy presets were a single raw CABROT tree. A selected legacy slot
    // loads directly, then clones into the other slot on first entry.
    CabRotProcessor legacySource;
    setParam (legacySource, params::digitalSand, 42.0f);
    setParam (legacySource, params::aOrB, 1.0f);
    juce::MemoryBlock legacyData;
    if (auto legacyXml = legacySource.getApvts().copyState().createXml())
        juce::AudioProcessor::copyXmlToBinary (*legacyXml, legacyData);

    CabRotProcessor legacyRestored;
    legacyRestored.setStateInformation (legacyData.getData(),
                                        static_cast<int> (legacyData.getSize()));
    report.check (legacyRestored.getActiveAbSlot() == 1
                  && getParamValue (legacyRestored, params::digitalSand) == 42.0f,
                  "legacy single-tree presets still load");

    setParam (legacyRestored, params::aOrB, 0.0f);
    report.check (pumpUntilSlot (legacyRestored, 0)
                  && getParamValue (legacyRestored, params::digitalSand) == 42.0f,
                  "a legacy preset clones safely on first entry to the other slot");

    CabRotProcessor rapid;
    setParam (rapid, params::digitalSand, 17.0f);
    setParam (rapid, params::aOrB, 1.0f);
    setParam (rapid, params::aOrB, 0.0f);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (40);
    report.check (rapid.getActiveAbSlot() == 0
                  && getParamValue (rapid, params::digitalSand) == 17.0f,
                  "rapid A to B to A writes coalesce without corrupting A");
}

// --------------------------------------------------------------------------
// 12. CPU. One instance, stereo, 48 kHz, everything working hard.
// --------------------------------------------------------------------------
void testCpuBudget (Report& report)
{
    constexpr double sr = 48000.0;
    constexpr double seconds = 2.0;
    const int length = (int) (sr * seconds);

    // Two configurations. Idle exercises the band split and nothing else, so
    // the gap between them is what the detectors and reducers actually cost.
    //
    // Timed as the best of several passes over one cache-resident buffer.
    // A plugin processes a small block over and over out of warm cache, so
    // that is the shape being measured; the minimum is used because every
    // source of noise here can only ever make a run slower.
    constexpr int kPasses = 9;

    const auto measure = [&] (bool working)
    {
        CabRotProcessor processor;
        prepareStereo (processor, sr, kDefaultBlockSize);
        setNeutral (processor);

        if (working)
        {
            setParam (processor, params::fizzHunt,    100.0f);
            setParam (processor, params::cabSmooth,   100.0f);
            setParam (processor, params::digitalSand, 100.0f);
            setParam (processor, params::airRot,      100.0f);
            setParam (processor, params::reapMix,     100.0f);
            setParam (processor, params::maxReapDb,    12.0f);
        }

        warmUp (processor, sr, kDefaultBlockSize);

        juce::AudioBuffer<float> source (2, length);
        PinkNoise noise;
        for (int ch = 0; ch < 2; ++ch)
            for (int n = 0; n < length; ++n)
                source.setSample (ch, n, noise.next() * 0.4f);

        juce::AudioBuffer<float> buffer (2, length);
        double best = 1.0e9;

        for (int pass = 0; pass < kPasses; ++pass)
        {
            buffer.makeCopyOf (source);

            const auto started = std::chrono::steady_clock::now();
            runInPlace (processor, buffer, kDefaultBlockSize);
            const auto finished = std::chrono::steady_clock::now();

            best = juce::jmin (best, std::chrono::duration<double> (finished - started).count());
        }

        return best / seconds * 100.0;
    };

    const double idle    = measure (false);
    const double working = measure (true);

    // This is the one check in the file that measures the machine rather than
    // the code, so a loaded box fails it while the DSP is untouched. Filming
    // with OBS and a capture card running pushed the same binary from 2.7% to
    // 4.0% across three consecutive runs. Set CABROT_SKIP_CPU_BENCH=1 when the
    // machine is busy, and read the printed figures instead of the gate.
    if (juce::SystemStats::getEnvironmentVariable ("CABROT_SKIP_CPU_BENCH", {}).isNotEmpty())
    {
        report.note ("CPU budget check SKIPPED via CABROT_SKIP_CPU_BENCH");
    }
    else
    {
        report.check (working < 3.0, "worst case costs under 3% of a core at 48 kHz stereo");
    }

    report.note ("all bands wide open " + juce::String (working, 3) + "%, idle split alone "
                 + juce::String (idle, 3) + "%, so reduction costs "
                 + juce::String (working - idle, 3) + "%");
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;

    Report report;

    try
    {
        std::cout << "Cab Rot Phase 4 DSP gate\n\n";

        testReconstruction (report);
        testZeroKnobsAreExact (report);
        testFizzAttenuation (report);
        testTransientPreservation (report);
        testNoZipperNoise (report);
        testSampleRates (report);
        testBlockSizes (report);
        testDenormalsFlush (report);
        testUiTelemetry (report);
        testDeltaListen (report);
        testAbSnapshots (report);
        testCpuBudget (report);
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION: " << e.what() << '\n';
        return 99;
    }

    std::cout << "\n" << report.passed << " passed, " << report.failed << " failed\n";
    return report.exitCode();
}
