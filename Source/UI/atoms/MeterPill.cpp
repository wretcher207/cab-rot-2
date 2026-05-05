#include "MeterPill.h"
#include "../../Theme/Fonts.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
MeterPill::MeterPill (juce::String labelText)
    : label (std::move (labelText))
{
    setInterceptsMouseClicks (false, false);
}

void MeterPill::setLevel (float zeroToOne)
{
    level = juce::jlimit (0.0f, 1.0f, zeroToOne);
    repaint();
}

void MeterPill::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    // Label uses GlyphArrangement to compute width (Font::getStringWidth is
    // deprecated in JUCE 8; this is the recommended replacement).
    juce::GlyphArrangement ga;
    ga.addLineOfText (theme::Fonts::uiChrome(), label, 0.0f, 0.0f);
    const int labelW = juce::jmax (24, juce::roundToInt (ga.getBoundingBox (0, -1, true).getWidth() + 6.0f));
    auto labelArea = area.removeFromLeft (labelW);
    area.removeFromLeft (4);

    g.setColour (theme::mutedForeground);
    g.setFont   (theme::Fonts::uiChrome());
    g.drawText  (label, labelArea, juce::Justification::centredLeft, false);

    auto pill = area.toFloat();
    const float pillH = juce::jmin (pill.getHeight(), 6.0f);
    pill = pill.withSizeKeepingCentre (pill.getWidth(), pillH);
    const float radius = pill.getHeight() * 0.5f;

    g.setColour (theme::surfaceContainerHighest);
    g.fillRoundedRectangle (pill, radius);
    g.setColour (theme::toxic);
    g.fillRoundedRectangle (pill.withWidth (pill.getWidth() * level), radius);
}
} // namespace cabrot::ui
