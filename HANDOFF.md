# Cab Rot — Session Handoff

**Last updated**: 2026-08-10 (Phase 4 listened to, lightly retuned, rebuilt, and installed)
**Repo**: https://github.com/wretcher207/cab-rot-2 (PUBLIC)
**Working dir**: `C:\Users\wretc\workspace\cab-rot` (the old `C:\Users\david\...` Mac-era paths in this file are dead)
**Branch / HEAD**: `phase-4-dsp` at `c2e08a6`, synced with `origin/phase-4-dsp`; the 2026-08-10 control-response tuning is local and uncommitted.
**Current phase**: 4 complete. David confirmed the plugin is useful on real guitar material and the REAPER behavior looks good. Phase 5 (mode system) is next.

---

## TL;DR

Cab Rot is a JUCE 8 VST3 / Standalone plugin for Dead Pixel Harmonix. It makes sound and does a decent job on real high-gain guitar material according to David's 2026-08-09 REAPER test. Phases 0 through 4 are committed: the four-band split, transient detector, dynamic reducer, mixer, and trims are wired. The Phase 4 machine gate is measured by test executables; sound quality was confirmed separately by David.

David chose to keep the **Cab Rot** identity and toxic-green UI. Do not resume the Sunder rebrand. After listening, he asked for every main knob to become effective by "just a hair." The local change adds a shallow response lift: a control at 50% drives the DSP at 52%, while 0% remains exact and 100% is unchanged. The strength constant is `kMainControlLift` in `Source/DSP/Tuning.h`; the curve is applied to all six main controls in `CabRotProcessor::updateDspParameters()`.

The retuned VST3 was built and installed to `C:\Users\wretc\AppData\Local\Programs\Common\VST3\Cab Rot.vst3`. The built and installed binaries matched exactly at SHA-256 `A14A4BDA7DF76D1252D1FDA77825A9BC1C13BDEDC4F065ED5EE0855E4073E2F1`.

The single source of truth for the build sequence is [PLAN.md](PLAN.md). Per-element design specs live in [design/CANONICAL-UI.md](design/CANONICAL-UI.md). This document is the cold-boot onboarding.

---

## What's done

| Phase | Commit | Lands |
|---|---|---|
| 0 | `3fc625d` → `bf195cc` | CMake build, JUCE 8.0.12 submodule, empty processor + editor, sample-perfect null test, snapshot tool |
| 0 | `e61ac76` | Branded placeholder editor (Space Grotesk wordmark, DPD mark, footer chrome) |
| 1 | `9a1a52c` | Design system port: 55-token `Source/Theme/Palette.h` from `tools/oklch-to-srgb.py`; bundled fonts; `SpectreLookAndFeel`; theme-test sibling app; automated visual diff via `tools/render-stitch.ps1` + `tools/compare-pngs.py` |
| 2 | `a723464` | Static UI skeleton: 6 atoms (`DpdMark`, `LivePill`, `GhostToggle`, `SpectreKnob`, `ModeButton`, `MeterPill`) + 6 region components (`HeaderBar`, `WaspMeter`, `FizzReadout`, `AmpProfileGrid`, `KnobRow`, `FooterBar`); editor composes the regions; layout scales 1000×650 → 1600×1040 with no clipping |
| 3 | `29ef836` | APVTS schema (22 parameters), UndoManager, save/restore via XML, six SliderAttachments, ButtonAttachment for Delta Listen, custom ParameterAttachment for A/B (radio-group desync fix), six ParameterAttachments for the Mode choice, ComboBoxAttachment for OS, TooltipWindow at 500 ms, Ctrl+Z / Ctrl+Y undo |
| 3.5 | `82df973` | Tactile knob render: 7-layer drawRotarySlider with shadow, recessed track, 3-stack conic glow (16 / 9 / 5 px), domed cap with overhead-lighting gradient, top highlight + spec arc, bottom inner shadow, indicator with halo + specular highlight |
| 4 | `58e18e5` + `c2e08a6` | DSP MVP. `Source/DSP/`: `Tuning.h`, `InputTrim`, `BandSplitter`, `TransientDetector`, `DynamicReducer`, `ReapMixer`. `processBlock` wired, two measured test gates, and gotchas documented. |
| 4 tuning | uncommitted | Subtle response lift across Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, and Reap Mix. Midpoint maps 50% → 52%; endpoints stay fixed. |

Build is 0 warnings, 0 errors across `CabRot_VST3`, `CabRot_Standalone`, `CabRot_PassthroughTest`, `CabRot_DspTest`, `CabRot_ThemeTest`.

Latest editor screenshot: [design/screenshots/phase-3.5-knobs-v2.png](design/screenshots/phase-3.5-knobs-v2.png).

---

## Phase 4 as built

The signal path, per chunk:

```
in -> InputTrim -> BandSplitter -> 6 bands
                                   bands summed = the reference
                                   bands 1..4: TransientDetector -> DynamicReducer -> delta
      reference + (Reap Mix * summed deltas) -> OutputTrim -> out
```

Two decisions in there are worth knowing before you change anything.

**The reducer emits a delta, not a processed band.** Each band is replaced by
`band * (gain - 1)`, the material to remove, and those deltas are added to the
untouched band sum. The alternative, crossfading a dry signal against a
processed one, comb filters, because a Linkwitz-Riley split is phase-shifted
relative to its own input. Working in deltas means Reap Mix scales how much
fizz comes out and can never comb the source. It also makes Phase 7's Delta
Listen close to free: the delta buffer already exists.

**A band knob at zero is bit-exactly inert.** No reduction is possible, so the
delta is hard zeroed and that band contributes literally nothing. Guarded by a
test, because "off" quietly meaning "nearly off" is the kind of thing that
rots a plugin's reputation.

### The Gate 4 conflict, and what was done about it

PLAN.md's Gate 4 opens with "output null-tests against bypass within -80 dB".
**That box cannot be ticked on the locked crossover topology, and it is not a
bug.** A Linkwitz-Riley pair sums to an allpass, not to unity; JUCE says so in
`juce_LinkwitzRileyFilter.h`. The idle plugin is therefore magnitude-flat and
phase-shifted. Measured: worst bin deviation 0.048 dB from 30 Hz to 20 kHz,
mean -0.0002 dB. The time-domain residual against the raw input is about
-3.9 dB, which is what allpass phase looks like and says nothing about
correctness.

This does not affect host bypass. REAPER bypassing Cab Rot does not call
processBlock, so the original signal comes back untouched.

If literal bit-transparency while active is ever wanted, the route is a
different topology: dynamic bell filters on the full-band signal with the
crossover demoted to a detection-only sidechain. That is genuinely how
soothe-class tools work, it is cheaper, and it would make Gate 4's first box
true. It is also a rewrite of the whole reduction path and a change to a
locked decision, so it is David's call, not a silent switch.

---

## Phase 3 host closure

David reported that everything looked good in REAPER on 2026-08-09 after running the requested host/listening pass. Treat these as user-confirmed rather than agent-observed:

- [x] VST3 loads and the controls behave in REAPER
- [x] Parameters and control interactions look correct
- [x] State/restore behavior looks correct
- [x] Fizz reduction is useful on real guitar material
- [x] Overall result retains enough guitar character to proceed

If a future failure report needs exact reproduction, re-run the individual automation-list, context-menu, save/restore, undo, and tooltip checks. There is no known host blocker now.

---

## Course correction in flight (2026-05-05)

David flagged a visual-direction shift after seeing the **Throat-Wire** mockup. The pivot is *depth and flow*, not colour. Cab Rot keeps its toxic-green Spectre Codex palette; what we're stealing is:

1. **Tactile control render** — done for knobs in commit `82df973`. Pattern can extend to mode buttons, A/B toggle, ghost icon if desired.
2. **Spectral curve with draggable nodes** — NOT done. Phase 3.5 (b). Replaces the static 16-bar `WaspMeter` histogram with a smooth curve plus 4 interactive control-point nodes (one per detection band). Each node binds to a parameter so the curve's shape is itself a control surface. This wiring overlaps with Phase 4 DSP plumbing, so doing it before Phase 4 saves a refactor later.
3. **Idle breath animation layer** — NOT done. Phase 3.5 (c). Single 30 Hz timer in `PluginEditor` driving phase offsets the LookAndFeel reads when painting glows / panel highlights. Cheap on CPU, makes the panel never look static.

Status: knob layer landed and committed. Curve and breath remain. David indicated "we'll iterate" — pick one when resuming.

---

## Open decisions

| # | Decision | Status |
|---|---|---|
| 1 | Bundle Space Grotesk Black (900) for the wordmark? | Currently using Bold (700). Black is on Google Fonts but the sandbox denied an agent-chosen download. David needs to either grab the TTF manually into `Resources/fonts/` and re-run `juce_add_binary_data`, or grant network permission once. |
| 2 | Bundle JetBrains Mono Medium (500) for ui-chrome? | Currently using Regular (400). Same as above. |
| 3 | ~~Phase 3.5 (b) spectral curve before Phase 4 DSP?~~ | **Resolved by events.** Phase 4 went first. The WaspMeter is still the static 16-bar histogram and now wants real data, so the curve work folds naturally into Phase 6 rather than standing alone. `getBandReductionDb()` is already on the processor waiting for it. |
| 4 | Phase 3.5 (c) idle breath layer? | Open. Self-contained polish; can land any time. |
| 5 | Verify 2026 JUCE Indie license pricing/terms before Phase 10? | Open per locked decision #10. Not blocking until Phase 10 packaging. |
| 6 | **Keep the Cab Rot identity or finish the Sunder rebrand?** | **Resolved 2026-08-09: keep Cab Rot.** The toxic-green UI and current six-control product direction remain canonical. Sunder is rejected for this product. |
| 7 | **Crossover topology.** | **Resolved for the current build:** keep the Linkwitz-Riley topology. David found the real-guitar result useful and reported no unacceptable bypass issue. Reopen only if a specific mix exposes an audible phase problem. |

---

## Phase 4 — DSP MVP (DONE, measured)

**Goal per PLAN.md**: 5150 mode plays, six knobs functional, fizz audibly removed without killing pick attack. Minimum viable harshness controller.

Built in the order below. Kept here because it still describes the code.

1. **`Source/DSP/InputTrim.{h,cpp}`** — single SmoothedValue gain stage on a per-channel basis, dB to gain conversion. Tied to `params::inputGain`.
2. **`Source/DSP/BandSplitter.{h,cpp}`** — 4-band Linkwitz-Riley crossover at 3.8 / 5.5 / 8 kHz. Bands: Bite (2.4–3.8 kHz, with HPF), Plastic (3.8–5.5 kHz), Wasp (5.5–8 kHz), Ice (8–12 kHz, with LPF). **Phase 4 explicit null test required**: feed pink noise, sum bands, confirm sum == input within −60 dB. If not, switch crossover topology immediately. Risk register flagged this as Medium / High.
3. **`Source/DSP/TransientDetector.{h,cpp}`** — per-band fast/slow envelope ratio. Output is a 0..1 in-transient gate. Threshold biased by `params::edgePreserve`.
4. **`Source/DSP/DynamicReducer.{h,cpp}`** — per-band envelope follower with attenuation scaled by `(1 − transient_gate)`. The transient suppression is the architectural differentiator vs. plain dynamic EQ.
5. **`Source/DSP/ReapMixer.{h,cpp}`** — wet/dry blend driven by `params::reapMix`.
6. **Wire processBlock**: `InputTrim` → `BandSplitter` → per-band [`TransientDetector` → `DynamicReducer`] → sum → `ReapMixer` → output gain.
7. **Knob ↔ DSP mapping** per PLAN.md line 230:
   - `fizzHunt`: detection threshold (lower = more aggressive)
   - `edgePreserve`: transient gate width / sensitivity
   - `cabSmooth`: mid-band reducer max attenuation
   - `digitalSand`: upper-mid (Plastic + Wasp) reducer
   - `airRot`: high-band (Ice) reducer + gentle LPF shelf
   - `reapMix`: wet/dry
8. **Smoothing**: every parameter goes through `juce::SmoothedValue` to avoid zipper noise.

### Phase 4 self-review gate: measured results

Run `CabRot_DspTest.exe` to reproduce all of these. The latest complete run was
on this machine after the response-lift change, 2026-08-09.

- [x] ~~All knobs at 0 + Reap Mix at 0: null vs bypass within −80 dB~~ **Superseded.** Not achievable on an LR crossover, see the conflict note above. Replaced by two checks that are: magnitude flat within **0.048 dB** worst bin, 30 Hz to 20 kHz, and band ceilings at zero are **bit-exactly** inert (max difference 0.000000000000).
- [x] Fizz Hunt 100 + Reap Mix 100 + others 50: attenuation in 4-8 kHz. **-9.18 dB** in 4-8 kHz after the slight control-response lift, **-0.02 dB** below 900 Hz. Surgical, not a broadband dip.
- [x] Edge Preserve 100: pick attack measurably preserved. Attack peaks survive **4.28 dB** louder at Edge Preserve 100 than at 0, same reduction settings. This is the architectural claim, and it holds.
- [x] No clicks / pops / zipper noise on fast knob sweeps. Worst output slew **1.486x** the input's while throwing Fizz Hunt and Reap Mix end to end every block.
- [x] No denormals. Silence after a burst decays to **true zero**.
- [x] CPU < 3%, stereo 48 kHz, oversampling off. Latest clean-build run measured **2.184%** worst case and **1.633%** for the idle split, with the reduction costing **0.551%**. Earlier repeatable runs measured 2.6 to 2.9%, so retain the <3% gate rather than treating the lowest single run as a new baseline.
- [x] 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz. All sane. Block sizes 1 / 7 / 64 / 512 / 2048 too, including blocks larger than the prepared size, which get sliced.

David closed the subjective gate on 2026-08-09: the effect does a decent job,
the controls and host behavior look good, and the product is worth continuing.

The pluginval mention in the risk register: start running pluginval continuously from Phase 4, not waiting for Phase 9. **Not yet run on this machine.**

### 2026-08-10 cold-start state

- Local changes: `Source/DSP/Tuning.h` and `Source/PluginProcessor.cpp` only.
- Verification after the change: `CabRot_PassthroughTest` **3/3 passed**; `CabRot_DspTest` **13/13 passed**.
- Fresh Release artifacts exist for VST3, Standalone, passthrough test, and DSP test.
- Installed VST3 is byte-for-byte the tested build, proven by matching SHA-256 above.
- No commit or push has been made for the response-lift change.
- **Next concrete step:** review and commit the two-file tuning change, push `phase-4-dsp`, run pluginval, then begin Phase 5's six-mode system.

### The test harness

- `tests/TestSupport.h` — shared helpers: deterministic pink noise, parameter setters, averaged FFT spectra, band-delta maths.
- `tests/passthrough_test.cpp` — the fast transparency gate. Repurposed from the Phase 0 sample-perfect null, which the Phase 4 topology retired.
- `tests/dsp_test.cpp` — the Phase 4 gate, nine checks, about 30 s.

---

## Phases 5 → 10 (one-line each, refer PLAN.md for detail)

- **Phase 5** — Mode system: `ModeConfig` struct + 6 instances; mode change ramps coefficients via SmoothedValue; informal listening test confirms 6 audibly distinct outputs.
- **Phase 6** — Wasp Meter live: 2048-pt FFT on audio thread, lock-free FIFO to UI; FIZZ % readout from real reduction; mode-button live dot pulses on detection. Total UI cost < 1% CPU at 60 Hz.
- **Phase 7** — Delta Listen audio (already toggles visually); A/B snapshot pair (Phase 3 stubbed only the toggle, the snapshot logic lands here); `juce::dsp::Oversampling` wrap for 2x / 4x.
- **Phase 8** — 12 starter presets + The Crypt advanced overlay (Crypt parameters already declared in APVTS so state migration is free).
- **Phase 9** — pluginval strictness 10; 100-instance + 64-track stress; full automation lane test; resize stress.
- **Phase 10** — Windows installer (Inno Setup or NSIS), 3 demo audio clips, demo video, Quickstart PDF, Gumroad bundle prep, landing page. macOS becomes Phase 11 after v1.0 ships.

---

## Build & install commands (verified working)

**Changed 2026-08-07.** The old machine used the CMake bundled with VS 2026
(18) Build Tools. This machine has both toolchains installed, but the VS 18
install here has **no CMake component**, so the path in the old instructions
does not exist. Use the VS 2022 Build Tools pair instead. Both are present,
and 2022 is the one that is complete.

```powershell
$cmake = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

# Configure (only needed once, or after CMakeLists.txt changes)
& $cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# Build everything
& $cmake --build build --config Release `
    --target CabRot_VST3 CabRot_Standalone CabRot_PassthroughTest CabRot_DspTest CabRot_ThemeTest `
    -- /m /nologo /verbosity:minimal /clp:Summary

# Transparency gate (fast)
.\build\CabRot_PassthroughTest_artefacts\Release\CabRot_PassthroughTest.exe

# Phase 4 DSP gate (about 30 s, mostly the CPU benchmark)
.\build\CabRot_DspTest_artefacts\Release\CabRot_DspTest.exe

# Snapshot the editor (Standalone -> PNG, with chrome cropped)
.\tools\visual-diff.ps1 -SkipBuild

# Snapshot the theme test (sibling app)
.\tools\visual-diff.ps1 -SkipBuild -ThemeTest

# Resize stress
.\tools\visual-diff.ps1 -SkipBuild -WindowSize 1000x680 -OutPath design\screenshots\min.png
.\tools\visual-diff.ps1 -SkipBuild -WindowSize 1600x1070 -OutPath design\screenshots\max.png

# Install to user-scope VST3 folder Reaper scans (close DAW first)
.\tools\install-vst3.ps1
# or, if you're certain no DAW has unsaved work:
.\tools\install-vst3.ps1 -KillBlockers

# Regenerate Source/Theme/Palette.h from the Stitch tailwind config
python tools\oklch-to-srgb.py
python tools\oklch-to-srgb.py --check    # exits 0 only if up-to-date

# Render Stitch reference HTML to PNG (Chrome headless)
.\tools\render-stitch.ps1

# Diff two PNGs (PIL fallback if ImageMagick isn't on PATH)
python tools\compare-pngs.py design\screenshots\stitch-reference.png `
                              design\screenshots\phase-3.5-knobs-v2.png `
                              --fuzz 1 --threshold 8
```

VST3 install path (user-scope, no admin): `C:\Users\wretc\AppData\Local\Programs\Common\VST3\Cab Rot.vst3`. Reaper scans this without configuration.

`CABROT_COPY_AFTER_BUILD` is OFF by default in `CMakeLists.txt` so the post-build copy doesn't fight a running DAW. Use `tools/install-vst3.ps1` to install on demand.

---

## Risks and gotchas captured during phases 0–3

These are documented because they bit at least once during the build:

- **Reaper locks the VST3** when it has the plugin loaded. Auto-copy-on-build was disabled in commit `e61ac76` for this reason. Use `tools/install-vst3.ps1` after closing Reaper, or pass `-KillBlockers` if no DAW has unsaved work.
- **C4996 / strcpy warnings** from the VST3 manifest helper come from JUCE 8.0.12's bundled Steinberg SDK. Silenced with target-scoped `_CRT_SECURE_NO_WARNINGS` + `/wd4996` on `CabRot_vst3_helper` only. See `~/.claude/memory/tools/juce-windows.md`.
- **`JUCE_DISPLAY_SPLASH_SCREEN`** is ignored in JUCE 8.0.12 — the splash is gone. Don't set the macro; setting it triggers a warning.
- **`/wd4458`** is needed on the plugin target for juceaide-generated headers under `/W4`.
- **`juce::Colour` is not constexpr** in JUCE 8. The palette generator emits `inline const`, not `inline constexpr`.
- **`juce::ParameterAttachment` is non-copyable** — vectors of it require `unique_ptr` storage.
- **`Font::getStringWidth` is deprecated** in JUCE 8. Use `juce::GlyphArrangement::getBoundingBox`.
- **A/B `ButtonAttachment` desync**: a plain `ButtonAttachment` on B alone leaves both A and B dark when the host writes `aOrB=false`, because JUCE's radio group only deselects on button-on. Workaround in `PluginEditor.cpp`: a single `ParameterAttachment` drives both buttons in lockstep.
- **`PrintWindow` flag 2** is required on JUCE 8 (Direct2D). Flag 0 returns black. `tools/visual-diff.ps1` already uses flag 2.
- **Benchmarking a plugin over one long buffer measures the wrong thing.** The first CPU test here streamed 30 seconds of audio (11.5 MB) in a single pass and read 3 to 7% with wild run-to-run variance. That is memory bandwidth, not the plugin: a real host hands over 512 samples at a time out of warm cache. Timing the best of nine passes over a 2-second cache-resident buffer gives 2.6% and repeats to within 0.05%. Same code, same flags. If a CPU figure here ever looks alarming, check the harness before optimising anything.
- **The test targets did not link LTO** while the plugin target did, so the benchmark was measuring a slower binary than the one that ships. `juce::juce_recommended_lto_flags` is on all three test targets now. Keep it that way when adding a target.
- **`juce::dsp::LinkwitzRileyFilter::processSample` with two outputs ignores `setType`.** It computes both low and high from the same state regardless. Harmless, but do not read a `setType (lowpass)` call next to it as meaningful.
- **JUCE Standalone audio-settings dialog** can briefly grab `MainWindowHandle` and trip the snapshot tool's "Window too small" guard. The snapshot tool retries once after a 2 s sleep.
- **Visual diff vs Stitch**: current 67–70% per-pixel difference is honest, not a regression. The canonical decisions in `design/CANONICAL-UI.md` §8 (Reap Mix, ghost icon, named zones, footer chrome) intentionally diverge from the Stitch export. The 5% / 8% targets in PLAN.md gates 1 and 2 are documented as canonical-divergence-aware.

---

## Where things live

```
cab-rot-2/
├── CMakeLists.txt                  # build, source lists split: theme / atoms / regions / shared
├── HANDOFF.md                      # this file
├── PLAN.md                         # build phases 0..10 with self-review gates
├── README.md
├── design/
│   ├── CANONICAL-UI.md             # per-element spec (Phase 1 deliverable)
│   ├── stitch-reference.html       # original visual reference
│   └── screenshots/                # gitignored; visual-diff outputs land here
├── JUCE/                           # submodule pinned to tag 8.0.12
├── Resources/
│   └── fonts/                      # DPD Display + Inter + JetBrainsMono ttf (BinaryData)
├── Source/
│   ├── PluginProcessor.{h,cpp}     # APVTS schema lives here, processBlock is passthrough
│   ├── PluginEditor.{h,cpp}        # composes regions, owns attachments + LookAndFeel
│   ├── Theme/
│   │   ├── Palette.h               # GENERATED. 55 sRGB tokens. tools/oklch-to-srgb.py.
│   │   ├── Fonts.{h,cpp}           # SpinLock-guarded typeface cache + 5 typography slots
│   │   └── SpectreLookAndFeel.{h,cpp}  # drawRotarySlider, drawButtonBackground, etc.
│   └── UI/
│       ├── atoms/                  # DpdMark, LivePill, GhostToggle, SpectreKnob, ModeButton, MeterPill
│       ├── HeaderBar.{h,cpp}
│       ├── WaspMeter.{h,cpp}       # to be replaced by spectral curve in Phase 3.5b
│       ├── FizzReadout.{h,cpp}
│       ├── AmpProfileGrid.{h,cpp}
│       ├── KnobRow.{h,cpp}
│       ├── FooterBar.{h,cpp}
│       ├── ThemeTest.{h,cpp}       # phase-1 visual-diff harness, separate target
│       └── ThemeTestApp.cpp        # JUCEApplication entry for ThemeTest
├── tests/
│   └── passthrough_test.cpp        # null test, runs as CTest "passthrough"
└── tools/
    ├── oklch-to-srgb.py            # palette generator (run + --check)
    ├── render-stitch.ps1           # Chrome headless -> PNG of Stitch HTML
    ├── compare-pngs.py             # PIL-based image diff with ImageMagick fallback
    ├── visual-diff.ps1             # build / launch / capture / crop / optional diff
    └── install-vst3.ps1            # user-scope install with optional -KillBlockers
```

The Crypt advanced parameters are declared in APVTS but the UI lands in Phase 8. New phases that add UI for them won't trigger a state migration since the params already exist.

---

## David's working preferences (still apply)

- **No em dashes** in delivered prose (commit messages OK, user-facing copy not). Use commas, periods, parens, colons.
- **Terse responses**. No trailing summaries when the diff already shows the change.
- **Work autonomously**. Don't ask permission for routine commits, pushes to feature branches, rebases, branch creation.
- **Ask only when destructive** — `git reset --hard` with uncommitted changes, force push, deleting branches with unmerged commits, `rm -rf` outside scratch dirs.
- **Honesty rules**: never fabricate file contents, command output, or API behaviour. If unverified, say so, then verify or ask.
- **Stay on the path**: when David specifies a tool / approach, finish on that path. Real blocker -> stop and report. No silent pivots.
- **No lectures** about things he obviously knows.

---

## How to boot a new session

1. Read this file end-to-end first.
2. Skim [PLAN.md](PLAN.md) for the phase you're about to work on.
3. Run a quick state check:

```powershell
git log --oneline | Select-Object -First 6
.\build\CabRot_PassthroughTest_artefacts\Release\CabRot_PassthroughTest.exe   # null test
```

4. Confirm with David that no decisions in §"Open decisions" have changed. Decisions 6 (identity) and 7 (topology) are the ones that gate real work.
5. Pick the next item:
   - Phase 3 host verification still has not happened. It needs David at the keyboard in REAPER, not an agent.
   - Otherwise: Phase 5 mode system is the next agent-shaped block, but the six mode tunings are ear work and land better after David has listened to Phase 4 on his own material.

---

## Changelog

- **2026-05-05** — Initial handoff written (Phase 0 ready to start).
- **2026-08-07** — Phase 4 DSP MVP built and measured on the Windows machine. First build of this repo on `wretc`; the Mac-era source compiled with 0 warnings and 0 errors once the CMake path was pointed at VS 2022 instead of the incomplete VS 18 install. `Source/DSP/` created (Tuning, InputTrim, BandSplitter, TransientDetector, DynamicReducer, ReapMixer), processBlock wired, `juce_dsp` added to the link lines. Two test executables replace the retired Phase 0 null test. Found and documented a genuine conflict between the locked LR crossover and Gate 4's first checkbox. The plugin makes sound and has never been heard by anyone.
- **2026-05-05** — Updated. Phases 0 / 1 / 2 / 3 / 3.5(a) shipped. Visual direction shifted toward Throat-Wire depth/flow language while keeping the toxic-green Spectre Codex palette. Phase 3.5(b) spectral curve and 3.5(c) idle breath layer remain. Phase 4 is the next concrete audio milestone. Repo pushed to GitHub at `wretcher207/cab-rot-2` private.
