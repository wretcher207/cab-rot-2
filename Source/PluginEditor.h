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
    void paintBackground (juce::Graphics&, juce::Rectangle<int> bounds);
    void paintScanlines  (juce::Graphics&, juce::Rectangle<int> bounds);
    void paintDpdMark    (juce::Graphics&, juce::Rectangle<float> area);
    void paintWordmark   (juce::Graphics&, juce::Rectangle<int> bounds);
    void paintFooter     (juce::Graphics&, juce::Rectangle<int> bounds);

    CabRotProcessor& processorRef;
    juce::Typeface::Ptr displayTypeface; // Space Grotesk Bold
    juce::Typeface::Ptr monoTypeface;    // JetBrains Mono Regular

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabRotEditor)
};
} // namespace cabrot
