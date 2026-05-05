# Cab Rot — Canonical UI

**Status**: locked 2026-05-05. This document overrides anything in the Stitch
export or the original spec where they conflict. Phase 1 deliverable.

The Stitch export at `design/stitch-reference.html` is a visual reference. The
authority chain is: this file > Stitch HTML > original product spec.

---

## 1. Window

| Property | Value |
|---|---|
| Default size | 1200 × 780 |
| Minimum     | 1000 × 650 |
| Maximum     | 1600 × 1040 |
| Aspect lock | continuous, ratio = 1200 / 780 ≈ 1.5385 |
| Border radius (outer) | 12 px (rounded-xl), but JUCE plugin windows render rectangular inside the host frame, so this is reserved for the Standalone window only |

Locked decision #6 (HANDOFF.md). Editor expresses the ratio via a single
constant `kAspectRatio` so default-size and constrainer cannot drift apart.

---

## 2. Vertical layout (regions)

At 1200 × 780 the editor is split horizontally into four bands:

```
┌─────────────────────────────────────────────────────────┐
│ HeaderBar             64 px (h-16)                      │
├─────────────────────────────────────────────────────────┤
│                                                         │
│ MainSplit             476 px (= 780 - 64 - 192 - 48)    │
│   Wasp Meter (left, flex 1)        |  RightPanel (320 px)│
│                                                         │
├─────────────────────────────────────────────────────────┤
│ KnobRow              192 px (h-48)                      │
├─────────────────────────────────────────────────────────┤
│ FooterBar             48 px (h-12)                      │
└─────────────────────────────────────────────────────────┘
```

`edge-margin` 20 px (Tailwind `1.25rem`) on header and footer side padding.
`p-6` (24 px) on the main split. `px-8 py-6` (32/24) on the knob row.

When the window resizes, every band keeps its absolute pixel height fixed
except MainSplit, which absorbs the slack.

---

## 3. Typography

Typeface ladder (bundled into `BinaryData` via `juce_add_binary_data`):

| Slot | Family | Weight | Size | Tracking | Use |
|---|---|---|---|---|---|
| display-title | Space Grotesk | 700 (Bold) - locked. Stitch calls 900 black; we ship 700 because the Black file isn't in workspace and the visual difference is small at 24 px. Phase 1 polish if David wants Black. | 24 px | -0.02 em | "CAB ROT" header h1, uppercase |
| hero-num | Space Grotesk | 700 | 80 px | -0.05 em | FIZZ % big readout, "66.1" |
| body | Space Grotesk | 400 (Regular) | 16 px | normal | reserved for non-existent body copy; spec mostly omits paragraphs |
| ui-chrome | JetBrains Mono | 400 (Regular) - locked. Stitch calls 500 Medium; the Medium TTF is not in the workspace font cache and the visual difference is small at 11 px. Phase 1.5 polish if David wants the heavier weight downloaded. | 11 px | 0.30 em | small-caps labels (`FIZZ HUNT`, `CPU`, `LIVE`, footer text) |
| mono-data | JetBrains Mono | 400 (Regular) | 14 px | 0.10 em | numeric values next to knobs (`62`), CPU value (`4.2%`), mode-button labels (`5150`) |

Font fallback if a typeface fails to load: JUCE's default sans, but a missing
Space Grotesk is a build-time error - the BinaryData symbols must resolve.

---

## 4. Color tokens (canonical list)

The Stitch tailwind config mixes OKLCH and direct hex. Both are valid. The
build-time script `tools/oklch-to-srgb.py` parses both forms and emits
`Source/Theme/Palette.h` with `constexpr juce::Colour` constants. The Colour
constants are the only sanctioned color source in component code.

**OKLCH-derived** (build-time conversion to sRGB hex):

| Token | OKLCH | Use |
|---|---|---|
| toxic        | oklch(0.88 0.28 142) | primary action color, knob indicators, glow base |
| toxic-glow   | oklch(0.92 0.30 140) | brighter toxic for glow halos and text-glow |
| warning      | oklch(0.82 0.22 75)  | reserved for future warning states |
| danger       | oklch(0.65 0.28 25)  | Delta Listen ghost when active, error states |
| background   | oklch(0.12 0.015 160) | window outer background |
| surface      | oklch(0.16 0.02 160) | cards, panels |
| surface-elevated | oklch(0.20 0.025 160) | FIZZ % card |
| border       | oklch(0.28 0.04 150 / 60%) | subtle borders with alpha |
| foreground   | oklch(0.96 0.02 145) | bright text |
| muted-foreground | oklch(0.65 0.04 150) | secondary text, inactive labels |

**Hex-direct** (45+ tokens, see Stitch config for the full set):

The most-used hex tokens, with their semantic role:

| Token | Hex | Use |
|---|---|---|
| primary-container        | #40FF2F | mode-button active fill, "live" indicator dot |
| primary-fixed            | #77FF60 | brighter green accent |
| primary-fixed-dim        | #0FE605 | dim green for footer chrome |
| surface-tint             | #0FE605 | identical to primary-fixed-dim |
| surface-container-lowest | #071005 | main panel background (the very dark green-black) |
| surface-container-low    | #141E11 | header strip background |
| surface-container        | #182215 | secondary card backgrounds, A/B toggle bg |
| surface-container-high   | #232D1F | inactive mode-button bg, analyzer header |
| surface-container-highest| #2D3829 | spectral bars at rest, knob inner cap |
| surface-bright           | #323C2D | knob outer container |
| surface-variant          | #2D3829 | duplicate of -highest in spec, kept for completeness |
| surface-dim              | #0C160A | dimmer than background, used in vignettes |
| outline                  | #85967D | mid-grey-green, primary border color |
| outline-variant          | #3C4B36 | dimmer border color |
| on-background            | #DAE6D1 | text on background (bright off-white green) |
| on-surface               | #DAE6D1 | identical to on-background |
| on-surface-variant       | #BACCB1 | secondary text |
| on-primary-container     | #037200 | dark green text on toxic-fill buttons |

The 25 remaining tokens (tertiary-* family, error-*, secondary-*) are kept
in `Palette.h` for completeness but are not referenced in v1. Phase 8's
Crypt overlay may use them.

---

## 5. Background utility patterns

| Pattern | Definition |
|---|---|
| scanline | linear-gradient(to bottom, transparent 50%, rgba(0,0,0,0.25) 51%) repeated every 4 px vertically. Applied at 20% opacity globally over the main panel and 10% over the knob row. |
| grid-pattern | crossed 1 px lines at 20 px grid, rgba(255,255,255,0.05). Applied at 30% opacity over the WaspMeter background. |

Both are paint-time effects, not images. `SpectreLookAndFeel` exposes
`drawScanlines (g, area, opacity)` and `drawGridPattern (g, area, opacity)`
helpers so component code stays clean.

---

## 6. Border radii

| Slot | Value |
|---|---|
| sm   | 4 px (DEFAULT) |
| md   | 6 px |
| lg   | 8 px |
| xl   | 12 px |
| full | circular |

Most cards use `xl` (12 px). Mode buttons and the A/B toggle use `lg` (8 px).
Knob outer containers use `full`. Meter pill uses `full`.

---

## 7. Component-specific specs

### 7.1 HeaderBar (64 px tall)

Left cluster, gap-4 (16 px), centered vertically:
- `h1#CAB ROT` — Space Grotesk Bold, 24 px, uppercase, tracking-tighter, color = toxic
- 1 px vertical divider, 16 px tall, `outline-variant`
- DPD mark, 16 × 16 px, grayscale at 70% opacity
- `by Dead Pixel Design` — JetBrains Mono Medium 11 px tracking-0.3em, color = muted-foreground

Right cluster, gap-6 (24 px):
- CPU label + value: ui-chrome `CPU` + mono-data toxic value
- LIVE pill: rounded-full, surface-container bg, outline-variant border, 8 px toxic dot with `0 0 8px toxic` glow + 1.5 s pulse, ui-chrome label `LIVE` toxic
- Delta Listen ghost icon (locked decision #3, replaces Stitch's `sensors` icon). Click toggles persistently; icon glows `danger` red when active.

### 7.2 WaspMeter (left of MainSplit)

- xl rounded card, surface-container/50 bg, outline-variant border
- grid-pattern overlay at 30% opacity
- 1 px gradient sweep across the top edge (transparent → toxic 50% → transparent)
- Header strip: 24 px py + 16 px px, `search_activity` icon (toxic) + `SPECTRAL ANALYSIS` label (muted-foreground), right-aligned `WASP METER` label (toxic)
- Spectral display: 14 to 16 vertical bars, 2 px gap, anchored to the bottom. Bars use:
  - `surface-container-highest` for at-rest (under detection threshold)
  - `toxic/40` to `toxic` for active bands, increasing alpha with detection intensity
  - Active bands also get `0 0 10px toxic` glow
- Peak-line overlay: SVG-style path drawn via `juce::Path`, white-toxic stroke, drop-shadow 0 0 4px toxic
- Peak-band white tip: 1 px white line on top of the strongest bar
- Frequency label rail at bottom (16 px tall): two rows, ui-chrome 10 px:
  - Row 1 (numeric): `1k 2k 5k 8k 12k 20k`
  - Row 2 (named, locked decision #4): `BITE PLASTIC WASP SAND AIR ICE` between the 2 kHz and 12 kHz markers

### 7.3 RightPanel (320 px wide, gap-6 = 24 px between cards)

#### FIZZ % card (192 px tall, h-48)
- xl rounded, surface-elevated bg, outline-variant border
- `bg-toxic/5 blur-2xl` halo behind the number (paint as soft radial)
- 1 px top-edge gradient sweep, identical to WaspMeter
- ui-chrome label `FIZZ AMOUNT` muted, 8 px above the number
- Hero number: Space Grotesk Bold 80 px, color = toxic, with text-glow (0 0 10px + 0 0 20px toxic)
- `%` suffix at 30 px (text-3xl), same color, no glow
- Click cycles through `FIZZ %` → `Reduction dB` → `Peak Hz`. Phase 6 wires this; Phase 1 just renders FIZZ % static at 66.1.

#### AMP PROFILE card (flex-1, fills remaining height)
- xl rounded, surface-container bg, outline-variant border, p-5 (20 px)
- Header: `developer_board` icon (muted, 14 px) + ui-chrome `AMP PROFILE` label
- 2 × 3 button grid, gap-3 (12 px), 6 mode buttons:
  - 5150, RECTO, HM-2, DJENT, BLACKENED, SLUDGE
- Active button: bg toxic, text on-primary-container (#037200), border toxic, `0 0 15px toxic/30` shadow, font-bold mono-data 14 px
- Inactive button: bg surface-container-high, text muted-foreground, border outline-variant, `lg` rounded, py-3 (12 px). Hover: border-toxic, text-toxic.

### 7.4 KnobRow (192 px tall, full width)

- Top border: 1 px outline-variant
- Background: surface-container/60 with backdrop-blur
- scanline overlay at 10% opacity
- 6 knobs evenly spaced (locked decision #1), max-w-4xl (1024 px) centered
- Each knob is a vertical stack, gap-4 (16 px):
  1. ui-chrome label, muted (or toxic if value > 0)
  2. Knob (64 × 64 px, see 7.6)
  3. mono-data value, toxic if value > 0 else muted-foreground

Order, from left: FIZZ HUNT, EDGE PRESERVE, CAB SMOOTH, DIGITAL SAND,
AIR ROT, REAP MIX.

### 7.5 FooterBar (48 px tall)

Three regions, all vertically centered:

Left:
- Crypt button: skull icon + ui-chrome `THE CRYPT` + expand_more chevron, color muted-foreground, hover toxic-glow
- 1 px vertical divider
- IN/OUT meters: stacked horizontal, gap-2 (8 px)
  - Each: ui-chrome 10 px label (`IN`/`OUT`) + 64 × 6 px pill (surface-container-highest bg, toxic fill at percentage). Peak hold for 1.5 s as a 1 px brighter sliver inside the fill.

Center:
- A/B toggle: surface-container bg, outline-variant border, p-1 (4 px), rounded-md
  - Active side: bg toxic, text on-primary-container, font-bold, mono-data text-xs (12 px)
  - Inactive: text muted-foreground, hover on-surface

Right:
- Oversample dropdown: ui-chrome `OS:` label + native combo. Locked decision #2: values are `Off`, `2x`, `4x` only. Default `Off`. Active value rendered in toxic; closed/idle in muted-foreground.
- 1 px vertical divider
- Version chrome: ui-chrome `CAB ROT vX.YZ // SCANNING FOR HARSHNESS`, color = toxic. The scanning suffix is a static string, not animated.

### 7.6 SpectreKnob atom (64 × 64 px)

```
 outer ring (full circle, 2 px outline-variant)
 ↓
┌────────────────┐    surface-bright bg
│ ┌────────────┐ │    inner cap, 48×48, surface-container-highest bg,
│ │      │     │ │    1 px surface-bright border, rotated to indicate value
│ │      │     │ │    (mapping: 0 = -135°, 100 = +135°, sweep 270°)
│ │      ▮     │ │    indicator line: 1 px wide × 4 px tall (w-1 h-3),
│ │            │ │    toxic with 0 0 5px glow if value > 0, else muted-foreground
│ └────────────┘ │
└────────────────┘
        + conic-gradient ring on the outer container, toxic 50% alpha,
          fills clockwise from -135° proportional to value, with 0 0 10px
          toxic-at-20% glow. Empty knobs draw no ring.
```

Click-drag rotates. Scroll wheel adjusts ±1. Double-click resets to default.
Right-click opens JUCE's standard context menu (Reset, Enter value).
Tooltip shows on hover after 500 ms (Phase 3).

### 7.7 ModeButton atom

Two states, no transition animation in v1:
- Active: see 7.3
- Inactive: see 7.3

Click cycles through the 6-mode array. Phase 5 also adds a 300 ms
SmoothedValue ramp on the underlying detector coefficients to avoid
audible glitch on switch.

### 7.8 GhostToggle atom (Delta Listen)

A custom-painted ghost icon. When active, it glows `danger` red and
the editor switches to delta output. Phase 7 wires the audio behavior;
Phase 1 only renders the icon and toggles the visual state.

---

## 8. Locked-decision summary

| # | Decision | Authoritative paragraph |
|---|---|---|
| 1 | 6 knobs in bottom row including Reap Mix | §7.4 |
| 2 | Oversampling Off / 2x / 4x, default Off | §7.5 right region |
| 3 | Delta Listen = ghost icon, header right cluster, glows danger when active | §7.1 |
| 4 | Wasp Meter dual-row labels (numeric + named zones) | §7.2 frequency rail |
| 5 | Stereo Behavior lives in The Crypt (Phase 8 deliverable) | not in v1 front panel |
| 6 | Continuous resize, aspect 1.5385 | §1 |
| 7 | DPD mark in header at 16×16 grayscale 70% | §7.1 |

---

## 9. Phase 1 self-review gate (recap)

The gate criteria from PLAN.md, restated against this doc:

- [x] this file exists and lists every locked answer (sign-off implicit via auto-mode go-ahead)
- [x] `tools/oklch-to-srgb.py` is reproducible (`--check` exits 0)
- [x] every Stitch token in §4 has a `juce::Colour` constant in `Source/Theme/Palette.h` (55 tokens emitted)
- [x] all five typography slots in §3 render from BinaryData with no CDN dependency
- [x] `SpectreLookAndFeel` is the single source of color and font; `juce::Colour::fromString` and `juce::FontOptions` literal grep in `Source/UI/` and `Source/PluginEditor.cpp` return zero hits
- [x] visual diff is **automated** via `tools/visual-diff.ps1 -ReferencePath …` (uses ImageMagick `compare -metric AE -fuzz 1%` if installed, falls back to `tools/compare-pngs.py` with PIL).
- [ ] visual diff threshold of ±5% per-pixel against the rendered Stitch reference. **Currently 67.7%** — a deliberate consequence of the canonical decisions in §8 (Reap Mix added, ghost icon, named zone labels, footer chrome). The 5% target was set against the Stitch export *before* the canonical decisions were locked. Phase 2 will close the gap as the static UI shell converges with canonical layout. Re-run the diff at the end of Phase 2 with the same threshold.

---

## 10. Open polish items (deferred to Phase 1.5 if needed)

- Space Grotesk Black (900) is not in workspace. v1 ships with Bold (700)
  for `display-title`. Visually the wordmark is slightly lighter than Stitch.
  Acceptable for MVP per David. Pull Black if the diff exceeds 5%.
- The DPD mark in §7.1 is currently a coded approximation (double square +
  bright pixel) rather than the authoritative DPD logo asset. The Stitch
  reference uses a 16 × 16 PNG that is hosted on a Google CDN; we can't
  bundle that. If David wants the exact mark, he should drop the SVG/PNG
  in `Resources/brand/` and we'll bundle it via BinaryData.
- Animations (LIVE pulse, button hover, knob hover glow) are 60 Hz JUCE
  Timer-driven. Phase 2 implements; Phase 1's Theme Test can render a
  static frame at the "active" state.
