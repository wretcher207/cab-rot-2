#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace cabrot::dsp
{
/**
    Emits a 0..1 "hands off, this is a pick attack" gate for one band.

    Two envelope followers race each other. The fast one leaps onto a leading
    edge; the slow one lags. The gap between them, in dB, is the transient
    signal. Above threshold the gate opens, holds for the pick window, then
    falls away, which leaves the sustain exposed to the reducer while the
    attack passes untouched.

    Detection is stereo-linked on purpose: a gate that opened on one side only
    would pull the image around on every chug.
*/
class TransientDetector
{
public:
    void prepare (double sampleRate);
    void reset() noexcept;

    /** 0 = never protect the attack, 1 = protect on the faintest edge. */
    void setEdgePreserve (float amount01) noexcept;

    /** How long the gate stays fully open after an edge. */
    void setPickWindowMs (float ms) noexcept;

    void process (const float* const* channels,
                  int numChannels,
                  int numSamples,
                  float* gateOut) noexcept;

private:
    static float onePoleCoeff (float timeMs, double sampleRate) noexcept;

    double sr { 48000.0 };

    float fastAttack { 0.0f }, fastRelease { 0.0f };
    float slowAttack { 0.0f }, slowRelease { 0.0f };
    float gateRelease { 0.0f };

    float fastEnv { 0.0f }, slowEnv { 0.0f }, gate { 0.0f };

    int holdLength { 0 }, holdCounter { 0 };

    float thresholdDb { 6.0f };
    float depth { 0.5f };
};
} // namespace cabrot::dsp
