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
    // at display size. No glow, no halo: just primary ink on the ground.
    const float numPx = juce::jlimit (44.0f, 96.0f, static_cast<float> (area.getHeight()) * 0.62f);
    auto heroFont = theme::Fonts::mono (numPx, -0.04f);
    g.setFont (heroFont);

    const auto numText = juce::String (displayValue, 1);
    g.setColour (theme::inkPrimary);
    g.drawText (numText, area, juce::Justification::centred, false);

    // Percent sign in metadata grey, smaller, hugging the number.
    auto suffixFont = theme::Fonts::mono (juce::jmax (20.0f, numPx * 0.36f));
    juce::GlyphArrangement ga;
    ga.addLineOfText (heroFont, numText, 0.0f, 0.0f);
    const float numW = ga.getBoundingBox (0, -1, true).getWidth();

    const float suffixX = area.getCentreX() + numW * 0.5f + 4.0f;
    g.setFont   (suffixFont);
    g.setColour (theme::inkMeta);
    g.drawText ("%",
                juce::Rectangle<float> (suffixX,
                                        area.getY() + numPx * 0.16f,
                                        suffixFont.getHeight() * 1.0f,
                                        suffixFont.getHeight()),
                juce::Justification::centredLeft, false);

    const auto bottom = getLocalBounds();
    g.setColour (theme::rule);
    g.fillRect (bottom.getX(),
                bottom.getBottom() - 1,
                bottom.getWidth(),
                1);
}
} // namespace cabrot::ui
