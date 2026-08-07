#include "TransientDetector.h"
#include "Tuning.h"

#include <cmath>

namespace cabrot::dsp
{
namespace
{
// 20 / ln(10). Turns one natural log into a dB value.
constexpr float kLogToDb = 8.685889638f;
constexpr float kFloor   = 1.0e-9f;
} // namespace

float TransientDetector::onePoleCoeff (float timeMs, double sampleRate) noexcept
{
    const auto tau = (double) juce::jmax (0.001f, timeMs) * 0.001;
    return (float) std::exp (-1.0 / (tau * sampleRate));
}

void TransientDetector::prepare (double sampleRate)
{
    sr = sampleRate;

    fastAttack  = onePoleCoeff (tuning::kTransientFastAttackMs,  sampleRate);
    fastRelease = onePoleCoeff (tuning::kTransientFastReleaseMs, sampleRate);
    slowAttack  = onePoleCoeff (tuning::kTransientSlowAttackMs,  sampleRate);
    slowRelease = onePoleCoeff (tuning::kTransientSlowReleaseMs, sampleRate);
    gateRelease = onePoleCoeff (tuning::kTransientGateReleaseMs, sampleRate);

    setPickWindowMs (5.0f);
    reset();
}

void TransientDetector::reset() noexcept
{
    fastEnv = 0.0f;
    slowEnv = 0.0f;
    gate    = 0.0f;
    holdCounter = 0;
}

void TransientDetector::setEdgePreserve (float amount01) noexcept
{
    const auto amount = juce::jlimit (0.0f, 1.0f, amount01);

    thresholdDb = tuning::kTransientThresholdMaxDb
                + amount * (tuning::kTransientThresholdMinDb - tuning::kTransientThresholdMaxDb);

    // At zero the gate is not merely insensitive, it is switched off, so the
    // knob's bottom end really does mean "clamp everything, attack included".
    depth = amount;
}

void TransientDetector::setPickWindowMs (float ms) noexcept
{
    holdLength = juce::jmax (0, juce::roundToInt ((double) juce::jmax (0.0f, ms) * 0.001 * sr));
}

void TransientDetector::process (const float* const* channels,
                                 int numChannels,
                                 int numSamples,
                                 float* gateOut) noexcept
{
    if (depth <= 0.0f)
    {
        juce::FloatVectorOperations::clear (gateOut, numSamples);
        gate = 0.0f;
        holdCounter = 0;
        return;
    }

    const float knee = tuning::kTransientKneeDb;

    // Below this ratio the gate is shut, so the log never needs computing.
    const float openingRatio = std::exp (thresholdDb / kLogToDb);

    for (int n = 0; n < numSamples; ++n)
    {
        float mag = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            mag = juce::jmax (mag, std::abs (channels[ch][n]));

        const float fc = (mag > fastEnv) ? fastAttack : fastRelease;
        fastEnv = mag + fc * (fastEnv - mag);

        const float sc = (mag > slowEnv) ? slowAttack : slowRelease;
        slowEnv = mag + sc * (slowEnv - mag);

        const float fast = fastEnv + kFloor;
        const float slow = slowEnv + kFloor;

        float raw = 0.0f;
        if (fast > slow * openingRatio)
        {
            const float leadDb = kLogToDb * std::log (fast / slow);
            const float t = juce::jlimit (0.0f, 1.0f, (leadDb - thresholdDb) / knee);
            raw = depth * t * t * (3.0f - 2.0f * t);
        }

        if (raw >= gate)
        {
            gate = raw;
            holdCounter = holdLength;
        }
        else if (holdCounter > 0)
        {
            --holdCounter;
        }
        else
        {
            gate = raw + gateRelease * (gate - raw);
        }

        gateOut[n] = gate;
    }
}
} // namespace cabrot::dsp
