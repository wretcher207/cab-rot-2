#include "SpectreLookAndFeel.h"
#include "Palette.h"
#include "Fonts.h"

namespace cabrot::theme
{
SpectreLookAndFeel::SpectreLookAndFeel()
{
    seedDefaultColours();
    setDefaultSansSerifTypeface (Fonts::displayRegular());
}

void SpectreLookAndFeel::seedDefaultColours()
{
    using LF = juce::LookAndFeel_V4;

    // Slider/Knob defaults
    setColour (juce::Slider::backgroundColourId,        surfaceBright);
    setColour (juce::Slider::thumbColourId,             toxic);
    setColour (juce::Slider::trackColourId,             surfaceContainerHighest);
    setColour (juce::Slider::rotarySliderFillColourId,  toxic);
    setColour (juce::Slider::rotarySliderOutlineColourId, outlineVariant);
    setColour (juce::Slider::textBoxTextColourId,       toxic);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);

    // Button defaults (inactive state colours; the active state is computed
    // in drawButtonBackground based on getToggleState).
    setColour (juce::TextButton::buttonColourId,    surfaceContainerHigh);
    setColour (juce::TextButton::buttonOnColourId,  toxic);
    setColour (juce::TextButton::textColourOffId,   mutedForeground);
    setColour (juce::TextButton::textColourOnId,    onPrimaryContainer);

    // Label defaults
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,       onSurface);
    setColour (juce::Label::outlineColourId,    juce::Colours::transparentBlack);

    // ComboBox
    setColour (juce::ComboBox::backgroundColourId,       juce::Colours::transparentBlack);
    setColour (juce::ComboBox::textColourId,             toxic);
    setColour (juce::ComboBox::outlineColourId,          juce::Colours::transparentBlack);
    setColour (juce::ComboBox::buttonColourId,           juce::Colours::transparentBlack);
    setColour (juce::ComboBox::arrowColourId,            mutedForeground);
    setColour (juce::ComboBox::focusedOutlineColourId,   toxic.withAlpha (0.4f));

    setColour (juce::PopupMenu::backgroundColourId,         surfaceContainer);
    setColour (juce::PopupMenu::textColourId,               mutedForeground);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, toxic.withAlpha (0.20f));
    setColour (juce::PopupMenu::highlightedTextColourId,    toxic);

    // Progress bar (used as meter)
    setColour (juce::ProgressBar::backgroundColourId, surfaceContainerHighest);
    setColour (juce::ProgressBar::foregroundColourId, toxic);

    // Tooltip
    setColour (juce::TooltipWindow::backgroundColourId, surfaceContainer);
    setColour (juce::TooltipWindow::textColourId,       toxic);
    setColour (juce::TooltipWindow::outlineColourId,    outlineVariant);

    juce::ignoreUnused (typeid (LF));
}

void SpectreLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float sliderPos,
                                           float rotaryStartAngle,
                                           float rotaryEndAngle,
                                           juce::Slider& slider)
{
    const auto bounds  = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto centre  = bounds.getCentre();
    const auto outerR  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto innerR  = outerR * 0.75f;     // 48/64 of the outer
    const auto thumbA  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool active  = sliderPos > 0.0f && slider.isEnabled();

    // 1. Outer container: surface-bright fill, outline-variant ring.
    g.setColour (surfaceBright);
    g.fillEllipse (bounds.reduced (1.0f));
    g.setColour (outlineVariant);
    g.drawEllipse (bounds.reduced (1.0f), 2.0f);

    // 2. Conic ring (toxic 50%, fills clockwise from start angle to thumb).
    if (active)
    {
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y,
                           outerR - 4.0f, outerR - 4.0f,
                           0.0f,
                           rotaryStartAngle, thumbA,
                           true);
        g.setColour (toxic.withAlpha (0.55f));
        g.strokePath (arc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        // Soft outer glow ring at low alpha
        juce::Path glow;
        glow.addCentredArc (centre.x, centre.y,
                            outerR - 4.0f, outerR - 4.0f,
                            0.0f, rotaryStartAngle, thumbA, true);
        g.setColour (toxic.withAlpha (0.18f));
        g.strokePath (glow, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // 3. Inner cap.
    const auto innerBounds = juce::Rectangle<float> (innerR * 2.0f, innerR * 2.0f).withCentre (centre);
    g.setColour (surfaceContainerHighest);
    g.fillEllipse (innerBounds);
    g.setColour (surfaceBright);
    g.drawEllipse (innerBounds.reduced (0.5f), 1.0f);

    // 4. Indicator line at thumb angle. Always points outward from the cap
    //    centre toward the rim. Length is half the inner radius.
    const auto needleColour = active ? toxic : mutedForeground;
    juce::Path needle;
    const float needleW = juce::jmax (1.5f, outerR * 0.06f);
    const float needleL = innerR * 0.65f;
    needle.addRoundedRectangle (-needleW * 0.5f, -innerR * 0.95f,
                                 needleW,         needleL,
                                 needleW * 0.4f);
    g.setColour (needleColour);
    g.fillPath (needle, juce::AffineTransform::rotation (thumbA).translated (centre));
    if (active)
    {
        // soft halo
        g.setColour (toxic.withAlpha (0.25f));
        g.fillPath (needle, juce::AffineTransform::scale (1.6f, 1.4f, 0.0f, -innerR * 0.65f)
                                                  .rotated (thumbA)
                                                  .translated (centre));
    }
}

void SpectreLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour&,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    const auto area    = button.getLocalBounds().toFloat().reduced (0.5f);
    const float radius = juce::jmin (8.0f, area.getHeight() * 0.25f);
    const bool on      = button.getToggleState();
    const bool hovered = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;

    juce::Colour fill, edge;
    if (on)
    {
        fill = toxic;
        edge = toxic;
    }
    else
    {
        fill = surfaceContainerHigh;
        edge = hovered ? toxic : outlineVariant;
    }

    g.setColour (fill);
    g.fillRoundedRectangle (area, radius);
    g.setColour (edge);
    g.drawRoundedRectangle (area, radius, on ? 1.2f : 1.0f);

    if (on)
    {
        // outer glow halo
        g.setColour (toxic.withAlpha (0.18f));
        g.drawRoundedRectangle (area.expanded (3.0f), radius + 3.0f, 4.0f);
    }
}

juce::Font SpectreLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    auto f = Fonts::monoData();
    return f.withStyle (juce::Font::bold);
}

void SpectreLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
                                       bool, int, int, int, int, juce::ComboBox& box)
{
    const auto area = juce::Rectangle<float> (0.0f, 0.0f,
                                              static_cast<float> (width),
                                              static_cast<float> (height));
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRect (area);

    auto edge = box.findColour (juce::ComboBox::outlineColourId);
    if (! edge.isTransparent())
    {
        g.setColour (edge);
        g.drawRect (area, 1.0f);
    }
    juce::ignoreUnused (g, width, height, box);
}

juce::Font SpectreLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return Fonts::monoData();
}

void SpectreLookAndFeel::drawProgressBar (juce::Graphics& g, juce::ProgressBar& bar,
                                          int width, int height,
                                          double progress,
                                          const juce::String&)
{
    const auto area    = juce::Rectangle<float> (0.0f, 0.0f,
                                                 static_cast<float> (width),
                                                 static_cast<float> (height));
    const float radius = area.getHeight() * 0.5f;

    g.setColour (bar.findColour (juce::ProgressBar::backgroundColourId));
    g.fillRoundedRectangle (area, radius);

    const float p = static_cast<float> (juce::jlimit (0.0, 1.0, progress));
    if (p > 0.0f)
    {
        const auto fill = area.withWidth (area.getWidth() * p);
        g.setColour (bar.findColour (juce::ProgressBar::foregroundColourId));
        g.fillRoundedRectangle (fill, radius);
    }
}

void SpectreLookAndFeel::drawScanlines (juce::Graphics& g,
                                        juce::Rectangle<int> area,
                                        float opacity, int spacing)
{
    const auto colour = juce::Colour::fromFloatRGBA (0.0f, 0.0f, 0.0f, opacity);
    g.setColour (colour);
    for (int y = area.getY(); y < area.getBottom(); y += spacing)
        g.fillRect (area.getX(), y, area.getWidth(), 1);
}

void SpectreLookAndFeel::drawGridPattern (juce::Graphics& g,
                                          juce::Rectangle<int> area,
                                          float opacity, int spacing)
{
    const auto colour = juce::Colour::fromFloatRGBA (1.0f, 1.0f, 1.0f, 0.05f * opacity * 4.0f);
    g.setColour (colour);
    for (int x = area.getX(); x < area.getRight(); x += spacing)
        g.fillRect (x, area.getY(), 1, area.getHeight());
    for (int y = area.getY(); y < area.getBottom(); y += spacing)
        g.fillRect (area.getX(), y, area.getWidth(), 1);
}

void SpectreLookAndFeel::drawGlowText (juce::Graphics& g, const juce::String& text,
                                       juce::Rectangle<float> area, juce::Justification just,
                                       juce::Colour textColour, juce::Colour glowColour,
                                       float glowRadius)
{
    // Cheap glow: 4 offset draws at low alpha + the crisp top draw. JUCE's
    // DropShadow on text is expensive at 60 Hz; this is good enough for
    // static frames and the on-screen glow we want.
    g.setColour (glowColour.withAlpha (0.30f));
    for (int i = 0; i < 4; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * static_cast<float> (i) / 4.0f;
        const auto offset = area.translated (std::cos (angle) * glowRadius * 0.35f,
                                              std::sin (angle) * glowRadius * 0.35f);
        g.drawText (text, offset, just, false);
    }
    g.setColour (textColour);
    g.drawText (text, area, just, false);
}
} // namespace cabrot::theme
