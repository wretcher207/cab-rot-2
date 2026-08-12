# Cab Rot: Canonical UI

**Status**: updated 2026-08-10 through the Tier 3 UI truth pass at `24fa790`.
Layout geometry, region sizes, spacing and component dimensions below remain
authoritative. All colour, radius and typography rules match the DPD brand kit
(`dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md`), which is the binding
document for anything visual.

This document also governs data truth and control availability. If a control
or instrument is not backed by the current DSP, it stays hidden. No placeholder
telemetry, decorative activity, or fabricated graph data is permitted.

The Stitch export was the visual reference the old toxic design was ported
from. It is deleted; git history keeps it. Nothing in this document
inherits from it anymore.

---

## 1. Window

| Property | Value |
|---|---|
| Default size | 1200 x 780 |
| Minimum     | 1000 x 650 |
| Maximum     | 1600 x 1040 |
| Aspect lock | continuous, ratio = 1200 / 780 (1.5385) |
| Border radius | 0 everywhere, no exceptions |

The editor expresses the ratio via a single constant `kAspectRatio` so
default size and constrainer cannot drift apart.

---

## 2. Vertical layout (regions)

At 1200 x 780 the editor is split horizontally into four bands:

```
+----------------------------------------------------------+
| HeaderBar             64 px                              |
+----------------------------------------------------------+
|                                                          |
| MainSplit             476 px (= 780 - 64 - 192 - 48)     |
|   Wasp Meter (left, flex 1)        |  RightPanel (320 px)|
|                                                          |
+----------------------------------------------------------+
| KnobRow              192 px                              |
+----------------------------------------------------------+
| FooterBar             48 px                              |
+----------------------------------------------------------+
```

Header and footer carry 24 px side padding. The main split carries 24 px
padding. The knob row carries 32 px horizontal and 16 px vertical padding.
The 4 px spacing atom governs every other gap and inset.

When the window resizes, every band keeps its pixel height derived from the
default-size proportions clamped to a floor; MainSplit absorbs the slack.

---

## 3. Typography

Three faces, bundled into `BinaryData` via `juce_add_binary_data`. Served
through `Source/Theme/Fonts.cpp`. Nothing constructs a `juce::Font` from a
system family name.

| Slot | Family | Weight | Tracking | Use |
|---|---|---|---|---|
| wordmark | DPD Display | 400 only, never synthetically bolded | 0.30 em | "CAB ROT" header wordmark, uppercase |
| display | DPD Display | 400 | 0.06 em to 0.10 em | Display headings only. Uppercase. |
| body | Inter | 400, 500 for emphasis | 0 | Body and controls, sentence case (currently tooltips and long-form copy) |
| mono-label | JetBrains Mono | 400 | 0.18 em | Short uppercase labels: headers, knob names, button labels, CPU, LIVE, footer status |
| mono | JetBrains Mono | 400 | 0 to 0.10 em | Every numeric readout: fizz hero number, knob values, CPU value, dB scale, frequency labels |

Note the numeric-readout rule: even the large fizz figure is JetBrains
Mono. DPD Display is for words, not digits.

---

## 4. Colour tokens (complete list)

Hand-maintained in `Source/Theme/Palette.h`. No hex literal appears
anywhere else in `Source/`.

| Token | Hex | Use |
|---|---|---|
| canvas      | `#060606` | window ground, knob row, footer |
| surface1    | `#0B0B0B` | spectral analysis frame ground |
| surface2    | `#101010` | popup menus, tooltip ground |
| surface3    | `#151514` | quiet fills: meter bar tracks |
| inkPrimary  | `#F2F2EF` | wordmark, reduction curve, value arcs, selected outlines, active ghost |
| inkBody     | `#B4B4B0` | knob values, zone names, combo text, hovered unselected |
| inkMeta     | `#8A8A85` | labels, scales, unselected text, tick marks |
| inkDisabled | `#4A4A47` | inactive ghost, disabled controls |
| rule        | `#2E2E2C` | every 1 px hairline, every unfilled track and border |
| stateLive   | `#7FA57A` | live state only: header live dot, IN/OUT meter fill |
| stateError  | `#C4574C` | reduction-column edges past 12 dB and held output clipping |

The damage threshold is `theme::kReductionDamageThresholdDb = 12.0f` in
`Palette.h`. Green and red are not accents; nothing else in the interface
is coloured. If hierarchy seems to need colour, it actually needs type or
spacing.

---

## 5. Background utility patterns

Scanlines are the only texture. They are permitted inside the spectral
display frame only, rendered by `SpectreLookAndFeel::drawScanlines` at 5
percent opacity, 3 px spacing. Nothing else in the window has texture,
gradients, vignettes, shadows, or emissive effects.

---

## 6. Border radii

Zero. Everywhere. Corners are square on the wordmark mark, the spectral
frame, the mode buttons, the A/B toggle, the oversampling combo, the meter
bars, and the knob arcs render as stroked paths with no filled bodies.

---

## 7. Component-specific specs

### 7.1 HeaderBar (64 px tall)

- Ground: canvas. Bottom edge: 1 px hairline in `rule`.
- Left cluster: "CAB ROT" wordmark (DPD Display, inkPrimary, 0.30 em),
  1 px vertical divider (rule, 16 px), DPD mark 16 x 16 in inkBody/inkPrimary,
  "DEAD PIXEL HARMONIX" in mono-label, inkMeta.
- Right cluster: "CPU" mono-label inkMeta + measured `processBlock` EMA in
  mono, inkBody. It renders `--` until a real measurement exists. The LIVE
  indicator is a static square in `stateLive` only when post-input-trim signal
  has exceeded -72 dBFS within the last second; otherwise it is
  `inkDisabled`. No breathing or decorative activity. Delta Listen is the
  ghost icon and outputs the material the wet path removes.

### 7.2 WaspMeter (left of MainSplit)

The single focal event of the editor.

- One 1 px `rule` frame on a `surface1` ground. Nothing else frames it.
- Scanlines inside the frame only, 5 percent opacity.
- dB scale down the left inside the frame (0, -6, -12, -18, -24 dB),
  mono 9.5 px, inkMeta. Horizontal guides at 40 percent `rule`; the -12 dB
  guide draws at full `rule` because it marks the damage threshold.
- The horizontal axis is logarithmic from 1 kHz through 20 kHz. The labelled
  ticks are 1k, 2k, 4k, 8k, 12k, and 20k.
- Four live reduction columns span the actual processed bands: BITE
  (2.4-3.8 kHz), PLASTIC (3.8-5.5 kHz), WASP (5.5-8 kHz), and ICE
  (8-12 kHz). Their downward height is the measured reduction on the
  0 to -24 dB scale. Columns use `surface3` with a 1.5 px `inkPrimary`
  moving edge. An edge past 12 dB uses `stateError`.
- Each column holds a 1 px `inkBody` peak line at 60 percent for about
  800 ms. Displayed reduction releases at 24 dB/s. On silence the columns
  decay to zero and the empty ruled frame is the correct resting state.
- Tick marks (1 x 5 px, inkMeta at 60 percent) inside the frame bottom at
  each labelled frequency.
- Below the frame: six log-positioned numeric frequency labels (mono 9.5,
  inkMeta) and four named bands centered geometrically between their real
  boundaries (mono-label, inkBody): BITE, PLASTIC, WASP, ICE.

The editor's single 30 Hz telemetry timer consumes audio-thread maxima and
updates this display. Phase 6 may add a real FFT input spectrum and history
trace. Until those exist, no spectrum silhouette or substitute shape is
drawn.

### 7.3 RightPanel (320 px wide)

The panel carries no card background or border. Structure comes from
hairlines and spacing.

FIZZ section (192 px tall):
- mono-label header "FIZZ AMOUNT", inkMeta.
- Hero number in JetBrains Mono, capped at 72 px, inkPrimary, no glow.
- Percent sign at about a third of the number height, inkMeta, on the
  number's baseline.
- While live, the value is the 140 ms smoothed maximum four-band reduction
  divided by the 12 dB damage threshold, clamped to 0-100 percent. At idle it
  renders `--` and suppresses the percent sign.
- A 1 px `rule` hairline separates the section from AMP PROFILE.

AMP PROFILE section (fills remaining height when available):
- mono-label header, inkMeta.
- 2 x 3 grid of square buttons (8 px gaps), 1 px hairline, no fill in
  either state: selected is inkPrimary border + inkPrimary text,
  unselected is rule border + inkMeta text, hover lifts unselected ink to
  inkBody.
- Modes: 5150, RECTO, HM-2, DJENT, BLACKENED, SLUDGE.
- The section is visible because `ModeConfig.h` now gives every switch real
  DSP derivatives. Threshold offset, four ceiling scales, attack scale, edge
  bias, and shelf start ramp over 300 ms. These are provisional voicings and
  still require David's by-ear approval before release.

### 7.4 KnobRow (192 px tall, full width)

- Top edge: 1 px hairline in `rule`. Ground: canvas. No scanlines.
- Six knobs evenly spaced, order: FIZZ HUNT, EDGE PRESERVE, CAB SMOOTH,
  DIGITAL SAND, AIR ROT, REAP MIX.
- Each knob: mono-label name above (inkMeta), dial, mono value below
  (inkBody), with a `%` suffix.
- Dial, drawn by `SpectreLookAndFeel::drawRotarySlider`: 1 px track arc
  in `rule`, 1.5 px value arc in inkPrimary, 1 px indicator line in
  inkPrimary (inkDisabled when the control is off). No filled body, no
  gradient, no bevel, no ring glow. Dials size between 56 and 104 px.
- Hover or drag increases the value arc to 2.5 px and the indicator to
  1.5 px. Double-click returns each control to its APVTS default, including
  Reap Mix at 100 percent.

### 7.5 FooterBar (48 px tall)

- Top edge: 1 px hairline in `rule`. Ground: canvas.
- Left: real post-input-trim and post-output-trim peak meters. Mono-label in
  inkMeta, then a square `surface3` track with a 1 px `rule` frame and
  `stateLive` fill. Linear peaks map to -60 through 0 dB, attack is immediate,
  and release is 12 dB/s. Output clipping holds a 1 px `stateError` segment
  for 1.5 seconds.
- Center: real A/B comparison, two square hairline buttons sharing a border.
  Selected side: inkPrimary border + text. Each side owns a detached deep
  APVTS snapshot. The version 2 preset wrapper persists both snapshots and
  the active slot; legacy raw `CABROT` state still loads and clones safely on
  first entry to the other side. A/B is a non-automatable meta parameter:
  button changes complete synchronously on the message thread, while an
  audio-thread generation guard prevents a block from reading a half-applied
  snapshot.
- Right: the oversampling combo (`OFF` / `2X` / `4X`, 64 x 22 px, hairline
  outline, square) sits left of the status text. It is visible because it
  drives real DSP: the reduction core runs at 1x/2x/4x with linear-phase
  half-band FIR stages, and the resulting latency is reported to the host.
  Status text is `V0.1.0 / CLIPPING`, `/ PROCESSING`, or `/ IDLE`, with
  that priority, and is driven by the same real telemetry as the meters.

The Crypt is deleted from the front panel. The button opened nothing;
Phase 8's advanced overlay will get a real entry point when it ships.

### 7.6 GhostToggle atom (Delta Listen)

Custom-painted ghost outline. Inactive: inkDisabled. Hover: inkBody.
Active: inkPrimary. It never glows, in any state. The ghost is discovered,
never announced. When active, the audio output is the removed signal with
the correct polarity, scaled by Reap Mix and followed by Output Trim. Reap Mix
at zero produces exact silence.

### 7.7 ModeButton atom

Square, hairline, no fill, per 7.3. Labels in mono-label 11 px.

---

## 8. Locked-decision summary

| # | Decision |
|---|---|
| 1 | 6 knobs in bottom row including Reap Mix |
| 2 | Oversampling Off / 2x / 4x, default Off; visible since the real 2x/4x chain with host latency reporting landed |
| 3 | Delta Listen = ghost icon in the header, inkDisabled to inkPrimary |
| 4 | Wasp Meter has six log-frequency ticks and four truthful band names: BITE / PLASTIC / WASP / ICE |
| 5 | Stereo Behavior lives in the advanced overlay (Phase 8 deliverable) |
| 6 | Continuous resize, aspect 1.5385 |
| 7 | DPD mark in header at 16 x 16 |
| 8 | Palette is the DPD brand kit, hand-maintained in `Source/Theme/Palette.h` |
| 9 | Border radius 0 everywhere; every border is a 1 px hairline in `rule` |
| 10 | Damage threshold 12 dB of reduction in any band (stateError) |
| 11 | No fabricated telemetry or dead controls; unavailable behavior remains hidden |
