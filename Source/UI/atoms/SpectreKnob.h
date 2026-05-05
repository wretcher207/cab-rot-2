#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Cab Rot's main rotary control. Inherits juce::Slider so Phase 3 can
// attach it via juce::AudioProcessorValueTreeState::SliderAttachment with
// no further wiring. Visual styling lives in SpectreLookAndFeel's
// drawRotarySlider override.
//
// The component bundles the label-above and value-below text directly so
// callers don't have to position three siblings. Dimensions: 64x64 knob
// itself, 16 px gap above/below to label/value.

class SpectreKnob final : public juce::Component
{
public:
    SpectreKnob (juce::String label, juce::String unitsSuffix = juce::String());
    ~SpectreKnob() override;

    juce::Slider& getSlider() noexcept { return slider; }

    void setValueDisplay (double value, int decimals = 0);

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    juce::String   label;
    juce::String   suffix;
    juce::Slider   slider;
    juce::String   valueDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectreKnob)
};
} // namespace cabrot::ui
