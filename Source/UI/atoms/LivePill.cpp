#include "LivePill.h"
#include "../../Theme/Fonts.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
LivePill::LivePill()
{
    setInterceptsMouseClicks (false, false);
}

void LivePill::setLive (bool shouldBeLive)
{
    if (live == shouldBeLive)
        return;

    live = shouldBeLive;
    repaint();
}

void LivePill::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();

    const float side  = 7.0f;
    const auto  centre = juce::Point<float> (area.getX() + side + 2.0f, area.getCentreY());

    g.setColour (live ? theme::stateLive : theme::inkDisabled);
    g.fillRect (juce::Rectangle<float> (side, side).withCentre (centre));

    auto labelArea = area.withTrimmedLeft (side * 2.0f + 10.0f);
    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  ("LIVE", labelArea, juce::Justification::centredLeft, false);
}
} // namespace cabrot::ui
