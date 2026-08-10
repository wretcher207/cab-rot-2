// Hand-maintained colour system for Cab Rot, transcribed from the Dead Pixel
// Design brand kit (dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md).
//
// The earlier Stitch palette and its generator (tools/oklch-to-srgb.py) were
// retired on 2026-08-10. This file is owned by hand; the brand brief is the
// binding document.
//
// Rules that apply to every consumer of these tokens:
//   - No colour literal may appear anywhere else in Source/.
//   - stateLive appears only where the plugin is genuinely live and
//     processing (signal present, harness running).
//   - stateError appears only where the plugin is past the damage point:
//     the canonical threshold is kReductionDamageThresholdDb below, 12 dB
//     of gain reduction in any detection band.
//   - Everything else is monochrome. Hierarchy comes from type size,
//     spacing, alignment and hairlines, never from filled panels.

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace cabrot::theme
{

inline const juce::Colour canvas       { juce::uint32 (0xFF060606) };  // window ground
inline const juce::Colour surface1     { juce::uint32 (0xFF0B0B0B) };  // analysis ground, header strip
inline const juce::Colour surface2     { juce::uint32 (0xFF101010) };  // raised strip
inline const juce::Colour surface3     { juce::uint32 (0xFF151514) };  // quiet fills (meter tracks)
inline const juce::Colour inkPrimary   { juce::uint32 (0xFFF2F2EF) };  // wordmark, data, active control ink
inline const juce::Colour inkBody      { juce::uint32 (0xFFB4B4B0) };  // control values, body copy
inline const juce::Colour inkMeta      { juce::uint32 (0xFF8A8A85) };  // scales, labels, unselected text
inline const juce::Colour inkDisabled  { juce::uint32 (0xFF4A4A47) };  // idle ghosts, disabled controls
inline const juce::Colour rule         { juce::uint32 (0xFF2E2E2C) };  // every 1px hairline
inline const juce::Colour stateLive    { juce::uint32 (0xFF7FA57A) };  // live state only
inline const juce::Colour stateError   { juce::uint32 (0xFFC4574C) };  // past the damage threshold only

// Gain reduction past 12 dB in any single band reads as damage, not
// treatment. The spectral display renders those deltas in stateError.
// Lockstep with the WaspMeter dB scale (-24 .. 0 dB range).
inline constexpr float kReductionDamageThresholdDb = 12.0f;
inline constexpr float kReductionScaleMaxDb        = 24.0f;

} // namespace cabrot::theme
