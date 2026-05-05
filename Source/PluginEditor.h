#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "UI/AmpProfileGrid.h"
#include "UI/FizzReadout.h"
#include "UI/FooterBar.h"
#include "UI/HeaderBar.h"
#include "UI/KnobRow.h"
#include "UI/WaspMeter.h"

namespace cabrot::theme { class SpectreLookAndFeel; }

namespace cabrot
{
class CabRotEditor final : public juce::AudioProcessorEditor,
                           private juce::KeyListener
{
public:
    explicit CabRotEditor (CabRotProcessor&);
    ~CabRotEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

    bool keyPressed (const juce::KeyPress&, juce::Component*) override;

private:
    void wireAttachments();
    juce::AudioProcessorValueTreeState& apvts() noexcept { return processorRef.getApvts(); }

    CabRotProcessor& processorRef;
    std::unique_ptr<theme::SpectreLookAndFeel> lookAndFeel;

    ui::HeaderBar       headerBar;
    ui::WaspMeter       waspMeter;
    ui::FizzReadout     fizzReadout;
    ui::AmpProfileGrid  ampProfile;
    ui::KnobRow         knobRow;
    ui::FooterBar       footerBar;

    juce::TooltipWindow tooltipWindow { this, 500 };

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>>     knobAttachments;
    std::unique_ptr<ButtonAttachment>                  deltaAttachment;
    std::unique_ptr<juce::ParameterAttachment>         abAttachment; // drives both A and B
    std::unique_ptr<ComboBoxAttachment>                osAttachment;

    // Mode buttons drive a Choice parameter. JUCE's ButtonAttachment is for
    // bool params, not choices, so we build a small parameter listener that
    // mirrors the param value into the radio-group selection. Stored by
    // unique_ptr because juce::ParameterAttachment is non-copyable.
    std::vector<std::unique_ptr<juce::ParameterAttachment>> modeAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotEditor)
};
} // namespace cabrot
