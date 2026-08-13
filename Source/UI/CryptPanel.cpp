#include "CryptPanel.h"

#include "../PluginProcessor.h"
#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
namespace
{
constexpr int kRowHeight       = 26;
constexpr int kAdvancedRowH    = 30;
constexpr int kPanelMarginX    = 56;
constexpr int kPanelMarginY    = 44;
constexpr int kPadding         = 24;
constexpr int kHeaderHeight    = 44;
constexpr int kActionHeight    = 26;
constexpr int kLabelColumn     = 128;
constexpr int kValueColumn     = 62;

// The scrim is a flat fill, not a shadow. It has to be dark enough that the
// panel reads as the foreground and light enough that the plugin underneath
// is still legible as context.
constexpr float kScrimAlpha = 0.88f;
} // namespace

CryptPanel::CryptPanel (juce::AudioProcessorValueTreeState& state,
                        presets::PresetManager& manager)
    : apvts (state), presets (manager)
{
    setWantsKeyboardFocus (true);
    setInterceptsMouseClicks (true, true);

    presetList.setModel (this);
    presetList.setRowHeight (kRowHeight);
    presetList.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    presetList.setColour (juce::ListBox::outlineColourId, theme::rule);
    presetList.setOutlineThickness (1);
    addAndMakeVisible (presetList);

    bindButton.setTooltip ("Save the current voicing as a user preset");
    banishButton.setTooltip ("Delete the selected user preset. Factory presets cannot be banished.");
    closeButton.setTooltip ("Close The Crypt (Esc)");

    bindButton  .onClick = [this] { bindSigil(); };
    banishButton.onClick = [this] { banish(); };
    closeButton .onClick = [this] { if (onDismiss) onDismiss(); };

    addAndMakeVisible (bindButton);
    addAndMakeVisible (banishButton);
    addAndMakeVisible (closeButton);

    buildAdvancedControls();
    refreshPresetList();
}

CryptPanel::~CryptPanel()
{
    presetList.setModel (nullptr);
}

//==============================================================================
void CryptPanel::buildAdvancedControls()
{
    // No setTextValueSuffix here: each parameter already carries its own
    // unit through the APVTS attributes, and setting both prints it twice.
    const auto configureSlider = [this] (juce::Slider& slider,
                                         const juce::String& paramId,
                                         const juce::String& tooltip)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, kValueColumn, kAdvancedRowH - 8);
        slider.setTooltip (tooltip);
        slider.setColour (juce::Slider::textBoxTextColourId, theme::inkBody);
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);

        sliderAttachments.push_back (
            std::make_unique<SliderAttachment> (apvts, paramId, slider));
    };

    configureSlider (focusSlider, params::detectorFocus,
                     "Aims the detection threshold across the four bands. "
                     "Below 50 favours BITE and PLASTIC, above 50 favours WASP and ICE. "
                     "50 is untilted.");
    configureSlider (clampSlider, params::clampSpeed,
                     "How fast the reducer clamps once a band crosses the threshold.");
    configureSlider (reapSlider, params::maxReapDb,
                     "The hard ceiling on how much any one band may be reduced.");
    configureSlider (pickSlider, params::pickWindow,
                     "How long a detected pick attack holds the reducer off.");

    stereoBox.addItem ("LINKED",    1);
    stereoBox.addItem ("PARTIAL",   2);
    stereoBox.addItem ("DUAL MONO", 3);
    stereoBox.setJustificationType (juce::Justification::centredLeft);
    stereoBox.setTooltip ("Whether the two channels share one detector, half share it, or run independently.");
    addAndMakeVisible (stereoBox);
    stereoAttachment = std::make_unique<ComboBoxAttachment> (apvts, params::stereoLink, stereoBox);

    autoGainButton.setTooltip ("Makes up the level the reducer took away, up to 6 dB.");
    uiAnimationButton.setTooltip ("Off freezes every on-screen instrument on its last real value. "
                                  "It does not change the audio.");
    addAndMakeVisible (autoGainButton);
    addAndMakeVisible (uiAnimationButton);

    toggleAttachments.push_back (
        std::make_unique<ButtonAttachment> (apvts, params::autoGain, autoGainButton));
    toggleAttachments.push_back (
        std::make_unique<ButtonAttachment> (apvts, params::uiAnimation, uiAnimationButton));

    advancedRows = {
        { "DETECTOR FOCUS",  &focusSlider },
        { "CLAMP SPEED",     &clampSlider },
        { "MAX REAP",        &reapSlider },
        { "PICK WINDOW",     &pickSlider },
        { "STEREO BEHAVIOR", &stereoBox },
        { "",                &autoGainButton },
        { "",                &uiAnimationButton }
    };
}

//==============================================================================
void CryptPanel::refreshPresetList()
{
    userNames = presets.userPresetNames();

    const auto current = presets.currentPresetName();
    selectedRow = -1;

    if (current.isNotEmpty())
        for (int row = 0; row < getNumRows(); ++row)
            if (nameForRow (row) == current)
            {
                selectedRow = row;
                break;
            }

    presetList.updateContent();

    if (selectedRow >= 0)
        presetList.selectRow (selectedRow, true, true);
    else
        presetList.deselectAllRows();

    banishButton.setEnabled (selectedRow >= 0 && ! rowIsFactory (selectedRow));
    repaint();
}

bool CryptPanel::rowIsFactory (int row) const
{
    return row >= 0 && row < presets::PresetManager::numFactoryPresets();
}

juce::String CryptPanel::nameForRow (int row) const
{
    if (rowIsFactory (row))
        return presets::PresetManager::factoryName (row);

    const int userIndex = row - presets::PresetManager::numFactoryPresets();
    return juce::isPositiveAndBelow (userIndex, userNames.size())
        ? userNames[userIndex]
        : juce::String();
}

int CryptPanel::getNumRows()
{
    return presets::PresetManager::numFactoryPresets() + userNames.size();
}

void CryptPanel::paintListBoxItem (int row, juce::Graphics& g,
                                   int width, int height, bool selected)
{
    const auto name = nameForRow (row);
    if (name.isEmpty())
        return;

    // The boundary between the shipped twelve and the user's own is a
    // hairline, not a header row: it costs one pixel instead of a whole row.
    const int firstUserRow = presets::PresetManager::numFactoryPresets();
    if (row == firstUserRow)
    {
        g.setColour (theme::rule);
        g.fillRect (0, 0, width, 1);
    }

    g.setColour (selected ? theme::inkPrimary : theme::inkBody);
    g.setFont (theme::Fonts::body (13.0f, selected));
    g.drawText (name, juce::Rectangle<int> (10, 0, width - 56, height),
                juce::Justification::centredLeft, true);

    if (! rowIsFactory (row))
    {
        g.setColour (theme::inkMeta);
        g.setFont (theme::Fonts::monoLabel (9.0f));
        g.drawText ("USER", juce::Rectangle<int> (width - 46, 0, 38, height),
                    juce::Justification::centredRight, false);
    }
}

void CryptPanel::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    selectedRow = row;
    banishButton.setEnabled (row >= 0 && ! rowIsFactory (row));
}

void CryptPanel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    loadRow (row);
}

void CryptPanel::loadRow (int row)
{
    if (rowIsFactory (row))
        presets.loadFactory (row);
    else
        presets.loadUser (nameForRow (row));

    selectedRow = row;
    banishButton.setEnabled (row >= 0 && ! rowIsFactory (row));
    repaint();
}

//==============================================================================
void CryptPanel::bindSigil()
{
    nameWindow = std::make_unique<juce::AlertWindow> (
        "BIND SIGIL",
        "Name this voicing. Saving over an existing user preset replaces it.",
        juce::MessageBoxIconType::NoIcon,
        this);

    nameWindow->addTextEditor ("name", presets.currentPresetName(), "Preset name");
    nameWindow->addButton ("BIND",   1, juce::KeyPress (juce::KeyPress::returnKey));
    nameWindow->addButton ("CANCEL", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    nameWindow->enterModalState (true, juce::ModalCallbackFunction::create (
        [this] (int result)
        {
            const auto name = nameWindow != nullptr
                ? nameWindow->getTextEditorContents ("name").trim()
                : juce::String();

            nameWindow.reset();

            if (result == 0 || name.isEmpty())
                return;

            if (presets::PresetManager::isFactoryName (name))
            {
                juce::NativeMessageBox::showAsync (
                    juce::MessageBoxOptions()
                        .withIconType (juce::MessageBoxIconType::NoIcon)
                        .withTitle ("BIND SIGIL")
                        .withMessage ("That name belongs to a factory preset. Pick another.")
                        .withButton ("OK")
                        .withAssociatedComponent (this),
                    nullptr);
                return;
            }

            if (presets.saveUser (name))
                refreshPresetList();
        }), false);
}

void CryptPanel::banish()
{
    if (selectedRow < 0 || rowIsFactory (selectedRow))
        return;

    const auto name = nameForRow (selectedRow);
    if (name.isEmpty())
        return;

    juce::NativeMessageBox::showOkCancelBox (
        juce::MessageBoxIconType::NoIcon,
        "BANISH",
        "Delete the preset \"" + name + "\"? This removes it from disk.",
        this,
        juce::ModalCallbackFunction::create (
            [this, name] (int result)
            {
                if (result == 0)
                    return;

                if (presets.deleteUser (name))
                {
                    selectedRow = -1;
                    refreshPresetList();
                }
            }));
}

//==============================================================================
juce::Rectangle<int> CryptPanel::panelBounds() const
{
    return getLocalBounds().reduced (kPanelMarginX, kPanelMarginY);
}

bool CryptPanel::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        if (onDismiss)
            onDismiss();
        return true;
    }

    if (key == juce::KeyPress::returnKey && selectedRow >= 0)
    {
        loadRow (selectedRow);
        return true;
    }

    return false;
}

void CryptPanel::mouseDown (const juce::MouseEvent& event)
{
    // A click on the scrim, outside the panel, leaves. A click inside the
    // panel but not on a control does nothing.
    if (! panelBounds().contains (event.getPosition()) && onDismiss)
        onDismiss();
}

//==============================================================================
void CryptPanel::paint (juce::Graphics& g)
{
    g.fillAll (theme::canvas.withAlpha (kScrimAlpha));

    const auto panel = panelBounds();
    g.setColour (theme::canvas);
    g.fillRect (panel);
    g.setColour (theme::rule);
    g.drawRect (panel, 1);

    auto header = panel.reduced (kPadding, 0).withHeight (kHeaderHeight);

    g.setColour (theme::inkPrimary);
    g.setFont (theme::Fonts::display (16.0f, 0.14f));
    g.drawText ("THE CRYPT", header, juce::Justification::centredLeft, false);

    g.setColour (theme::rule);
    g.fillRect (panel.getX(), panel.getY() + kHeaderHeight, panel.getWidth(), 1);

    // Column headings.
    auto body = panel.reduced (kPadding, 0)
                     .withTrimmedTop (kHeaderHeight + kPadding)
                     .withTrimmedBottom (kPadding);

    const int listWidth = juce::roundToInt (static_cast<float> (body.getWidth()) * 0.38f);

    g.setColour (theme::inkMeta);
    g.setFont (theme::Fonts::monoLabel (10.0f));
    g.drawText ("PRESETS", body.withWidth (listWidth).withHeight (16),
                juce::Justification::topLeft, false);
    g.drawText ("ADVANCED",
                body.withTrimmedLeft (listWidth + kPadding).withHeight (16),
                juce::Justification::topLeft, false);

    // Advanced row labels, drawn here so the rows stay plain Components.
    auto advanced = body.withTrimmedLeft (listWidth + kPadding).withTrimmedTop (24);

    g.setFont (theme::Fonts::monoLabel (10.0f));
    for (const auto& row : advancedRows)
    {
        auto strip = advanced.removeFromTop (kAdvancedRowH);
        if (row.label.isNotEmpty())
        {
            g.setColour (theme::inkMeta);
            g.drawText (row.label, strip.withWidth (kLabelColumn),
                        juce::Justification::centredLeft, false);
        }
        advanced.removeFromTop (6);
    }
}

void CryptPanel::resized()
{
    const auto panel = panelBounds();

    closeButton.setBounds (panel.getRight() - kPadding - 72,
                           panel.getY() + (kHeaderHeight - kActionHeight) / 2,
                           72, kActionHeight);

    auto body = panel.reduced (kPadding, 0)
                     .withTrimmedTop (kHeaderHeight + kPadding)
                     .withTrimmedBottom (kPadding);

    const int listWidth = juce::roundToInt (static_cast<float> (body.getWidth()) * 0.38f);

    auto left = body.withWidth (listWidth).withTrimmedTop (24);
    auto actions = left.removeFromBottom (kActionHeight);
    left.removeFromBottom (12);
    presetList.setBounds (left);

    const int actionWidth = (actions.getWidth() - 8) / 2;
    bindButton  .setBounds (actions.removeFromLeft (actionWidth));
    actions.removeFromLeft (8);
    banishButton.setBounds (actions.removeFromLeft (actionWidth));

    auto advanced = body.withTrimmedLeft (listWidth + kPadding).withTrimmedTop (24);

    for (const auto& row : advancedRows)
    {
        auto strip = advanced.removeFromTop (kAdvancedRowH);

        if (row.label.isEmpty())
        {
            // Toggles carry their own text, so they start at the label column.
            row.control->setBounds (strip);
        }
        else if (row.control == &stereoBox)
        {
            row.control->setBounds (strip.withTrimmedLeft (kLabelColumn)
                                         .withWidth (140)
                                         .reduced (0, 3));
        }
        else
        {
            row.control->setBounds (strip.withTrimmedLeft (kLabelColumn));
        }

        advanced.removeFromTop (6);
    }
}
} // namespace cabrot::ui
