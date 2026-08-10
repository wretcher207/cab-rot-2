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

    auto headerStrip = area.removeFromTop (32);
    paintHeader (g, headerStrip);
    area.removeFromTop (8);

    auto labelStrip = area.removeFromBottom (44);
    paintLabels (g, labelStrip);
    area.removeFromBottom (4);

    paintFrame (g, area);
}

void WaspMeter::resized()
{
    // Fully paint-driven; no children.
}

void WaspMeter::paintHeader (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto inner = area.withTrimmedRight (4);

    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.setColour (theme::inkMeta);
    g.drawText  ("SPECTRAL ANALYSIS",
                 inner.removeFromLeft (200),
                 juce::Justification::centredLeft, false);

    g.setColour (theme::inkMeta);
    g.drawText  ("WASP METER",
                 inner, juce::Justification::centredRight, false);
}

void WaspMeter::paintFrame (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (theme::surface1);
    g.fillRect (area);
    g.setColour (theme::rule);
    g.drawRect (area, 1);

    theme::SpectreLookAndFeel::drawScanlines (g, area.reduced (1), 0.05f, 3);

    // The plot area leaves room on the left for the dB scale.
    auto inner   = area.reduced (1);
    auto scale   = inner.removeFromLeft (48);
    paintDbScale (g, scale);

    auto plot = inner.reduced (12, 12).withTrimmedLeft (4);
    paintReference (g, plot);
    paintReductionCurve (g, plot);
}

float WaspMeter::dbToY (float dB, juce::Rectangle<int> plot) const noexcept
{
    const float t = juce::jlimit (0.0f, 1.0f, -dB / theme::kReductionScaleMaxDb);
    return plot.getY() + t * static_cast<float> (plot.getHeight());
}

void WaspMeter::paintDbScale (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto plot = area.reduced (0, 12);

    const juce::StringArray ticks { "0", "-6", "-12", "-18", "-24" };

    g.setFont (theme::Fonts::mono (9.5f));

    for (int i = 0; i < ticks.size(); ++i)
    {
        const float y = plot.getY() + static_cast<float> (plot.getHeight())
                                     * static_cast<float> (i)
                                     / static_cast<float> (ticks.size() - 1);
        g.setColour (theme::inkMeta);
        g.drawText (ticks[i] + " dB",
                    juce::Rectangle<int> (area.getX() + 4,
                                          juce::roundToInt (y) - 6,
                                          area.getWidth() - 4,
                                          12),
                    juce::Justification::centredLeft, false);

        // Guide line across the full meter width, kept as dim as possible.
        g.setColour (theme::rule.withAlpha (0.55f));
        g.drawLine (static_cast<float> (area.getRight() + 4), y,
                    static_cast<float> (getLocalBounds().getRight()) , y, 1.0f);
    }
}

void WaspMeter::paintReference (juce::Graphics& g, juce::Rectangle<int> plot)
{
    // Quiet reference spectrum: a single unbroken filled shape in the rule
    // colour, never highlighted, never animated in v1.
    juce::Path shape;
    const float w = static_cast<float> (plot.getWidth());

    shape.startNewSubPath (static_cast<float> (plot.getX()),
                           static_cast<float> (plot.getBottom()));
    for (int i = 0; i < kNumColumns; ++i)
    {
        const float x = plot.getX() + w * static_cast<float> (i) / static_cast<float> (kNumColumns - 1);
        const float top = plot.getBottom() - reference[i] * plot.getHeight();
        if (i == 0)
            shape.lineTo (x, top);
        else
        {
            const float px = plot.getX() + w * static_cast<float> (i - 1) / static_cast<float> (kNumColumns - 1);
            const float py = plot.getBottom() - reference[i - 1] * plot.getHeight();
            shape.cubicTo ((px + x) * 0.5f - (x - px) * 0.15f, py,
                           (px + x) * 0.5f - (x - px) * 0.15f, top,
                           x, top);
        }
    }
    shape.lineTo (static_cast<float> (plot.getRight()),
                  static_cast<float> (plot.getBottom()));
    shape.closeSubPath();

    g.setColour (theme::rule.withAlpha (0.55f));
    g.fillPath (shape);
}

void WaspMeter::paintReductionCurve (juce::Graphics& g, juce::Rectangle<int> plot)
{
    const float w = static_cast<float> (plot.getWidth());

    auto pointFor = [&] (int i)
    {
        const float x = plot.getX() + w * static_cast<float> (i) / static_cast<float> (kNumCurvePoints - 1);
        return juce::Point<float> (x, dbToY (reductionDb[i], plot));
    };

    juce::Path curve;
    curve.startNewSubPath (pointFor (0));
    for (int i = 1; i < kNumCurvePoints; ++i)
    {
        const auto p  = pointFor (i);
        const auto prev = pointFor (i - 1);
        curve.quadraticTo (prev, p);
    }

    // Reductions past the damage threshold render in the error colour;
    // everything above the threshold is primary ink. We draw the full
    // curve twice with a clip on the error pass, which keeps antialiasing
    // clean at the crossing point.
    g.setColour (theme::inkPrimary);
    g.strokePath (curve, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));

    // Rows below thresholdY (larger y) are past the damage threshold.
    const float thresholdY = dbToY (-theme::kReductionDamageThresholdDb, plot);
    {
        juce::Graphics::ScopedSaveState saved (g);
        g.reduceClipRegion (plot.getX(), juce::roundToInt (thresholdY),
                            plot.getWidth(), plot.getBottom() - juce::roundToInt (thresholdY));
        g.setColour (theme::stateError);
        g.strokePath (curve, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));
    }
}

void WaspMeter::paintLabels (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto numericRow = area.removeFromTop (16);
    area.removeFromTop (4);
    auto namedRow = area;

    const juce::StringArray freqs { "1k", "2k", "5k", "8k", "12k", "20k" };
    const juce::StringArray zones { "BITE", "PLASTIC", "WASP", "SAND", "AIR", "ICE" };

    // Center labels under the plot area (which is inset by the dB scale).
    auto rowFor = [&] (juce::Rectangle<int> row)
    {
        auto r = row;
        r.removeFromLeft (49);
        return r.reduced (16, 0).withTrimmedLeft (0);
    };

    auto labelSlot = [] (juce::Rectangle<int> row, int idx, int count, int width)
    {
        const float step = static_cast<float> (row.getWidth()) / static_cast<float> (count - 1);
        const int   cx   = row.getX() + juce::roundToInt (idx * step);
        const int   x    = juce::jlimit (row.getX(), row.getRight() - width, cx - width / 2);
        return juce::Rectangle<int> (x, row.getY(), width, row.getHeight());
    };

    g.setFont (theme::Fonts::mono (9.5f));
    g.setColour (theme::inkMeta);
    const auto numeric = rowFor (numericRow);
    for (int i = 0; i < freqs.size(); ++i)
        g.drawText (freqs[i], labelSlot (numeric, i, freqs.size(), 36),
                    juce::Justification::centred, false);

    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.setColour (theme::inkBody);
    const auto named = rowFor (namedRow);
    for (int i = 0; i < zones.size(); ++i)
        g.drawText (zones[i], labelSlot (named, i, zones.size(), 64),
                    juce::Justification::centred, false);
}
} // namespace cabrot::ui
