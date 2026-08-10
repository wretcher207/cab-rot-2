# Cab Rot: Project Context

JUCE 8 VST3/AU plugin. Dynamic harshness controller for high-gain guitar amp sims. Targets 2-12 kHz "wasp nest" with 4-band dynamic reduction.
Tagline: "Kill the wasp nest. Keep the teeth." | Label: Dead Pixel Harmonix

## Read first
- `PLAN.md`: 11-phase build plan with Self-Review Gates
- `HANDOFF.md`: locked decisions, authority order, cold-start brief
- `design/CANONICAL-UI.md`: UI spec and reference

## Current status
See HANDOFF.md for current phase, what's shipped, and what's next. Don't duplicate that point-in-time state here, it goes stale.

## Locked decisions
- 6 knobs: Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, Reap Mix
- 4-band crossover at 3.8 / 5.5 / 8 kHz
- Delta Listen: ghost icon in header. The ghost never glows. Inactive is
  `#4A4A47`, active is `#F2F2EF`.
- Wasp Meter labels: BITE / PLASTIC / WASP / SAND / AIR / ICE
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
history. If you find `#40FF2F`, `#0FE605` or `#54FF00` in a design here, it is
wrong. `design/CANONICAL-UI.md` is authoritative for layout geometry only; its
colour, radius and typography sections are superseded.

## Key files
- `CMakeLists.txt`: build config
- `Source/Theme/Palette.h`: the colour system. Hand-maintained from the DPD
  brand kit. No hex literal belongs anywhere else in `Source/`.
- `Resources/fonts/`: DPD Display, Inter and JetBrains Mono as TTF. JUCE cannot
  read the woff2 files the brand kit ships, so these were converted with
  fontTools. Inter is pinned at weights 400 and 500, not variable.
- `Source/DSP/`: BandSplitter, TransientDetector, DynamicReducer, ReapMixer, InputTrim
- `Source/DSP/Tuning.h`: **every constant that decides how it sounds.** Re-voice here, nowhere else.
- `tools/oklch-to-srgb.py`: retired. It generated the old Stitch palette.
- `tests/passthrough_test.cpp`: transparency gate
- `tests/dsp_test.cpp`: Phase 4 DSP gate. Its CPU check measures the machine,
  so set `CABROT_SKIP_CPU_BENCH=1` on a loaded box and read the printed figures
  instead. The other 12 checks are hard.

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
