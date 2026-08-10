#pragma once

namespace cabrot::dsp
{
/**
    Phase 5 amp-profile derivatives.

    These are provisional starting points, not release voicings. Every mode
    still needs to be validated by ear against representative guitar tracks.
*/
struct ModeConfig
{
    float thresholdOffsetDb;
    float ceilingScale;
    float attackScale;
    float edgeBias;
    float shelfStart;
};

inline constexpr int kNumModeConfigs = 6;

// Index order must stay aligned with the AudioParameterChoice in
// PluginProcessor.cpp: 5150, Recto, HM-2, Djent, Blackened, Sludge.
static constexpr ModeConfig kModes[kNumModeConfigs] {
    {  0.0f, 1.00f, 1.0f,  0.00f, 0.50f },
    { -2.0f, 1.15f, 0.8f,  0.00f, 0.50f },
    { -1.0f, 1.30f, 0.9f, -0.10f, 0.50f },
    { -1.0f, 1.10f, 0.6f,  0.15f, 0.55f },
    { -3.0f, 1.25f, 1.3f,  0.00f, 0.45f },
    {  0.0f, 0.90f, 1.6f, -0.10f, 0.35f }
};

inline constexpr int kHm2ModeIndex = 2;
inline constexpr int kWaspProcessedBandIndex = 2;
inline constexpr float kHm2WaspCeilingScale = 1.50f;
} // namespace cabrot::dsp
