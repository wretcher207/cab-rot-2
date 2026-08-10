#include "ThemeTest.h"
#include "../Theme/Palette.h"
#include "../Theme/Fonts.h"
#include "../Theme/SpectreLookAndFeel.h"

namespace cabrot::ui
{
namespace
{
void drawRule (juce::Graphics& g, juce::Rectangle<int> r)
{
    g.setColour (theme::rule);
    g.fillRect (r);
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

    g.setColour (theme::canvas);
    g.fillRect (bounds);

    auto remaining = bounds;
    paintHeader (g, remaining.removeFromTop (64));

    auto footer  = remaining.removeFromBottom (48);
    auto knobBay = remaining.removeFromBottom (192);
    auto main    = remaining.reduced (24);

    auto right = main.removeFromRight (344 + 24);
    right.removeFromRight (24);

    paintMeters (g, main.withTrimmedRight (24));

    auto hero = right.removeFromTop (192);
    paintHero (g, hero);
    right.removeFromTop (24);
    paintModes (g, right);

    paintKnobs (g, knobBay);
    paintFooter (g, footer);
}

void ThemeTest::resized()
{
    // No child components - a single paint() pass draws everything.
}

void ThemeTest::paintHeader (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::canvas);
    g.fillRect (area);

    auto inner = area.reduced (24, 0);

    auto left = inner.removeFromLeft (520);
    auto wm   = left.removeFromLeft (170);
    g.setColour (theme::inkPrimary);
    g.setFont (theme::Fonts::wordmark());
    g.drawText ("CAB ROT", wm, juce::Justification::centredLeft, false);

    drawRule (g, { left.getX(), left.getCentreY() - 8, 1, 16 });
    left.removeFromLeft (16);

    paintDpdMark (g, juce::Rectangle<float> (static_cast<float> (left.getX()),
                                              static_cast<float> (left.getCentreY() - 8),
                                              16.0f, 16.0f));

    left.removeFromLeft (24);
    g.setColour (theme::inkMeta);
    g.setFont (theme::Fonts::monoLabel (10.5f));
    g.drawText ("DEAD PIXEL HARMONIX", left, juce::Justification::centredLeft, false);

    drawRule (g, inner.withY (area.getBottom() - 1).withHeight (1)
                    .expanded (24, 0));
}

void ThemeTest::paintDpdMark (juce::Graphics& g, juce::Rectangle<float> area)
{
    const float stroke = juce::jmax (1.0f, area.getWidth() * 0.06f);
    const float offset = area.getWidth() * 0.14f;

    g.setColour (theme::inkBody.withAlpha (0.70f));
    g.drawRect (area.reduced (stroke * 0.5f), stroke * 0.7f);
    g.drawRect (area.reduced (stroke * 0.5f).translated (-offset, offset), stroke * 0.55f);

    const float pix = area.getWidth() * 0.18f;
    g.setColour (theme::inkPrimary.withAlpha (0.92f));
    g.fillRect (juce::Rectangle<float> (
        area.getRight() - pix - stroke,
        area.getY() + pix * 0.4f,
        pix, pix));
}

void ThemeTest::paintKnobs (juce::Graphics& g, juce::Rectangle<int> area)
{
    drawRule (g, area.removeFromTop (1));
    g.setColour (theme::canvas);
    g.fillRect (area);

    auto inner = area.reduced (32, 24);
    const int n = static_cast<int> (knobSamples.size());
    if (n == 0)
        return;

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

        g.setColour (theme::inkMeta);
        g.setFont (theme::Fonts::monoLabel (10.0f));
        g.drawText (sample.label, labelArea, juce::Justification::centred, false);

        const float startA = juce::degreesToRadians (-135.0f);
        const float endA   = juce::degreesToRadians ( 135.0f);

        juce::Slider stub;
        stub.setLookAndFeel (lnf.get());
        lnf->drawRotarySlider (g,
                               knobArea.getX(), knobArea.getY(),
                               knobArea.getWidth(), knobArea.getHeight(),
                               sample.value, startA, endA, stub);
        stub.setLookAndFeel (nullptr);

        g.setColour (theme::inkBody);
        g.setFont (theme::Fonts::mono (13.0f, 0.05f));
        g.drawText (juce::String (juce::roundToInt (sample.value * 100.0f)),
                    valueArea, juce::Justification::centred, false);
    }
}

void ThemeTest::paintModes (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto inner = area;
    auto headerStrip = inner.removeFromTop (16);

    g.setColour (theme::inkMeta);
    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.drawText ("AMP PROFILE", headerStrip, juce::Justification::centredLeft, false);

    inner.removeFromTop (16);

    const int cols = 2;
    const int rows = 3;
    const int gap  = 8;

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

        g.setColour (sample.active ? theme::inkPrimary
                     : sample.hover ? theme::inkBody
                                    : theme::inkMeta);
        g.setFont (theme::Fonts::monoLabel (11.0f));
        g.drawText (sample.label, cell, juce::Justification::centred, false);
    }
}

void ThemeTest::paintHero (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::inkMeta);
    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.drawText ("FIZZ AMOUNT", area.removeFromTop (16),
                juce::Justification::centredLeft, false);
    area.removeFromTop (12);

    g.setFont (theme::Fonts::mono (66.0f, -0.04f));
    g.setColour (theme::inkPrimary);
    g.drawText ("--", area, juce::Justification::centred, false);

    drawRule (g, { area.getX(), area.getBottom() - 1, area.getWidth(), 1 });
}

void ThemeTest::paintMeters (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto headerStrip = area.removeFromTop (32);
    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.setColour (theme::inkMeta);
    g.drawText ("SPECTRAL ANALYSIS", headerStrip.removeFromLeft (200),
                juce::Justification::centredLeft, false);
    g.drawText ("WASP METER", headerStrip, juce::Justification::centredRight, false);
    area.removeFromTop (8);

    auto frame = area.withTrimmedBottom (48);
    g.setColour (theme::surface1);
    g.fillRect (frame);
    g.setColour (theme::rule);
    g.drawRect (frame, 1);

    theme::SpectreLookAndFeel::drawScanlines (g, frame.reduced (1), 0.05f, 3);

    auto inner = frame.reduced (1);
    inner.removeFromLeft (48);
    auto plot = inner.reduced (12, 12).withTrimmedLeft (4);

    // Reference spectrum, quiet fill.
    constexpr int N = 16;
    constexpr float heights[N] = {
        0.10f, 0.15f, 0.25f, 0.22f, 0.35f,
        0.30f, 0.60f, 0.85f, 0.75f,
        0.50f, 0.30f, 0.20f, 0.10f, 0.05f,
        0.05f, 0.05f
    };
    juce::Path shape;
    const float w = static_cast<float> (plot.getWidth());
    shape.startNewSubPath (static_cast<float> (plot.getX()),
                           static_cast<float> (plot.getBottom()));
    for (int i = 0; i < N; ++i)
        shape.lineTo (plot.getX() + w * static_cast<float> (i) / static_cast<float> (N - 1),
                      plot.getBottom() - heights[i] * plot.getHeight());
    shape.lineTo (static_cast<float> (plot.getRight()),
                  static_cast<float> (plot.getBottom()));
    shape.closeSubPath();
    g.setColour (theme::rule.withAlpha (0.55f));
    g.fillPath (shape);

    // Reduction curve, primary ink.
    juce::Path curve;
    curve.startNewSubPath (static_cast<float> (plot.getX()),
                           static_cast<float> (plot.getBottom()));
    const float py = static_cast<float> (plot.getY());
    const float cx = static_cast<float> (plot.getCentreX());
    const float pb = static_cast<float> (plot.getBottom());
    const float ph = static_cast<float> (plot.getHeight());
    curve.quadraticTo (cx - w * 0.1f, py + ph * 0.1f,
                       cx,            py + ph * 0.35f);
    curve.quadraticTo (cx + w * 0.1f, py + ph * 0.6f,
                       static_cast<float> (plot.getRight()), pb);
    g.setColour (theme::inkPrimary);
    g.strokePath (curve, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));
}

void ThemeTest::paintFooter (juce::Graphics& g, juce::Rectangle<int> area)
{
    drawRule (g, area.removeFromTop (1));

    auto inner = area.reduced (24, 0);

    const juce::String status { "V0.1.0 / PROCESSING" };
    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.setColour (theme::inkMeta);
    g.drawText (status, inner, juce::Justification::centredRight, false);

    auto left = inner.removeFromLeft (240);
    g.setColour (theme::inkMeta);
    g.drawText ("IN", left.removeFromLeft (28), juce::Justification::centredLeft, false);
}
} // namespace cabrot::ui
