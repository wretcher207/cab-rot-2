Title: SUNDER UI Specification v1
Document Type: UI Spec
Author: Claude (design skill, adapted from joshband/locusq)
Created Date: 2026-05-21
Last Modified Date: 2026-05-21

# SUNDER — UI Specification v1

Scientific Luxury redesign of Cab Rot. Produced with the `design` GUI skill
(framework-agnostic mockup, **preview only** — production is native JUCE at impl).

## Source decisions (locked this session)
- **In-place rename**: Cab Rot → SUNDER (full).
- **Keep planned DSP**: 4-band dynamic-reduction architecture stays. This is a
  **re-skin + relabel**, not the spec's 5-param spectral model.
- Aesthetic: "Scientific Luxury" — charcoal monolith + single warm accent,
  negative space over boxes, vector encoders, smoothed spectral canvas.

## Divergence from re-design-spec.md (flagged for veto)
The spec proposes 5 new encoders (suppression / target-focus-low / target-focus-high
/ selectivity / recovery) tied to *new* DSP. Because we are keeping the planned
4-band DSP, v1 instead **restyles + relabels the existing 6 controls**. The spec's
aesthetic is adopted in full; its parameter model is not. `TARGET FOCUS` becomes a
*visual* element on the canvas (the highlighted analysis band), not a control.

## Layout (960 × 580, aspect 48:29, resizable proportional)
Three vertical zones, all coordinates derived from window W×H — no magic numbers.

| Zone | Height | Contents |
|------|--------|----------|
| Header | 48px | Left: `SUNDER` wordmark. Right: PRESET ▾ · Δ (delta listen) · A/B · OS (1×/2×/4×) · BYPASS |
| Spectral Canvas | 360px | Faint grid (4 dB lines, 5 freq lines), smoothed input spectrum, amber reduction veil draping the targeted band, dB scale (left) + freq scale (bottom) |
| Control Dock | 172px | Top: thin PROFILE selector (5150 · RECTO · HM-2 · DJENT · BLACKENED · SLUDGE). Below: 6 vector encoders, wide even spacing |

Margins: `pad = round(W * 0.025)` (≈24px at default). Zones separated by negative
space + 1px hairlines at ≤12% opacity, never boxes.

## Controls
| Param ID | Legacy label | v1 label | Type | Range | Default |
|----------|--------------|----------|------|-------|---------|
| fizzHunt | Fizz Hunt | SUPPRESSION | vector encoder | 0–100% | 50 |
| edgePreserve | Edge Preserve | TRANSIENT | vector encoder | 0–100% | 50 |
| cabSmooth | Cab Smooth | RESONANCE | vector encoder | 0–100% | 50 |
| digitalSand | Digital Sand | GRAIN | vector encoder | 0–100% | 50 |
| airRot | Air Rot | AIR | vector encoder | 0–100% | 50 |
| reapMix | Reap Mix | MIX | vector encoder | 0–100% | 100 |
| mode | Mode | PROFILE | segmented (6) | 5150…SLUDGE | 5150 |
| deltaListen | Delta Listen | Δ toggle | header toggle | bool | off |
| aOrB | A/B Slot | A/B | header toggle | bool | A |
| oversampling | Oversampling | OS | header cycle | 1×/2×/4× | 1× |

Input/Output gain and The Crypt advanced params are retained in state but not
surfaced in the v1 minimal layout (header "⊕ ADVANCED" affordance reserved for them).

## Color palette
- Background `#0C0C0E` · Surface `#16161A` · Gridline `#1C1C21`
- Secondary text / lines `#8E8E93` · Value text `#F5F5F7`
- Accent (Imperial Amber) `#D4A359` — v2 will trial Slate Blue `#5B84B1` per spec

## Style notes
- Vector encoders: no bitmap, no enclosing box. Thin track ring (`#16161A`),
  amber value arc, center monospace value text, label below in muted silver.
- Spectral veil: amber fill, 25% opacity at the ceiling fading to 0% at its lowest
  drape — "translucent curtain," heavily smoothed, anti-aliased.
- Gridlines almost imperceptible; they recede, never compete with the curve.
- No scanlines, no corner stamps, no monospace metadata strips.

## Open for iteration
1. Accent: amber (v1) vs slate blue (v2).
2. PROFILE names — keep metal-amp identities or relabel to clinical descriptors.
3. Whether INPUT/OUTPUT trim deserve flanking micro-encoders in the dock.
