// Transparency gate.
//
// Phase 0 asserted a sample-perfect null, because processBlock did nothing at
// all. From Phase 4 the signal goes through a Linkwitz-Riley band split, and
// an LR pair sums to an ALLPASS rather than to unity (JUCE says as much in
// juce_LinkwitzRileyFilter.h). So the idle plugin is magnitude-flat and
// phase-shifted, not bit-identical, and a sample-perfect null is no longer a
// property this architecture can have. Host bypass is unaffected: the DAW
// never calls processBlock, so bypassing Cab Rot still returns the original
// signal untouched.
//
// What IS still exact, and is worth guarding, is that a reduction knob at
// zero does precisely nothing. That invariant is what makes "off" mean off.
//
// Returns 0 on success, non-zero on failure. CTest reads the exit code.

#include "TestSupport.h"

using namespace cabrot;
using namespace cabrot::test;

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int    kFftOrder   = 13;
constexpr int    kFftSize    = 1 << kFftOrder;
constexpr int    kLength     = kFftSize * 8;

void testIdleIsMagnitudeFlat (Report& report)
{
    CabRotProcessor processor;
    prepareStereo (processor, kSampleRate, kDefaultBlockSize);
    setNeutral (processor);
    warmUp (processor, kSampleRate, kDefaultBlockSize);

    juce::AudioBuffer<float> signal (2, kLength);
    PinkNoise noise;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < kLength; ++n)
            signal.setSample (ch, n, noise.next() * 0.35f);

    juce::AudioBuffer<float> reference;
    reference.makeCopyOf (signal);

    runInPlace (processor, signal, kDefaultBlockSize);

    report.check (allFinite (signal), "idle output is finite");

    const int skip = kFftSize;
    const auto measured = averageSpectrum (signal   .getReadPointer (0) + skip, kLength - skip, kFftOrder);
    const auto original = averageSpectrum (reference.getReadPointer (0) + skip, kLength - skip, kFftOrder);

    const float worst = worstDeltaDb (measured, original, 30.0, 20000.0, kSampleRate, kFftSize);

    report.check (worst < 0.5f, "idle plugin is magnitude-flat within 0.5 dB");
    report.note ("worst bin deviation " + juce::String (worst, 3) + " dB");
}

void testZeroMeansZero (Report& report)
{
    const auto render = [] (float reductionKnobs)
    {
        CabRotProcessor processor;
        prepareStereo (processor, kSampleRate, kDefaultBlockSize);
        setNeutral (processor);
        setParam (processor, params::fizzHunt,    100.0f);
        setParam (processor, params::reapMix,     100.0f);
        setParam (processor, params::cabSmooth,   reductionKnobs);
        setParam (processor, params::digitalSand, reductionKnobs);
        setParam (processor, params::airRot,      reductionKnobs);
        warmUp (processor, kSampleRate, kDefaultBlockSize);

        juce::AudioBuffer<float> buffer (2, 48000);
        PinkNoise noise;
        for (int ch = 0; ch < 2; ++ch)
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                buffer.setSample (ch, n, noise.next() * 0.4f);

        runInPlace (processor, buffer, kDefaultBlockSize);
        return buffer;
    };

    // Fizz Hunt wide open and Reap Mix at 100, but every band ceiling at
    // zero. Nothing may move.
    const auto silentKnobs = render (0.0f);

    CabRotProcessor bare;
    prepareStereo (bare, kSampleRate, kDefaultBlockSize);
    setNeutral (bare);
    warmUp (bare, kSampleRate, kDefaultBlockSize);

    juce::AudioBuffer<float> expected (2, 48000);
    {
        PinkNoise noise;
        for (int ch = 0; ch < 2; ++ch)
            for (int n = 0; n < expected.getNumSamples(); ++n)
                expected.setSample (ch, n, noise.next() * 0.4f);
    }
    runInPlace (bare, expected, kDefaultBlockSize);

    float worst = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
        for (int n = 0; n < expected.getNumSamples(); ++n)
            worst = juce::jmax (worst, std::abs (silentKnobs.getSample (ch, n) - expected.getSample (ch, n)));

    report.check (worst == 0.0f, "band ceilings at zero are bit-exactly inert");
    report.note ("max difference " + juce::String (worst, 12));
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;

    Report report;

    try
    {
        std::cout << "Cab Rot transparency gate\n\n";

        testIdleIsMagnitudeFlat (report);
        testZeroMeansZero (report);
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION: " << e.what() << '\n';
        return 99;
    }

    std::cout << "\n" << report.passed << " passed, " << report.failed << " failed\n";
    return report.exitCode();
}
