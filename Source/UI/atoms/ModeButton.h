#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Mode button atom for the AMP PROFILE grid. Behaves like a juce::TextButton
// (so Phase 3 can attach it via ButtonAttachment for radio-group selection),
// styled by SpectreLookAndFeel's drawButtonBackground override. Active state
// fills toxic with on-primary-container text; inactive renders surface-
// container-high with muted text.

class ModeButton final : public juce::TextButton
{
public:
    explicit ModeButton (juce::String name);

    void paintButton (juce::Graphics&,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModeButton)
};
} // namespace cabrot::ui
