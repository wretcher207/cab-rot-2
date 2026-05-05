#include "FizzReadout.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"
#include "../Theme/SpectreLookAndFeel.h"

namespace cabrot::ui
{
FizzReadout::FizzReadout()
{
    setOpaque (false);
}

void FizzReadout::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    g.setColour (theme::surfaceElevated);
    g.fillRoundedRectangle (area, 12.0f);
    g.setColour (theme::outlineVariant);
    g.drawRoundedRectangle (area, 12.0f, 1.0f);

    // Top-edge sweep
    juce::ColourGradient sweep (
        juce::Colours::transparentBlack, area.getX(),     area.getY(),
        juce::Colours::transparentBlack, area.getRight(), area.getY(), false);
    sweep.addColour (0.5, theme::toxic);
    g.setGradientFill (sweep);
    g.fillRect (juce::Rectangle<float> (area.getX(), area.getY(), area.getWidth(), 1.0f));

    auto inner = getLocalBounds().reduced (24);

    auto labelArea = inner.removeFromTop (20);
    g.setColour (theme::mutedForeground);
    g.setFont   (theme::Fonts::uiChrome());
    g.drawText  ("FIZZ AMOUNT", labelArea, juce::Justification::centredLeft, false);

    // Hero number with glow. Scaled to fit the available height while
    // honoring the canonical 80 px reference at default size.
    const float scale = static_cast<float> (inner.getHeight()) / 124.0f;
    auto heroFont = theme::Fonts::heroNumScaled (scale).withExtraKerningFactor (-0.05f);
    g.setFont (heroFont);

    const auto numText = juce::String (displayValue, 1);
    auto numArea = inner.toFloat().translated (0, 4.0f);
    theme::SpectreLookAndFeel::drawGlowText (g, numText, numArea,
                                             juce::Justification::centred,
                                             theme::toxic, theme::toxic, 12.0f);

    // Smaller % suffix to the right.
    auto suffixFont = theme::Fonts::displayTitle().withHeight (juce::jmax (24.0f, heroFont.getHeight() * 0.40f));
    g.setFont   (suffixFont);
    g.setColour (theme::toxic);

    juce::GlyphArrangement ga;
    ga.addLineOfText (heroFont, numText, 0.0f, 0.0f);
    const float numW = ga.getBoundingBox (0, -1, true).getWidth();

    const float suffixX = inner.getCentreX() + numW * 0.5f + 4.0f;
    g.drawText ("%",
                juce::Rectangle<float> (suffixX, numArea.getY() + numArea.getHeight() * 0.18f,
                                        suffixFont.getHeight() * 0.85f, suffixFont.getHeight()),
                juce::Justification::centredLeft, false);
}
} // namespace cabrot::ui
