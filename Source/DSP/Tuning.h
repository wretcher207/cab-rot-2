#pragma once

// Cab Rot's ear-tunable constants, all in one file.
//
// Phase 4 sets these to defensible starting values, not final ones. The
// character of the plugin lives here: where the bands sit, how fast the
// detectors move, how hard each knob is allowed to pull. Retune by ear
// against real high-gain material, rebuild, listen. Nothing outside this
// file needs to change to re-voice the plugin.

namespace cabrot::dsp::tuning
{
// ---------------------------------------------------------------------------
// Band layout
// ---------------------------------------------------------------------------
// Five crossover points carve the spectrum into six bands. Only the middle
// four are reduced; the outer two are split off anyway so the reconstruction
// stays whole.
//
//   band 0  LOW       < 2.4 kHz     untouched
//   band 1  BITE      2.4 - 3.8 kHz  Cab Smooth
//   band 2  PLASTIC   3.8 - 5.5 kHz  Digital Sand
//   band 3  WASP      5.5 - 8.0 kHz  Digital Sand
//   band 4  ICE       8.0 - 12  kHz  Air Rot
//   band 5  TOP       > 12 kHz       untouched
//
// The three interior splits (3.8 / 5.5 / 8.0) are the locked decision in
// CLAUDE.md. 2.4 and 12.0 are the outer bounds of the "wasp nest" region
// named in PLAN.md's band table.
inline constexpr int kNumCrossovers = 5;
inline constexpr int kNumBands      = kNumCrossovers + 1;

inline constexpr float kCrossoverHz[kNumCrossovers] = { 2400.0f, 3800.0f, 5500.0f, 8000.0f, 12000.0f };

inline constexpr int kFirstProcessedBand = 1; // BITE
inline constexpr int kLastProcessedBand  = 4; // ICE
inline constexpr int kNumProcessedBands  = kLastProcessedBand - kFirstProcessedBand + 1;

// ---------------------------------------------------------------------------
// Transient detector (the architectural differentiator)
// ---------------------------------------------------------------------------
// A fast envelope racing a slow one. When a pick attack lands, the fast
// envelope leaps ahead; that lead, in dB, is the transient signal. While it
// is above threshold the reducer is told to keep its hands off, so the
// attack survives and only the sustain gets clamped.
inline constexpr float kTransientFastAttackMs  = 0.20f;
inline constexpr float kTransientFastReleaseMs = 3.0f;
inline constexpr float kTransientSlowAttackMs  = 15.0f;
inline constexpr float kTransientSlowReleaseMs = 120.0f;

// Edge Preserve sweeps the threshold: at 0 the gate never opens, at 100 it
// opens on the faintest leading edge.
inline constexpr float kTransientThresholdMaxDb = 14.0f; // Edge Preserve = 0
inline constexpr float kTransientThresholdMinDb = 1.5f;  // Edge Preserve = 100
inline constexpr float kTransientKneeDb         = 6.0f;

// How fast the gate falls once the hold window expires.
inline constexpr float kTransientGateReleaseMs = 12.0f;

// ---------------------------------------------------------------------------
// Dynamic reducer
// ---------------------------------------------------------------------------
// Fizz Hunt sweeps the detection threshold. High-gain guitar sits around
// -40 to -25 dBFS in the 4-8 kHz region, so this range runs from "only the
// loudest fizz gets touched" to "almost everything does".
inline constexpr float kThresholdAtZeroDb    = -8.0f;  // Fizz Hunt = 0
inline constexpr float kThresholdAtHundredDb = -52.0f; // Fizz Hunt = 100

// Slope above threshold. 0.75 is a 4:1 equivalent.
inline constexpr float kReductionSlope = 0.75f;

// Reducer ballistics. Clamp Speed (Crypt) scales the attack; release is
// derived so fast settings do not chatter.
inline constexpr float kReducerReleaseRatio = 6.0f; // release = attack * this
inline constexpr float kReducerMinReleaseMs = 25.0f;

// Final anti-zipper smoothing on the applied gain. Short enough not to blunt
// the clamp, long enough to kill stepping on fast knob sweeps.
inline constexpr float kGainSmoothingMs = 0.6f;

// ---------------------------------------------------------------------------
// Air Rot's static shelf
// ---------------------------------------------------------------------------
// PLAN.md asks Air Rot for a "gentle low-pass shelf" on top of its dynamic
// reduction. It only engages in the upper half of the knob's travel, so the
// midpoint stays honest and the top of the range genuinely rots the air.
inline constexpr float kAirRotShelfStart     = 0.5f; // knob position where it begins
inline constexpr float kAirRotShelfMaxCutDb  = 2.5f; // cut at knob = 100

// ---------------------------------------------------------------------------
// Auto gain
// ---------------------------------------------------------------------------
// The reducer only ever cuts, so makeup only ever adds. Slow enough not to
// breathe, clamped so a wild setting cannot run away.
inline constexpr float kAutoGainTimeMs = 300.0f;
inline constexpr float kAutoGainMaxDb  = 6.0f;

// ---------------------------------------------------------------------------
// Parameter smoothing (knob moves, not audio)
// ---------------------------------------------------------------------------
inline constexpr float kParamSmoothingMs = 20.0f;
} // namespace cabrot::dsp::tuning
