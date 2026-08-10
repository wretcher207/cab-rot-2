#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::theme
{
// SpectreLookAndFeel renders the Cab Rot UI atoms per
// design/CANONICAL-UI.md and the Dead Pixel Design brand kit. It is the
// only place that owns control styling. UI components should never
// construct juce::Colour or juce::Font directly; both come from
// cabrot::theme:: helpers.
//
// Geometry is flat and square: zero border radius, 1px hairlines at
// `rule`, no shadows, no emissive effects anywhere.

class SpectreLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SpectreLookAndFeel();
    ~SpectreLookAndFeel() override = default;

    // Knob: hairline track arc, bright value arc, 1px indicator line,
    // no filled body.
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    // Buttons: square outline only, no fill in either state. Selected is
    // inkPrimary, unselected is rule / inkMeta.
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    // ComboBox (oversampling): transparent ground, hairline border.
    void drawComboBox (juce::Graphics&, int width, int height,
                       bool isButtonDown, int buttonX, int buttonY,
                       int buttonW, int buttonH, juce::ComboBox&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;

    // Linear bar, square corners. Kept for any ProgressBar consumer.
    void drawProgressBar (juce::Graphics&, juce::ProgressBar&,
                          int width, int height,
                          double progress,
                          const juce::String& textToShow) override;

    // Scanlines are permitted inside the spectral display frame only, at
    // 6 percent opacity or less.
    static void drawScanlines (juce::Graphics&, juce::Rectangle<int> area,
                               float opacity = 0.05f, int spacing = 4);

private:
    void seedDefaultColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectreLookAndFeel)
};
} // namespace cabrot::theme
