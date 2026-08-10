#include "AmpProfileGrid.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
namespace
{
const std::array<juce::String, AmpProfileGrid::kNumModes> kModeNames {
    "5150", "RECTO", "HM-2", "DJENT", "BLACKENED", "SLUDGE"
};
}

AmpProfileGrid::AmpProfileGrid()
{
    modes.reserve (kNumModes);
    for (int i = 0; i < kNumModes; ++i)
    {
        auto button = std::make_unique<ModeButton> (kModeNames[(size_t) i]);
        // setClickingTogglesState already wired in ModeButton ctor; only the
        // radio-group affiliation is grid-level state.
        button->setRadioGroupId (1, juce::dontSendNotification);
        addAndMakeVisible (*button);
        modes.push_back (std::move (button));
    }
}

void AmpProfileGrid::paint (juce::Graphics& g)
{
    auto inner = getLocalBounds();
    auto headerStrip = inner.removeFromTop (16);

    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  ("AMP PROFILE", headerStrip, juce::Justification::centredLeft, false);
}

void AmpProfileGrid::resized()
{
    auto inner = getLocalBounds();
    inner.removeFromTop (16 + 16); // header + gap

    constexpr int cols = 2;
    constexpr int rows = 3;
    constexpr int gap  = 8;

    const int cellW = (inner.getWidth()  - gap * (cols - 1)) / cols;
    const int cellH = (inner.getHeight() - gap * (rows - 1)) / rows;

    for (int i = 0; i < kNumModes; ++i)
    {
        const int col = i % cols;
        const int row = i / cols;
        modes[(size_t) i]->setBounds (
            inner.getX() + col * (cellW + gap),
            inner.getY() + row * (cellH + gap),
            cellW, cellH);
    }
}
} // namespace cabrot::ui
