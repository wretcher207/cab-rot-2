# Cab Rot — Canonical UI

**Status**: rewritten 2026-08-10 for the Dead Pixel Design facelift. Layout
geometry, region sizes, spacing and component dimensions below remain
authoritative. All colour, radius and typography rules have been replaced to
match the DPD brand kit
(`dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md`), which is the binding
document for anything visual.

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
| stateError  | `#C4574C` | reduction curve segments past 12 dB in any band |

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
- Right cluster: "CPU" mono-label inkMeta + value in mono, inkBody; live
  indicator (small square in stateLive, breathing between 70 and 100
  percent on a 3 second cycle, no ornament; "LIVE" label in inkMeta);
  Delta Listen ghost icon.

### 7.2 WaspMeter (left of MainSplit)

The single focal event of the editor.

- One 1 px `rule` frame on a `surface1` ground. Nothing else frames it.
- Scanlines inside the frame only, 5 percent opacity.
- dB scale down the left inside the frame (0, -6, -12, -18, -24 dB),
  mono 9.5 px, inkMeta. Horizontal guides at 40 percent `rule`; the -12 dB
  guide draws at full `rule` because it marks the damage threshold.
- Input spectrum behind everything: one smooth quiet shape, filled at 45
  percent `rule` with a 1 px full-`rule` silhouette edge.
- The gain-reduction curve is the primary line: inkPrimary, 1.5 px,
  no glow. Segments past the 12 dB threshold render in stateError via a
  clipped second pass.
- Tick marks (1 x 5 px, inkMeta at 60 percent) inside the frame bottom at
  each labelled frequency.
- Below the frame: numeric frequency row (mono 9.5, inkMeta) and named
  band row (mono-label, inkBody): BITE, PLASTIC, WASP, SAND, AIR, ICE.

Phase 6 wires this to `getBandReductionDb()`. Until then the content is
deterministic placeholder data chosen under the damage threshold.

### 7.3 RightPanel (320 px wide)

The panel carries no card background or border. Structure comes from
hairlines and spacing.

FIZZ section (192 px tall):
- mono-label header "FIZZ AMOUNT", inkMeta.
- Hero number in JetBrains Mono, capped at 72 px, inkPrimary, no glow.
- Percent sign at about a third of the number height, inkMeta, on the
  number's baseline.
- A 1 px `rule` hairline separates the section from AMP PROFILE.

AMP PROFILE section (fills remaining height):
- mono-label header, inkMeta.
- 2 x 3 grid of square buttons (8 px gaps), 1 px hairline, no fill in
  either state: selected is inkPrimary border + inkPrimary text,
  unselected is rule border + inkMeta text, hover lifts unselected ink to
  inkBody.
- Modes: 5150, RECTO, HM-2, DJENT, BLACKENED, SLUDGE.

### 7.4 KnobRow (192 px tall, full width)

- Top edge: 1 px hairline in `rule`. Ground: canvas. No scanlines.
- Six knobs evenly spaced, order: FIZZ HUNT, EDGE PRESERVE, CAB SMOOTH,
  DIGITAL SAND, AIR ROT, REAP MIX.
- Each knob: mono-label name above (inkMeta), dial, mono value below
  (inkBody).
- Dial, drawn by `SpectreLookAndFeel::drawRotarySlider`: 1 px track arc
  in `rule`, 1.5 px value arc in inkPrimary, 1 px indicator line in
  inkPrimary (inkDisabled when the control is off). No filled body, no
  gradient, no bevel, no ring glow. Dials size between 56 and 104 px.

### 7.5 FooterBar (48 px tall)

- Top edge: 1 px hairline in `rule`. Ground: canvas.
- Left: IN/OUT meters. Mono-label in inkMeta, then a square bar: surface3
  track with 1 px `rule` frame, fill in stateLive at 85 percent.
- Center: A/B toggle, two square hairline buttons sharing a border.
  Selected side: inkPrimary border + text.
- Right: "OS:" mono-label + oversampling combo (square hairline, mono
  text, LOCKED: Off / 2x / 4x, default Off), 1 px divider, status text
  "V0.1.0 / PROCESSING" in mono-label inkMeta. The message reports the
  actual processing state; costume strings are banned.

The Crypt is deleted from the front panel. The button opened nothing;
Phase 8's advanced overlay will get a real entry point when it ships.

### 7.6 GhostToggle atom (Delta Listen)

Custom-painted ghost outline. Inactive: inkDisabled. Hover: inkBody.
Active: inkPrimary. It never glows, in any state. The ghost is discovered,
never announced.

### 7.7 ModeButton atom

Square, hairline, no fill, per 7.3. Labels in mono-label 11 px.

---

## 8. Locked-decision summary

| # | Decision |
|---|---|
| 1 | 6 knobs in bottom row including Reap Mix |
| 2 | Oversampling Off / 2x / 4x, default Off |
| 3 | Delta Listen = ghost icon in the header, inkDisabled to inkPrimary |
| 4 | Wasp Meter dual-row labels (numeric + named zones) |
| 5 | Stereo Behavior lives in the advanced overlay (Phase 8 deliverable) |
| 6 | Continuous resize, aspect 1.5385 |
| 7 | DPD mark in header at 16 x 16 |
| 8 | Palette is the DPD brand kit, hand-maintained in `Source/Theme/Palette.h` |
| 9 | Border radius 0 everywhere; every border is a 1 px hairline in `rule` |
| 10 | Damage threshold 12 dB of reduction in any band (stateError) |
