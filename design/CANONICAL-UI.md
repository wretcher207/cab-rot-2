# Cab Rot: Canonical UI

**Status**: rewritten 2026-08-10 for the Dead Pixel Design facelift, then
amended 2026-08-10 after a hostile review (see §9). Layout geometry, region
sizes, spacing and component dimensions below remain authoritative. All
colour, radius and typography rules have been replaced to match the DPD
brand kit (`dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md`), which is the
binding document for anything visual.

**Read §9 before trusting §7.** Several clauses below describe intent that
the shipping code does not yet honour. Each divergence is listed in §9 with
the file and line that contradicts it. Where §9 and §1-§8 disagree, §9
describes what actually ships.

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
default size and constrainer cannot drift apart. Min and max are the only
reachable extremes: 1000 x 650 and 1600 x 1040, both on the locked ratio.
Any figure other than these two (earlier notes cited 1000 x 680 and
1600 x 1070) describes a window the constrainer cannot produce.

One caveat in the resize implementation: `PluginEditor.cpp:210` derives the
320 px right column from the *height* scale factor. This is correct output
only because the aspect ratio is locked. If the lock is ever relaxed, that
line must be rewritten against width first (§9.2).

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
percent opacity, 3 px spacing. The helper's default `spacing = 4`
(`SpectreLookAndFeel.h:53`) is wrong against this rule; it is unreachable
today because the only call site passes 3 explicitly, so it is a latent
trap rather than a live defect (§9.6). Nothing else in the window has texture,
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
  percent on a 3 second cycle, no ornament; "LIVE" label in inkMeta;
  the shipping amplitude is 40 to 100 percent, `LivePill.cpp:43` and `:50`,
  see §9.6);
  Delta Listen ghost icon.

### 7.2 WaspMeter (left of MainSplit)

The single focal event of the editor.

- One 1 px `rule` frame on a `surface1` ground. Nothing else frames it.
- Scanlines inside the frame only, 5 percent opacity.
- dB scale down the left inside the frame (0, -6, -12, -18, -24 dB),
  mono 9.5 px, inkMeta. Horizontal guides at 40 percent `rule`; the -12 dB
  guide draws at full `rule` because it marks the damage threshold. The
  scale is linear in dB over `kReductionScaleMaxDb` (24 dB) and governs the
  reduction curve only. Guides currently run to the component's right edge
  rather than the frame's inside edge (§9.3).
- Input spectrum behind everything: one smooth quiet shape, filled at 45
  percent `rule` with a 1 px full-`rule` silhouette edge. **It does not
  share the dB axis.** It is drawn on a normalised 0-1 amplitude mapping
  (`WaspMeter.cpp:145`), so its height is not readable against the dB
  labels. It is a texture indicating where energy sits across frequency,
  not a measurement. §9.1 owns resolving this.
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
  Ships as a hardcoded `66.1` (`FizzReadout.h:24`), a value inherited
  from the deleted Stitch reference. §9.4 owns this.
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
  gradient, no bevel, no ring glow. Dials size between 56 and 104 px
  (`SpectreKnob.cpp:60-72`; the "cap at 96" comment at `:65` is stale, §9.6).

### 7.5 FooterBar (48 px tall)

- Top edge: 1 px hairline in `rule`. Ground: canvas.
- Left: IN/OUT meters. Mono-label in inkMeta, then a square bar: surface3
  track with 1 px `rule` frame, fill in stateLive. The 0.70 / 0.85 fills
  and the 12 px width difference between IN and OUT are placeholder
  artefacts, not design intent (`FooterBar.cpp:28-29`, `:87-92`); both
  meters should be equal width once Phase 6 wires real levels (§9.4).
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

Decisions 2 and 3, and the six amp profiles in §7.3, are locked *interface*
decisions only. No DSP code reads `mode`, `oversampling` or `deltaListen`
today; see §9.5.

---

## 9. Known divergences and open defects

Added 2026-08-10 after a hostile review of this spec and its implementation.
Everything here is a place where the document above, the facelift report, or
the shipping code makes a claim the other two do not support. Ordered by how
much a user would be misled.

### 9.1 The spectral display plots two curves on one axis, and only one of them respects it

The dB scale (`WaspMeter.cpp:99-107`) labels 0/-6/-12/-18/-24 evenly across
the plot, and `dbToY` (`:87-92`) maps the reduction curve linearly over
`kReductionScaleMaxDb`. Those agree. The input reference spectrum does not
participate: `:145` and `:171` place it at
`plot.getBottom() - reference[i] * plot.getHeight()`, a raw 0-1 amplitude.

So the frame carries one labelled vertical axis and two unrelated vertical
mappings. A user reading the reference shape against the "-12 dB" label
reads a number that means nothing. §7.2 previously described this as "a real
dB scale" without qualification, which was the single most misleading
sentence in the document.

Resolution, required before Phase 6 ships live data, pick one:
- **(a)** Convert the reference to the same dB domain and scale, so both
  curves are readable against the labels. Preferred.
- **(b)** Keep the reference as pure texture and move the dB labels visually
  onto the reduction curve's ownership, e.g. tie the guides to the curve and
  make the reference obviously non-metric (lower alpha, no silhouette edge).

Until one of those lands, §7.2 must keep saying the reference is texture.

### 9.2 The resize evidence described windows that cannot exist

`FACELIFT-REPORT.md` ("Verified outcomes") cited `min.png` at 1000 x 680 and
`max.png` at 1600 x 1070. Neither is on the locked 1.5385 ratio, and neither
matches `setResizeLimits (1000, 650, 1600, 1040)` (`PluginEditor.cpp:49`).
The reachable extremes are 1000 x 650 and 1600 x 1040. §1 now states this,
and the report line is corrected.

Separately, `PluginEditor.cpp:210` scales the right column width by the
height ratio. Correct only under the aspect lock. Flagged in §1.

### 9.3 Evidence cited in the facelift report is not reproducible

`design/screenshots/` is gitignored (`.gitignore:45`), so every screenshot
the report cites as iteration evidence is absent from the repository.
`build/` does not exist in a fresh checkout, so the five-target build result
and the two test-binary exit codes cannot be re-observed either.

The claims that *are* reproducible are the greps, and those were re-run
during this review and hold: no `RoundedRectangle`, no glow or shadow, no
colour literal outside `Palette.h` except `Colours::transparentBlack` in
LookAndFeel seeding and one `Colours::black` in `ThemeTestApp.cpp`.

Anyone re-verifying this facelift should treat the grep claims as confirmed
and the build/screenshot claims as unverified. Future reports must either
commit their evidence or stop citing it.

Partial remedy, added with this amendment: `tools/verify-doc-citations.py`
checks every `File.ext:N` citation in this document and in
`FACELIFT-REPORT.md` against the referenced file, and content-checks a set
of anchor lines that the arguments in §9 depend on. Run it after touching
either the docs or the cited sources:

```
python tools/verify-doc-citations.py
```

It currently reports `OK: 75 citations and anchors verified`. It caught
three dangling citations in the facelift report when first run (a
paint-function range, a knob-sizing range and a placeholder-data range, all
stale after earlier edits); those are fixed. This closes the cheapest gap,
citation drift. It does not substitute for committing build and screenshot
evidence, which remains open.

Also in this area: the dB guide lines are drawn to
`getLocalBounds().getRight()` (`WaspMeter.cpp:120`, `:128`) rather
than the frame's inside edge, so they cross the 1 px frame that §7.2 says is
the only thing framing the plot.

### 9.4 The interface currently displays four fabricated values

None of these are wired to anything:

| Element | Hardcoded value | Site |
|---|---|---|
| Fizz hero number | `66.1` | `FizzReadout.h:24` |
| Header CPU | `4.2%` | `HeaderBar.h:33` |
| IN / OUT meters | `0.70` / `0.85` | `FooterBar.cpp:28-29` |
| Spectral display | 16-column reference + 24-point curve | `WaspMeter.h:44-63` |

Meanwhile the header LIVE indicator breathes on a timer whenever the editor
is visible, regardless of whether audio is running. For a design whose
stated posture is instrument-grade honesty, that combination is the most
serious open issue in the product, independent of any brand rule.

The fizz `66.1` is additionally a direct inheritance from the Stitch
reference that the header of this document declares deleted and no longer
inherited from. Both statements cannot be true; the code is right.

Phase 6 owns all of these. Until then, no screenshot of this UI should be
presented as showing plugin behaviour.

### 9.5 Three parameters have full UI and no consumer

`mode`, `oversampling` and `deltaListen` are declared in
`buildParameterLayout` and fully wired to controls in
`PluginEditor::wireAttachments`, but nothing in `Source/DSP/` reads them.
The six amp profiles are the visual centre of the right panel and change
nothing audible.

This matters as a consistency problem: The Crypt was deleted from the footer
on the stated grounds that its button opened nothing (`FACELIFT-REPORT.md`,
"Copy"). The same standard applied to the amp profile
grid would delete or visibly disable it. Choose one standard. The
recommendation is to keep the controls (they are locked decisions with
Phase 6/7 owners) and render unwired controls in `inkDisabled` until their
DSP lands, which is honest and costs one predicate per control.

### 9.6 Stale comments and small contradictions

| Site | Says | Actually |
|---|---|---|
| `LivePill.cpp:43` | "between 40 and 100 percent" | `0.70 + 0.30*sin` = 40 to 100; §7.1 said 70 to 100. §7.1 corrected. |
| `SpectreKnob.cpp:65` | "cap the dial at 96" | caps at 104 (`:70`) |
| `HeaderBar.h:12` | header reads "BY DEAD PIXEL DESIGN" | draws "DEAD PIXEL HARMONIX" (`HeaderBar.cpp:43`) |
| `SpectreLookAndFeel.h:53` | `spacing = 4` default | rule is 3; unreachable, only call site passes 3 |
| `FizzReadout.h:9` and `:22` | "static 66.1% from Stitch" | contradicts the Stitch-deleted claim at the top of this file |

Also: `Fonts::displayScaled`, `monoScaled` and `monoLabelScaled` are dead
code (only `wordmarkScaled` has a call site), and `KnobRow.cpp:49` uses
integer cell division, dropping up to 5 px on the right edge at arbitrary
widths in a layout that otherwise insists on a 4 px spacing atom.

### 9.7 The stateError path is unproven, not "proven by construction"

`FACELIFT-REPORT.md` ("Known honest limits") concedes the damage path is
never exercised because placeholder reductions stay under 12 dB, then
asserted it was proven by construction. It is not. `WaspMeter.cpp:216` clips
with `juce::roundToInt (thresholdY)` against a 1.5 px float-stroked path, so
the crossing point can leave up to a pixel of `inkPrimary` inside damaged
territory, which is exactly the seam the two-pass render exists to avoid.
(That report line is now corrected.)

This is cheap to falsify: set one entry of `reductionDb` above 12.0f and
look. Do that before claiming the path works.

### 9.8 Checked against the binding brand brief

The brief is `dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md`. It is **not
in this repository**; it resolves to
`C:\Users\wretc\workspace\dead-pixel-design-v4\brand-kit\AI-BRAND-BRIEF.md`
on the current machine. Both this document and `FACELIFT-REPORT.md` cite it
by a bare relative path that resolves from nowhere in a clean checkout. A
future agent cannot verify brand compliance without being told where it
lives. That path dependency is itself a defect; the fix is either vendoring
the brief or recording its origin, and this note is the interim record.

Read directly and compared, 2026-08-10:

- **Palette: exact match.** All eleven tokens in `Source/Theme/Palette.h`
  are byte-identical to the brief's list (`AI-BRAND-BRIEF.md:41-51`),
  including the semantics that `#7FA57A` is "real success or live state
  only" and `#C4574C` is "real error only".
- **Radius: match.** "Use square geometry. Border radius is zero." (`:16`).
- **Typography: match.** DPD Display 400 uppercase at 0.06-0.10em with
  wordmark up to 0.32em, Inter 400/500 sentence case, JetBrains Mono at
  about 0.18em for short uppercase labels (`:59-61`). The shipping wordmark
  tracking of 0.30em is inside the permitted range.
- **Scanlines: match.** "Keep scanlines inside screens or media frames. Do
  not cover the whole page with them." (`:91`). Confined to the WaspMeter
  frame.

Three new divergences the brief exposes, none previously recorded:

1. **The live-state colour is used for a state that is not real.** The brief
   restricts `#7FA57A` to "real success or live state only" (`:50`). The
   header LIVE mark pulses on a timer whenever the editor is visible, and
   the footer meters are hardcoded to 0.70 / 0.85. So the one colour the
   brief reserves for genuine live state is currently the plugin's largest
   fiction. This makes §9.4 a brand violation, not merely an honesty
   complaint.
2. **Motion is out of the brief's range.** Atmospheric movement is specified
   at 14 to 48 seconds (`:100`). The LIVE pulse runs a 3 second cycle
   (`LivePill.cpp:32`), roughly five times faster than the slowest
   permitted atmospheric motion. Either reclassify it as a functional
   indicator (durations 160/240/700 ms, `:99`) or slow it into range.
3. **Reduced motion is not respected** (`:103`). Nothing in `Source/` reads
   a reduced-motion preference, and the `uiAnimation` parameter that exists
   for exactly this purpose has no consumer (see §9.5). Wiring `uiAnimation`
   to `LivePill::visibilityChanged` is a small change that closes both.

---

## 10. Amendment log

- **2026-08-10, post-review**: §9 added. §1 gained the reachable-extremes
  note and the right-column scaling caveat. §5 gained the scanline default
  discrepancy. §7.1 corrected the LIVE pulse range to match the code. §7.2
  now states that the reference spectrum does not share the dB axis. §7.3
  and §7.5 now mark their placeholder values as placeholders. §7.4 notes the
  stale dial-cap comment. §8 notes that decisions 2, 3 and the amp profiles
  are interface-only until their DSP lands.
- **2026-08-10, post-review follow-up**: every `File.ext:N` citation in this
  document and in `FACELIFT-REPORT.md` was re-derived from the files rather
  than from memory; roughly a dozen had drifted and are corrected.
  `tools/verify-doc-citations.py` added to keep them honest, and verified to
  fail on both a dangling line number and an anchor whose content changed.

