#include "PluginEditor.h"

namespace cabrot
{
namespace
{
// Locked decision #6: continuous resize, aspect-locked. Min 1000x650,
// default 1200x780, max 1600x1040. All three sizes share the same ratio
// so this constant is the single source of truth - if the default ever
// shifts, edit it here, not in setSize.
constexpr int    kDefaultWidth  = 1200;
constexpr int    kDefaultHeight = 780;
constexpr double kAspectRatio   = static_cast<double> (kDefaultWidth)
                                / static_cast<double> (kDefaultHeight);
}

CabRotEditor::CabRotEditor (CabRotProcessor& p)
    : juce::AudioProcessorEditor (&p), processorRef (p)
{
    setResizable (true, true);
    setResizeLimits (1000, 650, 1600, 1040);
    getConstrainer()->setFixedAspectRatio (kAspectRatio);
    setSize (kDefaultWidth, kDefaultHeight);

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
