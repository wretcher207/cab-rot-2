// Phase 0 sample-perfect null test.
//
// Verifies that the empty-DSP processor passes audio through unchanged. The
// gate criterion is "measurable null when comparing input vs output (sample-
// perfect, no offset)". With Phase 0's pure passthrough that null is exact;
// from Phase 4 onward this test is repurposed to verify Reap Mix = 0 dry
// passthrough.
//
// Returns 0 on success, non-zero on failure. CTest reads the exit code.

#include "../Source/PluginProcessor.h"

#include <iostream>
#include <random>

namespace
{
constexpr double kSampleRate    = 48000.0;
constexpr int    kBlockSize     = 512;
constexpr int    kNumChannels   = 2;
constexpr int    kBlocksToTest  = 16;

int run()
{
    cabrot::CabRotProcessor processor;

    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses .add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (juce::AudioChannelSet::stereo());
    if (! processor.checkBusesLayoutSupported (layout))
    {
        std::cerr << "FAIL: stereo bus layout not supported\n";
        return 1;
    }
    processor.setBusesLayout (layout);

    processor.setRateAndBufferSizeDetails (kSampleRate, kBlockSize);
    processor.prepareToPlay (kSampleRate, kBlockSize);

    juce::AudioBuffer<float> input  (kNumChannels, kBlockSize);
    juce::AudioBuffer<float> output (kNumChannels, kBlockSize);
    juce::MidiBuffer midi;

    std::mt19937 rng (0xCAB80B); // deterministic
    std::uniform_real_distribution<float> dist (-0.9f, 0.9f);

    int totalSamples       = 0;
    float maxAbsDifference = 0.0f;

    for (int block = 0; block < kBlocksToTest; ++block)
    {
        for (int ch = 0; ch < kNumChannels; ++ch)
            for (int s = 0; s < kBlockSize; ++s)
                input.setSample (ch, s, dist (rng));

        output.makeCopyOf (input);
        processor.processBlock (output, midi);

        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            const auto* in  = input .getReadPointer (ch);
            const auto* out = output.getReadPointer (ch);
            for (int s = 0; s < kBlockSize; ++s)
            {
                const float diff = std::abs (in[s] - out[s]);
                if (diff > maxAbsDifference)
                    maxAbsDifference = diff;
                ++totalSamples;
            }
        }
    }

    processor.releaseResources();

    std::cout << "Passthrough null test: " << totalSamples
              << " samples, max |in - out| = " << maxAbsDifference << '\n';

    if (maxAbsDifference != 0.0f)
    {
        std::cerr << "FAIL: passthrough is not sample-perfect (expected 0.0)\n";
        return 2;
    }

    std::cout << "PASS\n";
    return 0;
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI gui; // some JUCE plumbing wants this even headless
    try { return run(); }
    catch (const std::exception& e) { std::cerr << "EXCEPTION: " << e.what() << '\n'; return 99; }
}
