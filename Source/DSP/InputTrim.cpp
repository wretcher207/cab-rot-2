#include "InputTrim.h"
#include "Tuning.h"

namespace cabrot::dsp
{
void InputTrim::prepare (double sampleRate, int numChannels)
{
    channels = numChannels;
    gain.reset (sampleRate, (double) tuning::kParamSmoothingMs * 0.001);
    gain.setCurrentAndTargetValue (1.0f);
}

void InputTrim::reset() noexcept
{
    gain.setCurrentAndTargetValue (gain.getTargetValue());
}

void InputTrim::setGainDb (float db) noexcept
{
    gain.setTargetValue (juce::Decibels::decibelsToGain (db, -60.0f));
}

void InputTrim::snapToTarget() noexcept
{
    gain.setCurrentAndTargetValue (gain.getTargetValue());
}

void InputTrim::process (juce::AudioBuffer<float>& buffer, int numChannels, int numSamples) noexcept
{
    // Unity and not ramping: nothing to do, and the buffer stays bit-exact.
    if (! gain.isSmoothing() && gain.getCurrentValue() == 1.0f)
        return;

    auto* const* data = buffer.getArrayOfWritePointers();

    for (int n = 0; n < numSamples; ++n)
    {
        const float g = gain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            data[ch][n] *= g;
    }
}
} // namespace cabrot::dsp
