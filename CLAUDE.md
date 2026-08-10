# Cab Rot: Project Context

JUCE 8 VST3/AU plugin. Dynamic harshness controller for high-gain guitar amp sims. Targets 2-12 kHz "wasp nest" with 4-band dynamic reduction.
Tagline: "Kill the wasp nest. Keep the teeth." | Label: Dead Pixel Harmonix

## Read first
- `PLAN.md`: 11-phase build plan with Self-Review Gates
- `HANDOFF.md`: locked decisions, authority order, cold-start brief
- `design/CANONICAL-UI.md`: UI spec and reference
- `design/UI-FIX-SPEC.md`: governing UI truth pass. It wins where older phase
  notes or the canonical document describe placeholder data or unavailable
  controls.

## Current status
See HANDOFF.md for the current branch, verified baseline, and next gate. The
non-negotiable UI invariant is stable: an on-screen measurement must come from
real processing, and a control without audio behavior stays hidden.

## Locked decisions
- 6 knobs: Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, Reap Mix
- Four processed reduction bands: BITE 2.4-3.8 kHz, PLASTIC 3.8-5.5 kHz,
  WASP 5.5-8 kHz, ICE 8-12 kHz
- Delta Listen: ghost icon in header. The ghost never glows. Inactive is
  `#4A4A47`, active is `#F2F2EF`. Active audio is the removed signal with
  the correct polarity; Reap Mix zero produces silence.
- Wasp Meter: six log-frequency ticks, four live reduction columns, and four
  band labels: BITE / PLASTIC / WASP / ICE. No fake input spectrum.
- The editor owns one 30 Hz telemetry timer for reduction, Fizz, peak meters,
  measured CPU, LIVE, and CLIPPING / PROCESSING / IDLE. `uiAnimation=false`
  freezes the last real display values.
- A/B owns two deep APVTS snapshots, persists them in the version 2 wrapper,
  accepts legacy raw `CABROT` state, and is deliberately non-automatable.
  UI changes complete synchronously on the message thread; a generation guard
  keeps audio blocks on one coherent slot while APVTS state is replaced.
- Mode buttons are visible because the provisional six-profile DSP table now
  changes audio with 300 ms derivative ramps. The voicings still require
  David's by-ear approval. Oversampling remains hidden until the real 2x / 4x
  chain lands.
- Continuous resize, aspect-locked 1.54:1 (1000x650 min to 1600x1040 max)
- Native JUCE rendering, no WebView
- Aesthetic: the Dead Pixel Design brand system. Near-black monochrome, phosphor
  off-white ink, hairline rules, border radius 0 everywhere, no glow and no drop
  shadows. Colour is either a real state or part of the work being shown, so the
  only conditional colours are `#7FA57A` for genuinely live and `#C4574C` past
  12 dB of reduction. Binding document is
  `dead-pixel-design-v4/brand-kit/AI-BRAND-BRIEF.md`.
- Pricing: $29 launch / $49 normal / $69 bundle

**Superseded on 2026-08-10, do not reach for these.** "Spectre Codex v2", deep
black with toxic neon green and scanlines, was the aesthetic through Phase 4 and
is now dead. So is the Sunder amber rebrand in `design/sunder/`, and the copper
and knurled-hardware direction in `visual-upgrade-1.md`. All three are kept for
history. If you find the old Spectre neon greens in a design here, it is
wrong. `design/CANONICAL-UI.md` is authoritative for the current layout and
visual system, with `design/UI-FIX-SPEC.md` taking priority for data truth and
control availability.

## Key files
- `CMakeLists.txt`: build config
- `Source/Theme/Palette.h`: the colour system. Hand-maintained from the DPD
  brand kit. No hex literal belongs anywhere else in `Source/`.
- `Resources/fonts/`: DPD Display, Inter and JetBrains Mono as TTF. JUCE cannot
  read the woff2 files the brand kit ships, so these were converted with
  fontTools. Inter is pinned at weights 400 and 500, not variable.
- `Source/DSP/`: BandSplitter, TransientDetector, DynamicReducer, ReapMixer, InputTrim
- `Source/DSP/ModeConfig.h`: provisional six-profile derivative table.
- `Source/DSP/Tuning.h`: **every constant that decides how it sounds.** Re-voice here, nowhere else.
- `Source/PluginEditor.cpp`: APVTS attachments plus the single 30 Hz telemetry
  poll that drives all visible instruments.
- `Source/PluginProcessor.cpp`: DSP, lock-free UI telemetry, Delta Listen,
  and message-thread A/B snapshot handoff and persistence.
- `tools/oklch-to-srgb.py`: deleted. It generated the old Stitch palette.
- `tests/passthrough_test.cpp`: transparency gate
- `tests/dsp_test.cpp`: DSP and UI-truth gate. Its CPU check measures the
  machine, so set `CABROT_SKIP_CPU_BENCH=1` on a loaded box and read the printed
  figures instead. The verified baseline at `24fa790` reports 37/37 assertions
  with that benchmark skipped.

## Build
There is no `build.ps1` in this repo; earlier notes here were wrong about that. Use the CMake commands in HANDOFF.md.
Toolchain on this machine is **VS 2022 Build Tools** (generator `Visual Studio 17 2022`). The VS 18 / 2026 install present here has no CMake component, so the old Mac-era command path fails.
VST3 installs to: `C:\Users\wretc\AppData\Local\Programs\Common\VST3\`
Baseline: 0 warnings, 0 errors.

## Sibling project
`wretcher207/dead-pixel-harmonix` (private, remote only) is MSV-1, the first DPH plugin: Projucer-based, four DSP modules, brass/CRT UI. Cab Rot is DPH product #2.
Earlier notes here pointed at `../throatwire/` as a sibling. **No such repo exists**, locally or on GitHub. "Throat-Wire" was a visual mockup that influenced the Phase 3.5 knob work, nothing more.

## Writing
Any marketing copy, product descriptions, or tagline work: read David's voice profile before writing anything.
`C:\Users\wretc\.claude\voice\david-voice-profile.md`
