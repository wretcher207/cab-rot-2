#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "atoms/DpdMark.h"
#include "atoms/GhostToggle.h"
#include "atoms/LivePill.h"

namespace cabrot::ui
{
// 64 px tall header band per CANONICAL-UI §7.1. Composes:
//   left:  CAB ROT wordmark, divider, DPD mark, "BY DEAD PIXEL DESIGN"
//   right: CPU label/value, LIVE pill, Delta Listen ghost toggle.

class HeaderBar final : public juce::Component
{
public:
    HeaderBar();
    ~HeaderBar() override = default;

    void paint   (juce::Graphics&) override;
    void resized() override;

    void setEngineLive (bool live)          { livePill.setLive (live); }
    void setDeltaAvailable (bool available) { ghost.setVisible (available); }

    GhostToggle& getDeltaToggle() noexcept { return ghost; }

private:
    DpdMark      dpdMark;
    LivePill     livePill;
    GhostToggle  ghost;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderBar)
};
} // namespace cabrot::ui
