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
class CabRotEditor final : public juce::AudioProcessorEditor
{
public:
    explicit CabRotEditor (CabRotProcessor&);
    ~CabRotEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    CabRotProcessor& processorRef;
    std::unique_ptr<theme::SpectreLookAndFeel> lookAndFeel;

    ui::HeaderBar       headerBar;
    ui::WaspMeter       waspMeter;
    ui::FizzReadout     fizzReadout;
    ui::AmpProfileGrid  ampProfile;
    ui::KnobRow         knobRow;
    ui::FooterBar       footerBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotEditor)
};
} // namespace cabrot
