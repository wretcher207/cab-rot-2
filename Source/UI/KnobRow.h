#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "atoms/SpectreKnob.h"

namespace cabrot::ui
{
// CANONICAL-UI §7.4. Bottom band, 192 px tall, 6 knobs evenly spaced
// (locked decision #1).

class KnobRow final : public juce::Component
{
public:
    KnobRow();
    ~KnobRow() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

    static constexpr int kNumKnobs = 6;
    SpectreKnob& getKnob (int index) { return *knobs[(size_t) index]; }

private:
    std::vector<std::unique_ptr<SpectreKnob>> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobRow)
};
} // namespace cabrot::ui
