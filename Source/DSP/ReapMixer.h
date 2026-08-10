#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace cabrot::dsp
{
/**
    Blends the removed material back in, and optionally makes up the level.

    Because the reducer hands over a delta rather than a processed band, the
    blend is an addition rather than a crossfade: out = bandSum + mix * delta.
    Both terms come from the same band split, so they are phase-identical and
    the mix knob scales how much fizz is taken out without ever comb filtering
    the source. At mix = 0 the output is the band sum untouched.

    Auto gain only ever adds, because the reducer only ever cuts.
*/
class ReapMixer
{
public:
    void prepare (double sampleRate, int numChannels);
    void reset() noexcept;

    void setMix (float mix01) noexcept;
    void setAutoGain (bool shouldApply) noexcept;

    /** In place on `bandSum`. */
    void process (juce::AudioBuffer<float>& bandSum,
                  const juce::AudioBuffer<float>& delta,
                  int numChannels,
                  int numSamples) noexcept;

    /** Writes only the material removed by the normal path: -mix * delta. */
    void processRemovedSignal (juce::AudioBuffer<float>& output,
                               const juce::AudioBuffer<float>& delta,
                               int numChannels,
                               int numSamples) noexcept;

private:
    double sr { 48000.0 };

    juce::SmoothedValue<float> mix;
    juce::SmoothedValue<float> makeup;

    bool  autoGain { true };
    float trackedDryMs { 0.0f };
    float trackedWetMs { 0.0f };
};
} // namespace cabrot::dsp
