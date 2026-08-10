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
    ga.addLineOfText (theme::Fonts::monoLabel (10.0f), label, 0.0f, 0.0f);
    const int labelW = juce::jmax (24, juce::roundToInt (ga.getBoundingBox (0, -1, true).getWidth() + 6.0f));
    auto labelArea = area.removeFromLeft (labelW);
    area.removeFromLeft (4);

    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  (label, labelArea, juce::Justification::centredLeft, false);

    // Square level bar: hairline-ruled track in surface3, fill in the live
    // state colour. No rounded ends.
    auto bar = area.toFloat();
    const float barH = juce::jmin (bar.getHeight(), 5.0f);
    bar = bar.withSizeKeepingCentre (bar.getWidth(), barH);

    g.setColour (theme::surface3);
    g.fillRect (bar);
    g.setColour (theme::stateLive.withAlpha (0.85f));
    g.fillRect (bar.withWidth (bar.getWidth() * level));
}
} // namespace cabrot::ui
