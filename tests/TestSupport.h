#pragma once

// Shared helpers for the Cab Rot test executables.

#include "../Source/PluginProcessor.h"

#include <juce_dsp/juce_dsp.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

namespace cabrot::test
{
inline constexpr int kDefaultBlockSize = 512;

/** Deterministic pink noise (Paul Kellet's economy filter). */
class PinkNoise
{
public:
    explicit PinkNoise (juce::uint32 seed = 0xCAB80Bu) : rng (seed) {}

    float next() noexcept
    {
        const float white = dist (rng);

        b0 = 0.99765f * b0 + white * 0.0990460f;
        b1 = 0.96300f * b1 + white * 0.2965164f;
        b2 = 0.57000f * b2 + white * 1.0526913f;

        return (b0 + b1 + b2 + white * 0.1848f) * 0.25f;
    }

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
    float b0 { 0.0f }, b1 { 0.0f }, b2 { 0.0f };
};

/** Sets a parameter by its real-world value, not its normalised one. */
inline void setParam (CabRotProcessor& processor, const juce::String& id, float value)
{
    if (auto* param = processor.getApvts().getParameter (id))
        param->setValueNotifyingHost (param->convertTo0to1 (value));
}

/** Puts every knob in a known place: no reduction, fully dry, unity trim. */
inline void setNeutral (CabRotProcessor& processor)
{
    setParam (processor, params::fizzHunt,     0.0f);
    setParam (processor, params::edgePreserve, 0.0f);
    setParam (processor, params::cabSmooth,    0.0f);
    setParam (processor, params::digitalSand,  0.0f);
    setParam (processor, params::airRot,       0.0f);
    setParam (processor, params::reapMix,      0.0f);
    setParam (processor, params::inputGain,    0.0f);
    setParam (processor, params::outputGain,   0.0f);

    if (auto* autoGain = processor.getApvts().getParameter (params::autoGain))
        autoGain->setValueNotifyingHost (0.0f);
}

inline void prepareStereo (CabRotProcessor& processor, double sampleRate, int blockSize)
{
    juce::AudioProcessor::BusesLayout layout;
    layout.inputBuses .add (juce::AudioChannelSet::stereo());
    layout.outputBuses.add (juce::AudioChannelSet::stereo());

    processor.setBusesLayout (layout);
    processor.setRateAndBufferSizeDetails (sampleRate, blockSize);
    processor.prepareToPlay (sampleRate, blockSize);
}

/** Runs a whole signal through the processor in blocks, in place. */
inline void runInPlace (CabRotProcessor& processor, juce::AudioBuffer<float>& buffer, int blockSize)
{
    juce::MidiBuffer midi;
    const int numSamples = buffer.getNumSamples();

    for (int offset = 0; offset < numSamples; offset += blockSize)
    {
        const int chunk = juce::jmin (blockSize, numSamples - offset);

        juce::AudioBuffer<float> slice (buffer.getArrayOfWritePointers(),
                                        buffer.getNumChannels(),
                                        offset,
                                        chunk);
        processor.processBlock (slice, midi);
    }
}

/** Feeds silence so smoothed parameters reach their targets before measuring. */
inline void warmUp (CabRotProcessor& processor, double sampleRate, int blockSize, float milliseconds = 250.0f)
{
    const int total = juce::roundToInt (sampleRate * (double) milliseconds * 0.001);

    juce::AudioBuffer<float> scratch (2, juce::jmax (blockSize, total));
    scratch.clear();

    PinkNoise noise;
    for (int ch = 0; ch < scratch.getNumChannels(); ++ch)
        for (int n = 0; n < scratch.getNumSamples(); ++n)
            scratch.setSample (ch, n, noise.next() * 0.2f);

    runInPlace (processor, scratch, blockSize);
}

/** Averaged magnitude spectrum, linear, one value per bin below Nyquist. */
inline std::vector<float> averageSpectrum (const float* data, int numSamples, int fftOrder)
{
    const int fftSize = 1 << fftOrder;
    const int hop     = fftSize / 2;

    juce::dsp::FFT fft (fftOrder);
    juce::dsp::WindowingFunction<float> window ((size_t) fftSize,
                                                juce::dsp::WindowingFunction<float>::hann,
                                                false);

    std::vector<float> accumulator ((size_t) (fftSize / 2), 0.0f);
    std::vector<float> scratch ((size_t) fftSize * 2, 0.0f);

    int frames = 0;

    for (int start = 0; start + fftSize <= numSamples; start += hop)
    {
        std::fill (scratch.begin(), scratch.end(), 0.0f);
        std::copy (data + start, data + start + fftSize, scratch.begin());

        window.multiplyWithWindowingTable (scratch.data(), (size_t) fftSize);
        fft.performFrequencyOnlyForwardTransform (scratch.data());

        for (int bin = 0; bin < fftSize / 2; ++bin)
            accumulator[(size_t) bin] += scratch[(size_t) bin] * scratch[(size_t) bin];

        ++frames;
    }

    const float scale = 1.0f / (float) juce::jmax (1, frames);
    for (auto& value : accumulator)
        value = std::sqrt (value * scale);

    return accumulator;
}

inline int binForHz (double hz, double sampleRate, int fftSize) noexcept
{
    return juce::jlimit (1, fftSize / 2 - 1, (int) std::round (hz / sampleRate * (double) fftSize));
}

/** Mean level difference, in dB, of `measured` against `reference` over a band. */
inline float bandDeltaDb (const std::vector<float>& measured,
                          const std::vector<float>& reference,
                          double lowHz, double highHz,
                          double sampleRate, int fftSize)
{
    const int lowBin  = binForHz (lowHz,  sampleRate, fftSize);
    const int highBin = binForHz (highHz, sampleRate, fftSize);

    double sum = 0.0;
    int count = 0;

    for (int bin = lowBin; bin <= highBin; ++bin)
    {
        const float a = measured [(size_t) bin];
        const float b = reference[(size_t) bin];

        if (b > 1.0e-9f && a > 1.0e-9f)
        {
            sum += 20.0 * std::log10 ((double) a / (double) b);
            ++count;
        }
    }

    return count > 0 ? (float) (sum / count) : 0.0f;
}

/** Largest single-bin deviation, in dB, over a band. */
inline float worstDeltaDb (const std::vector<float>& measured,
                           const std::vector<float>& reference,
                           double lowHz, double highHz,
                           double sampleRate, int fftSize)
{
    const int lowBin  = binForHz (lowHz,  sampleRate, fftSize);
    const int highBin = binForHz (highHz, sampleRate, fftSize);

    float worst = 0.0f;

    for (int bin = lowBin; bin <= highBin; ++bin)
    {
        const float a = measured [(size_t) bin];
        const float b = reference[(size_t) bin];

        if (b > 1.0e-9f && a > 1.0e-9f)
            worst = juce::jmax (worst, std::abs ((float) (20.0 * std::log10 ((double) a / (double) b))));
    }

    return worst;
}

inline bool allFinite (const juce::AudioBuffer<float>& buffer) noexcept
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const auto* data = buffer.getReadPointer (ch);

        for (int n = 0; n < buffer.getNumSamples(); ++n)
            if (! std::isfinite (data[n]))
                return false;
    }

    return true;
}

struct Report
{
    int passed { 0 };
    int failed { 0 };

    void check (bool condition, const juce::String& label)
    {
        std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';

        if (condition)
            ++passed;
        else
            ++failed;
    }

    void note (const juce::String& label) const
    {
        std::cout << "        " << label << '\n';
    }

    int exitCode() const noexcept { return failed == 0 ? 0 : 1; }
};
} // namespace cabrot::test
