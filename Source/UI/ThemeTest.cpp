#include "ThemeTest.h"
#include "../Theme/Palette.h"
#include "../Theme/Fonts.h"
#include "../Theme/SpectreLookAndFeel.h"

namespace cabrot::ui
{
namespace
{
// Section heading rendered in the small uppercase ui-chrome style.
void drawSectionLabel (juce::Graphics& g, juce::Rectangle<int> area, juce::String text,
                       juce::Colour colour = theme::mutedForeground)
{
    g.setColour (colour);
    g.setFont (theme::Fonts::uiChrome());
    g.drawText (text, area, juce::Justification::centredLeft, false);
}

void drawCard (juce::Graphics& g, juce::Rectangle<float> area,
               juce::Colour fill = theme::surfaceContainer,
               juce::Colour edge = theme::outlineVariant,
               float radius = 12.0f)
{
    g.setColour (fill);
    g.fillRoundedRectangle (area, radius);
    g.setColour (edge);
    g.drawRoundedRectangle (area, radius, 1.0f);
}
}

ThemeTest::ThemeTest()
    : lnf (std::make_unique<theme::SpectreLookAndFeel>())
{
    setLookAndFeel (lnf.get());
    setSize (1200, 780);

    knobSamples = {
        { "FIZZ HUNT",     0.62f },
        { "EDGE PRESERVE", 0.45f },
        { "CAB SMOOTH",    0.35f },
        { "DIGITAL SAND",  0.55f },
        { "AIR ROT",       0.40f },
        { "REAP MIX",      0.50f },
    };

    modeSamples = {
        { "5150",      true,  false },
        { "RECTO",     false, false },
        { "HM-2",      false, false },
        { "DJENT",     false, true  }, // hover sample
        { "BLACKENED", false, false },
        { "SLUDGE",    false, false },
    };
}

ThemeTest::~ThemeTest()
{
    setLookAndFeel (nullptr);
}

void ThemeTest::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Outer surface - the surface-container-lowest plate.
    g.setColour (theme::surfaceContainerLowest);
    g.fillRect (bounds);

    // Subtle radial vignette toward the centre to keep the panel from
    // looking flat.
    juce::ColourGradient grad (
        theme::surfaceContainerLow,
        static_cast<float> (bounds.getCentreX()), static_cast<float> (bounds.getCentreY()),
        theme::surfaceContainerLowest,
        0.0f, static_cast<float> (bounds.getHeight()),
        true);
    g.setGradientFill (grad);
    g.fillRect (bounds);

    auto remaining = bounds;
    paintHeader (g, remaining.removeFromTop (64));

    auto footer  = remaining.removeFromBottom (48);
    auto knobBay = remaining.removeFromBottom (192);
    auto main    = remaining; // middle band

    // Main is split: spectral (left, flex 1) + right column (320 wide)
    auto right = main.removeFromRight (320 + 24).reduced (12);
    auto wasp  = main.reduced (24);

    // Wasp meter card
    drawCard (g, wasp.toFloat(), theme::surfaceContainer.withAlpha (0.5f), theme::outlineVariant, 12.0f);
    theme::SpectreLookAndFeel::drawGridPattern (g, wasp.reduced (1), 0.30f, 20);
    paintMeters (g, wasp.reduced (12));

    // Right column: hero readout + amp profile grid
    auto hero = right.removeFromTop (192);
    paintHero (g, hero);
    right.removeFromTop (24);
    paintModes (g, right);

    // Knob bay
    paintKnobs (g, knobBay);
    paintFooter (g, footer);

    // Global scanline overlay (very subtle)
    theme::SpectreLookAndFeel::drawScanlines (g, bounds, 0.18f, 4);
}

void ThemeTest::resized()
{
    // No child components - single paint() does everything.
}

void ThemeTest::paintHeader (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::surfaceContainerLow.withAlpha (0.90f));
    g.fillRect (area);
    g.setColour (theme::outlineVariant);
    g.fillRect (area.removeFromBottom (1));

    auto inner = area.reduced (20, 0);

    // Left cluster: wordmark + divider + DPD mark + by-line
    auto left = inner.removeFromLeft (480);
    auto wm   = left.removeFromLeft (140);
    g.setColour (theme::toxic);
    // The Stitch h1 is `text-display-title font-black` -> 24 px Space Grotesk
    // Black uppercase. The Black weight isn't in workspace yet so we use Bold
    // and bump from 24 to 28 px to compensate visually.
    auto df = theme::Fonts::displayTitleScaled (1.18f).withExtraKerningFactor (-0.04f);
    g.setFont (df);
    g.drawText ("CAB ROT", wm, juce::Justification::centredLeft, false);

    // 1 px divider, 16 px tall
    g.setColour (theme::outlineVariant);
    auto divider = juce::Rectangle<int> (left.getX(), left.getCentreY() - 8, 1, 16);
    g.fillRect (divider);
    left.removeFromLeft (16);

    // DPD mark, 16 x 16, grayscale 70%
    auto markBounds = juce::Rectangle<float> (
        static_cast<float> (left.getX()),
        static_cast<float> (left.getCentreY() - 8),
        16.0f, 16.0f);
    paintDpdMark (g, markBounds);

    left.removeFromLeft (24);
    g.setColour (theme::mutedForeground);
    g.setFont (theme::Fonts::uiChrome());
    g.drawText ("BY DEAD PIXEL DESIGN", left, juce::Justification::centredLeft, false);

    // Right cluster: CPU + LIVE pill + Delta Listen ghost (placeholder dot)
    auto right = inner.removeFromRight (320);
    auto deltaArea = right.removeFromRight (24);
    auto liveArea  = right.removeFromRight (96);
    auto cpuArea   = right;

    g.setFont (theme::Fonts::uiChrome());
    g.setColour (theme::mutedForeground);
    g.drawText ("CPU", cpuArea.removeFromLeft (40), juce::Justification::centredLeft, false);
    g.setFont (theme::Fonts::monoData());
    g.setColour (theme::toxic);
    g.drawText ("4.2%", cpuArea, juce::Justification::centredLeft, false);

    // LIVE pill
    auto pill = liveArea.toFloat().reduced (4.0f);
    g.setColour (theme::surfaceContainer);
    g.fillRoundedRectangle (pill, pill.getHeight() * 0.5f);
    g.setColour (theme::outlineVariant);
    g.drawRoundedRectangle (pill, pill.getHeight() * 0.5f, 1.0f);
    auto dot = juce::Rectangle<float> (
        pill.getX() + 8.0f,
        pill.getCentreY() - 4.0f,
        8.0f, 8.0f);
    g.setColour (theme::toxic.withAlpha (0.4f));
    g.fillEllipse (dot.expanded (4.0f));
    g.setColour (theme::toxic);
    g.fillEllipse (dot);
    g.setFont (theme::Fonts::uiChrome());
    g.drawText ("LIVE",
                juce::Rectangle<int> (juce::roundToInt (pill.getX() + 22.0f),
                                       juce::roundToInt (pill.getY()),
                                       juce::roundToInt (pill.getRight() - pill.getX() - 26.0f),
                                       juce::roundToInt (pill.getHeight())),
                juce::Justification::centredLeft, false);

    // Delta Listen ghost - simple coded glyph for now.
    g.setColour (theme::mutedForeground.withAlpha (0.85f));
    auto ghost = deltaArea.toFloat().reduced (2.0f);
    juce::Path ghostPath;
    ghostPath.startNewSubPath (ghost.getCentreX(), ghost.getY() + 2.0f);
    ghostPath.cubicTo (ghost.getRight(),  ghost.getY() + 2.0f,
                       ghost.getRight(),  ghost.getBottom() - 4.0f,
                       ghost.getRight(),  ghost.getBottom() - 4.0f);
    ghostPath.lineTo (ghost.getRight() - 3.0f, ghost.getBottom());
    ghostPath.lineTo (ghost.getRight() - 6.0f, ghost.getBottom() - 4.0f);
    ghostPath.lineTo (ghost.getRight() - 9.0f, ghost.getBottom());
    ghostPath.lineTo (ghost.getX() + 6.0f, ghost.getBottom() - 4.0f);
    ghostPath.lineTo (ghost.getX() + 3.0f, ghost.getBottom());
    ghostPath.lineTo (ghost.getX(), ghost.getBottom() - 4.0f);
    ghostPath.lineTo (ghost.getX(), ghost.getY() + 2.0f);
    ghostPath.cubicTo (ghost.getX(), ghost.getY() + 2.0f,
                       ghost.getX(), ghost.getY() + 2.0f,
                       ghost.getCentreX(), ghost.getY() + 2.0f);
    ghostPath.closeSubPath();
    g.strokePath (ghostPath, juce::PathStrokeType (1.4f));
}

void ThemeTest::paintDpdMark (juce::Graphics& g, juce::Rectangle<float> area)
{
    const float stroke = juce::jmax (1.0f, area.getWidth() * 0.06f);
    const float offset = area.getWidth() * 0.14f;

    g.setColour (theme::onSurface.withAlpha (0.70f));
    g.drawRect (area.reduced (stroke * 0.5f), stroke * 0.7f);
    g.drawRect (area.reduced (stroke * 0.5f).translated (-offset, offset), stroke * 0.55f);

    // dead pixel
    const float pix = area.getWidth() * 0.18f;
    g.setColour (juce::Colours::white.withAlpha (0.95f));
    g.fillRect (juce::Rectangle<float> (
        area.getRight() - pix - stroke,
        area.getY() + pix * 0.4f,
        pix, pix));
}

void ThemeTest::paintKnobs (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::outlineVariant);
    g.fillRect (area.removeFromTop (1));
    g.setColour (theme::surfaceContainer.withAlpha (0.6f));
    g.fillRect (area);

    theme::SpectreLookAndFeel::drawScanlines (g, area, 0.10f, 4);

    auto inner   = area.reduced (32, 24);
    const int n  = static_cast<int> (knobSamples.size());
    if (n == 0)
        return;

    // Each cell: label (top), knob (middle), value (bottom).
    const int cellW = inner.getWidth() / n;
    const int knobSize = 64;

    for (int i = 0; i < n; ++i)
    {
        auto cell = inner.withX (inner.getX() + i * cellW).withWidth (cellW);
        auto labelArea = cell.removeFromTop (16);
        auto valueArea = cell.removeFromBottom (16);
        auto knobArea  = juce::Rectangle<int> (
            cell.getCentreX() - knobSize / 2,
            cell.getY() + (cell.getHeight() - knobSize) / 2,
            knobSize, knobSize);

        const auto& sample = knobSamples[(size_t) i];
        const bool active = sample.value > 0.0f;

        g.setColour (active ? theme::toxic : theme::mutedForeground);
        g.setFont (theme::Fonts::uiChrome());
        g.drawText (sample.label, labelArea, juce::Justification::centred, false);

        const float startA = juce::degreesToRadians (-135.0f);
        const float endA   = juce::degreesToRadians ( 135.0f);

        // We draw the knob via SpectreLookAndFeel directly. Sliders aren't
        // needed - this is a paint-only screen.
        juce::Slider stub;
        stub.setLookAndFeel (lnf.get());
        lnf->drawRotarySlider (g,
                               knobArea.getX(), knobArea.getY(),
                               knobArea.getWidth(), knobArea.getHeight(),
                               sample.value, startA, endA, stub);
        stub.setLookAndFeel (nullptr);

        g.setColour (active ? theme::toxic : theme::mutedForeground);
        g.setFont (theme::Fonts::monoData());
        g.drawText (juce::String (juce::roundToInt (sample.value * 100.0f)),
                    valueArea, juce::Justification::centred, false);
    }
}

void ThemeTest::paintModes (juce::Graphics& g, juce::Rectangle<int> area)
{
    drawCard (g, area.toFloat(), theme::surfaceContainer, theme::outlineVariant, 12.0f);

    auto inner = area.reduced (20);
    auto headerStrip = inner.removeFromTop (24);
    drawSectionLabel (g, headerStrip, "AMP PROFILE", theme::mutedForeground);

    inner.removeFromTop (12);

    // 2 columns x 3 rows
    const int cols = 2;
    const int rows = 3;
    const int gap  = 12;

    const int cellW = (inner.getWidth()  - gap * (cols - 1)) / cols;
    const int cellH = (inner.getHeight() - gap * (rows - 1)) / rows;

    for (int i = 0; i < (int) modeSamples.size(); ++i)
    {
        const int col = i % cols;
        const int row = i / cols;
        auto cell = juce::Rectangle<int> (
            inner.getX() + col * (cellW + gap),
            inner.getY() + row * (cellH + gap),
            cellW, cellH);

        const auto& sample = modeSamples[(size_t) i];
        // Use the LAF directly so the rendering exactly matches what real
        // mode buttons will get in Phase 2. drawButtonBackground reads
        // button.getLocalBounds() which is always (0,0,w,h) - JUCE's normal
        // paint pipeline translates Graphics before calling, so we do the
        // same here for a paint-only path.
        juce::TextButton stub (sample.label);
        stub.setToggleState (sample.active, juce::dontSendNotification);
        stub.setBounds (0, 0, cell.getWidth(), cell.getHeight());
        {
            juce::Graphics::ScopedSaveState saved (g);
            g.addTransform (juce::AffineTransform::translation (
                static_cast<float> (cell.getX()),
                static_cast<float> (cell.getY())));
            lnf->drawButtonBackground (g, stub,
                                       stub.findColour (juce::TextButton::buttonColourId),
                                       sample.hover, false);
        }

        // Draw label in screen-space coords directly.
        g.setColour (sample.active ? theme::onPrimaryContainer : theme::mutedForeground);
        auto labelFont = theme::Fonts::monoData();
        if (sample.active)
            labelFont = labelFont.withStyle (juce::Font::bold);
        g.setFont (labelFont);
        g.drawText (sample.label, cell, juce::Justification::centred, false);
    }
}

void ThemeTest::paintHero (juce::Graphics& g, juce::Rectangle<int> area)
{
    drawCard (g, area.toFloat(), theme::surfaceElevated, theme::outlineVariant, 12.0f);

    // Top edge gradient sweep
    auto sweep = juce::Rectangle<float> (
        static_cast<float> (area.getX()),
        static_cast<float> (area.getY()),
        static_cast<float> (area.getWidth()), 1.0f);
    juce::ColourGradient sg (
        juce::Colours::transparentBlack, sweep.getX(), sweep.getY(),
        juce::Colours::transparentBlack, sweep.getRight(), sweep.getY(), false);
    sg.addColour (0.5, theme::toxic);
    g.setGradientFill (sg);
    g.fillRect (sweep);

    auto inner = area.reduced (24);
    auto labelArea = inner.removeFromTop (20);
    drawSectionLabel (g, labelArea, "FIZZ AMOUNT", theme::mutedForeground);

    // Big number with %
    g.setFont (theme::Fonts::heroNum());
    auto numArea = inner.toFloat().translated (0, 4);
    theme::SpectreLookAndFeel::drawGlowText (g, "66.1",
                                             numArea, juce::Justification::centred,
                                             theme::toxic, theme::toxic, 12.0f);

    // % suffix is smaller; draw on top of the number's right edge
    g.setFont (theme::Fonts::displayTitle().withHeight (32.0f));
    g.setColour (theme::toxic);
    auto suffix = numArea.translated (160.0f, 12.0f);
    g.drawText ("%", suffix, juce::Justification::centred, false);
}

void ThemeTest::paintMeters (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto headerStrip = area.removeFromTop (40);
    drawSectionLabel (g, headerStrip.removeFromLeft (200), "SPECTRAL ANALYSIS", theme::mutedForeground);
    g.setColour (theme::toxic);
    g.setFont (theme::Fonts::uiChrome());
    g.drawText ("WASP METER", headerStrip, juce::Justification::centredRight, false);

    g.setColour (theme::outlineVariant.withAlpha (0.4f));
    g.fillRect (area.removeFromTop (1));

    // Spectral bars - 16 simulated heights matching the Stitch reference set.
    auto label = area.removeFromBottom (32);
    auto bars  = area.reduced (8, 4);

    constexpr int  N = 16;
    constexpr float heights[N] = {
        0.10f, 0.15f, 0.25f, 0.40f, 0.35f,
        0.60f, 0.85f, 0.95f, 0.75f,
        0.50f, 0.30f, 0.20f, 0.10f, 0.05f,
        0.05f, 0.05f
    };

    const float gap   = 2.0f;
    const float total = static_cast<float> (bars.getWidth());
    const float bw    = (total - gap * (N - 1)) / N;

    for (int i = 0; i < N; ++i)
    {
        const float h = bars.getHeight() * heights[i];
        const float x = bars.getX() + i * (bw + gap);
        const float y = bars.getBottom() - h;
        const auto rect = juce::Rectangle<float> (x, y, bw, h);

        juce::Colour fill = theme::surfaceContainerHighest;
        if (heights[i] > 0.55f)      fill = theme::toxic;
        else if (heights[i] > 0.35f) fill = theme::toxic.withAlpha (0.5f);

        g.setColour (fill);
        g.fillRoundedRectangle (rect, 1.5f);

        if (heights[i] > 0.55f)
        {
            g.setColour (theme::toxic.withAlpha (0.18f));
            g.fillRoundedRectangle (rect.expanded (3.0f), 3.0f);
        }
    }

    // Frequency labels
    g.setFont (theme::Fonts::uiChrome().withHeight (10.0f));
    g.setColour (theme::mutedForeground);
    const juce::StringArray freqs { "1k", "2k", "5k", "8k", "12k", "20k" };
    const float step = static_cast<float> (label.getWidth()) / static_cast<float> (freqs.size() - 1);
    for (int i = 0; i < freqs.size(); ++i)
    {
        auto lr = juce::Rectangle<int> (
            label.getX() + (int) (i * step) - 16, label.getY(),
            32, label.getHeight() / 2);
        g.drawText (freqs[i], lr, juce::Justification::centred, false);
    }
    const juce::StringArray zones { "BITE", "PLASTIC", "WASP", "SAND", "AIR", "ICE" };
    g.setColour (theme::toxic.withAlpha (0.55f));
    for (int i = 0; i < zones.size(); ++i)
    {
        auto lr = juce::Rectangle<int> (
            label.getX() + (int) (i * step) - 28,
            label.getY() + label.getHeight() / 2 + 2,
            56, label.getHeight() / 2);
        g.drawText (zones[i], lr, juce::Justification::centred, false);
    }
}

void ThemeTest::paintFooter (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::outlineVariant);
    g.fillRect (area.removeFromTop (1));
    g.setColour (theme::surfaceContainerLowest);
    g.fillRect (area);

    auto inner = area.reduced (20, 0);

    // Left: THE CRYPT + IN/OUT meters
    auto left = inner.removeFromLeft (320);
    g.setColour (theme::mutedForeground);
    g.setFont (theme::Fonts::uiChrome());
    g.drawText ("THE CRYPT", left.removeFromLeft (96), juce::Justification::centredLeft, false);
    g.setColour (theme::outlineVariant);
    g.fillRect (left.removeFromLeft (1).reduced (0, 8));
    left.removeFromLeft (16);

    auto inMeter  = left.removeFromLeft (112).reduced (0, 18);
    left.removeFromLeft (8);
    auto outMeter = left.removeFromLeft (124).reduced (0, 18);

    auto labelWidth = [] (const juce::Font& font, const juce::String& s)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (font, s, 0.0f, 0.0f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    };

    auto drawMeter = [&] (juce::Rectangle<int> area, juce::String label, float fill)
    {
        const int labelW = juce::jmax (24,
            juce::roundToInt (labelWidth (theme::Fonts::uiChrome(), label) + 6.0f));
        g.setColour (theme::mutedForeground);
        g.drawText (label, area.removeFromLeft (labelW),
                    juce::Justification::centredLeft, false);
        const auto pill = area.toFloat();
        g.setColour (theme::surfaceContainerHighest);
        g.fillRoundedRectangle (pill, pill.getHeight() * 0.5f);
        g.setColour (theme::toxic);
        g.fillRoundedRectangle (pill.withWidth (pill.getWidth() * fill),
                                pill.getHeight() * 0.5f);
    };

    drawMeter (inMeter,  "IN",  0.70f);
    drawMeter (outMeter, "OUT", 0.85f);

    // Right: OS + version chrome
    auto right = inner.removeFromRight (480);
    g.setFont (theme::Fonts::uiChrome());
    g.setColour (theme::toxic);
    g.drawText ("V0.1.0  /  SCANNING FOR HARSHNESS",
                right, juce::Justification::centredRight, false);

    auto osArea = right.removeFromRight (180).translated (-220, 0);
    g.setColour (theme::mutedForeground);
    g.drawText ("OS:", osArea.removeFromLeft (28), juce::Justification::centredLeft, false);
    g.setColour (theme::toxic);
    g.setFont (theme::Fonts::monoData());
    g.drawText ("OFF", osArea, juce::Justification::centredLeft, false);
}
} // namespace cabrot::ui
