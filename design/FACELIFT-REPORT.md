# Cab Rot facelift: compliance report

Date: 2026-08-10. Branch: `facelift-dpd`. Scope: Dead Pixel Design brand
facelift of the plugin UI. DSP, `PluginProcessor.cpp`, `Source/DSP/`, and
`tests/` are untouched.

## Binding document

`dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md`, read in full before any
code was written. The plugin-specific reading below is implemented in the
files cited under each rule.

## Rule by rule

### Palette
Rule: near-black monochrome, the ten-token set, no other colour anywhere.

- Complete token set with the exact brief hexes: `Source/Theme/Palette.h:25-36`.
- Conditional green `stateLive` only where the plugin is live and
  processing: live dot in `Source/UI/atoms/LivePill.cpp:49`,
  meter fills in `Source/UI/atoms/MeterPill.cpp:43`.
- Conditional red `stateError` only past the damage point: clipped second
  pass of the reduction curve in `Source/UI/WaspMeter.cpp:211-220`.
- Damage threshold set at 12 dB and documented at definition site:
  `Source/Theme/Palette.h:38-41` (`kReductionDamageThresholdDb = 12.0f`,
  scale max 24 dB).
- Every Stitch green deleted. Verified: `grep -rni "40FF2F\|0FE605\|54FF00\|
  SpaceGrotesk" .` returns nothing outside `.git` and `design/sunder`
  (build directory included in the scan).
- `Palette.h` banner now points at the brand kit instead of the retired
  generator: `Source/Theme/Palette.h:1-15`. `tools/oklch-to-srgb.py`
  deleted; no build step or test ever called it (verified by grep over
  CMakeLists.txt, tools, and tests).

### Geometry
Rule: border radius 0, 1px `#2E2E2C` (`rule`) hairlines, no shadows, no
glow, scanlines only inside the spectral frame at 6 percent or less, 4px
spacing atom.

- `grep -rn "RoundedRectangle" Source/` returns nothing.
- `grep -rni "glow\|dropshadow\|drop_shadow" Source/` returns nothing.
- Flat knob (track arc, value arc, indicator line, no filled body, no
  gradient, no bevel): `Source/Theme/SpectreLookAndFeel.cpp:57-98`.
- Square, unfilled buttons (1px hairline, selected inkPrimary, unselected
  rule border with inkMeta text): `Source/Theme/SpectreLookAndFeel.cpp:101-116`,
  label render in `Source/UI/atoms/ModeButton.cpp:23-27`.
- Scanlines confined to the WaspMeter frame at 5 percent opacity, 3px
  spacing: `Source/UI/WaspMeter.cpp:58`; helper defaults also at 5
  percent: `Source/Theme/SpectreLookAndFeel.cpp:171-180`. Helper signature
  caps at the documented site in `Source/Theme/SpectreLookAndFeel.h:41-43`.
- Editor background is flat canvas: `Source/PluginEditor.cpp:191-196`.
- All gaps and insets are multiples of 4px (24/16/40/8/12/32 used
  throughout; the 8px mode-grid gaps and 12px dB insets are atom multiples).

### Typography
Rule: DPD Display 400 uppercase display/wordmark at 0.06 to 0.10em, up to
0.32em on the wordmark; Inter 400/500 body; JetBrains Mono metadata and
every numeric readout; Space Grotesk deleted everywhere.

- Three faces bundled: `CMakeLists.txt:90-97`.
- Served through rewritten cache: `Source/Theme/Fonts.cpp`, slots in
  `Source/Theme/Fonts.h:28-50`. Wordmark at 0.30em tracking:
  `Source/Theme/Fonts.cpp:66` and `:88-91`.
- Numeric readouts in JetBrains Mono even at hero size:
  `Source/UI/FizzReadout.cpp:24-28` (fizz figure), knob values
  `Source/UI/atoms/SpectreKnob.cpp:56-58`, CPU value
  `Source/UI/HeaderBar.cpp:57-59`, dB scale `Source/UI/WaspMeter.cpp:103`.
- Space Grotesk TTFs deleted from `Resources/fonts/`, BinaryData, and all
  consumer code.

### Spectral display (the one focal event)
Rule: curve primary in inkPrimary at 1.5px, input spectrum quiet in
`rule`, real dB scale in mono metadata, 1px frame on surface1, frequency
and band labels beneath, nothing else frames it.

- Frame + ground: `Source/UI/WaspMeter.cpp:52-57`.
- Quiet reference (45 percent fill, full-strength silhouette edge):
  `Source/UI/WaspMeter.cpp:134-170`.
- Reduction curve 1.5px inkPrimary: `Source/UI/WaspMeter.cpp:174-210`.
- dB scale 0/-6/-12/-18/-24 in mono 9.5 metadata with tiered guides and a
  full-strength -12 dB damage guide: `Source/UI/WaspMeter.cpp:101-133`.
- Frequency ticks inside the frame bottom: `Source/UI/WaspMeter.cpp:71-87`
  plus dual-row labels below the frame: `Source/UI/WaspMeter.cpp:235-270`.
- Header strip reads SPECTRAL ANALYSIS / WASP METER in mono metadata:
  `Source/UI/WaspMeter.cpp:28-44`.

### Fizz readout
Rule: number stays large in inkPrimary, no glow, percent sign metadata
grey, both on one baseline.

- `Source/UI/FizzReadout.cpp:24-63`.

### Knobs
Rule: flat ring control, mono name above in inkMeta, mono value below in
inkBody.

- Dial: `Source/Theme/SpectreLookAndFeel.cpp:57-98` (56 to 104 px sizing
  in `Source/UI/atoms/SpectreKnob.cpp:71-77`).
- Labels/values: `Source/UI/atoms/SpectreKnob.cpp:45-59`.
- Row ground flat canvas with a single top hairline, no scanlines:
  `Source/UI/KnobRow.cpp:37-45`.

### Amp profile selector
Rule: square hairline buttons, no fill in either state, selected reads
inkPrimary, unselected rule + inkMeta.

- Background: `Source/Theme/SpectreLookAndFeel.cpp:101-116`, labels:
  `Source/UI/atoms/ModeButton.cpp`, grid: `Source/UI/AmpProfileGrid.cpp`.

### Copy
- Header reads DEAD PIXEL HARMONIX (Dead Pixel Harmonix is the shipping
  label): `Source/UI/HeaderBar.cpp:41-43`.
- SCANNING FOR HARSHNESS deleted. Footer shows the real processing state:
  `Source/UI/FooterBar.cpp:12` and `:58-65` ("V0.1.0 / PROCESSING").
- THE CRYPT deleted (it had no handler; verified the button opened
  nothing): `Source/UI/FooterBar.h`, `Source/UI/FooterBar.cpp`, and the
  tooltip reference removed from `Source/PluginEditor.cpp:168`.
- Ghost (Delta Listen) kept, inkDisabled at rest, inkPrimary while
  listening, never emits light: `Source/UI/atoms/GhostToggle.cpp:13-23`.

## Verified outcomes

- Five-target Release build (VST3, Standalone, PassthroughTest, DspTest,
  ThemeTest): 0 compiler warnings, 0 errors from project sources.
- `build/CabRot_PassthroughTest_artefacts/Release/CabRot_PassthroughTest.exe`
  exits 0 (3 passed, 0 failed).
- `CABROT_SKIP_CPU_BENCH=1 build/CabRot_DspTest_artefacts/Release/CabRot_DspTest.exe`
  exits 0, 12 passed, 0 failed.
- `grep -rn "RoundedRectangle" Source/` empty;
  `grep -rni "glow\|dropshadow\|drop_shadow" Source/` empty;
  no hex literal outside `Source/Theme/Palette.h` (verified with
  `grep -rnE "0x[0-9A-Fa-f]{6}|#[0-9A-Fa-f]{6}|fromString" Source/`); no
  banned tokens anywhere outside `.git` and `design/sunder`.
- Iteration evidence: `design/screenshots/facelift-00-before.png` through
  `facelift-06.png`, each critiqued in chat before the next change.
  `facelift-final.png`, plus `min.png` at 1000 x 680 and `max.png` at
  1600 x 1070. Both extremes hold the locked 1.54:1 layout with no
  clipping: dials cap at 104 px, chrome fonts floor at legibility minimums,
  main split absorbs the resize slack.
- `design/CANONICAL-UI.md` rewritten so its colour, radius and typography
  sections describe exactly what now ships.

## Known honest limits

- The spectral display data is still deterministic placeholder content
  (documented in `Source/UI/WaspMeter.h:52-70`); Phase 6 owns the live
  wiring via `getBandReductionDb()`. Placeholder values stay under the 12
  dB threshold, so the `stateError` path is proven by construction (clipped
  second stroke pass) but not yet exercised by real audio.
- The `stateLive` breathing in the header mark runs at 30 Hz on a 3 second
  cycle, consistent with "motion is quiet"; it suspends when the component
  is hidden.
- `ThemeTest` (`Source/UI/ThemeTest.cpp`) is now a paint-only brand
  reference frame for the same system rather than a Stitch diff target.
