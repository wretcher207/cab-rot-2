#include "FooterBar.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
namespace
{
const juce::String kVersionText { "V0.1.0" };
}

FooterBar::FooterBar()
{
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);
    inMeter.setVisible (false);
    outMeter.setVisible (false);

    aButton.setRadioGroupId (2, juce::dontSendNotification);
    bButton.setRadioGroupId (2, juce::dontSendNotification);
    aButton.setClickingTogglesState (true);
    bButton.setClickingTogglesState (true);
    aButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    bButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    addAndMakeVisible (aButton);
    addAndMakeVisible (bButton);
    setABAvailable (false);

    // Locked decision: Off / 2x / 4x. Default Off.
    oversample.addItem ("OFF", 1);
    oversample.addItem ("2X",  2);
    oversample.addItem ("4X",  3);
    oversample.setSelectedId (1, juce::dontSendNotification);
    oversample.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (oversample);
    oversample.setVisible (false);
}

void FooterBar::setABAvailable (bool available)
{
    aButton.setVisible (available);
    bButton.setVisible (available);
}

void FooterBar::setStatusText (juce::String text)
{
    text = text.trim().toUpperCase();
    if (statusText == text)
        return;

    statusText = std::move (text);
    resized();
    repaint();
}

juce::String FooterBar::formattedStatusText() const
{
    return statusText.isEmpty() ? kVersionText
                                : kVersionText + " / " + statusText;
}

int FooterBar::statusTextWidth() const
{
    juce::GlyphArrangement va;
    va.addLineOfText (theme::Fonts::monoLabel (10.0f), formattedStatusText(), 0.0f, 0.0f);
    return juce::roundToInt (va.getBoundingBox (0, -1, true).getWidth() + 8.0f);
}

void FooterBar::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    g.setColour (theme::rule);
    g.fillRect (area.removeFromTop (1));
    g.setColour (theme::canvas);
    g.fillRect (area);

    auto inner = area.reduced (24, 0);

    const int versionW = statusTextWidth();
    auto versionArea = inner.removeFromRight (versionW);

    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  (formattedStatusText(), versionArea,
                 juce::Justification::centredRight, false);
}

void FooterBar::resized()
{
    const auto inner = getLocalBounds().reduced (24, 0);

    const int meterY = inner.getCentreY() - 8;
    const int meterH = 16;
    const int meterW = 96;

    int cursor = inner.getX();
    inMeter.setBounds  (cursor, meterY, meterW, meterH);
    cursor += meterW + 16;
    outMeter.setBounds (cursor, meterY, meterW + 12, meterH);

    // A/B toggle, 56 px wide, centered on the full inner width.
    const int abW = 56;
    const int abH = 22;
    const int abY = inner.getCentreY() - abH / 2;
    const int abX = inner.getX() + (inner.getWidth() - abW) / 2;
    aButton.setBounds (abX, abY, abW / 2, abH);
    bButton.setBounds (abX + abW / 2, abY, abW / 2, abH);

    oversample.setBounds ({ });
}
} // namespace cabrot::ui
