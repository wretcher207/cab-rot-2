#include "DpdMark.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
DpdMark::DpdMark()
{
    setOpaque (false);
    setInterceptsMouseClicks (false, false);
}

void DpdMark::paint (juce::Graphics& g)
{
    const auto area    = getLocalBounds().toFloat();
    const float stroke = juce::jmax (1.0f, area.getWidth() * 0.06f);
    const float offset = area.getWidth() * 0.14f;

    const auto outer = area.reduced (stroke * 0.5f);
    const auto inner = outer.translated (-offset, offset);

    g.setColour (theme::inkBody.withAlpha (0.70f));
    g.drawRect (outer, stroke * 0.7f);
    g.drawRect (inner, stroke * 0.55f);

    // The dead pixel: a single filled square near the upper-right corner.
    const float pix = area.getWidth() * 0.18f;
    g.setColour (theme::inkPrimary.withAlpha (0.92f));
    g.fillRect (juce::Rectangle<float> (
        outer.getRight() - pix - stroke,
        outer.getY() + pix * 0.4f,
        pix, pix));
}
} // namespace cabrot::ui
