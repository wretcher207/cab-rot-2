#include "PluginEditor.h"

#include "Theme/Palette.h"
#include "Theme/SpectreLookAndFeel.h"

namespace cabrot
{
namespace
{
// Locked decision #6: continuous resize, aspect-locked. Min 1000x650,
// default 1200x780, max 1600x1040. All three sizes share the same ratio.
constexpr int    kDefaultWidth  = 1200;
constexpr int    kDefaultHeight = 780;
constexpr double kAspectRatio   = static_cast<double> (kDefaultWidth)
                                / static_cast<double> (kDefaultHeight);

// Canonical band sizes at default 1200x780. They scale linearly with editor
// height so the layout breathes at min (1000x650) and max (1600x1040)
// without the AMP PROFILE card being squeezed below useful height.
constexpr int kHeaderHeight = 64;
constexpr int kKnobsHeight  = 192;
constexpr int kFooterHeight = 48;
constexpr int kMainPad      = 24;
constexpr int kRightColumnW = 320;
constexpr int kFizzCardH    = 192;
constexpr int kCardGap      = 24;

int scaleH (int reference, float scale, int floor)
{
    return juce::jmax (floor, juce::roundToInt (static_cast<float> (reference) * scale));
}
}

CabRotEditor::CabRotEditor (CabRotProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processorRef (p),
      lookAndFeel (std::make_unique<theme::SpectreLookAndFeel>())
{
    setLookAndFeel (lookAndFeel.get());

    addAndMakeVisible (headerBar);
    addAndMakeVisible (waspMeter);
    addAndMakeVisible (fizzReadout);
    addAndMakeVisible (ampProfile);
    addAndMakeVisible (knobRow);
    addAndMakeVisible (footerBar);

    setResizable (true, true);
    setResizeLimits (1000, 650, 1600, 1040);
    getConstrainer()->setFixedAspectRatio (kAspectRatio);
    setSize (kDefaultWidth, kDefaultHeight);
}

CabRotEditor::~CabRotEditor()
{
    setLookAndFeel (nullptr);
}

void CabRotEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds();

    // Outer surface
    g.setColour (theme::surfaceContainerLowest);
    g.fillRect (bounds);

    // Soft radial vignette - centre warm, edges deep
    juce::ColourGradient grad (
        theme::surfaceContainerLow,
        static_cast<float> (bounds.getCentreX()),
        static_cast<float> (bounds.getCentreY()),
        theme::surfaceContainerLowest,
        0.0f, static_cast<float> (bounds.getHeight()),
        true);
    g.setGradientFill (grad);
    g.fillRect (bounds);

    g.setColour (theme::outlineVariant.withAlpha (0.30f));
    g.drawRect (bounds.reduced (1), 1);

    // Global scanline overlay rests on top of everything else; runs after
    // child components paint, see paintOverChildren.
}

void CabRotEditor::resized()
{
    auto bounds = getLocalBounds();

    const float scale = static_cast<float> (bounds.getHeight()) / static_cast<float> (kDefaultHeight);
    const int   header = scaleH (kHeaderHeight, scale, 52);
    const int   footer = scaleH (kFooterHeight, scale, 40);
    const int   knobs  = scaleH (kKnobsHeight,  scale, 152);
    const int   pad    = scaleH (kMainPad,      scale, 16);
    const int   gap    = scaleH (kCardGap,      scale, 12);
    const int   fizzH  = scaleH (kFizzCardH,    scale, 152);
    const int   rightW = scaleH (kRightColumnW, scale, 260);

    headerBar.setBounds (bounds.removeFromTop    (header));
    footerBar.setBounds (bounds.removeFromBottom (footer));
    knobRow .setBounds  (bounds.removeFromBottom (knobs));

    auto main = bounds.reduced (pad);

    auto rightColumn = main.removeFromRight (rightW);
    main.removeFromRight (gap);

    waspMeter.setBounds (main);

    fizzReadout.setBounds (rightColumn.removeFromTop (fizzH));
    rightColumn.removeFromTop (gap);
    ampProfile.setBounds (rightColumn);
}
} // namespace cabrot
