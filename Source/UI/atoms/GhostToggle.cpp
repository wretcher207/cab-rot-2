#include "GhostToggle.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
GhostToggle::GhostToggle()
    : juce::Button ("delta-listen")
{
    setClickingTogglesState (true);
    setTooltip ("Delta Listen: hear what the plugin is removing");
}

void GhostToggle::paintButton (juce::Graphics& g, bool isHighlighted, bool)
{
    const auto area = getLocalBounds().toFloat().reduced (2.0f);
    const bool active = getToggleState();

    // The ghost is discovered, never announced: disabled-grey at rest,
    // primary ink while listening. No halo, no tint, no emphasis.
    const auto colour = active ? theme::inkPrimary
                        : (isHighlighted ? theme::inkBody
                                         : theme::inkDisabled);

    juce::Path p;
    const float w = area.getWidth();
    const float h = area.getHeight();
    const float top = area.getY();
    const float left = area.getX();

    // Round-topped silhouette with three "tail" bumps along the bottom.
    p.startNewSubPath (left, top + h * 0.55f);
    p.cubicTo (left,           top - h * 0.10f,
               left + w,       top - h * 0.10f,
               left + w,       top + h * 0.55f);
    p.lineTo (left + w,                top + h);
    p.lineTo (left + w * 0.83f,        top + h * 0.78f);
    p.lineTo (left + w * 0.66f,        top + h);
    p.lineTo (left + w * 0.50f,        top + h * 0.78f);
    p.lineTo (left + w * 0.33f,        top + h);
    p.lineTo (left + w * 0.17f,        top + h * 0.78f);
    p.lineTo (left,                    top + h);
    p.closeSubPath();

    g.setColour (colour);
    g.strokePath (p, juce::PathStrokeType (1.4f));

    // Two eye-holes
    const float eyeR = juce::jmin (w, h) * 0.06f;
    g.fillEllipse (juce::Rectangle<float> (
        left + w * 0.32f - eyeR, top + h * 0.34f - eyeR,
        eyeR * 2.0f, eyeR * 2.0f));
    g.fillEllipse (juce::Rectangle<float> (
        left + w * 0.62f - eyeR, top + h * 0.34f - eyeR,
        eyeR * 2.0f, eyeR * 2.0f));
}
} // namespace cabrot::ui
