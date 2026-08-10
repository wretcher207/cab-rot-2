#include "KnobRow.h"

#include "../Theme/Palette.h"

namespace cabrot::ui
{
namespace
{
const std::array<juce::String, KnobRow::kNumKnobs> kLabels {{
    "FIZZ HUNT",
    "EDGE PRESERVE",
    "CAB SMOOTH",
    "DIGITAL SAND",
    "AIR ROT",
    "REAP MIX",
}};
}

KnobRow::KnobRow()
{
    knobs.reserve (kNumKnobs);
    for (const auto& label : kLabels)
    {
        auto knob = std::make_unique<SpectreKnob> (label);
        addAndMakeVisible (*knob);
        knobs.push_back (std::move (knob));
    }
}

void KnobRow::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour (theme::rule);
    g.fillRect (area.removeFromTop (1));
    g.setColour (theme::canvas);
    g.fillRect (area);
}

void KnobRow::resized()
{
    auto area = getLocalBounds().reduced (32, 16);
    const int cellW = area.getWidth() / kNumKnobs;

    for (int i = 0; i < kNumKnobs; ++i)
    {
        const int x = area.getX() + i * cellW;
        knobs[(size_t) i]->setBounds (x, area.getY(), cellW, area.getHeight());
    }
}
} // namespace cabrot::ui
