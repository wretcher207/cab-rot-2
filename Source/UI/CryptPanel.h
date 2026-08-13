#pragma once

#include "../Presets/PresetManager.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <vector>

namespace cabrot::ui
{
/**
    The Crypt: the advanced overlay, covering the whole editor.

    Two columns. Left is the preset browser: the twelve factory presets, then
    whatever the user has saved, with Bind Sigil to save and Banish to delete.
    Right is the advanced parameter list.

    Every control here drives real audio behavior. `quality` is deliberately
    absent: it is declared in APVTS but reaches nothing, and a control without
    audio behavior stays hidden. UI Animation is the one non-audio control,
    and it is honest about what it does, which is freeze the instruments on
    their last real values.
*/
class CryptPanel final : public juce::Component,
                         private juce::ListBoxModel
{
public:
    CryptPanel (juce::AudioProcessorValueTreeState& state,
                presets::PresetManager& manager);
    ~CryptPanel() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

    bool keyPressed (const juce::KeyPress&) override;
    void mouseDown (const juce::MouseEvent&) override;

    /** Re-reads the preset list from disk and repaints the selection. */
    void refreshPresetList();

    /** Called when the user asks to leave, by the close control, by Escape,
        or by clicking the scrim outside the panel. */
    std::function<void()> onDismiss;

private:
    //==============================================================================
    int  getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;

    //==============================================================================
    /** One labelled advanced row: name, control, and (for sliders) a value. */
    struct AdvancedRow
    {
        juce::String label;
        juce::Component* control;
    };

    void buildAdvancedControls();
    void loadRow (int row);
    void bindSigil();
    void banish();

    /** Rows 0..11 are factory; the rest are user presets. */
    bool  rowIsFactory (int row) const;
    juce::String nameForRow (int row) const;

    juce::Rectangle<int> panelBounds() const;

    juce::AudioProcessorValueTreeState& apvts;
    presets::PresetManager& presets;

    juce::ListBox    presetList;
    juce::StringArray userNames;
    int selectedRow { -1 };

    juce::TextButton bindButton   { "BIND SIGIL" };
    juce::TextButton banishButton { "BANISH" };
    juce::TextButton closeButton  { "CLOSE" };

    juce::Slider     focusSlider, clampSlider, reapSlider, pickSlider;
    juce::ComboBox   stereoBox;
    juce::ToggleButton autoGainButton   { "AUTO GAIN" };
    juce::ToggleButton uiAnimationButton { "UI ANIMATION" };

    std::vector<AdvancedRow> advancedRows;

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::vector<std::unique_ptr<SliderAttachment>>   sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>>   toggleAttachments;
    std::unique_ptr<ComboBoxAttachment>              stereoAttachment;

    std::unique_ptr<juce::AlertWindow> nameWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CryptPanel)
};
} // namespace cabrot::ui
