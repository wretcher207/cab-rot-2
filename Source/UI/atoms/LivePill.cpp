#include "LivePill.h"
#include "../../Theme/Fonts.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
LivePill::LivePill()
{
    setInterceptsMouseClicks (false, false);
}

LivePill::~LivePill()
{
    // Defensive: juce::Timer's own destructor stops timing, but it does so
    // AFTER LivePill's destructor body has run. If the timer happens to
    // dispatch on the message thread between subclass tear-down and base
    // tear-down, the vtable slot for timerCallback already points at junk.
    // Stop here while the vtable is still complete.
    stopTimer();
}

void LivePill::visibilityChanged()
{
    if (isShowing())
        startTimerHz (30);
    else
        stopTimer();
}

void LivePill::timerCallback()
{
    pulsePhase += 1.0f / 90.0f; // 3 second cycle, quiet atmosphere motion
    if (pulsePhase > 1.0f)
        pulsePhase -= 1.0f;
    repaint();
}

void LivePill::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();

    // Live state mark: a small square in stateLive, quietly breathing
    // between 40 and 100 percent. This, and the footer meters, are the
    // only places state colour appears outside the damage threshold.
    const float pulse = 0.70f + 0.30f * std::sin (pulsePhase * juce::MathConstants<float>::twoPi);
    const float side  = 7.0f;
    const auto  centre = juce::Point<float> (area.getX() + side + 2.0f, area.getCentreY());

    g.setColour (theme::stateLive.withAlpha (pulse));
    g.fillRect (juce::Rectangle<float> (side, side).withCentre (centre));

    auto labelArea = area.withTrimmedLeft (side * 2.0f + 10.0f);
    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.0f));
    g.drawText  ("LIVE", labelArea, juce::Justification::centredLeft, false);
}
} // namespace cabrot::ui
