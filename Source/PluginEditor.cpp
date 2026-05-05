#include "PluginEditor.h"

#include "Theme/Fonts.h"
#include "Theme/Palette.h"
#include "Theme/SpectreLookAndFeel.h"

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
    : juce::AudioProcessorEditor (&p),
      processorRef (p),
      lookAndFeel (std::make_unique<theme::SpectreLookAndFeel>())
{
    setLookAndFeel (lookAndFeel.get());

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
    paintBackground (g, bounds);

    const float scale = static_cast<float> (bounds.getHeight()) / static_cast<float> (kDefaultHeight);
    const float markSize = juce::jlimit (24.0f, 36.0f, 28.0f * scale);
    const float pad      = juce::jlimit (16.0f, 32.0f, 24.0f * scale);
    paintDpdMark (g, juce::Rectangle<float> (pad, pad, markSize, markSize));

    paintWordmark (g, bounds);
    paintFooter   (g, bounds);

    theme::SpectreLookAndFeel::drawScanlines (g, bounds, 0.18f, 4);
}

void CabRotEditor::paintBackground (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.fillAll (theme::surfaceContainerLowest);

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
}

void CabRotEditor::paintDpdMark (juce::Graphics& g, juce::Rectangle<float> area)
{
    const float stroke = juce::jmax (1.0f, area.getWidth() * 0.05f);
    const float offset = area.getWidth() * 0.16f;

    const auto outer = area.reduced (stroke * 0.5f);
    const auto inner = outer.translated (-offset, offset);

    g.setColour (theme::onSurface.withAlpha (0.85f));
    g.drawRect (outer, stroke);
    g.setColour (theme::onSurface.withAlpha (0.55f));
    g.drawRect (inner, stroke * 0.75f);

    const float pixSize = area.getWidth() * 0.16f;
    auto pix = juce::Rectangle<float> (
        inner.getRight() - pixSize - stroke,
        inner.getY() + pixSize * 0.55f,
        pixSize, pixSize);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.fillRect (pix.expanded (pixSize * 0.4f));
    g.setColour (juce::Colours::white.withAlpha (0.92f));
    g.fillRect (pix);
}

void CabRotEditor::paintWordmark (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const float scale = static_cast<float> (bounds.getHeight()) / static_cast<float> (kDefaultHeight);

    auto wordmarkFont = theme::Fonts::heroNumScaled (scale * 1.4f).withExtraKerningFactor (-0.04f);
    g.setFont (wordmarkFont);

    auto wordmarkArea = bounds.toFloat();
    theme::SpectreLookAndFeel::drawGlowText (g, "CAB ROT",
                                             wordmarkArea, juce::Justification::centred,
                                             theme::toxic, theme::toxic, 14.0f);

    auto monoFont = theme::Fonts::uiChromeScaled (scale).withExtraKerningFactor (0.30f);
    g.setFont (monoFont);

    auto tagArea = bounds;
    tagArea.removeFromTop ((bounds.getHeight() / 2) + static_cast<int> (wordmarkFont.getHeight() * 0.6f));
    tagArea.setHeight (static_cast<int> (monoFont.getHeight() * 2.5f));

    g.setColour (theme::primaryFixedDim.withAlpha (0.65f));
    g.drawText ("KILL THE WASP NEST  /  KEEP THE TEETH",
                tagArea, juce::Justification::centred, false);
}

void CabRotEditor::paintFooter (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const float scale = static_cast<float> (bounds.getHeight()) / static_cast<float> (kDefaultHeight);

    auto chromeFont = theme::Fonts::uiChromeScaled (scale).withExtraKerningFactor (0.30f);
    g.setFont (chromeFont);

    const auto pad    = juce::jlimit (16.0f, 32.0f, 24.0f * scale);
    auto       footer = bounds.removeFromBottom (static_cast<int> (chromeFont.getHeight() * 2.4f))
                              .reduced (static_cast<int> (pad), 0);

    g.setColour (theme::mutedForeground.withAlpha (0.85f));
    g.drawText ("DEAD PIXEL HARMONIX",
                footer, juce::Justification::centredLeft, false);

    g.setColour (theme::primaryFixedDim.withAlpha (0.55f));
    g.drawText ("V0.1.0  /  SCANNING FOR HARSHNESS",
                footer, juce::Justification::centredRight, false);
}

void CabRotEditor::resized()
{
    // Phase 0: paint() handles everything. Real components arrive in Phase 2.
}
} // namespace cabrot
