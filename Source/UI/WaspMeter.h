#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace cabrot::ui
{
// The spectral display is the single focal event of the editor.
// Composition, per design/CANONICAL-UI.md and the DPD brand kit:
//
//   - One 1px `rule` frame on a `surface1` ground. Nothing else frames it.
//   - A dB scale runs down the left inside the frame, mono metadata grey.
//   - Frequency labels and the four real processed-band names sit beneath it.
//   - Scanlines stay inside the frame at 6 percent opacity or less.
//
// The frame intentionally rests empty until live reduction data arrives.

class WaspMeter final : public juce::Component
{
public:
    WaspMeter();
    ~WaspMeter() override = default;

    void updateBandReduction (const std::array<float, 4>& reductionDb,
                              bool engineLive,
                              float elapsedSeconds);

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    void paintHeader  (juce::Graphics&, juce::Rectangle<int>);
    void paintFrame   (juce::Graphics&, juce::Rectangle<int>);
    void paintDbScale (juce::Graphics&, juce::Rectangle<int>);
    void paintTicks   (juce::Graphics&, juce::Rectangle<int>);
    void paintColumns (juce::Graphics&, juce::Rectangle<int>);
    void paintLabels  (juce::Graphics&, juce::Rectangle<int>);

    std::array<float, 4> displayedReductionDb {};
    std::array<float, 4> heldPeakDb {};
    std::array<float, 4> holdRemainingSeconds {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaspMeter)
};
} // namespace cabrot::ui
