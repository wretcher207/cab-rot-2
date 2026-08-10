#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Header live state mark: a small square in the live colour, quietly
// breathing on a three second cycle, with the LIVE label in the mono
// metadata style. One of exactly two places state colour exists in the
// interface (the other is the meter bar fill). Animation runs at 30 Hz
// when visible and suspends when hidden.

class LivePill final : public juce::Component, private juce::Timer
{
public:
    LivePill();
    ~LivePill() override; // stops the timer before any subclass / vtable teardown

    void paint (juce::Graphics&) override;

    void visibilityChanged() override;

private:
    void timerCallback() override;

    float pulsePhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LivePill)
};
} // namespace cabrot::ui
