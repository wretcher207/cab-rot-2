#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "atoms/MeterPill.h"

namespace cabrot::ui
{
// CANONICAL-UI §7.5. 48 px tall band at the bottom of the editor.
// Three regions:
//   left:   THE CRYPT button + IN/OUT meters
//   center: A/B compare toggle
//   right:  Oversampling combo + version chrome

class FooterBar final : public juce::Component
{
public:
    FooterBar();
    ~FooterBar() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

    juce::TextButton& getCryptButton()  noexcept { return cryptButton; }
    juce::TextButton& getButtonA()      noexcept { return aButton; }
    juce::TextButton& getButtonB()      noexcept { return bButton; }
    juce::ComboBox&   getOversampleBox() noexcept { return oversample; }

private:
    juce::TextButton cryptButton  { "THE CRYPT" };
    juce::TextButton aButton      { "A" };
    juce::TextButton bButton      { "B" };
    MeterPill        inMeter      { "IN" };
    MeterPill        outMeter     { "OUT" };
    juce::ComboBox   oversample;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FooterBar)
};
} // namespace cabrot::ui
