#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// The spectral display is the single focal event of the editor.
// Composition, per design/CANONICAL-UI.md and the DPD brand kit:
//
//   - One 1px `rule` frame on a `surface1` ground. Nothing else frames it.
//   - The input spectrum sits behind as a quiet reference in `rule`,
//     never highlighted.
//   - The gain-reduction curve is the primary line: inkPrimary, 1.5 px.
//   - A dB scale runs down the left inside the frame, mono metadata grey.
//   - Frequency labels and the six band names sit beneath the frame.
//   - Scanlines stay inside the frame at 6 percent opacity or less.
//
// Data is still placeholder (Phase 6 wires the real reduction deltas from
// the DSP). Any delta past kReductionDamageThresholdDb renders in
// stateError from that point downward.

class WaspMeter final : public juce::Component
{
public:
    WaspMeter();
    ~WaspMeter() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    void paintHeader  (juce::Graphics&, juce::Rectangle<int>);
    void paintFrame   (juce::Graphics&, juce::Rectangle<int>);
    void paintDbScale (juce::Graphics&, juce::Rectangle<int>);
    void paintTicks   (juce::Graphics&, juce::Rectangle<int>);
    void paintReference (juce::Graphics&, juce::Rectangle<int>);
    void paintReductionCurve (juce::Graphics&, juce::Rectangle<int>);
    void paintLabels  (juce::Graphics&, juce::Rectangle<int>);

    float dbToY (float reductionMagnitudeDb, juce::Rectangle<int> plot) const noexcept;

    // Placeholder input spectrum, 16 columns. Phase 6 replaces with FFT
    // bin data via lock-free FIFO.
    static constexpr int kNumColumns = 16;
    float reference[kNumColumns] = {
        0.10f, 0.15f, 0.25f, 0.22f, 0.35f,
        0.30f, 0.60f, 0.85f, 0.75f,
        0.50f, 0.30f, 0.20f, 0.10f, 0.05f,
        0.05f, 0.05f
    };

    // Placeholder gain-reduction magnitudes in dB (positive down from 0).
    // Phase 6 replaces with the four live band envelopes interpolated per
    // column (getBandReductionDb()).
    static constexpr int kNumCurvePoints = 24;
    float reductionDb[kNumCurvePoints] = {
        0.0f,  0.2f,  0.6f,  1.2f,  2.0f,  3.0f,
        4.2f,  5.6f,  7.0f,  8.1f,  9.0f,  9.6f,
        9.9f,  9.6f,  8.8f,  7.6f,  6.2f,  4.8f,
        3.6f,  2.6f,  1.8f,  1.1f,  0.6f,  0.2f
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaspMeter)
};
} // namespace cabrot::ui
