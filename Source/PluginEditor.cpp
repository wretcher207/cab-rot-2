#include "PluginEditor.h"
#include "BinaryData.h"

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

// Phase 0 palette, hand-eyeballed off the Stitch tokens. Phase 1 ports the
// full OKLCH set into Source/Theme/Palette.h. Until then, four constants do.
const juce::Colour kBackground    = juce::Colour::fromString ("FF050B03"); // deeper than surface-container-lowest
const juce::Colour kSurface       = juce::Colour::fromString ("FF0B1408");
const juce::Colour kToxic         = juce::Colour::fromString ("FF40FF2F"); // primary-container
const juce::Colour kToxicDim      = juce::Colour::fromString ("FF0FE605"); // primary-fixed-dim
const juce::Colour kMuted         = juce::Colour::fromString ("FF85967D"); // outline
const juce::Colour kScanline      = juce::Colour (0x10000000);             // pure black @ ~6%
const juce::Colour kVignette      = juce::Colour (0x40000000);

juce::Font makeDisplayFont (juce::Typeface::Ptr face, float size)
{
    juce::FontOptions opts (size);
    if (face != nullptr)
        opts = opts.withTypeface (face);
    return juce::Font (opts);
}
}

CabRotEditor::CabRotEditor (CabRotProcessor& p)
    : juce::AudioProcessorEditor (&p), processorRef (p)
{
    setResizable (true, true);
    setResizeLimits (1000, 650, 1600, 1040);
    getConstrainer()->setFixedAspectRatio (kAspectRatio);
    setSize (kDefaultWidth, kDefaultHeight);

    displayTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::SpaceGroteskBold_ttf, BinaryData::SpaceGroteskBold_ttfSize);
    monoTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
}

void CabRotEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds();
    paintBackground (g, bounds);
    paintScanlines  (g, bounds);

    // DPD mark sits in the upper-left, modest scale - the same place the
    // Stitch reference puts it in the header strip.
    const float markSize = juce::jlimit (24.0f, 36.0f, static_cast<float> (bounds.getHeight()) * 0.045f);
    const float pad      = static_cast<float> (bounds.getHeight()) * 0.04f;
    paintDpdMark (g, juce::Rectangle<float> (pad, pad, markSize, markSize));

    paintWordmark (g, bounds);
    paintFooter   (g, bounds);
}

void CabRotEditor::paintBackground (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.fillAll (kBackground);

    // soft radial vignette warming the centre, deepening the corners
    juce::ColourGradient grad (
        kSurface.withAlpha (1.0f),  static_cast<float> (bounds.getCentreX()), static_cast<float> (bounds.getCentreY()),
        kBackground.withAlpha (1.0f), 0.0f, static_cast<float> (bounds.getHeight()),
        true);
    g.setGradientFill (grad);
    g.fillRect (bounds);

    // tight border ring
    g.setColour (kMuted.withAlpha (0.18f));
    g.drawRect (bounds.reduced (1), 1);

    // corner vignette pass to deepen edges
    juce::ColourGradient edge (
        juce::Colours::transparentBlack, static_cast<float> (bounds.getCentreX()), static_cast<float> (bounds.getCentreY()),
        kVignette, 0.0f, 0.0f, true);
    g.setGradientFill (edge);
    g.fillRect (bounds);
}

void CabRotEditor::paintScanlines (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour (kScanline);
    for (int y = bounds.getY(); y < bounds.getBottom(); y += 3)
        g.fillRect (bounds.getX(), y, bounds.getWidth(), 1);
}

void CabRotEditor::paintDpdMark (juce::Graphics& g, juce::Rectangle<float> area)
{
    // Two nested squares, the inner offset by a few pixels to fake a
    // pixel-bleed double-exposure. A single bright "dead pixel" sits inside
    // the inner square's upper-right quadrant.
    const float stroke = juce::jmax (1.0f, area.getWidth() * 0.04f);
    const float offset = area.getWidth() * 0.16f;

    const auto outer = area.reduced (stroke * 0.5f);
    const auto inner = outer.translated (-offset, offset);

    g.setColour (kMuted.withAlpha (0.85f));
    g.drawRect (outer, stroke);
    g.setColour (kMuted.withAlpha (0.55f));
    g.drawRect (inner, stroke * 0.75f);

    // dead pixel: a small filled square in the upper-right of the inner box
    const float pixSize = area.getWidth() * 0.14f;
    const auto pix = juce::Rectangle<float> (
        inner.getRight() - pixSize - stroke,
        inner.getY() + pixSize * 0.6f,
        pixSize, pixSize);
    g.setColour (juce::Colours::white.withAlpha (0.92f));
    g.fillRect (pix);
    // soft bloom around it
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.fillRect (pix.expanded (pixSize * 0.4f));
}

void CabRotEditor::paintWordmark (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const float wordmarkSize = static_cast<float> (bounds.getHeight()) * 0.21f;
    auto wordmarkFont = makeDisplayFont (displayTypeface, wordmarkSize);
    wordmarkFont = wordmarkFont.withExtraKerningFactor (-0.04f); // tracking-tighter

    g.setColour (kToxic);
    g.setFont (wordmarkFont);
    g.drawText ("CAB ROT", bounds, juce::Justification::centred, false);

    // Tagline below the wordmark, JetBrains Mono Regular, dimmer, spaced out.
    const float tagSize = juce::jlimit (10.0f, 14.0f, static_cast<float> (bounds.getHeight()) * 0.018f);
    juce::Font monoFont (juce::FontOptions (tagSize).withTypeface (monoTypeface));
    monoFont = monoFont.withExtraKerningFactor (0.20f);

    auto tagArea = bounds;
    tagArea.removeFromTop ((bounds.getHeight() / 2) + static_cast<int> (wordmarkSize * 0.6f));
    tagArea.setHeight (static_cast<int> (tagSize * 2.5f));

    g.setColour (kToxicDim.withAlpha (0.65f));
    g.setFont (monoFont);
    g.drawText ("KILL THE WASP NEST  /  KEEP THE TEETH",
                tagArea, juce::Justification::centred, false);
}

void CabRotEditor::paintFooter (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    const float chromeSize = juce::jlimit (10.0f, 13.0f, static_cast<float> (bounds.getHeight()) * 0.016f);
    juce::Font chromeFont (juce::FontOptions (chromeSize).withTypeface (monoTypeface));
    chromeFont = chromeFont.withExtraKerningFactor (0.30f);
    g.setFont (chromeFont);

    const auto pad = static_cast<int> (bounds.getHeight() * 0.04f);
    const auto footer = bounds.removeFromBottom (static_cast<int> (chromeSize * 2.4f))
                              .reduced (pad, 0);

    g.setColour (kMuted.withAlpha (0.75f));
    g.drawText ("DEAD PIXEL HARMONIX",
                footer, juce::Justification::centredLeft, false);

    g.setColour (kToxicDim.withAlpha (0.55f));
    g.drawText ("V0.1.0  /  SCANNING FOR HARSHNESS",
                footer, juce::Justification::centredRight, false);
}

void CabRotEditor::resized()
{
    // Phase 0: paint() handles everything. Real components arrive in Phase 2.
}
} // namespace cabrot
