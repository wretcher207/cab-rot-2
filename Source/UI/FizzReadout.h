#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::ui
{
// CANONICAL-UI §7.3. The "FIZZ AMOUNT" hero card. 192 px tall by spec.
//
// Phase 2: static 66.1% from Stitch. Phase 6 wires real reduction
// average. A click-to-cycle (FIZZ % / Reduction dB / Peak Hz) lands in
// Phase 6 too.

class FizzReadout final : public juce::Component
{
public:
    FizzReadout();
    ~FizzReadout() override = default;

    void paint   (juce::Graphics&) override;

private:
    // PHASE 2 PLACEHOLDER: 66.1 mirrors the Stitch reference. Replaced
    // with live reduction in Phase 6.
    float displayValue = 66.1f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FizzReadout)
};
} // namespace cabrot::ui
