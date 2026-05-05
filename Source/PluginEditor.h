#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace cabrot
{
class CabRotEditor final : public juce::AudioProcessorEditor
{
public:
    explicit CabRotEditor (CabRotProcessor&);
    ~CabRotEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CabRotProcessor& processorRef;
    juce::Label brandLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotEditor)
};
} // namespace cabrot
