#include "ModeButton.h"
#include "../../Theme/Fonts.h"
#include "../../Theme/Palette.h"

namespace cabrot::ui
{
ModeButton::ModeButton (juce::String name)
    : juce::TextButton (std::move (name))
{
    setClickingTogglesState (true);
}

void ModeButton::paintButton (juce::Graphics& g, bool isHighlighted, bool isDown)
{
    if (auto* lf = &getLookAndFeel())
    {
        // Background (the hairline outline) comes from the LookAndFeel so
        // the styling stays centralised. We only own the label render here.
        lf->drawButtonBackground (g, *this,
                                  findColour (juce::TextButton::buttonColourId),
                                  isHighlighted, isDown);
    }

    const bool active = getToggleState();
    g.setFont   (theme::Fonts::monoLabel (11.0f));
    g.setColour (active ? theme::inkPrimary
                 : (isHighlighted || isDown) ? theme::inkBody
                                             : theme::inkMeta);
    g.drawText  (getButtonText(), getLocalBounds(), juce::Justification::centred, false);
}
} // namespace cabrot::ui
