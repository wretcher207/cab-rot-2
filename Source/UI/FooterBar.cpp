#include "FooterBar.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
FooterBar::FooterBar()
{
    cryptButton.setLookAndFeel (nullptr); // use default LAF behaviour for now
    cryptButton.setConnectedEdges (0);
    cryptButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    cryptButton.setColour (juce::TextButton::textColourOffId, theme::mutedForeground);
    addAndMakeVisible (cryptButton);

    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    // PHASE 2 PLACEHOLDER: meter levels echo Stitch's static values.
    inMeter.setLevel  (0.70f);
    outMeter.setLevel (0.85f);

    aButton.setRadioGroupId (2, juce::dontSendNotification);
    bButton.setRadioGroupId (2, juce::dontSendNotification);
    aButton.setClickingTogglesState (true);
    bButton.setClickingTogglesState (true);
    aButton.setToggleState (true, juce::dontSendNotification); // PHASE 2 PLACEHOLDER
    aButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    bButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    addAndMakeVisible (aButton);
    addAndMakeVisible (bButton);

    // Locked decision #2: Off / 2x / 4x. Default Off.
    oversample.addItem ("OFF", 1);
    oversample.addItem ("2X",  2);
    oversample.addItem ("4X",  3);
    oversample.setSelectedId (1, juce::dontSendNotification); // PHASE 2 PLACEHOLDER: APVTS attachment in Phase 3
    oversample.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (oversample);
}

void FooterBar::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    g.setColour (theme::outlineVariant);
    g.fillRect (area.removeFromTop (1));
    g.setColour (theme::surfaceContainerLowest);
    g.fillRect (area);

    auto inner = area.reduced (20, 0);

    // Vertical divider after THE CRYPT
    const int cryptEnd = inner.getX() + 96;
    g.setColour (theme::outlineVariant);
    g.fillRect (juce::Rectangle<int> (cryptEnd + 12, inner.getCentreY() - 8, 1, 16));

    // OS: label
    // Version chrome on the far right (measure width so we never clip it).
    juce::GlyphArrangement va;
    const auto versionText = juce::String ("V0.1.0  /  SCANNING FOR HARSHNESS");
    va.addLineOfText (theme::Fonts::uiChrome(), versionText, 0.0f, 0.0f);
    const int versionW = juce::roundToInt (va.getBoundingBox (0, -1, true).getWidth() + 8.0f);
    auto versionArea = inner.removeFromRight (versionW);

    g.setColour (theme::toxic);
    g.setFont   (theme::Fonts::uiChrome());
    g.drawText  (versionText, versionArea, juce::Justification::centredRight, false);

    // 1 px divider between OS combo and version chrome
    g.setColour (theme::outlineVariant);
    g.fillRect (juce::Rectangle<int> (versionArea.getX() - 12,
                                      inner.getCentreY() - 8, 1, 16));
    inner.removeFromRight (24);

    // OS combo box reserves its own slot; resized() positions it. Reserve
    // 64 px on the right side here so the OS: label sits to its left.
    inner.removeFromRight (64);
    inner.removeFromRight (8);
    auto osLabel = inner.removeFromRight (28);
    g.setColour (theme::mutedForeground);
    g.setFont   (theme::Fonts::uiChrome());
    g.drawText  ("OS:", osLabel, juce::Justification::centredRight, false);
}

void FooterBar::resized()
{
    const auto inner = getLocalBounds().reduced (20, 0);

    cryptButton.setBounds (inner.getX(), inner.getY(), 96, inner.getHeight());

    const int meterY  = inner.getCentreY() - 9;
    const int meterH  = 18;
    const int meterW  = 100;

    int cursor = inner.getX() + 96 + 24;
    inMeter.setBounds  (cursor, meterY, meterW, meterH);
    cursor += meterW + 12;
    outMeter.setBounds (cursor, meterY, meterW + 16, meterH);

    // A/B toggle, 56 px wide centered
    const int abW = 56;
    const int abH = 22;
    const int abY = inner.getCentreY() - abH / 2;
    const int abX = inner.getX() + (inner.getWidth() - abW) / 2;
    aButton.setBounds (abX, abY, abW / 2, abH);
    bButton.setBounds (abX + abW / 2, abY, abW / 2, abH);

    // Oversample combo: between the OS: label and the version chrome divider.
    juce::GlyphArrangement va;
    va.addLineOfText (theme::Fonts::uiChrome(), "V0.1.0  /  SCANNING FOR HARSHNESS", 0.0f, 0.0f);
    const int versionW = juce::roundToInt (va.getBoundingBox (0, -1, true).getWidth() + 8.0f);

    const int comboW = 64;
    const int comboH = 22;
    const int comboY = inner.getCentreY() - comboH / 2;
    // versionArea right edge -> versionW left of inner.getRight()
    // divider gap 24, then the combo.
    const int comboRight = inner.getRight() - versionW - 24;
    oversample.setBounds (comboRight - comboW, comboY, comboW, comboH);
}
} // namespace cabrot::ui
