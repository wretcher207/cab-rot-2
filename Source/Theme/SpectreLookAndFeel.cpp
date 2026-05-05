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
    // Tactile knob render, layered:
    //   1. base shadow ring  (under-the-knob depth)
    //   2. outer track groove (recessed dark ring carrying the conic indicator)
    //   3. conic indicator + glow halo
    //   4. cap base fill with overhead-lighting gradient
    //   5. cap top highlight + bottom inner shadow (simulates a domed surface)
    //   6. centre detent (subtle hardware-like pivot mark)
    //   7. indicator line: bright core + outer halo + thin specular highlight

    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto centre = bounds.getCentre();
    const auto outerR = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto trackR = outerR * 0.94f;
    const auto innerR = outerR * 0.72f;
    const auto thumbA = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool active = sliderPos > 0.0f && slider.isEnabled();
    const bool hover  = slider.isMouseOverOrDragging (true);

    const auto toxicMain   = hover ? toxicGlow : toxic;
    const auto haloAlpha   = hover ? 0.45f     : 0.30f;  // +50% vs first pass
    const auto coreAlpha   = hover ? 1.00f     : 0.95f;

    // 1. Base shadow ring just below the knob - a single soft dark band that
    //    sells the impression that the knob sits ON the panel, not in it.
    {
        const auto shadowBounds = juce::Rectangle<float> (outerR * 2.0f, outerR * 2.0f)
                                    .withCentre (centre.translated (0.0f, outerR * 0.06f));
        juce::ColourGradient shadowGrad (
            juce::Colours::black.withAlpha (0.45f),
            shadowBounds.getCentreX(), shadowBounds.getCentreY(),
            juce::Colours::transparentBlack,
            shadowBounds.getCentreX(), shadowBounds.getBottom() + 2.0f,
            true);
        g.setGradientFill (shadowGrad);
        g.fillEllipse (shadowBounds.expanded (2.0f));
    }

    // 2. Outer track groove. A recessed dark ring with a hairline rim catches
    //    the conic fill; the rim alone is the same as the old simple outline.
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, trackR, trackR,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (surfaceContainerLowest);
        g.strokePath (track, juce::PathStrokeType (5.5f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
        g.setColour (outlineVariant.withAlpha (0.55f));
        g.strokePath (track, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
    }

    // 3. Conic indicator + soft halo. Painted as three stacked arcs (outer
    //    bloom, mid halo, bright core) so the glow has perceptible depth.
    if (active)
    {
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, trackR, trackR,
                           0.0f, rotaryStartAngle, thumbA, true);

        // outer bloom
        g.setColour (toxicMain.withAlpha (haloAlpha * 0.55f));
        g.strokePath (arc, juce::PathStrokeType (16.0f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        // mid halo
        g.setColour (toxicMain.withAlpha (haloAlpha));
        g.strokePath (arc, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        // bright core
        g.setColour (toxicMain.withAlpha (coreAlpha));
        g.strokePath (arc, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }

    // 4. Cap fill. Overhead lighting via a vertical gradient: brighter at the
    //    top, darker at the bottom. Range extended so the dome reads stronger.
    const auto capBounds = juce::Rectangle<float> (innerR * 2.0f, innerR * 2.0f).withCentre (centre);
    {
        juce::ColourGradient capGrad (
            surfaceBright.brighter (0.20f),             capBounds.getCentreX(), capBounds.getY(),
            surfaceContainerLowest,                     capBounds.getCentreX(), capBounds.getBottom(),
            false);
        capGrad.addColour (0.40, surfaceContainerHighest);
        capGrad.addColour (0.85, surfaceContainerLow);
        g.setGradientFill (capGrad);
        g.fillEllipse (capBounds);
    }

    // 5a. Top highlight - off-white arc on the upper edge. Longer + stronger
    //     reads as a more reflective surface (Throat-Wire territory).
    {
        juce::Path topHi;
        topHi.addCentredArc (centre.x, centre.y,
                             innerR - 1.2f, innerR - 1.2f, 0.0f,
                             juce::degreesToRadians (-78.0f),
                             juce::degreesToRadians ( 78.0f), true);
        g.setColour (onSurface.withAlpha (0.30f));
        g.strokePath (topHi, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));

        // Plus a brighter spec arc tighter to top centre for the wet shine.
        juce::Path specArc;
        specArc.addCentredArc (centre.x, centre.y,
                               innerR - 2.5f, innerR - 2.5f, 0.0f,
                               juce::degreesToRadians (-30.0f),
                               juce::degreesToRadians ( 30.0f), true);
        g.setColour (juce::Colours::white.withAlpha (0.22f));
        g.strokePath (specArc, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                                            juce::PathStrokeType::rounded));
    }

    // 5b. Bottom inner shadow - a deeper line on the lower edge of the cap so
    //     the dome doesn't look pasted on.
    {
        juce::Path bottomShade;
        bottomShade.addCentredArc (centre.x, centre.y,
                                   innerR - 1.2f, innerR - 1.2f, 0.0f,
                                   juce::degreesToRadians (135.0f),
                                   juce::degreesToRadians (225.0f), true);
        g.setColour (juce::Colours::black.withAlpha (0.40f));
        g.strokePath (bottomShade, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                                                juce::PathStrokeType::rounded));
    }

    // 5c. Cap rim outline (1 px, just inside the highlight) for crisp edge.
    g.setColour (surfaceContainerLowest);
    g.drawEllipse (capBounds.reduced (0.5f), 1.0f);

    // 6. Centre detent - tiny darker pit at the rotation pivot. Sells the
    //    "real machined hardware" feeling without dominating the glyph.
    {
        const float dotR = innerR * 0.05f;
        const auto dotBounds = juce::Rectangle<float> (dotR * 2.0f, dotR * 2.0f).withCentre (centre);
        g.setColour (surfaceContainerLowest);
        g.fillEllipse (dotBounds);
    }

    // 7. Indicator line: build at 12 o'clock and rotate to thumbA.
    {
        const float needleW = juce::jmax (2.0f, outerR * 0.07f);
        const float needleL = innerR * 0.55f;
        const float needleOuter = innerR * 0.95f;

        const auto rotate = juce::AffineTransform::rotation (thumbA).translated (centre);

        if (active)
        {
            // Outer halo
            juce::Path halo;
            halo.addRoundedRectangle (-needleW,           -needleOuter - needleW * 0.4f,
                                       needleW * 2.0f,    needleL + needleW * 0.8f,
                                       needleW);
            g.setColour (toxicMain.withAlpha (0.32f * (hover ? 1.2f : 1.0f)));
            g.fillPath (halo, rotate);
        }

        // Bright core
        juce::Path needle;
        needle.addRoundedRectangle (-needleW * 0.5f, -needleOuter,
                                     needleW,         needleL,
                                     needleW * 0.45f);
        g.setColour (active ? toxicMain : mutedForeground);
        g.fillPath (needle, rotate);

        // Specular highlight - a slightly brighter sliver running along one
        // side of the needle so it reads as 3D rather than a printed mark.
        if (active)
        {
            juce::Path spec;
            spec.addRoundedRectangle (-needleW * 0.5f + needleW * 0.15f, -needleOuter + needleW * 0.5f,
                                       needleW * 0.18f,                    needleL - needleW * 1.0f,
                                       needleW * 0.18f);
            g.setColour (juce::Colours::white.withAlpha (0.45f));
            g.fillPath (spec, rotate);
        }
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
