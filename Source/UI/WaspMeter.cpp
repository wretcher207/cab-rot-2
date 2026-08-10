#include "WaspMeter.h"

#include "../DSP/Tuning.h"
#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"
#include "../Theme/SpectreLookAndFeel.h"

#include <array>
#include <cmath>

namespace cabrot::ui
{
namespace
{
constexpr std::array<float, 6> kFrequencyTicks {
    1000.0f, 2000.0f, 4000.0f, 8000.0f, 12000.0f, 20000.0f
};

const std::array<juce::String, 6> kFrequencyLabels {
    "1k", "2k", "4k", "8k", "12k", "20k"
};

const std::array<juce::String, dsp::tuning::kNumProcessedBands> kZoneLabels {
    "BITE", "PLASTIC", "WASP", "ICE"
};

float xForFrequency (float frequencyHz, juce::Rectangle<int> plot) noexcept
{
    const float t = std::log10 (frequencyHz / 1000.0f) / std::log10 (20.0f);
    return static_cast<float> (plot.getX()) + t * static_cast<float> (plot.getWidth());
}

juce::Rectangle<int> labelPlot (juce::Rectangle<int> row) noexcept
{
    return row.withTrimmedLeft (65).withTrimmedRight (13);
}

juce::Rectangle<int> centredLabel (juce::Rectangle<int> row, float centreX, int width) noexcept
{
    const int x = juce::jlimit (row.getX(), row.getRight() - width,
                                juce::roundToInt (centreX) - width / 2);
    return { x, row.getY(), width, row.getHeight() };
}
}

WaspMeter::WaspMeter()
{
    setOpaque (false);
}

void WaspMeter::updateBandReduction (const std::array<float, 4>& reductionDb,
                                     bool engineLive,
                                     float elapsedSeconds)
{
    constexpr float kReleaseDbPerSecond = 24.0f;
    constexpr float kPeakHoldSeconds = 0.8f;

    const float dt = juce::jlimit (0.0f, 0.25f, elapsedSeconds);
    bool changed = false;

    for (size_t i = 0; i < displayedReductionDb.size(); ++i)
    {
        const float target = engineLive
            ? juce::jlimit (0.0f, theme::kReductionScaleMaxDb, reductionDb[i])
            : 0.0f;

        const float previousDisplay = displayedReductionDb[i];
        displayedReductionDb[i] = target >= previousDisplay
            ? target
            : juce::jmax (target, previousDisplay - kReleaseDbPerSecond * dt);

        const float previousPeak = heldPeakDb[i];
        if (target > heldPeakDb[i])
        {
            heldPeakDb[i] = target;
            holdRemainingSeconds[i] = kPeakHoldSeconds;
        }
        else if (holdRemainingSeconds[i] > 0.0f)
        {
            holdRemainingSeconds[i] = juce::jmax (0.0f, holdRemainingSeconds[i] - dt);
        }
        else
        {
            heldPeakDb[i] = juce::jmax (displayedReductionDb[i],
                                        heldPeakDb[i] - kReleaseDbPerSecond * dt);
        }

        changed = changed
            || std::abs (displayedReductionDb[i] - previousDisplay) >= 0.01f
            || std::abs (heldPeakDb[i] - previousPeak) >= 0.01f;
    }

    if (changed)
        repaint();
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
    // The header lines up with the plot, not the frame: the dB scale column
    // sits under the label strip's left edge.
    auto inner = area.withTrimmedLeft (65).withTrimmedRight (4);

    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.setColour (theme::inkMeta);
    g.drawText  ("GAIN REDUCTION",
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

    auto plot = inner.reduced (12, 12).withTrimmedLeft (4);
    paintColumns (g, plot);
    paintDbScale (g, scale);
    paintTicks (g, plot);
}

void WaspMeter::paintColumns (juce::Graphics& g, juce::Rectangle<int> plot)
{
    auto reductionY = [plot] (float db)
    {
        const float t = juce::jlimit (0.0f, 1.0f, db / theme::kReductionScaleMaxDb);
        return static_cast<float> (plot.getY()) + t * static_cast<float> (plot.getHeight());
    };

    for (size_t i = 0; i < displayedReductionDb.size(); ++i)
    {
        const float x0 = xForFrequency (dsp::tuning::kCrossoverHz[i], plot) + 1.0f;
        const float x1 = xForFrequency (dsp::tuning::kCrossoverHz[i + 1], plot) - 1.0f;
        const float y = reductionY (displayedReductionDb[i]);

        if (displayedReductionDb[i] > 0.01f)
        {
            g.setColour (theme::surface3);
            g.fillRect (juce::Rectangle<float> (x0,
                                                 static_cast<float> (plot.getY()),
                                                 juce::jmax (1.0f, x1 - x0),
                                                 y - static_cast<float> (plot.getY())));

            g.setColour (displayedReductionDb[i] > theme::kReductionDamageThresholdDb
                             ? theme::stateError
                             : theme::inkPrimary);
            g.drawLine (x0, y, x1, y, 1.5f);
        }

        if (heldPeakDb[i] > displayedReductionDb[i] + 0.01f)
        {
            const float peakY = reductionY (heldPeakDb[i]);
            g.setColour (theme::inkBody.withAlpha (0.6f));
            g.drawLine (x0, peakY, x1, peakY, 1.0f);
        }
    }
}

void WaspMeter::paintTicks (juce::Graphics& g, juce::Rectangle<int> plot)
{
    g.setColour (theme::inkMeta.withAlpha (0.6f));
    for (const float frequency : kFrequencyTicks)
    {
        const int cx = juce::roundToInt (xForFrequency (frequency, plot));
        g.fillRect (cx, plot.getBottom() - 5, 1, 5);
    }
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
                                          juce::roundToInt (y) - 7,
                                          area.getWidth() - 4,
                                          12),
                    juce::Justification::centredLeft, false);

        // Guide line across the full meter width, kept as dim as possible.
        const auto guideY = y;
        g.setColour (theme::rule.withAlpha (0.40f));
        g.drawLine (static_cast<float> (area.getRight() + 4), guideY,
                    static_cast<float> (getLocalBounds().getRight()), guideY, 1.0f);

        // Damaged territory: the 12 dB guide is the one that matters, so it
        // draws at full strength while the others whisper.
        if (ticks[i] == "-12")
        {
            g.setColour (theme::rule);
            g.drawLine (static_cast<float> (area.getRight() + 4), guideY,
                        static_cast<float> (getLocalBounds().getRight()), guideY, 1.0f);
        }
    }
}

void WaspMeter::paintLabels (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto numericRow = area.removeFromTop (16);
    area.removeFromTop (4);
    auto namedRow = area;

    g.setFont (theme::Fonts::mono (9.5f));
    g.setColour (theme::inkMeta);
    const auto numeric = labelPlot (numericRow);
    for (size_t i = 0; i < kFrequencyLabels.size(); ++i)
        g.drawText (kFrequencyLabels[i],
                    centredLabel (numeric, xForFrequency (kFrequencyTicks[i], numeric), 36),
                    juce::Justification::centred, false);

    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.setColour (theme::inkBody);
    const auto named = labelPlot (namedRow);
    for (size_t i = 0; i < kZoneLabels.size(); ++i)
    {
        const float low = dsp::tuning::kCrossoverHz[i];
        const float high = dsp::tuning::kCrossoverHz[i + 1];
        const float centreFrequency = std::sqrt (low * high);
        g.drawText (kZoneLabels[i],
                    centredLabel (named, xForFrequency (centreFrequency, named), 72),
                    juce::Justification::centred, false);
    }
}
} // namespace cabrot::ui
