#pragma once

#include <juce_core/juce_core.h>

namespace cabrot::presets
{
/**
    The twelve presets that ship with Cab Rot.

    A preset carries voicing and nothing else: the six front-panel controls,
    the amp profile, and the Crypt advanced values. It deliberately does not
    carry input or output trim (level matching is per session), oversampling
    (a CPU choice, and loading a preset should never quietly cost four times
    the CPU), Delta Listen (a monitoring mode), the A/B slot, or UI Animation.

    These are provisional starting points in the same sense the mode table is.
    They are built from the tuning ranges rather than from listening, and want
    David's ear before release.
*/
struct FactoryPreset
{
    const char* name;

    int   mode;           // 0 5150, 1 Recto, 2 HM-2, 3 Djent, 4 Blackened, 5 Sludge
    float fizzHunt;       // 0-100
    float edgePreserve;   // 0-100
    float cabSmooth;      // 0-100, BITE
    float digitalSand;    // 0-100, PLASTIC + WASP
    float airRot;         // 0-100, ICE
    float reapMix;        // 0-100

    float detectorFocus;  // 0-100, 50 is untilted
    float clampSpeed;     // 0.5-50 ms
    float maxReapDb;      // 1-12 dB
    float pickWindow;     // 1-30 ms
    int   stereoLink;     // 0 Linked, 1 Partial, 2 Dual Mono
    bool  autoGain;
};

inline constexpr int kNumFactoryPresets = 12;

inline constexpr FactoryPreset kFactoryPresets[kNumFactoryPresets] {
    // name                        mode fizz edge smth sand  air  mix  focus clamp  reap  pick  st  auto
    { "5150 Wasp Coffin",            0, 70,  55,  35,  80,  55, 100,  65.0f,  6.0f,  8.0f,  5.0f, 0, true },
    { "Plastic IR Burial",           0, 65,  45,  45,  85,  30, 100,  40.0f,  8.0f,  9.0f,  5.0f, 0, true },
    { "Recto Sandpaper Mercy",       1, 62,  50,  65,  70,  35,  95,  35.0f, 10.0f,  8.0f,  6.0f, 0, true },
    { "Djent Razor Tax",             3, 70,  85,  40,  75,  60, 100,  60.0f,  4.0f,  7.0f,  8.0f, 0, true },
    { "HM-2 Without Regret",         2, 75,  45,  20,  70,  75, 100,  70.0f,  7.0f, 10.0f,  5.0f, 0, true },
    { "Blackened Ice Removal",       4, 68,  40,  15,  55,  95, 100,  90.0f,  9.0f, 10.0f,  4.0f, 0, true },
    { "Sludge Blanket Lift",         5, 55,  55,  80,  40,  20,  85,  15.0f, 14.0f,  6.0f,  7.0f, 0, true },
    { "Deathcore Dentist",           3, 88,  65,  55,  95,  85, 100,  65.0f,  3.0f, 12.0f,  6.0f, 0, true },
    { "Bedroom Amp Sim Rescue",      0, 60,  60,  50,  60,  50,  80,  50.0f, 10.0f,  6.0f,  5.0f, 0, true },
    { "Bus Glue Fizz Net",           0, 45,  70,  25,  35,  30,  55,  55.0f, 16.0f,  4.0f,  8.0f, 1, true },
    { "Lead Guitar Glass Cage",      0, 58,  80,  30,  50,  70,  90,  75.0f, 12.0f,  6.0f, 10.0f, 0, true },
    { "Raw Demo Salvage",            1, 85,  50,  70,  85,  75, 100,  50.0f,  5.0f, 11.0f,  5.0f, 0, true }
};
} // namespace cabrot::presets
