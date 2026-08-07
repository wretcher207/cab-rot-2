#pragma once

#include "Tuning.h"

#include <juce_dsp/juce_dsp.h>

#include <array>

namespace cabrot::dsp
{
/**
    Splits the input into the six bands described in Tuning.h using a cascade
    of Linkwitz-Riley crossovers.

    The cascade is serial: split at 2.4 kHz and keep the low band, split what
    is left at 3.8 kHz and keep the low band, and so on. That alone does NOT
    reconstruct, because each band that drops out early misses the phase shift
    of every crossover below it. Each band is therefore run through an allpass
    at each later crossover frequency, which is the standard fix and the
    reason this class carries ten extra filters.

    What "reconstruct" means here matters. A Linkwitz-Riley pair sums to an
    ALLPASS, not to unity: JUCE says so in its own header. So the sum of the
    six bands has a flat magnitude response and a shifted phase response. It
    is not, and cannot be, bit-identical to the input. Everything downstream
    is built to take the band sum as its reference so that the phase shift
    cancels out of the mix path instead of comb filtering it.
*/
class BandSplitter
{
public:
    static constexpr int numBands      = tuning::kNumBands;
    static constexpr int numCrossovers = tuning::kNumCrossovers;

    void prepare (double sampleRate, int numChannels, int maximumBlockSize);
    void reset() noexcept;

    /** Splits `input` into `bands`. Each band buffer must already be sized
        for at least `numChannels` x `numSamples`. */
    void process (const juce::AudioBuffer<float>& input,
                  std::array<juce::AudioBuffer<float>, numBands>& bands,
                  int numChannels,
                  int numSamples) noexcept;

private:
    /** Flat index of the allpass that fixes band `k` for crossover `j`. */
    static constexpr int allpassIndex (int k, int j) noexcept
    {
        // k runs 0..3, j runs k+1..4. Row offsets for k = 0,1,2,3.
        constexpr int rowStart[4] = { 0, 4, 7, 9 };
        return rowStart[k] + (j - k - 1);
    }

    static constexpr int numAllpasses = 10; // 4 + 3 + 2 + 1

    std::array<juce::dsp::LinkwitzRileyFilter<float>, numCrossovers> splits;
    std::array<juce::dsp::LinkwitzRileyFilter<float>, numAllpasses>  allpasses;
};
} // namespace cabrot::dsp
