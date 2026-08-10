#include "FizzReadout.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
FizzReadout::FizzReadout()
{
    setOpaque (false);
}

void FizzReadout::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    auto labelArea = area.removeFromTop (16);
    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  ("FIZZ AMOUNT", labelArea, juce::Justification::centredLeft, false);

    area.removeFromTop (12);

    // The number is a numeric readout, so it renders in JetBrains Mono even
    // at display size. No glow, no halo: primary ink on the ground, and the
    // percent sign sits on the same baseline in metadata grey.
    const float numPx = juce::jlimit (44.0f, 84.0f, static_cast<float> (area.getHeight()) * 0.55f);
    auto heroFont = theme::Fonts::mono (numPx, -0.04f);
    auto suffixFont = theme::Fonts::mono (juce::jmax (18.0f, numPx * 0.33f));

    const auto numText = juce::String (displayValue, 1);

    juce::GlyphArrangement na;
    na.addLineOfText (heroFont, numText, 0.0f, 0.0f);
    const auto numBounds = na.getBoundingBox (0, -1, true);
    const float numW = numBounds.getWidth();
    const float numH = numBounds.getHeight();

    juce::GlyphArrangement pa;
    pa.addLineOfText (suffixFont, "%", 0.0f, 0.0f);
    const float pctW = pa.getBoundingBox (0, -1, true).getWidth();

    const float gapP = 6.0f;
    const float totalW = numW + gapP + pctW;
    const float numX = area.getCentreX() - totalW * 0.5f;
    const float numY = static_cast<float> (area.getY())
                     + (area.getHeight() - numH) * 0.5f;

    g.setFont (heroFont);
    g.setColour (theme::inkPrimary);
    g.drawText (numText,
                juce::Rectangle<float> (numX, numY, numW + 16.0f, numH + 24.0f),
                juce::Justification::centredLeft, false);

    g.setFont (suffixFont);
    g.setColour (theme::inkMeta);
    g.drawText ("%",
                juce::Rectangle<float> (numX + numW + gapP, numY + 6.0f,
                                        pctW + 16.0f, suffixFont.getHeight() + 16.0f),
                juce::Justification::centredLeft, false);

    const auto bottom = getLocalBounds();
    g.setColour (theme::rule);
    g.fillRect (bottom.getX(),
                bottom.getBottom() - 1,
                bottom.getWidth(),
                1);
}
} // namespace cabrot::ui
