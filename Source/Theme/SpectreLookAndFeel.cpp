#include "SpectreLookAndFeel.h"
#include "Palette.h"
#include "Fonts.h"

namespace cabrot::theme
{
SpectreLookAndFeel::SpectreLookAndFeel()
{
    seedDefaultColours();
    setDefaultSansSerifTypeface (Fonts::inter());
}

void SpectreLookAndFeel::seedDefaultColours()
{
    // Slider (rotary knob): the surface is drawn flat elsewhere, so only
    // the track and value colours matter here.
    setColour (juce::Slider::backgroundColourId,          surface2);
    setColour (juce::Slider::thumbColourId,               inkPrimary);
    setColour (juce::Slider::trackColourId,               rule);
    setColour (juce::Slider::rotarySliderFillColourId,    inkPrimary);
    setColour (juce::Slider::rotarySliderOutlineColourId, rule);
    setColour (juce::Slider::textBoxTextColourId,         inkBody);
    setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);

    // Buttons: no fill in either state; outline + text carry the state.
    setColour (juce::TextButton::buttonColourId,   juce::Colours::transparentBlack);
    setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    setColour (juce::TextButton::textColourOffId,  inkMeta);
    setColour (juce::TextButton::textColourOnId,   inkPrimary);

    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,       inkBody);
    setColour (juce::Label::outlineColourId,    juce::Colours::transparentBlack);

    // ComboBox (oversampling): transparent ground, hairline outline.
    setColour (juce::ComboBox::backgroundColourId,     juce::Colours::transparentBlack);
    setColour (juce::ComboBox::textColourId,           inkBody);
    setColour (juce::ComboBox::outlineColourId,        rule);
    setColour (juce::ComboBox::buttonColourId,         juce::Colours::transparentBlack);
    setColour (juce::ComboBox::arrowColourId,          inkMeta);
    setColour (juce::ComboBox::focusedOutlineColourId, inkPrimary.withAlpha (0.4f));

    setColour (juce::PopupMenu::backgroundColourId,          surface2);
    setColour (juce::PopupMenu::textColourId,                inkBody);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, surface3);
    setColour (juce::PopupMenu::highlightedTextColourId,     inkPrimary);

    setColour (juce::ProgressBar::backgroundColourId, surface3);
    setColour (juce::ProgressBar::foregroundColourId, inkPrimary);

    setColour (juce::TooltipWindow::backgroundColourId, surface2);
    setColour (juce::TooltipWindow::textColourId,       inkBody);
    setColour (juce::TooltipWindow::outlineColourId,    rule);
}

void SpectreLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float sliderPos,
                                           float rotaryStartAngle,
                                           float rotaryEndAngle,
                                           juce::Slider& slider)
{
    // Flat instrument control: a 1px track arc in `rule`, a value arc in
    // inkPrimary over it, and a 1px indicator line at the thumb. No filled
    // body, no gradient, no bevel. The empty state is a complete track with
    // a disabled indicator.

    const auto bounds  = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto centre  = bounds.getCentre();
    const auto outerR  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto trackR  = outerR * 0.92f;
    const auto thumbA  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool enabled = slider.isEnabled();

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, trackR, trackR,
                         0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (rule);
    g.strokePath (track, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved));

    if (sliderPos > 0.0f && enabled)
    {
        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, trackR, trackR,
                                0.0f, rotaryStartAngle, thumbA, true);
        g.setColour (inkPrimary);
        g.strokePath (valueArc, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));
    }

    const float innerR = outerR * 0.52f;
    juce::Path indicator;
    indicator.startNewSubPath (0.0f, -trackR + 2.0f);
    indicator.lineTo (0.0f, -innerR);
    g.setColour (enabled ? inkPrimary : inkDisabled);
    g.strokePath (indicator,
                  juce::PathStrokeType (1.0f),
                  juce::AffineTransform::rotation (thumbA).translated (centre));
}

void SpectreLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour&,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    const auto area = juce::Rectangle<float> (0.5f, 0.5f,
                                              static_cast<float> (button.getWidth()) - 1.0f,
                                              static_cast<float> (button.getHeight()) - 1.0f);
    const bool on      = button.getToggleState();
    const bool hovered = shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown;

    g.setColour (on ? inkPrimary
                    : (hovered ? inkBody : rule));
    g.drawRect (area, 1.0f);
}

juce::Font SpectreLookAndFeel::getTextButtonFont (juce::TextButton&, int height)
{
    return Fonts::body (juce::jlimit (11.0f, 15.0f, static_cast<float> (height) * 0.5f), true);
}

void SpectreLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
                                       bool, int buttonX, int buttonY, int buttonW, int buttonH,
                                       juce::ComboBox& box)
{
    const auto area = juce::Rectangle<float> (0.5f, 0.5f,
                                              static_cast<float> (width) - 1.0f,
                                              static_cast<float> (height) - 1.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRect (area, 1.0f);

    // Chevron, drawn as two straight strokes so it stays square-geometry.
    const auto cx = static_cast<float> (buttonX + buttonW / 2);
    const auto cy = static_cast<float> (buttonY + buttonH / 2);
    const float half = 3.0f;
    juce::Path chevron;
    chevron.startNewSubPath (cx - half, cy - half * 0.6f);
    chevron.lineTo (cx, cy + half * 0.6f);
    chevron.lineTo (cx + half, cy - half * 0.6f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.strokePath (chevron, juce::PathStrokeType (1.0f));
}

juce::Font SpectreLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return Fonts::mono (13.0f, 0.05f);
}

void SpectreLookAndFeel::drawProgressBar (juce::Graphics& g, juce::ProgressBar& bar,
                                          int width, int height,
                                          double progress,
                                          const juce::String&)
{
    const auto area = juce::Rectangle<float> (0.5f, 0.5f,
                                              static_cast<float> (width) - 1.0f,
                                              static_cast<float> (height) - 1.0f);
    g.setColour (bar.findColour (juce::ProgressBar::backgroundColourId));
    g.fillRect (area);

    const float p = static_cast<float> (juce::jlimit (0.0, 1.0, progress));
    if (p > 0.0f)
    {
        g.setColour (bar.findColour (juce::ProgressBar::foregroundColourId));
        g.fillRect (area.withWidth (area.getWidth() * p));
    }
    g.setColour (rule);
    g.drawRect (area, 1.0f);
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
} // namespace cabrot::theme
