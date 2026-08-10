#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Header engine-state mark. It is disabled until the processor reports
// recent input, and uses the live token only while that report is true.

class LivePill final : public juce::Component
{
public:
    LivePill();
    ~LivePill() override = default;

    void setLive (bool shouldBeLive);
    void paint (juce::Graphics&) override;

private:
    bool live = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LivePill)
};
} // namespace cabrot::ui
