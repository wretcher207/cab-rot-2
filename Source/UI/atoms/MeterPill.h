#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Footer IN/OUT meter: an uppercase mono label, then a square bar in a
// quiet track with a 1 px hairline frame; the fill is the live state
// colour. Phase 2 just animates a static value; Phase 6 wires it to
// peak/RMS analysis off the audio thread.

class MeterPill final : public juce::Component
{
public:
    explicit MeterPill (juce::String label);
    ~MeterPill() override = default;

    void setLevel (float zeroToOne);
    void paint (juce::Graphics&) override;

private:
    juce::String label;
    float        level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterPill)
};
} // namespace cabrot::ui
