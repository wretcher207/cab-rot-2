#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Delta Listen lives in the header as a ghost icon. The ghost is
// discovered, never announced: inkDisabled at rest, inkBody on hover,
// inkPrimary while listening. It never emits light in any state. The
// is rendered as a juce::Path with a rounded head and three tail bumps
// along the bottom. Two open eye holes.
//
// Phase 2: visual-only toggle (click flips the state, no audio impact).
// Phase 7: wires the audio bypass.

class GhostToggle final : public juce::Button
{
public:
    GhostToggle();
    ~GhostToggle() override = default;

    void paintButton (juce::Graphics&,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GhostToggle)
};
} // namespace cabrot::ui
