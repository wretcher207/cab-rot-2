#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "atoms/MeterPill.h"

namespace cabrot::ui
{
// 48 px tall band at the bottom of the editor. Three regions:
//   left:   IN/OUT level meters
//   center: A/B compare toggle
//   right:  oversampling combo + version and processing state

class FooterBar final : public juce::Component
{
public:
    FooterBar();
    ~FooterBar() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

    void setStatusText (juce::String text);
    void setABAvailable (bool available);

    juce::TextButton& getButtonA()      noexcept { return aButton; }
    juce::TextButton& getButtonB()      noexcept { return bButton; }
    juce::ComboBox&   getOversampleBox() noexcept { return oversample; }

private:
    juce::String formattedStatusText() const;
    int statusTextWidth() const;

    juce::TextButton aButton      { "A" };
    juce::TextButton bButton      { "B" };
    MeterPill        inMeter      { "IN" };
    MeterPill        outMeter     { "OUT" };
    juce::ComboBox   oversample;
    juce::String     statusText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FooterBar)
};
} // namespace cabrot::ui
