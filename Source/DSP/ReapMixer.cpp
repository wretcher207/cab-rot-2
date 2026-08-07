#include "ReapMixer.h"
#include "Tuning.h"

#include <cmath>

namespace cabrot::dsp
{
namespace
{
constexpr float kLogToDb = 8.685889638f;
constexpr float kSilence = 1.0e-10f;
} // namespace

void ReapMixer::prepare (double sampleRate, int numChannels)
{
    juce::ignoreUnused (numChannels);

    sr = sampleRate;
    mix.reset (sampleRate, (double) tuning::kParamSmoothingMs * 0.001);
    mix.setCurrentAndTargetValue (1.0f);
    makeup.reset (sampleRate, (double) tuning::kParamSmoothingMs * 0.001);
    makeup.setCurrentAndTargetValue (1.0f);
    reset();
}

void ReapMixer::reset() noexcept
{
    mix.setCurrentAndTargetValue (mix.getTargetValue());
    makeup.setCurrentAndTargetValue (1.0f);
    trackedDryMs = 0.0f;
    trackedWetMs = 0.0f;
}

void ReapMixer::setMix (float mix01) noexcept
{
    mix.setTargetValue (juce::jlimit (0.0f, 1.0f, mix01));
}

void ReapMixer::setAutoGain (bool shouldApply) noexcept
{
    autoGain = shouldApply;

    if (! shouldApply)
        makeup.setTargetValue (1.0f);
}

void ReapMixer::process (juce::AudioBuffer<float>& bandSum,
                         const juce::AudioBuffer<float>& delta,
                         int numChannels,
                         int numSamples) noexcept
{
    auto* const* out = bandSum.getArrayOfWritePointers();

    double dryEnergy = 0.0;
    double wetEnergy = 0.0;

    // Pass 1: fold the delta in and measure what that cost us.
    if (mix.isSmoothing() || mix.getCurrentValue() > 0.0f)
    {
        const auto* const* deltaData = delta.getArrayOfReadPointers();

        for (int n = 0; n < numSamples; ++n)
        {
            const float m = mix.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float dry = out[ch][n];
                const float wet = dry + m * deltaData[ch][n];

                dryEnergy += (double) dry * dry;
                wetEnergy += (double) wet * wet;

                out[ch][n] = wet;
            }
        }
    }
    else if (autoGain)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            for (int n = 0; n < numSamples; ++n)
                dryEnergy += (double) out[ch][n] * out[ch][n];

        wetEnergy = dryEnergy;
    }

    // Pass 2: slow makeup for whatever the clamp took away.
    if (! autoGain)
    {
        makeup.setTargetValue (1.0f);
    }
    else
    {
        const auto scale = 1.0 / juce::jmax (1.0, (double) (numSamples * juce::jmax (1, numChannels)));
        const auto blockDry = (float) (dryEnergy * scale);
        const auto blockWet = (float) (wetEnergy * scale);

        const auto tau = (double) tuning::kAutoGainTimeMs * 0.001;
        const auto coeff = (float) std::exp (-(double) numSamples / (tau * sr));

        trackedDryMs = blockDry + coeff * (trackedDryMs - blockDry);
        trackedWetMs = blockWet + coeff * (trackedWetMs - blockWet);

        if (trackedDryMs > kSilence && trackedWetMs > kSilence)
        {
            // Mean-square ratio, so half the log gives the dB in amplitude.
            const float deficitDb = 0.5f * kLogToDb * std::log (trackedDryMs / trackedWetMs);
            const float clamped = juce::jlimit (0.0f, tuning::kAutoGainMaxDb, deficitDb);
            makeup.setTargetValue (juce::Decibels::decibelsToGain (clamped));
        }
        else
        {
            makeup.setTargetValue (1.0f);
        }
    }

    if (makeup.isSmoothing() || makeup.getCurrentValue() != 1.0f)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            const float g = makeup.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
                out[ch][n] *= g;
        }
    }
}
} // namespace cabrot::dsp
