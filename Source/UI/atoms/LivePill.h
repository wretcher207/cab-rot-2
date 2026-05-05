#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Header pill: rounded surface-container background, a toxic dot that
// pulses at ~0.8 Hz, "LIVE" label in ui-chrome typography. Pulse animation
// runs at 60 Hz when visible; suspends when hidden so we don't burn CPU.

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
