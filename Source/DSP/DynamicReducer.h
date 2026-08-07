#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

namespace cabrot::dsp
{
/**
    One band's dynamic clamp.

    An envelope follower watches the band. Whatever sits above the Fizz Hunt
    threshold gets pulled down, up to a per-band ceiling, and that reduction
    is then scaled by (1 - transient gate) so a pick attack walks through
    untouched. That scaling is the whole architectural argument for Cab Rot
    over a plain dynamic EQ.

    The band buffer is replaced with the amount to REMOVE from it rather than
    with the reduced band: band * (gain - 1). Downstream simply adds the
    deltas to the untouched band sum, which keeps the dry path and the wet
    path phase-identical and means the Reap Mix knob cannot comb.
*/
class DynamicReducer
{
public:
    void prepare (double sampleRate, int numChannels);
    void reset() noexcept;

    void setThresholdDb (float db) noexcept      { thresholdDb = db; }
    void setMaxReductionDb (float db) noexcept   { maxReductionDb = juce::jmax (0.0f, db); }
    void setStereoLink (float amount01) noexcept { stereoLink = juce::jlimit (0.0f, 1.0f, amount01); }
    void setAttackMs (float ms) noexcept;

    /** A constant cut on top of the dynamic one. Air Rot's shelf rides here
        so it stays inside the same delta and cannot come adrift of it. */
    void setStaticTrimDb (float db) noexcept
    {
        staticGain = (db == 0.0f) ? 1.0f : juce::Decibels::decibelsToGain (db);
    }

    /** Replaces `channels` with the per-sample delta. Returns the largest
        reduction applied over the block, in dB, for metering. */
    float processToDelta (float* const* channels,
                          const float* gate,
                          int numChannels,
                          int numSamples) noexcept;

private:
    static constexpr int maxChannels = 2;

    double sr { 48000.0 };

    float attackCoeff { 0.0f }, releaseCoeff { 0.0f }, gainCoeff { 0.0f };

    float thresholdDb    { -30.0f };
    float maxReductionDb { 0.0f };
    float stereoLink     { 1.0f };
    float staticGain     { 1.0f };

    std::array<float, maxChannels> env {};
    std::array<float, maxChannels> smoothedGain {};
};
} // namespace cabrot::dsp
