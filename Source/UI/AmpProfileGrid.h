#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "atoms/ModeButton.h"

namespace cabrot::ui
{
// CANONICAL-UI §7.3. AMP PROFILE card with 2x3 grid of mode buttons.
//
// Phase 2: 5150 starts active, others inactive. Buttons are real
// juce::TextButton subclasses so Phase 3 can attach a ParameterAttachment
// for the mode parameter without re-architecting.

class AmpProfileGrid final : public juce::Component
{
public:
    AmpProfileGrid();
    ~AmpProfileGrid() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

    static constexpr int kNumModes = 6;
    ModeButton& getModeButton (int index) { return *modes[(size_t) index]; }

private:
    std::vector<std::unique_ptr<ModeButton>> modes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpProfileGrid)
};
} // namespace cabrot::ui
