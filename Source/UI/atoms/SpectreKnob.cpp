#include "SpectreKnob.h"
#include "../../Theme/Fonts.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
SpectreKnob::SpectreKnob (juce::String labelText, juce::String unitsSuffix)
    : label (std::move (labelText)), suffix (std::move (unitsSuffix))
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    slider.setRotaryParameters (juce::degreesToRadians (-135.0f),
                                juce::degreesToRadians ( 135.0f),
                                true);
    slider.setRange (0.0, 100.0, 0.01);
    slider.setDoubleClickReturnValue (true, 50.0);
    slider.setPopupMenuEnabled (true);
    slider.setScrollWheelEnabled (true);
    slider.setVelocityModeParameters (1.0, 1, 0.0, false);
    slider.setMouseDragSensitivity (160);
    // Tooltip auto-formats current value via param.getText() once the
    // SliderAttachment is in place; until then the slider shows its raw
    // numeric value.
    slider.setTooltip (label);

    addAndMakeVisible (slider);
    setValueDisplay (slider.getValue());

    slider.onValueChange = [this]
    {
        setValueDisplay (slider.getValue());
    };
}

SpectreKnob::~SpectreKnob() = default;

void SpectreKnob::setValueDisplay (double value, int decimals)
{
    valueDisplay = juce::String (value, decimals) + suffix;
    repaint();
}

void SpectreKnob::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds();

    auto labelArea = area.withHeight (16);
    auto valueArea = area.withTop (area.getBottom() - 16);

    // Name above in metadata grey, value below in body ink. Both mono.
    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  (label, labelArea, juce::Justification::centred, false);

    g.setColour (theme::inkBody);
    g.setFont   (theme::Fonts::mono (13.0f, 0.05f));
    g.drawText  (valueDisplay, valueArea, juce::Justification::centred, false);
}

void SpectreKnob::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (20);
    area.removeFromBottom (20);
    // Cap the dial at 96 for resized layouts so it stays a control, not a
    // monument, and never below 56 so the indicator stays legible at min
    // size.
    const int side = juce::jlimit (56, 104,
                                   juce::jmin (area.getWidth(), area.getHeight()));
    auto knobArea = juce::Rectangle<int> (side, side).withCentre (area.getCentre());
    slider.setBounds (knobArea);
}
} // namespace cabrot::ui
