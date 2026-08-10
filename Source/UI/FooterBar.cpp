#include "FooterBar.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
namespace
{
// Version plus real processing state, mono metadata grey. The old
// "SCANNING FOR HARSHNESS" suffix described a costume, not a state.
const juce::String kStatusText { "V0.1.0 / PROCESSING" };

int statusTextWidth()
{
    juce::GlyphArrangement va;
    va.addLineOfText (theme::Fonts::monoLabel (10.0f), kStatusText, 0.0f, 0.0f);
    return juce::roundToInt (va.getBoundingBox (0, -1, true).getWidth() + 8.0f);
}
}

FooterBar::FooterBar()
{
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    // Placeholder levels until Phase 6 wires live metering.
    inMeter.setLevel  (0.70f);
    outMeter.setLevel (0.85f);

    aButton.setRadioGroupId (2, juce::dontSendNotification);
    bButton.setRadioGroupId (2, juce::dontSendNotification);
    aButton.setClickingTogglesState (true);
    bButton.setClickingTogglesState (true);
    addAndMakeVisible (aButton);
    addAndMakeVisible (bButton);

    // Locked decision: Off / 2x / 4x. Default Off.
    oversample.addItem ("OFF", 1);
    oversample.addItem ("2X",  2);
    oversample.addItem ("4X",  3);
    oversample.setSelectedId (1, juce::dontSendNotification);
    oversample.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (oversample);
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
    g.drawText  (kStatusText, versionArea, juce::Justification::centredRight, false);

    // Divider between OS combo and status text.
    g.setColour (theme::rule);
    g.fillRect (juce::Rectangle<int> (versionArea.getX() - 16,
                                      inner.getCentreY() - 8, 1, 16));
    inner.removeFromRight (32);

    // OS combo reserves 68 px; the label sits to its left.
    inner.removeFromRight (68);
    inner.removeFromRight (4);
    auto osLabel = inner.removeFromRight (24);
    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  ("OS:", osLabel, juce::Justification::centredRight, false);
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

    // Oversample combo: between the OS: label and the status divider.
    const int comboW = 68;
    const int comboH = 22;
    const int comboY = inner.getCentreY() - comboH / 2;
    const int comboRight = inner.getRight() - statusTextWidth() - 32;
    oversample.setBounds (comboRight - comboW, comboY, comboW, comboH);
}
} // namespace cabrot::ui
