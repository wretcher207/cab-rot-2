#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// CANONICAL-UI §7.2. Spectral display panel with grid pattern background,
// 14-segment bar visual, peak-line overlay, and dual-row frequency labels
// (numeric on top, named zones on bottom).
//
// Phase 2: bars and peak line are static placeholder data. Phase 6 wires
// real FFT analysis from the audio thread.

class WaspMeter final : public juce::Component
{
public:
    WaspMeter();
    ~WaspMeter() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    void paintHeader  (juce::Graphics&, juce::Rectangle<int>);
    void paintBars    (juce::Graphics&, juce::Rectangle<int>);
    void paintLabels  (juce::Graphics&, juce::Rectangle<int>);
    void paintPeakLine(juce::Graphics&, juce::Rectangle<int>);

    // PHASE 2 PLACEHOLDER: bar heights mirror Stitch's static reference.
    // Phase 6 replaces with FFT bin data via lock-free FIFO.
    static constexpr int kNumBars = 16;
    float barHeights[kNumBars] = {
        0.10f, 0.15f, 0.25f, 0.40f, 0.35f,
        0.60f, 0.85f, 0.95f, 0.75f,
        0.50f, 0.30f, 0.20f, 0.10f, 0.05f,
        0.05f, 0.05f
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaspMeter)
};
} // namespace cabrot::ui
