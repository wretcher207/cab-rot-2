#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace cabrot::dsp
{
/** A smoothed dB gain stage. Used for both the input and the output trim. */
class InputTrim
{
public:
    void prepare (double sampleRate, int numChannels);
    void reset() noexcept;

    void setGainDb (float db) noexcept;

    /** Snaps to the current target without ramping. Use after a state load. */
    void snapToTarget() noexcept;

    void process (juce::AudioBuffer<float>& buffer, int numChannels, int numSamples) noexcept;

private:
    juce::SmoothedValue<float> gain;
    int channels { 2 };
};
} // namespace cabrot::dsp
