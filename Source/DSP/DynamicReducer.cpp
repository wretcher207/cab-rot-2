#include "DynamicReducer.h"
#include "Tuning.h"

#include <cmath>

namespace cabrot::dsp
{
namespace
{
constexpr float kLogToDb = 8.685889638f;
constexpr float kDbToLog = 1.0f / kLogToDb;

float onePoleCoeff (float timeMs, double sampleRate) noexcept
{
    const auto tau = (double) juce::jmax (0.001f, timeMs) * 0.001;
    return (float) std::exp (-1.0 / (tau * sampleRate));
}

/** 10^(-db/20) without going through std::pow, which is a log and an exp. */
inline float attenuationFor (float db) noexcept
{
    return std::exp (db * -kDbToLog);
}
} // namespace

void DynamicReducer::prepare (double sampleRate, int numChannels)
{
    juce::ignoreUnused (numChannels);

    sr = sampleRate;
    setAttackMs (8.0f);
    gainCoeff = onePoleCoeff (tuning::kGainSmoothingMs, sampleRate);
    reset();
}

void DynamicReducer::reset() noexcept
{
    env.fill (0.0f);
    smoothedGain.fill (1.0f);
}

void DynamicReducer::setAttackMs (float ms) noexcept
{
    const auto attack  = juce::jmax (0.05f, ms);
    const auto release = juce::jmax (tuning::kReducerMinReleaseMs, attack * tuning::kReducerReleaseRatio);

    attackCoeff  = onePoleCoeff (attack,  sr);
    releaseCoeff = onePoleCoeff (release, sr);
}

float DynamicReducer::processToDelta (float* const* channels,
                                      const float* gate,
                                      int numChannels,
                                      int numSamples) noexcept
{
    // Knob at zero with no shelf. No reduction is possible, so the delta is
    // exactly zero and the band contributes nothing at all downstream.
    if (maxReductionDb <= 0.0f && staticGain == 1.0f)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::clear (channels[ch], numSamples);

        reset();
        return 0.0f;
    }

    const int chans = juce::jmin (numChannels, maxChannels);
    float peakReductionDb = 0.0f;

    // Comparing against the threshold in the linear domain keeps the log off
    // the hot path entirely whenever the band is quiet, which on real
    // material is most of the time.
    const float thresholdLinear = std::exp (thresholdDb * kDbToLog);
    const float invThreshold    = 1.0f / thresholdLinear;

    // Fully linked is the default, and there both channels would compute an
    // identical envelope, threshold test and gain. Do it once.
    const bool fullyLinked = (chans == 1) || (stereoLink >= 0.999f);

    if (fullyLinked)
    {
        for (int n = 0; n < numSamples; ++n)
        {
            float detect = 0.0f;
            for (int ch = 0; ch < chans; ++ch)
                detect = juce::jmax (detect, std::abs (channels[ch][n]));

            const float coeff = (detect > env[0]) ? attackCoeff : releaseCoeff;
            env[0] = detect + coeff * (env[0] - detect);

            float appliedDb = 0.0f;
            if (env[0] > thresholdLinear)
            {
                const float overDb = kLogToDb * std::log (env[0] * invThreshold);
                appliedDb = juce::jmin (overDb * tuning::kReductionSlope, maxReductionDb)
                          * (1.0f - gate[n]);
            }

            const float target = staticGain * (appliedDb <= 0.0f ? 1.0f : attenuationFor (appliedDb));
            smoothedGain[0] = target + gainCoeff * (smoothedGain[0] - target);

            const float delta = smoothedGain[0] - 1.0f;
            for (int ch = 0; ch < chans; ++ch)
                channels[ch][n] *= delta;

            peakReductionDb = juce::jmax (peakReductionDb, appliedDb);
        }

        env[1] = env[0];
        smoothedGain[1] = smoothedGain[0];
    }
    else
    {
        for (int n = 0; n < numSamples; ++n)
        {
            float linked = 0.0f;
            for (int ch = 0; ch < chans; ++ch)
                linked = juce::jmax (linked, std::abs (channels[ch][n]));

            const float openness = 1.0f - gate[n];

            for (int ch = 0; ch < chans; ++ch)
            {
                const float own = std::abs (channels[ch][n]);
                const float detect = own + stereoLink * (linked - own);

                const float coeff = (detect > env[(size_t) ch]) ? attackCoeff : releaseCoeff;
                env[(size_t) ch] = detect + coeff * (env[(size_t) ch] - detect);

                float appliedDb = 0.0f;
                if (env[(size_t) ch] > thresholdLinear)
                {
                    const float overDb = kLogToDb * std::log (env[(size_t) ch] * invThreshold);
                    appliedDb = juce::jmin (overDb * tuning::kReductionSlope, maxReductionDb) * openness;
                }

                const float target = staticGain * (appliedDb <= 0.0f ? 1.0f : attenuationFor (appliedDb));
                smoothedGain[(size_t) ch] = target + gainCoeff * (smoothedGain[(size_t) ch] - target);

                channels[ch][n] *= (smoothedGain[(size_t) ch] - 1.0f);

                peakReductionDb = juce::jmax (peakReductionDb, appliedDb);
            }
        }
    }

    for (int ch = chans; ch < numChannels; ++ch)
        juce::FloatVectorOperations::clear (channels[ch], numSamples);

    return peakReductionDb;
}
} // namespace cabrot::dsp
