# Cab Rot — Build Plan

**Owner**: David Russell / Dead Pixel Harmonix
**Plan date**: 2026-05-05
**Source spec**: `second-mind/wiki/sources/2026-05-05-cab-rot-plugin-spec.md`
**UI reference**: `cab-rot/design/CANONICAL-UI.md` (rewritten for the DPD brand facelift, 2026-08-10). The original Stitch export lived in `design/stitch-reference.html`; that direction is cancelled and the file is deleted (git history keeps it).
**Build target**: VST3 + AU, JUCE 8.x, Windows 11 first / macOS later

---

## Context

Cab Rot is Dead Pixel Harmonix's second confirmed product after [[2026-04-24-dead-pixel-harmonix-msv-1|MSV-1]]. It is a focused dynamic harshness controller for high-gain amp-sim guitars: targets the 2 kHz–12 kHz "wasp nest" range with 4 dynamic detection bands and a transient-protected reduction window. Tagline: **"Kill the wasp nest. Keep the teeth."**

This plan skips the spec's 3-stage iteration path (web prototype → JSFX → JUCE) and goes **straight to JUCE/VST3**. Reasoning: the Stitch UI export is already production-grade visual design (the slowest-to-iterate piece), and the DSP architecture in the spec is concrete enough to implement directly without a JSFX dry-run. Iteration risk is moved into the JUCE phase, where it's contained by phase gates.

Each phase below ends with a **Self-Review Gate**. Do not start the next phase until every pass-criterion is checked. If a gate fails, list what failed and re-execute the failing tasks. The gate is the only mechanism that prevents this build from drifting into a half-finished state.

---

## Stitch UI vs Spec — Gap Resolutions (locked 2026-05-05)

The Stitch export and the source spec disagreed on six points. Resolutions locked below — these override both the spec and the Stitch export wherever they conflict. Phase 1's `design/CANONICAL-UI.md` will codify these in detail with measurements; this table is the authority for now.

| # | Issue | Resolution |
|---|---|---|
| 1 | **Main controls count** | **6 knobs** in bottom row, evenly spaced: Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, **Reap Mix**. Reap Mix added per spec. |
| 2 | **Oversampling values** | **Off / 2x / 4x.** Drops 8x. Default = Off. |
| 3 | **Delta Listen toggle** | **Ghost icon in header**, replaces Stitch's `sensors` icon. Click toggles. Glows red (`destructive` token) when active. |
| 4 | **Wasp Meter labels** | **BITE / PLASTIC / WASP / SAND / AIR / ICE** between 2 kHz and 12 kHz. Two label rows: numeric frequencies on top, named zones below. Recommended resolution adopted, no contest. |
| 5 | **Stereo Link control** | Lives in **The Crypt** advanced panel as `Stereo Behavior` (Linked / Partial / Dual Mono). Recommended resolution adopted, no contest. |
| 6 | **Window resizing** | **Continuous resize**, aspect-locked at 1.54:1. Min 1000×650, default 1200×780, max 1600×1040. |

---

## Architecture

**Render strategy: Native JUCE (not WebView).**

Reasoning:
- The Wasp Meter and IN/OUT meters need 60 Hz updates from DSP-side analysis. WebView round-trips are sluggish and unpredictable; native `juce::Component::repaint()` is deterministic.
- The Stitch design is achievable with custom paint methods. Knobs are a conic-gradient ring + small line — easy `juce::Graphics::drawArc` work. Glow effects are box shadows which JUCE handles via `juce::DropShadow`.
- Single VST3/AU binary with no WebView2 / WKWebView runtime dependency = cleaner installer and smaller memory footprint.
- DPI-aware resize behavior is native to JUCE; in WebView it's an extra layer of pain.

**Color tokens**: convert OKLCH from Stitch tailwind config to sRGB once (build-time script), bake into a `Theme.h` header as `constexpr juce::Colour` constants. Don't re-implement OKLCH→sRGB at runtime — it's a one-shot conversion.

**Component layering**:
```
PluginProcessor (DSP, params via AudioProcessorValueTreeState)
  └─ PluginEditor (top-level Component, resizable)
      ├─ HeaderBar (brand, CPU, LIVE indicator, Delta Listen ghost toggle)
      ├─ MainSplit
      │   ├─ WaspMeter (left, FFT-driven spectral display)
      │   └─ RightPanel
      │       ├─ FizzReadout (FIZZ %, click cycles to Reduction dB / Peak Hz)
      │       └─ AmpProfileGrid (6 mode buttons)
      ├─ KnobRow (6 knobs: Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, Reap Mix)
      ├─ FooterBar (Crypt button, IN/OUT meters, A/B, Oversampling)
      └─ CryptOverlay (advanced panel, hidden by default)
```

**Code modules** (under `Source/`):
- `Theme/` — OKLCH→sRGB palette, fonts, custom LookAndFeel, scanline overlay paint helper
- `UI/` — every Component above, plus reusable atoms (`SpectreKnob`, `ModeButton`, `MeterBar`, `GhostToggle`)
- `DSP/` — `InputTrim`, `BandSplitter` (4-band crossover), `TransientDetector`, `DynamicReducer`, `ReapMixer`, `OversamplerWrap`
- `Modes/` — `ModeConfig` struct + 6 named instances (5150, Recto, HM-2, Djent, Blackened, Sludge)
- `Presets/` — APVTS-based preset save/load + 12 starter preset XMLs
- `Tests/` — pluginval automation, sine-sweep null tests, denormal/clipping smoke tests

---

## Repo Structure (after Phase 0)

```
cab-rot/
├── PLAN.md                          # this file
├── README.md
├── CMakeLists.txt                   # CMake-driven JUCE build (no Projucer)
├── design/
│   ├── stitch-reference.html        # Stitch export (already copied)
│   ├── CANONICAL-UI.md              # Phase 1 deliverable
│   └── screenshots/                 # phase-by-phase visual diffs
├── Source/
│   ├── PluginProcessor.{h,cpp}
│   ├── PluginEditor.{h,cpp}
│   ├── Theme/
│   ├── UI/
│   ├── DSP/
│   ├── Modes/
│   └── Presets/
├── Resources/
│   ├── fonts/                       # Space Grotesk, JetBrains Mono (bundled, not Google CDN)
│   └── presets/                     # 12 starter preset .xml files
├── tests/
│   ├── pluginval.ps1
│   └── sine-sweep-null.cpp
├── tools/
│   ├── oklch-to-srgb.py             # one-shot palette generator
│   └── visual-diff.ps1              # JUCE plugin screenshot vs HTML reference
└── JUCE/                            # git submodule, JUCE 8.x
```

---

## Phase 0 — Foundation

**Goal**: empty VST3 plugin compiles, loads in Reaper, opens its window. Nothing else. The whole point is to remove "does the build work?" as a variable before any creative work starts.

### Tasks
1. `git init` and first commit (PLAN.md, README.md, .gitignore for build artifacts).
2. Add JUCE 8.x as a git submodule under `JUCE/`.
3. Write `CMakeLists.txt` that builds a JUCE VST3 named `CabRot` with company name `Dead Pixel Harmonix`, bundle ID `com.deadpixelharmonix.cabrot`.
4. Build empty `PluginProcessor` (passes audio through unmodified) and empty `PluginEditor` (single label "CAB ROT").
5. Build VST3, install to `C:\Program Files\Common Files\VST3\`.
6. Open Reaper, instantiate Cab Rot on a track with a guitar DI → amp sim signal chain. Confirm window opens, audio passes through, no crashes.
7. Add `tools/visual-diff.ps1` that uses the existing `juce-standalone-snapshot` skill to PNG-capture the plugin window for later visual comparison.

### Self-Review Gate 0
- [ ] `cmake --build` succeeds clean from a fresh clone, no warnings
- [ ] VST3 loads in Reaper without errors in `~/.reaper/reaper-debug.log`
- [ ] Plugin passes audio through with measurable null when comparing input vs output (sample-perfect, no offset)
- [ ] `tools/visual-diff.ps1` produces a PNG of the plugin window
- [ ] Repo is committed; `.gitignore` excludes `build/`, `*.vst3` in source tree, IDE files

**Pass criterion**: all five boxes checked. If any box fails, the build foundation is broken — stop and fix. Do not proceed to Phase 1 with a flaky build.

**Common failure modes**: JUCE submodule missing, CMake target name conflicts, VST3 missing manifest, Reaper plugin scanner not finding the binary (often a permissions or path issue).

---

## Phase 1 — Canonical UI Lock + Design System Port

**Goal**: resolve all 6 Stitch-vs-spec gaps; port the OKLCH palette and fonts into a JUCE LookAndFeel that renders a single test screen pixel-comparable to the Stitch reference.

### Tasks
1. **Reconcile gaps** (see Gap Analysis table above). Write `design/CANONICAL-UI.md` with the locked-in answer for each of the 6 disputes. Get David's sign-off in writing before continuing.
2. **OKLCH→sRGB conversion**: write `tools/oklch-to-srgb.py` that parses the Stitch Tailwind config block and emits `Source/Theme/Palette.h` with `constexpr juce::Colour` constants. Use the standard `oklab → linear sRGB → sRGB` formula. Verify visually against a color picker that the output matches Stitch's rendered colors.
3. **Bundle fonts**: download Space Grotesk and JetBrains Mono `.ttf` files into `Resources/fonts/`. Bake into BinaryData via the `juce-binary-data-gen` skill. No runtime Google CDN dependency.
4. **LookAndFeel class**: `SpectreLookAndFeel` extends `juce::LookAndFeel_V4`, overrides `drawRotarySlider`, `drawButtonBackground`, `drawLabel`, `drawComboBox`, `drawProgressBar`. Match Stitch's borders, glows, scanline overlay.
5. **Test harness**: a single throwaway "Theme Test" Component that renders one of every UI atom (knob at 0/50/100, mode button active/inactive, hero readout, mini meter bar, scanline overlay). Add a `--theme-test` runtime flag.
6. **Visual diff**: capture PNG of the theme-test Component. Place side-by-side with `design/stitch-reference.html` rendered to PNG. Iterate until visually indistinguishable at 100% zoom.

### Self-Review Gate 1
- [ ] `design/CANONICAL-UI.md` exists, all 6 disputes have a single locked answer, David has signed off
- [ ] `tools/oklch-to-srgb.py` is reproducible — running it twice produces byte-identical `Palette.h`
- [ ] All Stitch tokens (~50 of them) have a matching `juce::Colour` constant
- [ ] Fonts load from BinaryData (no internet required); Space Grotesk and JetBrains Mono both render at all the spec's sizes
- [ ] `SpectreLookAndFeel` is the only place colors and fonts come from — no hardcoded `juce::Colour` literals in Component code
- [ ] Visual diff: theme-test PNG and Stitch HTML PNG match to within ±5% per-pixel difference (use `tools/visual-diff.ps1` with ImageMagick `compare`)

**Pass criterion**: all six boxes. If visual diff exceeds 5%, identify which atom is wrong and iterate the LookAndFeel.

**Common failure modes**: OKLCH formula off by chroma or hue; sRGB gamma not applied; font hinting differs between browser and JUCE's text engine (causes letter spacing drift); JUCE Component repaint doesn't pick up LookAndFeel changes (missed `setLookAndFeel()`).

---

## Phase 2 — Static UI Skeleton

**Goal**: full plugin window layout matches Stitch reference at every size. No interactivity, no DSP — just the layout shell with placeholder data.

### Tasks
1. Build `HeaderBar` Component: brand text, DPD logo (via BinaryData), CPU% display (placeholder static "4.2%"), LIVE indicator (animated dot), Delta Listen ghost toggle (visual only).
2. Build `WaspMeter` Component: grid pattern background, scanline overlay, 14-segment spectral bar visual (static heights from Stitch), peak line SVG-style overlay (use `juce::Path`), bottom frequency labels with both numeric (1k/2k/5k/8k/12k/20k) and named zones (BITE/PLASTIC/WASP/SAND/AIR/ICE).
3. Build `FizzReadout` Component: large hero number (66.1%), label.
4. Build `AmpProfileGrid` Component: 2×3 grid of 6 mode buttons. 5150 in active state, others inactive.
5. Build `KnobRow` Component: 6 knobs evenly spaced (Fizz Hunt 62, Edge Preserve 45, Cab Smooth 35, Digital Sand 55, Air Rot 40, Reap Mix 50). Knobs are visual-only — no drag.
6. Build `FooterBar` Component: Crypt button, IN/OUT meter bars (static fill), A/B toggle (static), Oversampling combo box (static), version text right-aligned.
7. Wire into `PluginEditor`: full layout with proper resizing using `juce::FlexBox` and `juce::Grid`.
8. Test resize: 1000×650 (min) → 1200×780 (Stitch default) → 1600×1040 (max). Layout must not break at any size.

### Self-Review Gate 2
- [ ] At default size 1200×780, plugin window matches Stitch reference visually (capture PNG, diff under 8% per-pixel)
- [ ] Resize from min to max produces no clipped text, no overflowing components, no broken Wasp Meter scaling
- [ ] All static "placeholder" values are clearly marked with a `// PHASE 2 PLACEHOLDER` comment so they're easy to find and replace later
- [ ] DPD logo loads from BinaryData (no Google CDN URL)
- [ ] LIVE indicator pulse animates smoothly at 60 Hz (no jank)
- [ ] No memory leaks reported by JUCE's debug build leak detector when opening/closing plugin window 50 times

**Pass criterion**: all six boxes. The 8% diff threshold is more lenient than Phase 1 because the layout has many components — small subpixel differences accumulate. If diff exceeds 8%, identify the culprit Component and iterate.

**Common failure modes**: FlexBox aspect ratios drift on resize; frequency labels overlap on narrow widths; meter bars don't gradient correctly; scanline overlay opacity stacks wrong with the grid pattern.

---

## Phase 3 — Interactive UI Components

**Goal**: every control on the front panel responds to user input and is bound to an APVTS parameter. Still no DSP — turning Fizz Hunt does nothing audible. But the parameter system is fully wired so DSP can hook in cleanly in Phase 4.

### Tasks
1. Define APVTS parameter schema in `PluginProcessor.cpp`:
   - 6 main float params: `fizzHunt`, `edgePreserve`, `cabSmooth`, `digitalSand`, `airRot`, `reapMix` (all 0–100, default per-mode)
   - 1 mode choice param: `mode` (0–5, default 0=5150)
   - Utilities: `inputGain` dB, `outputGain` dB, `oversampling` (Off/2x/4x), `deltaListen` bool, `aOrB` bool, `stereoLink` choice
   - Crypt advanced (will hook UI in Phase 8): `detectorFocus`, `clampSpeed`, `maxReapDb`, `pickWindow`, `quality`, `autoGain`, `uiAnimation`
2. Convert each placeholder Component into an interactive one:
   - `SpectreKnob` extends `juce::Slider` (rotary). Click-drag, scroll wheel, double-click reset, hover glow, value-while-dragging tooltip.
   - `ModeButton` extends `juce::Button`. Click cycles APVTS `mode` param. Active state has toxic fill + black text + animated live dot.
   - `GhostToggle` for Delta Listen.
   - A/B button: tap toggles, long-press copies A→B (use `juce::Timer` for long-press detection).
   - `OversampleCombo` extends `juce::ComboBox`.
3. Wire UI ↔ APVTS via `juce::AudioProcessorValueTreeState::SliderAttachment`, `ButtonAttachment`, `ComboBoxAttachment`. No manual listeners.
4. Tooltips on every control showing parameter name + current value.
5. Undo/redo via APVTS's built-in undo manager — bind to plugin host's undo and to a hidden Ctrl+Z keyboard shortcut.

### Self-Review Gate 3
- [ ] Every front-panel control responds to mouse: knobs drag and scroll, buttons click, A/B toggles
- [ ] Every control is bound to an APVTS param with the correct range (verified by automation in Reaper — write parameter automation, all params show up in Reaper's parameter list)
- [ ] Saving the plugin state (Reaper save) and reloading it restores all parameter values exactly
- [ ] Undo/redo works for at least 20 changes
- [ ] Right-clicking a knob shows the standard JUCE context menu with "Reset to default" and "Enter value..."
- [ ] Tooltips show on hover after a 500 ms delay
- [ ] No DSP changes — audio still passes through unchanged

**Pass criterion**: all seven boxes. The "audio still passes through unchanged" check is critical — it confirms Phase 4's DSP work hasn't been started prematurely or accidentally.

**Common failure modes**: APVTS parameter IDs don't match Attachment IDs (silent failure — controls don't bind); knob double-click reset uses the wrong default value; A/B long-press timer fires multiple times; oversampling combo's "Off" value doesn't map to enum 0.

---

## Phase 4 — DSP Engine MVP

**Goal**: the plugin actually removes fizz. One mode (5150 default), six knobs functional, no Delta Listen, no oversampling wrap, no per-mode bias yet. The minimum viable harshness controller.

### Tasks
1. Implement `InputTrim` — single gain stage, dB-scale, smoothed.
2. Implement `BandSplitter` — 4-band Linkwitz-Riley crossover at 3.8 / 5.5 / 8.0 kHz. Bands: Bite (2.4–3.8 kHz, with HPF), Plastic (3.8–5.5 kHz), Wasp (5.5–8 kHz), Ice (8–12 kHz, with LPF). Verify summed-bands == input via null test (within −60 dB).
3. Implement `TransientDetector` — tracks per-band envelope vs. fast peak; emits a 0–1 "in-transient" gate signal. The Edge Preserve knob biases this gate's threshold.
4. Implement `DynamicReducer` per band — fast attack/release envelope follower per band; applies dynamic attenuation when band energy exceeds Fizz-Hunt-controlled threshold; reduction is **scaled by (1 − transient_gate)** so reduction is suppressed during pick attack.
5. Implement `ReapMixer` — wet/dry blend.
6. Map knobs to DSP:
   - `fizzHunt` → detection threshold (lower threshold = more aggressive)
   - `edgePreserve` → transient gate width / sensitivity
   - `cabSmooth` → mid-band reducer max attenuation
   - `digitalSand` → upper-mid (Plastic + Wasp) reducer attenuation
   - `airRot` → high-band (Ice) reducer + gentle low-pass shelf
   - `reapMix` → wet/dry
7. Smooth all parameter changes with `juce::SmoothedValue` (avoid zipper noise).
8. Output gain stage with auto-gain-compensation toggle.
9. Run signal through Reaper:
   - Test signal 1: pink noise. Verify hi-mid is being attenuated dynamically.
   - Test signal 2: real high-gain DI guitar (David's existing track or library). Verify perceived fizz goes down without losing pick attack.

### Self-Review Gate 4
- [ ] Null test: at all knobs = 0 and Reap Mix = 0 (fully dry), output null-tests against bypass within −80 dB
- [ ] Setting Fizz Hunt = 100, Reap Mix = 100, all other knobs = 50: pink noise spectrum shows visible attenuation in 4–8 kHz range
- [ ] Setting Edge Preserve = 100: transient pick attack from a real DI guitar measurably preserved (use spectrogram — first 5 ms of attack peaks intact, sustain region attenuated)
- [ ] No clicks, pops, or zipper noise when sweeping any knob fast through full range
- [ ] No denormals (verify with `_MM_SET_FLUSH_ZERO_MODE` on by default)
- [ ] CPU usage at 4x oversampling-off: < 3% on David's machine for stereo 48 kHz
- [ ] Plugin handles sample rates 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz without crashing or producing garbage

**Pass criterion**: all seven boxes. The transient-preservation check is the differentiator vs. a plain dynamic EQ — if it doesn't measurably preserve attack, the architecture is wrong.

**Common failure modes**: Linkwitz-Riley crossover phase summing not flat (try to null bands and see if it adds up); transient detector firing too late and clamping the attack anyway; smoothed-value blocks updating on the wrong thread; ZIPPER noise from mode switching not being smoothed.

---

## Phase 5 — Mode System

**Goal**: 6 modes (5150 / Recto / HM-2 / Djent / Blackened / Sludge) each produce audibly distinct behavior on the same input. Mode is not just preset values — it shifts detector frequency bias, attack/release ratios, max reduction range, and Wasp Meter band labels.

### Tasks
1. Define `ModeConfig` struct: detector frequency weighting (4 floats per band), attack-ms, release-ms, edge-preserve-scaler, max-reduction-dB.
2. Six instances per spec table:
   - **5150**: 4–7 kHz presence fizz priority
   - **Recto**: 3–5.5 kHz grainy upper-mids
   - **HM-2**: 2.5–4.5 kHz mid-grind protection
   - **Djent**: 3.5–8 kHz, +50% Edge Preserve scaling
   - **Blackened**: 6–12 kHz ice/scrape
   - **Sludge**: 2–4 kHz low-mid grind
3. Mode change rebinds the four band weighting coefficients via `SmoothedValue` (300 ms ramp) — no audio glitch on switch.
4. `WaspMeter` reads the active mode and updates its zone labels accordingly (e.g. Djent mode might rename Bite zone to "Pick", Sludge mode might rename Ice to nothing).

### Self-Review Gate 5
- [ ] All 6 modes selectable via UI, parameter automatable, preserved across save/reload
- [ ] Same DI guitar input through each mode at default knob positions produces 6 audibly different outputs (informal listening test — record each, A/B in Reaper)
- [ ] Mode switching mid-playback produces no clicks/pops
- [ ] Wasp Meter zone labels change visibly within 1 frame of mode change
- [ ] CPU cost of mode change is bounded (no allocation, no IO — verify with `juce::Logger` timing inside the mode-change handler)

**Pass criterion**: all five boxes. The "6 audibly different" check is subjective — get David to do the listening test and tag any that sound too similar.

**Common failure modes**: mode coefficients don't actually change detector behavior because the band-energy comparison uses unweighted values; switching mode resets knob values (it shouldn't); Wasp Meter labels don't refresh.

---

## Phase 6 — Wasp Meter + Visual Feedback

**Goal**: every visual element is now driven by real DSP analysis. Wasp Meter shows actual spectral content with detected fizz highlighted; FIZZ % shows real-time reduction amount; IN/OUT meters show actual signal level; mode button live-dot pulses on detection.

### Tasks
1. FFT analyzer: 2048-point FFT, Hann window, ~30 Hz refresh, run on the audio thread but copy results to a lock-free FIFO that the UI reads on the message thread.
2. `WaspMeter::paint()` reads FFT bins, draws bars, overlays peak-line path, glows bins where current dynamic reduction > 0 dB.
3. `FizzReadout` shows current Fizz % = average of normalized reduction across the 4 bands. Click cycles to "Reduction dB" (peak reduction) and "Peak Hz" (which band currently has the most reduction).
4. `MeterBar` for IN/OUT: peak + RMS, segmented bar with sweep highlight, peak hold for 1.5 s.
5. Mode button live-dot: pulses brighter when its band cluster is currently being reduced.
6. CPU% display in header: actual CPU read from JUCE's `getCpuUsage()`.
7. Performance: total UI repaint at 60 Hz must not cause more than 1% CPU on David's machine.

### Self-Review Gate 6
- [ ] Wasp Meter visibly reacts to real audio — bars rise and fall with input
- [ ] When a band exceeds Fizz Hunt threshold, that band's bars glow more intensely (visible toxic green increase)
- [ ] FIZZ % readout never reads negative or > 100; always within 0–100 range; updates at 30 Hz
- [ ] Click cycles FIZZ % through 3 modes (Fizz % / Reduction dB / Peak Hz) and persists across plugin reload
- [ ] IN/OUT meters track signal level accurately (cross-check against Reaper's track meter — within 0.5 dB)
- [ ] CPU display matches JUCE's reported CPU within 0.5%
- [ ] Total UI repaint cost: < 1% CPU at 60 Hz (verify with profiler)
- [ ] No visual jank when audio is silent (meters smooth-fall to zero, no flickering)

**Pass criterion**: all eight boxes.

**Common failure modes**: FFT runs on audio thread without lock-free transfer → audio dropouts; UI repaints at 60 Hz but peak hold fires every frame; mode-button live dot pulses regardless of mode (all 6 dots pulse together).

---

## Phase 7 — Delta Listen, A/B, Oversampling

**Goal**: utility features that make the plugin feel like a serious tool. Delta Listen sells the plugin during demos; A/B is mandatory for serious producers; oversampling reduces aliasing on aggressive settings.

### Tasks
1. **Delta Listen**: when active, plugin output = (input − processed). Bypass any auto-gain. Toggle ghost-icon glows red when on. Hard-bypassed if Reap Mix = 0 (would be silent anyway — show a UI hint).
2. **A/B compare**: hold two complete parameter snapshots in the plugin state. Tap A/B toggles between them. Long-press (>800 ms) on the inactive side copies active → inactive ("copy A to B").
3. **Oversampling wrap**: use `juce::dsp::Oversampling` at 2x or 4x. Wraps `BandSplitter` + `DynamicReducer` chain. Off / 2x / 4x via combo. Verify spectrum: at 4x, no aliasing visible above 20 kHz when fed a 12 kHz sine + heavy distortion.
4. **Tooltips** on Delta Listen, A/B, Oversampling explaining what they do.

### Self-Review Gate 7
- [ ] Delta Listen produces audible "removed signal only" output that, summed with bypass output, reconstructs the original (within −80 dB null)
- [ ] A/B holds two distinct snapshots; toggling produces no clicks; long-press copy works reliably (test 20 times)
- [ ] Oversampling 4x: no audible aliasing on a 12 kHz sine + 5150 mode + Fizz Hunt 100; CPU within 8% (allowing for 4x cost increase)
- [ ] Oversampling Off: identical output to bypassing the oversampling wrap (null test)
- [ ] All three features survive plugin save/reload (state restored correctly)

**Pass criterion**: all five boxes.

**Common failure modes**: Delta Listen leaves auto-gain applied (output is louder than expected); A/B copy operation triggers two value-changes per parameter (causes audible burst); oversampling latency not reported correctly to host (causes drift in PDC).

---

## Phase 8 — Preset System + The Crypt Advanced Panel

**Goal**: 12 starter presets ship with the plugin; preset save/load works in any host; The Crypt overlay exposes advanced parameters with the brand's voice ("Settings = The Crypt", "Save = Bind Sigil", "Banish = Delete").

### Tasks
1. APVTS preset save/load: serialize to `juce::ValueTree` XML. Standard `setStateInformation` / `getStateInformation`.
2. Build 12 starter presets per spec table:
   - 5150 Wasp Coffin / Plastic IR Burial / Recto Sandpaper Mercy / Djent Razor Tax / HM-2 Without Regret / Blackened Ice Removal / Sludge Blanket Lift / Deathcore Dentist / Bedroom Amp Sim Rescue / Bus Glue Fizz Net / Lead Guitar Glass Cage / Raw Demo Salvage
3. Preset browser inside The Crypt panel: scrollable list, search box, click to load, right-click menu (Bind Sigil = save, Banish = delete).
4. The Crypt overlay: slides up from footer button click. Houses advanced params: Detector Focus, Clamp Speed, Max Reap dB (1–12 dB), Pick Window, Stereo Behavior, Quality (Eco/Normal/Ritual), Auto Gain, UI Animation toggle.
5. Verify all 12 presets sound distinctly different on the same DI input.
6. VST3 preset bank (`.vstpreset`) exported alongside, so users in Cubase/Studio One can browse them via the host's native preset menu.

### Self-Review Gate 8
- [ ] All 12 presets load correctly from a fresh plugin instance
- [ ] Saving a custom preset and reloading produces byte-identical APVTS state
- [ ] The Crypt panel opens and closes without animation glitches; doesn't break footer buttons underneath
- [ ] Each Crypt advanced parameter has an audible effect on the output (no dead controls)
- [ ] Quality = Eco mode reduces CPU by at least 30%; Ritual mode increases CPU but improves spectral resolution (verify FFT size doubles)
- [ ] `.vstpreset` files load in Reaper's VST3 preset menu

**Pass criterion**: all six boxes.

**Common failure modes**: APVTS state restore ignores parameters not present in the saved version (causes silent reset on reload after a version change); Crypt overlay z-order wrong (knobs render on top); preset names with apostrophes break the XML.

---

## Phase 9 — Validation & Polish

**Goal**: pluginval passes at strictness 10. CPU benchmarks documented. Resize, automation, undo/redo all robust. No crashes under hostile usage.

### Tasks
1. Run `pluginval --strictness-level 10 --validate-in-process Cab Rot.vst3` — fix every reported issue.
2. Stress test: open/close 100 plugin windows in Reaper without leak.
3. Stress test: load Cab Rot on 64 tracks simultaneously, verify CPU scales linearly (no per-instance contention).
4. Automation test: write parameter automation for every parameter, render the project, verify automation is followed.
5. Sample-rate stress: 44.1/48/96/192 kHz, mono/stereo, all combinations.
6. Resize stress: drag from min to max repeatedly for 60 seconds; verify no leaks, no jank, no broken layout.
7. Tooltips: every interactive control has a tooltip, no orphans, no copy-paste errors.
8. Add an "About" section in The Crypt: version, build date, Dead Pixel Harmonix credit.

### Self-Review Gate 9
- [ ] pluginval passes at strictness 10 with zero failures and zero warnings
- [ ] 100-instance test: no memory growth after settling
- [ ] 64-track test: linear CPU scaling, no shared-state contention
- [ ] Every parameter is automatable from the host
- [ ] All sample rates work without crashes or visible artifacts
- [ ] Resize is smooth at all aspect ratios within the locked range

**Pass criterion**: all six boxes. pluginval at strictness 10 is the deal-breaker — if it fails, identify the failure and circle back to the relevant earlier phase.

**Common failure modes**: pluginval flags missing factory presets, missing parameter labels, MIDI channel mismatches, threading violations (UI accessing DSP state without lock-free wrapper).

---

## Phase 10 — Release Packaging

**Goal**: ship-ready installer, demo content, Gumroad bundle, demo videos. The plugin is a product, not a build artifact.

### Tasks
1. Windows installer: Inno Setup or NSIS. Installs VST3 to `C:\Program Files\Common Files\VST3\` and presets to `%APPDATA%\Dead Pixel Harmonix\Cab Rot\`.
2. macOS package: `.pkg` installer signed and notarized (when David has a developer ID).
3. **3 demo audio clips** per spec:
   - Bedroom Amp Sim Rescue (raw harsh bedroom DI → Cab Rot smooths)
   - Modern Deathcore Rhythm (pick-attack fizz reduction with intact chug)
   - HM-2 Mix Fit (chainsaw still aggressive, vocal/snare have oxygen)
   Each rendered as before/after WAV.
4. **Demo video** — single 60–90 s landing-page video. Show the Wasp Meter detecting fizz live, then Delta Listen demo. End with the tagline.
5. **Quickstart PDF** (1 page): "Where to put it in your chain", knob explanations, 3 starter recipes.
6. **Gumroad bundle**:
   - `Cab Rot Plugin (VST3 + AU)` — $29 launch / $49 normal
   - `Cab Rot Cleanup Kit` (plugin + 12 presets + 10 DI examples + Quickstart PDF + Reaper chain examples + bonus mode preset pack) — $69 deluxe
7. Landing page on `deadpixelharmonix.com` (subdomain or new TLD per separate plan): hero with demo video, the 3 demo clips with audio comparison, FAQ, buy button.

### Self-Review Gate 10
- [ ] Installer runs on a clean Windows VM, plugin appears in Reaper after install
- [ ] All 3 demo clips render at 24-bit / 48 kHz, with bypass toggle audible difference
- [ ] Demo video uploaded to YouTube unlisted; link works; 60–90 s length
- [ ] Quickstart PDF renders correctly, no broken images, prints OK
- [ ] Gumroad bundle is set up but not published (David approves before going live)
- [ ] Landing page reviewed against `davids-writing-examples/business/` voice rules: no em dashes, no exclamation marks, no filler

**Pass criterion**: all six boxes. **Do not publish until David explicitly says "ship it."**

---

## Risk Register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Linkwitz-Riley crossover phase summing not flat | Medium | High (entire DSP architecture rests on this) | Phase 4 explicit null test; if summing isn't flat within −60 dB, switch to a different crossover topology |
| Stitch UI's OKLCH→sRGB conversion produces visible color drift | Medium | Medium | Phase 1 visual diff threshold (5%); if exceeded, validate the color formula against a reference image color picker |
| 6-mode detector bias matrix doesn't generalize across amp-sim brands | Medium | Medium | Phase 5 listening test with David's actual amp sim arsenal (Neural DSP, STL Tones, Mercuriall, Helix Native); per-mode tweak if needed |
| JSFX-first might have caught a sound problem JUCE-first won't | Low | High (would require rebuild) | Mitigated by skipping straight to JUCE — David accepted this risk explicitly. Backup plan: JSFX prototype if Phase 4 stalls |
| pluginval at strictness 10 surfaces fundamental architecture issues | Low | High | Run pluginval continuously starting Phase 4, not waiting until Phase 9 |
| OKLCH not supported by some host's plugin window compositing | Very Low | Low | Native JUCE rendering converts OKLCH→sRGB at build time — host doesn't see OKLCH at all |

---

## Decisions Locked (2026-05-05)

All 8 open questions have been resolved with David. Listed in plan order:

1. **Reap Mix** — 6th knob in bottom row, evenly spaced. Bottom row at 1200px width: 6 knobs at ~150px spacing. Treats wet/dry as a first-class control.
2. **Oversampling** — Off / 2x / 4x. Drops 8x. Default = Off for tracking, 2x mixing, 4x final render.
3. **Delta Listen** — Ghost icon in the header (replaces Stitch's `sensors` icon). Click toggles persistently. Ghost glows red (`destructive` token) when active.
4. **Window size** — Continuous resize, aspect-locked at 1.54:1. Min 1000×650, default 1200×780, max 1600×1040.
5. **Demo DI guitar** — Track DI passes on David's own rig. 3 short passages: chuggy 5150 rhythm, HM-2 grind, raw bedroom-tone phrase. Marketing angle: "this is what I tracked at home."
6. **Free version** — None. Paid-only from day 1. $29 launch / $49 normal / $69 deluxe Gumroad bundle.
7. **macOS** — Windows-first launch. macOS becomes Phase 11 after v1.0 ships and validates.
8. **JUCE 8 license** — Develop on free Personal license through Phase 9. Subscribe to Indie (~$40/month, 2026 pricing to verify) before Phase 10 release packaging. Verify current JUCE pricing/terms during Phase 0.

---

## Phase Index (one line each)

- **Phase 0** — Empty VST3 builds, loads in Reaper, audio passes through ✓
- **Phase 1** — Stitch/spec gap reconciled, OKLCH palette ported, theme test renders pixel-comparable
- **Phase 2** — Static UI skeleton matches Stitch reference, resizes cleanly
- **Phase 3** — Every control interactive and bound to APVTS; audio still unmodified
- **Phase 4** — DSP MVP: 6 knobs functional, default mode only, fizz audibly reduced without killing pick attack
- **Phase 5** — All 6 modes audibly distinct, no glitches on switch
- **Phase 6** — All visual feedback driven by real DSP analysis; Wasp Meter and FIZZ % live
- **Phase 7** — Delta Listen, A/B, Oversampling working
- **Phase 8** — 12 presets shipped; Crypt advanced panel functional with brand voice
- **Phase 9** — pluginval strictness 10 passes; stress tests clean
- **Phase 10** — Installer, demo content, Gumroad bundle, landing page ready (not yet published)

---

## How to use this plan

1. Resolve the **Open Questions** above before starting Phase 1.
2. Work one phase at a time. Do not start the next phase until the gate is passed.
3. If a gate fails, write down what failed in `LEARNINGS.md` (create on first failure) so future-you knows what to look for. Then re-run the failing tasks.
4. The plan is the source of truth. Update it when reality diverges (e.g. if Phase 4 reveals the Linkwitz-Riley crossover doesn't sum, document the alternative chosen).
5. Each phase's commit messages should reference the phase number (`phase-3: wire APVTS for 6 main knobs`) so the git log is a build journal.
