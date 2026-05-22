# Cab Rot — Session Handoff

**Last updated**: 2026-05-05 (after Phase 3.5 knob upgrade)
**Repo**: https://github.com/wretcher207/cab-rot-2 (private)
**Working dir**: `c:\Users\david\workspace\cab-rot`
**Current phase**: 3.5 partially done. Phase 4 (DSP MVP) is the next concrete milestone, but a few items in Phase 3 / 3.5 still want closure.

---

## TL;DR

Cab Rot is a JUCE 8 VST3 / Standalone plugin for Dead Pixel Harmonix. Phases 0 through 3 are committed and pushed, plus a tactile-knob upgrade started under "Phase 3.5". Audio is still a pure passthrough; the DSP work begins in Phase 4.

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

Build today is 0 warnings, 0 errors across `CabRot_VST3`, `CabRot_Standalone`, `CabRot_PassthroughTest`, `CabRot_ThemeTest`. Null test passes 16384/16384.

Latest editor screenshot: [design/screenshots/phase-3.5-knobs-v2.png](design/screenshots/phase-3.5-knobs-v2.png).

---

## What's NOT done (Phase 3 closure items)

These are wired in code but not runtime-verified. They need a host or interactive Standalone session to confirm:

- [ ] Drop the VST3 in Reaper, drag every knob, confirm parameters appear in the host's automation list
- [ ] Right-click a knob, confirm "Reset to default" + "Enter value..." are present
- [ ] Save a Reaper project with non-default knob/mode values, close, reopen, confirm restore
- [ ] Press Ctrl+Z 20 times in the editor and confirm rollback through prior changes
- [ ] Hover any control for 500 ms and confirm tooltip appears

Once those four pass, Phase 3 is signed off and Phase 4 is unblocked.

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
| 3 | Phase 3.5 (b) spectral curve before Phase 4 DSP? | Open. Doing it first means the curve nodes drive `fizzHunt` per-band parameters that Phase 4 also needs to define. Doing Phase 4 first means we redo the WaspMeter once the DSP is in. |
| 4 | Phase 3.5 (c) idle breath layer? | Open. Self-contained polish; can land any time. |
| 5 | Verify 2026 JUCE Indie license pricing/terms before Phase 10? | Open per locked decision #10. Not blocking until Phase 10 packaging. |

---

## Phase 4 — DSP MVP (next concrete milestone)

**Goal per PLAN.md**: 5150 mode plays, six knobs functional, fizz audibly removed without killing pick attack. Minimum viable harshness controller.

Tasks in PLAN.md lines 222–242. Order I'd recommend:

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

### Phase 4 self-review gate (PLAN.md lines 244–252)

- [ ] All knobs at 0 + Reap Mix at 0: null vs bypass within −80 dB
- [ ] Fizz Hunt 100 + Reap Mix 100 + others 50: pink-noise spectrum shows attenuation in 4–8 kHz
- [ ] Edge Preserve 100 on real DI guitar: first 5 ms of pick attack measurably preserved (spectrogram check)
- [ ] No clicks / pops / zipper noise on fast knob sweeps
- [ ] No denormals (`_MM_SET_FLUSH_ZERO_MODE` on)
- [ ] CPU < 3% on David's machine, stereo 48 kHz, oversampling off
- [ ] Plugin handles 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz without crashing

The pluginval mention in the risk register: start running pluginval continuously from Phase 4, not waiting for Phase 9.

### Phase 4 null-test reuse

`tests/passthrough_test.cpp` is the existing harness. For Phase 4, add a sibling test `tests/sine-sweep-null.cpp` (PLAN.md repo structure) that:
1. Sets all knobs to 0 except Reap Mix (also 0, so fully dry)
2. Feeds a sine sweep
3. Confirms output equals input within −80 dB

This goes into the same `CabRot_PassthroughTest` exe (or a new sibling) as a CTest target.

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

The bundled CMake from VS 2026 Build Tools is what works on this machine. Path in scripts and below.

```powershell
$cmake = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

# Configure (only needed once, or after CMakeLists.txt changes)
& $cmake -S . -B build -G "Visual Studio 18 2026" -A x64

# Build everything
& $cmake --build build --config Release `
    --target CabRot_VST3 CabRot_Standalone CabRot_PassthroughTest CabRot_ThemeTest `
    -- /m /nologo /verbosity:minimal /clp:Summary

# Run the null test
.\build\CabRot_PassthroughTest_artefacts\Release\CabRot_PassthroughTest.exe

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

VST3 install path (user-scope, no admin): `C:\Users\david\AppData\Local\Programs\Common\VST3\Cab Rot.vst3`. Reaper scans this without configuration.

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
│   └── fonts/                      # SpaceGrotesk + JetBrainsMono ttf files (BinaryData)
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

4. Confirm with David that no decisions in §"Open decisions" have changed since 2026-05-05.
5. Pick the next item:
   - If Phase 3 closure (host verification) hasn't happened yet, prompt David for the runtime checks.
   - Otherwise: Phase 4 DSP MVP, OR Phase 3.5 spectral curve, OR Phase 3.5 idle breath. David can redirect; default if unspecified is Phase 4.

---

## Changelog

- **2026-05-05** — Initial handoff written (Phase 0 ready to start).
- **2026-05-05** — Updated. Phases 0 / 1 / 2 / 3 / 3.5(a) shipped. Visual direction shifted toward Throat-Wire depth/flow language while keeping the toxic-green Spectre Codex palette. Phase 3.5(b) spectral curve and 3.5(c) idle breath layer remain. Phase 4 is the next concrete audio milestone. Repo pushed to GitHub at `wretcher207/cab-rot-2` private.
