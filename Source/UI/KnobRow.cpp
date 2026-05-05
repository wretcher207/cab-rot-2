#include "KnobRow.h"

#include "../Theme/Palette.h"
#include "../Theme/SpectreLookAndFeel.h"

namespace cabrot::ui
{
namespace
{
struct KnobSeed { juce::String label; double value; };

// PHASE 2 PLACEHOLDER: knob values mirror the Stitch reference. Phase 3
// replaces with APVTS bindings via SliderAttachment.
constexpr int kSeedCount = KnobRow::kNumKnobs;
const std::array<KnobSeed, kSeedCount> kSeeds {{
    { "FIZZ HUNT",     62.0 },
    { "EDGE PRESERVE", 45.0 },
    { "CAB SMOOTH",    35.0 },
    { "DIGITAL SAND",  55.0 },
    { "AIR ROT",       40.0 },
    { "REAP MIX",      50.0 },
}};
}

KnobRow::KnobRow()
{
    knobs.reserve (kNumKnobs);
    for (const auto& seed : kSeeds)
    {
        auto knob = std::make_unique<SpectreKnob> (seed.label);
        knob->getSlider().setValue (seed.value, juce::dontSendNotification);
        knob->setValueDisplay (seed.value);
        addAndMakeVisible (*knob);
        knobs.push_back (std::move (knob));
    }
}

void KnobRow::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour (theme::outlineVariant);
    g.fillRect (area.removeFromTop (1));
    g.setColour (theme::surfaceContainer.withAlpha (0.6f));
    g.fillRect (area);

    theme::SpectreLookAndFeel::drawScanlines (g, area, 0.10f, 4);
}

void KnobRow::resized()
{
    auto area = getLocalBounds().reduced (32, 16);
    const int cellW = area.getWidth() / kNumKnobs;

    for (int i = 0; i < kNumKnobs; ++i)
    {
        const int x = area.getX() + i * cellW;
        knobs[(size_t) i]->setBounds (x, area.getY(), cellW, area.getHeight());
    }
}
} // namespace cabrot::ui
