#include "HeaderBar.h"

#include "../Theme/Fonts.h"
#include "../Theme/Palette.h"

namespace cabrot::ui
{
HeaderBar::HeaderBar()
{
    addAndMakeVisible (dpdMark);
    addAndMakeVisible (livePill);
    addAndMakeVisible (ghost);
    ghost.setVisible (false);
}

void HeaderBar::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds();

    g.setColour (theme::canvas);
    g.fillRect (area);
    g.setColour (theme::rule);
    g.fillRect (area.withY (area.getBottom() - 1).withHeight (1));

    auto inner = area.reduced (24, 0);

    // Left cluster
    auto leftBlock = inner.removeFromLeft (520);

    auto wordmarkArea = leftBlock.removeFromLeft (170);
    g.setColour (theme::inkPrimary);
    g.setFont   (theme::Fonts::wordmarkScaled (getHeight() / 64.0f));
    g.drawText  ("CAB ROT", wordmarkArea, juce::Justification::centredLeft, false);

    g.setColour (theme::rule);
    g.fillRect (juce::Rectangle<int> (leftBlock.getX(), leftBlock.getCentreY() - 8, 1, 16));
    leftBlock.removeFromLeft (16);

    // DPD mark - laid out via setBounds in resized(), text continues from
    // the next chunk over.
    leftBlock.removeFromLeft (24);
    g.setColour (theme::inkMeta);
    g.setFont   (theme::Fonts::monoLabel (10.5f));
    g.drawText  ("DEAD PIXEL HARMONIX", leftBlock, juce::Justification::centredLeft, false);

}

void HeaderBar::resized()
{
    const auto inner = getLocalBounds().reduced (24, 0);

    // Mark sits 16x16 in the left cluster after the wordmark and divider.
    const int markSize = 16;
    const int markX    = inner.getX() + 170 + 16;
    const int markY    = inner.getCentreY() - markSize / 2;
    dpdMark.setBounds (markX, markY, markSize, markSize);

    const int liveW = 72;
    const int liveH = 28;
    const int ghostW = 28;

    const int rightEdge  = inner.getRight();
    const int ghostX     = rightEdge - ghostW;
    const int ghostY     = inner.getCentreY() - ghostW / 2;
    ghost.setBounds (ghostX, ghostY, ghostW, ghostW);

    const int liveX = ghostX - 24 - liveW;
    const int liveY = inner.getCentreY() - liveH / 2;
    livePill.setBounds (liveX, liveY, liveW, liveH);
}
} // namespace cabrot::ui
