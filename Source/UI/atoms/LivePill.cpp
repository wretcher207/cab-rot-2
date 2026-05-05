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
        startTimerHz (60);
    else
        stopTimer();
}

void LivePill::timerCallback()
{
    pulsePhase += 1.0f / 60.0f;
    if (pulsePhase > 1.0f)
        pulsePhase -= 1.0f;
    repaint();
}

void LivePill::paint (juce::Graphics& g)
{
    const auto area   = getLocalBounds().toFloat();
    const float radius = area.getHeight() * 0.5f;

    g.setColour (theme::surfaceContainer);
    g.fillRoundedRectangle (area, radius);
    g.setColour (theme::outlineVariant);
    g.drawRoundedRectangle (area, radius, 1.0f);

    const float pulse  = 0.55f + 0.45f * std::sin (pulsePhase * juce::MathConstants<float>::twoPi);
    const float dotRad = juce::jmin (area.getHeight() * 0.32f, 4.5f);
    const auto  centre = juce::Point<float> (area.getX() + radius, area.getCentreY());

    // Halo
    g.setColour (theme::toxic.withAlpha (0.18f * pulse));
    g.fillEllipse (juce::Rectangle<float> (dotRad * 4.0f, dotRad * 4.0f).withCentre (centre));
    g.setColour (theme::toxic.withAlpha (0.45f));
    g.fillEllipse (juce::Rectangle<float> (dotRad * 2.5f, dotRad * 2.5f).withCentre (centre));
    // Dot
    g.setColour (theme::toxic);
    g.fillEllipse (juce::Rectangle<float> (dotRad * 2.0f, dotRad * 2.0f).withCentre (centre));

    auto labelArea = area.withTrimmedLeft (radius * 2.0f + 4.0f).withTrimmedRight (radius * 0.5f);
    g.setColour (theme::toxic);
    g.setFont   (theme::Fonts::uiChrome());
    g.drawText  ("LIVE", labelArea, juce::Justification::centredLeft, false);
}
} // namespace cabrot::ui
