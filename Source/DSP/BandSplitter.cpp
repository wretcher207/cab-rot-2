#include "BandSplitter.h"

namespace cabrot::dsp
{
void BandSplitter::prepare (double sampleRate, int numChannels, int maximumBlockSize)
{
    juce::dsp::ProcessSpec spec {};
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax (1, maximumBlockSize);
    spec.numChannels      = (juce::uint32) juce::jmax (1, numChannels);

    // A crossover placed above Nyquist is meaningless. Park any such split
    // just under Nyquist so the filter stays stable at low sample rates; the
    // band above it simply ends up empty.
    const auto maxCutoff = (float) (sampleRate * 0.5 * 0.98);

    for (int k = 0; k < numCrossovers; ++k)
    {
        splits[(size_t) k].prepare (spec);
        splits[(size_t) k].setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
        splits[(size_t) k].setCutoffFrequency (juce::jmin (tuning::kCrossoverHz[k], maxCutoff));
    }

    for (int k = 0; k < numCrossovers - 1; ++k)
    {
        for (int j = k + 1; j < numCrossovers; ++j)
        {
            auto& ap = allpasses[(size_t) allpassIndex (k, j)];
            ap.prepare (spec);
            ap.setType (juce::dsp::LinkwitzRileyFilterType::allpass);
            ap.setCutoffFrequency (juce::jmin (tuning::kCrossoverHz[j], maxCutoff));
        }
    }
}

void BandSplitter::reset() noexcept
{
    for (auto& f : splits)
        f.reset();

    for (auto& f : allpasses)
        f.reset();
}

void BandSplitter::process (const juce::AudioBuffer<float>& input,
                            std::array<juce::AudioBuffer<float>, numBands>& bands,
                            int numChannels,
                            int numSamples) noexcept
{
    // Pass 1: serial split. Each crossover hands its low output to a band and
    // passes its high output down to the next crossover.
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* in = input.getReadPointer (ch);

        float* dst[numBands];
        for (int b = 0; b < numBands; ++b)
            dst[b] = bands[(size_t) b].getWritePointer (ch);

        for (int n = 0; n < numSamples; ++n)
        {
            float remaining = in[n];

            for (int k = 0; k < numCrossovers; ++k)
            {
                float low = 0.0f, high = 0.0f;
                splits[(size_t) k].processSample (ch, remaining, low, high);
                dst[k][n] = low;
                remaining = high;
            }

            dst[numCrossovers][n] = remaining;
        }
    }

    for (auto& f : splits)
        f.snapToZero();

    // Pass 2: allpass compensation. Band k gets the phase of every crossover
    // below it, so all six bands land back in step with each other.
    for (int k = 0; k < numCrossovers - 1; ++k)
    {
        for (int j = k + 1; j < numCrossovers; ++j)
        {
            juce::dsp::AudioBlock<float> block (bands[(size_t) k].getArrayOfWritePointers(),
                                                (size_t) numChannels,
                                                (size_t) numSamples);
            juce::dsp::ProcessContextReplacing<float> context (block);
            allpasses[(size_t) allpassIndex (k, j)].process (context);
        }
    }
}
} // namespace cabrot::dsp
