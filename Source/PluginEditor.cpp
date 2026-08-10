#include "PluginEditor.h"

#include "Theme/Palette.h"
#include "Theme/SpectreLookAndFeel.h"

#include <algorithm>
#include <cmath>

namespace cabrot
{
namespace
{
constexpr int    kDefaultWidth  = 1200;
constexpr int    kDefaultHeight = 780;
constexpr double kAspectRatio   = static_cast<double> (kDefaultWidth)
                                / static_cast<double> (kDefaultHeight);

constexpr int kHeaderHeight = 64;
constexpr int kKnobsHeight  = 192;
constexpr int kFooterHeight = 48;
constexpr int kMainPad      = 24;
constexpr int kRightColumnW = 320;
constexpr int kFizzCardH    = 192;
constexpr int kCardGap      = 24;

int scaleH (int reference, float scale, int floor)
{
    return juce::jmax (floor, juce::roundToInt (static_cast<float> (reference) * scale));
}
}

CabRotEditor::CabRotEditor (CabRotProcessor& p)
    : juce::AudioProcessorEditor (&p),
      processorRef (p),
      lookAndFeel (std::make_unique<theme::SpectreLookAndFeel>())
{
    setLookAndFeel (lookAndFeel.get());

    addAndMakeVisible (headerBar);
    addAndMakeVisible (waspMeter);
    addAndMakeVisible (fizzReadout);
    addAndMakeVisible (ampProfile);
    ampProfile.setVisible (false);
    addAndMakeVisible (knobRow);
    addAndMakeVisible (footerBar);

    wireAttachments();

    addKeyListener (this);
    setWantsKeyboardFocus (true);

    setResizable (true, true);
    setResizeLimits (1000, 650, 1600, 1040);
    getConstrainer()->setFixedAspectRatio (kAspectRatio);
    setSize (kDefaultWidth, kDefaultHeight);
}

CabRotEditor::~CabRotEditor()
{
    stopTimer();
    uiAnimationAttachment.reset();
    removeKeyListener (this);
    setLookAndFeel (nullptr);
}

void CabRotEditor::wireAttachments()
{
    auto& av = apvts();

    // Six main knobs - the order in KnobRow matches the param order.
    const std::array<juce::String, 6> knobIds {
        params::fizzHunt, params::edgePreserve, params::cabSmooth,
        params::digitalSand, params::airRot, params::reapMix
    };
    knobAttachments.reserve (knobIds.size());
    for (size_t i = 0; i < knobIds.size(); ++i)
    {
        auto& slider = knobRow.getKnob (static_cast<int> (i)).getSlider();
        knobAttachments.push_back (
            std::make_unique<SliderAttachment> (av, knobIds[i], slider));
    }

    // Mode buttons -> Choice parameter. ParameterAttachment's lambda fires
    // both on host writes (automation, undo) and on local UI changes;
    // dontSendNotification on setToggleState avoids feedback loops.
    auto* modeParam = av.getParameter (params::mode);
    jassert (modeParam != nullptr);

    for (int i = 0; i < ui::AmpProfileGrid::kNumModes; ++i)
    {
        auto& button = ampProfile.getModeButton (i);
        const int myIndex = i;

        auto attachment = std::make_unique<juce::ParameterAttachment> (
            *modeParam,
            [&button, myIndex] (float v)
            {
                const bool isSelected = juce::roundToInt (v) == myIndex;
                if (button.getToggleState() != isSelected)
                    button.setToggleState (isSelected, juce::dontSendNotification);
            });

        attachment->sendInitialUpdate();
        modeAttachments.push_back (std::move (attachment));

        button.onClick = [&button, modeParam, myIndex]
        {
            if (button.getToggleState())
            {
                const auto normalised = modeParam->convertTo0to1 (static_cast<float> (myIndex));
                modeParam->beginChangeGesture();
                modeParam->setValueNotifyingHost (normalised);
                modeParam->endChangeGesture();
            }
        };
    }

    // Boolean toggles
    deltaAttachment = std::make_unique<ButtonAttachment> (
        av, params::deltaListen, headerBar.getDeltaToggle());

    // A/B is one bool with two visual buttons. JUCE's ButtonAttachment can
    // only own one button, and the radio group only auto-deselects when a
    // button is turned ON (not when one is turned OFF) - so a host write of
    // aOrB=false would leave both A and B dark. Drive both buttons from a
    // single ParameterAttachment instead.
    auto* abParam = av.getParameter (params::aOrB);
    jassert (abParam != nullptr);

    auto& aButton = footerBar.getButtonA();
    auto& bButton = footerBar.getButtonB();

    abAttachment = std::make_unique<juce::ParameterAttachment> (
        *abParam,
        [&aButton, &bButton] (float v)
        {
            const bool isB = v >= 0.5f;
            if (aButton.getToggleState() == isB)
                aButton.setToggleState (! isB, juce::dontSendNotification);
            if (bButton.getToggleState() != isB)
                bButton.setToggleState (isB,   juce::dontSendNotification);
        });
    abAttachment->sendInitialUpdate();

    aButton.onClick = [&aButton, abParam]
    {
        if (aButton.getToggleState())
        {
            abParam->beginChangeGesture();
            abParam->setValueNotifyingHost (0.0f);
            abParam->endChangeGesture();
        }
    };
    bButton.onClick = [&bButton, abParam]
    {
        if (bButton.getToggleState())
        {
            abParam->beginChangeGesture();
            abParam->setValueNotifyingHost (1.0f);
            abParam->endChangeGesture();
        }
    };

    // Oversample combo
    osAttachment = std::make_unique<ComboBoxAttachment> (
        av, params::oversampling, footerBar.getOversampleBox());

    auto* animationParam = av.getParameter (params::uiAnimation);
    jassert (animationParam != nullptr);
    uiAnimationAttachment = std::make_unique<juce::ParameterAttachment> (
        *animationParam,
        [this] (float value)
        {
            setUiAnimationEnabled (value >= 0.5f);
        });
    uiAnimationAttachment->sendInitialUpdate();

    // Tooltips that refresh as the parameter changes. We rely on JUCE's
    // built-in juce::Slider::getTooltip override returning the slider's
    // setTooltip text; the SliderAttachment also routes parameter
    // formatting through param.getText() for popup-menu "Enter value..."
    // dialogs. No extra wiring needed.
    headerBar.getDeltaToggle().setTooltip ("Delta Listen: hear the removed signal");
    footerBar.getButtonA()     .setTooltip ("A/B: select snapshot A");
    footerBar.getButtonB()     .setTooltip ("A/B: select snapshot B");
    footerBar.getOversampleBox().setTooltip ("Oversampling factor");
}

void CabRotEditor::setUiAnimationEnabled (bool enabled)
{
    if (! enabled)
    {
        stopTimer();
        lastTimerMs = 0.0;
        return;
    }

    processorRef.discardUiPeakTelemetry();
    lastTimerMs = 0.0;
    startTimerHz (30);
}

void CabRotEditor::timerCallback()
{
    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    const float elapsedSeconds = lastTimerMs > 0.0
        ? juce::jlimit (0.0f, 0.25f, static_cast<float> ((nowMs - lastTimerMs) * 0.001))
        : (1.0f / 30.0f);
    lastTimerMs = nowMs;

    const auto telemetry = processorRef.consumeUiTelemetry();

    waspMeter.updateBandReduction (telemetry.bandReductionDb,
                                   telemetry.engineLive,
                                   elapsedSeconds);

    if (telemetry.engineLive)
    {
        const float maxBandReductionDb = *std::max_element (
            telemetry.bandReductionDb.begin(), telemetry.bandReductionDb.end());

        // fizzPct = smoothed( min(1.0, maxBandReductionDb /
        // kReductionDamageThresholdDb) ) * 100
        const float target01 = juce::jmin (
            1.0f, maxBandReductionDb / theme::kReductionDamageThresholdDb);

        if (! fizzSmoothingSeeded)
        {
            fizzSmoothed01 = target01;
            fizzSmoothingSeeded = true;
        }
        else
        {
            constexpr float kSmoothingSeconds = 0.14f;
            const float alpha = 1.0f - std::exp (-elapsedSeconds / kSmoothingSeconds);
            fizzSmoothed01 += alpha * (target01 - fizzSmoothed01);
        }

        fizzReadout.setValue (fizzSmoothed01 * 100.0f);
    }
    else
    {
        fizzSmoothingSeeded = false;
        fizzSmoothed01 = 0.0f;
        fizzReadout.setValue (std::nullopt);
    }

    if (telemetry.outputPeak >= 1.0f)
        clipHoldSeconds = 1.5f;
    else
        clipHoldSeconds = juce::jmax (0.0f, clipHoldSeconds - elapsedSeconds);

    const bool clipping = clipHoldSeconds > 0.0f;
    footerBar.setLevels (telemetry.inputPeak, telemetry.outputPeak,
                         elapsedSeconds, clipping);
    footerBar.setStatusText (clipping ? "CLIPPING"
                                     : telemetry.engineLive ? "PROCESSING"
                                                            : "IDLE");

    headerBar.setEngineLive (telemetry.engineLive);
    if (telemetry.cpuPercent >= 0.0f)
        headerBar.setCpuPercent (telemetry.cpuPercent);
    else
        headerBar.setCpuPercent (std::nullopt);
}

bool CabRotEditor::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    auto& um = processorRef.getUndoManager();
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier, 0))
    {
        um.undo();
        return true;
    }
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier
                                   | juce::ModifierKeys::shiftModifier, 0)
        || key == juce::KeyPress ('y', juce::ModifierKeys::commandModifier, 0))
    {
        um.redo();
        return true;
    }
    return false;
}

void CabRotEditor::paint (juce::Graphics& g)
{
    // Flat brand canvas. No vignette, no gradient, no chrome: the spectral
    // display is the only element allowed to dominate.
    g.setColour (theme::canvas);
    g.fillRect (getLocalBounds());
}

void CabRotEditor::resized()
{
    auto bounds = getLocalBounds();

    const float scale = static_cast<float> (bounds.getHeight()) / static_cast<float> (kDefaultHeight);
    const int   header = scaleH (kHeaderHeight, scale, 52);
    const int   footer = scaleH (kFooterHeight, scale, 40);
    const int   knobs  = scaleH (kKnobsHeight,  scale, 152);
    const int   pad    = scaleH (kMainPad,      scale, 16);
    const int   gap    = scaleH (kCardGap,      scale, 12);
    const int   fizzH  = scaleH (kFizzCardH,    scale, 152);
    const int   rightW = scaleH (kRightColumnW, scale, 260);

    headerBar.setBounds (bounds.removeFromTop    (header));
    footerBar.setBounds (bounds.removeFromBottom (footer));
    knobRow .setBounds  (bounds.removeFromBottom (knobs));

    auto main = bounds.reduced (pad);

    auto rightColumn = main.removeFromRight (rightW);
    main.removeFromRight (gap);

    waspMeter.setBounds (main);

    fizzReadout.setBounds (rightColumn.removeFromTop (fizzH));
    rightColumn.removeFromTop (gap);
    ampProfile.setBounds (rightColumn);
}
} // namespace cabrot
