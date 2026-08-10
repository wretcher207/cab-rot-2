#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <optional>

namespace cabrot::ui
{
// CANONICAL-UI §7.3. The "FIZZ AMOUNT" hero card. 192 px tall by spec.
//
// Shows an em dash-style idle value until the editor supplies live reduction
// data. A click-to-cycle (FIZZ % / Reduction dB / Peak Hz) remains deferred.

class FizzReadout final : public juce::Component
{
public:
    FizzReadout();
    ~FizzReadout() override = default;

    void setValue (std::optional<float> percent);
    void paint   (juce::Graphics&) override;

private:
    std::optional<float> displayValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FizzReadout)
};
} // namespace cabrot::ui
