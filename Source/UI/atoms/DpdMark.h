#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// Coded approximation of the Dead Pixel Design mark: two offset square
// outlines with a single bright "dead pixel" near the upper-right corner
// of the inner square. Renders entirely in JUCE primitives so the plugin
// has zero asset dependencies for the brand mark.
//
// Stitch reference uses the mark at 16x16, grayscale 70% opacity. We honor
// the size/opacity but use our own primitive rendering since the original
// PNG is on a Google CDN that we don't bundle.

class DpdMark final : public juce::Component
{
public:
    DpdMark();
    ~DpdMark() override = default;

    void paint (juce::Graphics&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DpdMark)
};
} // namespace cabrot::ui
