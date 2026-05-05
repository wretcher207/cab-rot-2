#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::theme
{
// SpectreLookAndFeel renders the Cab Rot UI atoms - knobs, mode buttons,
// meters, combo boxes - per design/CANONICAL-UI.md. It is the only place
// that owns visual styling. UI components should never construct juce::Colour
// or juce::Font directly; both come from cabrot::theme:: helpers.

class SpectreLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SpectreLookAndFeel();
    ~SpectreLookAndFeel() override = default;

    // Knob: see CANONICAL-UI §7.6. Outer ring + inner cap + indicator line.
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    // Mode button: bg toxic when active, surface-container-high otherwise.
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    // ComboBox (Oversampling): transparent bg, toxic active text.
    void drawComboBox (juce::Graphics&, int width, int height,
                       bool isButtonDown, int buttonX, int buttonY,
                       int buttonW, int buttonH, juce::ComboBox&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;

    // Linear progress bar -> repurposed as the IN/OUT meter pill in FooterBar.
    void drawProgressBar (juce::Graphics&, juce::ProgressBar&,
                          int width, int height,
                          double progress,
                          const juce::String& textToShow) override;

    // Painting helpers exposed for direct component use (the test screen and
    // PluginEditor call these without needing to subclass).
    static void drawScanlines    (juce::Graphics&, juce::Rectangle<int> area, float opacity = 0.20f, int spacing = 4);
    static void drawGridPattern  (juce::Graphics&, juce::Rectangle<int> area, float opacity = 0.30f, int spacing = 20);
    static void drawGlowText     (juce::Graphics&, const juce::String& text,
                                  juce::Rectangle<float> area, juce::Justification just,
                                  juce::Colour textColour, juce::Colour glowColour,
                                  float glowRadius = 12.0f);

private:
    void seedDefaultColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectreLookAndFeel)
};
} // namespace cabrot::theme
