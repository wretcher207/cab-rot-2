#include "PluginEditor.h"

namespace cabrot
{
CabRotEditor::CabRotEditor (CabRotProcessor& p)
    : juce::AudioProcessorEditor (&p), processorRef (p)
{
    setResizable (true, true);
    setResizeLimits (1000, 650, 1600, 1040);
    getConstrainer()->setFixedAspectRatio (1200.0 / 780.0);
    setSize (1200, 780);

    brandLabel.setText ("CAB ROT", juce::dontSendNotification);
    brandLabel.setJustificationType (juce::Justification::centred);
    brandLabel.setFont (juce::Font (juce::FontOptions (96.0f, juce::Font::bold)));
    brandLabel.setColour (juce::Label::textColourId, juce::Colour::fromString ("FF40FF2F"));
    addAndMakeVisible (brandLabel);
}

void CabRotEditor::paint (juce::Graphics& g)
{
    // Phase 0 placeholder background. Real palette ports in Phase 1.
    g.fillAll (juce::Colour::fromString ("FF071005"));
}

void CabRotEditor::resized()
{
    brandLabel.setBounds (getLocalBounds());
}
} // namespace cabrot
