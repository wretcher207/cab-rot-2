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
}

void HeaderBar::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds();

    g.setColour (theme::surfaceContainerLow.withAlpha (0.90f));
    g.fillRect (area);
    g.setColour (theme::outlineVariant);
    g.fillRect (area.withY (area.getBottom() - 1).withHeight (1));

    auto inner = area.reduced (20, 0);

    // Left cluster
    auto leftBlock = inner.removeFromLeft (480);

    auto wordmarkArea = leftBlock.removeFromLeft (140);
    g.setColour (theme::toxic);
    auto wmFont = theme::Fonts::displayTitleScaled (1.18f).withExtraKerningFactor (-0.04f);
    g.setFont (wmFont);
    g.drawText ("CAB ROT", wordmarkArea, juce::Justification::centredLeft, false);

    g.setColour (theme::outlineVariant);
    g.fillRect (juce::Rectangle<int> (leftBlock.getX(), leftBlock.getCentreY() - 8, 1, 16));
    leftBlock.removeFromLeft (16);

    // DPD mark - laid out via setBounds in resized(), text continues from
    // the next chunk over.
    leftBlock.removeFromLeft (24);
    g.setColour (theme::mutedForeground);
    g.setFont   (theme::Fonts::uiChrome());
    g.drawText  ("BY DEAD PIXEL DESIGN", leftBlock, juce::Justification::centredLeft, false);

    // Right cluster
    auto rightBlock = inner.removeFromRight (320);
    rightBlock.removeFromRight (40);                        // GhostToggle reserves ~32 px
    rightBlock.removeFromRight (16);
    auto liveArea = rightBlock.removeFromRight (96);
    juce::ignoreUnused (liveArea);                          // positioned in resized()
    rightBlock.removeFromRight (24);                        // gap

    auto cpuValueArea = rightBlock.removeFromRight (52);
    rightBlock.removeFromRight (6);
    auto cpuLabelArea = rightBlock.removeFromRight (36);

    g.setFont   (theme::Fonts::uiChrome());
    g.setColour (theme::mutedForeground);
    g.drawText  ("CPU", cpuLabelArea, juce::Justification::centredRight, false);

    g.setFont   (theme::Fonts::monoData());
    g.setColour (theme::toxic);
    g.drawText  (cpuValue, cpuValueArea, juce::Justification::centredLeft, false);
}

void HeaderBar::resized()
{
    const auto inner = getLocalBounds().reduced (20, 0);

    // Mark sits 16x16 in the left cluster, 156 px from the left edge of inner
    // (140 wordmark + 16 divider gap).
    const int markSize = 16;
    const int markX    = inner.getX() + 140 + 16;
    const int markY    = inner.getCentreY() - markSize / 2;
    dpdMark.setBounds (markX, markY, markSize, markSize);

    // LIVE pill in the right cluster
    const int liveW = 96;
    const int liveH = 28;
    const int ghostW = 28;

    const int rightEdge  = inner.getRight();
    const int ghostX     = rightEdge - ghostW;
    const int ghostY     = inner.getCentreY() - ghostW / 2;
    ghost.setBounds (ghostX, ghostY, ghostW, ghostW);

    const int liveX = ghostX - 16 - liveW;
    const int liveY = inner.getCentreY() - liveH / 2;
    livePill.setBounds (liveX, liveY, liveW, liveH);
}
} // namespace cabrot::ui
