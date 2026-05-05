#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cabrot::theme { class SpectreLookAndFeel; }

namespace cabrot::ui
{
// A single screen that exercises every UI atom in design/CANONICAL-UI.md so
// Phase 1's visual diff has something to measure. Not used in the real
// plugin editor; ThemeTestApp builds it into a standalone GUI app target
// so visual-diff.ps1 can capture and compare against the Stitch reference.

class ThemeTest final : public juce::Component
{
public:
    ThemeTest();
    ~ThemeTest() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    void paintHeader  (juce::Graphics&, juce::Rectangle<int>);
    void paintKnobs   (juce::Graphics&, juce::Rectangle<int>);
    void paintModes   (juce::Graphics&, juce::Rectangle<int>);
    void paintHero    (juce::Graphics&, juce::Rectangle<int>);
    void paintMeters  (juce::Graphics&, juce::Rectangle<int>);
    void paintFooter  (juce::Graphics&, juce::Rectangle<int>);
    void paintDpdMark (juce::Graphics&, juce::Rectangle<float>);

    std::unique_ptr<theme::SpectreLookAndFeel> lnf;

    // Sample knobs - we draw them via the LookAndFeel directly rather than
    // standing up real Slider components. Faster to iterate and more
    // deterministic for visual diffs.
    struct KnobSample { juce::String label; float value; };
    std::vector<KnobSample> knobSamples;

    // Sample mode-button states.
    struct ModeSample { juce::String label; bool active; bool hover; };
    std::vector<ModeSample> modeSamples;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemeTest)
};
} // namespace cabrot::ui
