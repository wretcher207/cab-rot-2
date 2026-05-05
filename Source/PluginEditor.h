#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"

namespace cabrot::theme { class SpectreLookAndFeel; }

namespace cabrot
{
class CabRotEditor final : public juce::AudioProcessorEditor
{
public:
    explicit CabRotEditor (CabRotProcessor&);
    ~CabRotEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void paintBackground (juce::Graphics&, juce::Rectangle<int> bounds);
    void paintDpdMark    (juce::Graphics&, juce::Rectangle<float> area);
    void paintWordmark   (juce::Graphics&, juce::Rectangle<int> bounds);
    void paintFooter     (juce::Graphics&, juce::Rectangle<int> bounds);

    CabRotProcessor& processorRef;
    std::unique_ptr<theme::SpectreLookAndFeel> lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotEditor)
};
} // namespace cabrot
