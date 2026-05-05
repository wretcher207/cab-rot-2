#include "WaspMeter.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"
#include "../Theme/SpectreLookAndFeel.h"

namespace cabrot::ui
{
WaspMeter::WaspMeter()
{
    setOpaque (false);
}

void WaspMeter::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour (theme::surfaceContainer.withAlpha (0.5f));
    g.fillRoundedRectangle (area.toFloat(), 12.0f);

    g.setColour (theme::outlineVariant);
    g.drawRoundedRectangle (area.toFloat(), 12.0f, 1.0f);

    auto inner = area.reduced (1);

    // 1 px sweep across the top edge
    juce::ColourGradient sweep (
        juce::Colours::transparentBlack, static_cast<float> (inner.getX()), static_cast<float> (inner.getY()),
        juce::Colours::transparentBlack, static_cast<float> (inner.getRight()), static_cast<float> (inner.getY()), false);
    sweep.addColour (0.5, theme::toxic.withAlpha (0.5f));
    g.setGradientFill (sweep);
    g.fillRect (inner.getX(), inner.getY(), inner.getWidth(), 1);

    theme::SpectreLookAndFeel::drawGridPattern (g, inner.reduced (1), 0.30f, 20);

    auto headerStrip = inner.removeFromTop (40);
    paintHeader (g, headerStrip);

    auto labelStrip = inner.removeFromBottom (32);
    paintBars   (g, inner.reduced (12, 8));
    paintPeakLine (g, inner.reduced (12, 8));
    paintLabels (g, labelStrip);
}

void WaspMeter::resized()
{
    // Fully paint-driven for now; no children.
}

void WaspMeter::paintHeader (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::outlineVariant.withAlpha (0.30f));
    g.fillRect (area.removeFromBottom (1));

    auto inner = area.reduced (16, 0);

    g.setFont   (theme::Fonts::uiChrome());
    g.setColour (theme::mutedForeground);
    g.drawText  ("SPECTRAL ANALYSIS",
                 inner.removeFromLeft (200),
                 juce::Justification::centredLeft, false);

    g.setColour (theme::toxic);
    g.drawText  ("WASP METER",
                 inner, juce::Justification::centredRight, false);
}

void WaspMeter::paintBars (juce::Graphics& g, juce::Rectangle<int> area)
{
    const float gap   = 2.0f;
    const float total = static_cast<float> (area.getWidth());
    const float bw    = (total - gap * (kNumBars - 1)) / kNumBars;

    for (int i = 0; i < kNumBars; ++i)
    {
        const float h = static_cast<float> (area.getHeight()) * barHeights[i];
        const float x = area.getX() + i * (bw + gap);
        const float y = area.getBottom() - h;
        const auto rect = juce::Rectangle<float> (x, y, bw, h);

        juce::Colour fill = theme::surfaceContainerHighest;
        if (barHeights[i] > 0.55f)      fill = theme::toxic;
        else if (barHeights[i] > 0.35f) fill = theme::toxic.withAlpha (0.5f);

        g.setColour (fill);
        g.fillRoundedRectangle (rect, 1.5f);

        if (barHeights[i] > 0.55f)
        {
            g.setColour (theme::toxic.withAlpha (0.18f));
            g.fillRoundedRectangle (rect.expanded (3.0f), 3.0f);
        }
    }
}

void WaspMeter::paintPeakLine (juce::Graphics& g, juce::Rectangle<int> area)
{
    juce::Path path;
    const float yMid = static_cast<float> (area.getCentreY());
    const float w    = static_cast<float> (area.getWidth());

    // Gentle SVG-style curve: rises, peaks, falls. Visual placeholder until
    // Phase 6 drives this from real spectral analysis.
    path.startNewSubPath (static_cast<float> (area.getX()),     yMid + area.getHeight() * 0.30f);
    path.quadraticTo (area.getX() + w * 0.20f, yMid + area.getHeight() * 0.05f,
                      area.getX() + w * 0.40f, yMid - area.getHeight() * 0.20f);
    path.quadraticTo (area.getX() + w * 0.55f, yMid - area.getHeight() * 0.45f,
                      area.getX() + w * 0.70f, yMid - area.getHeight() * 0.05f);
    path.quadraticTo (area.getX() + w * 0.85f, yMid + area.getHeight() * 0.20f,
                      area.getX() + w,         yMid + area.getHeight() * 0.30f);

    g.setColour (theme::toxic.withAlpha (0.22f));
    g.strokePath (path, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
    g.setColour (theme::toxic);
    g.strokePath (path, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}

void WaspMeter::paintLabels (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::outlineVariant.withAlpha (0.30f));
    g.fillRect (area.removeFromTop (1));

    auto numericRow = area.removeFromTop (area.getHeight() / 2);
    auto namedRow   = area;

    g.setFont (theme::Fonts::uiChrome().withHeight (10.0f));

    const juce::StringArray freqs { "1k", "2k", "5k", "8k", "12k", "20k" };
    const juce::StringArray zones { "BITE", "PLASTIC", "WASP", "SAND", "AIR", "ICE" };

    const int slots = freqs.size();

    auto labelSlot = [] (juce::Rectangle<int> row, int idx, int count, int width)
    {
        const float step = static_cast<float> (row.getWidth()) / static_cast<float> (count - 1);
        const int   cx   = row.getX() + juce::roundToInt (idx * step);
        const int   x    = juce::jlimit (row.getX(), row.getRight() - width, cx - width / 2);
        return juce::Rectangle<int> (x, row.getY(), width, row.getHeight());
    };

    g.setColour (theme::mutedForeground);
    for (int i = 0; i < slots; ++i)
        g.drawText (freqs[i], labelSlot (numericRow, i, slots, 36),
                    juce::Justification::centred, false);

    g.setColour (theme::toxic.withAlpha (0.55f));
    for (int i = 0; i < zones.size(); ++i)
        g.drawText (zones[i], labelSlot (namedRow, i, zones.size(), 64),
                    juce::Justification::centred, false);
}
} // namespace cabrot::ui
