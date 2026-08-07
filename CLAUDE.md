# Cab Rot — Project Context

JUCE 8 VST3/AU plugin. Dynamic harshness controller for high-gain guitar amp sims. Targets 2-12 kHz "wasp nest" with 4-band dynamic reduction.
Tagline: "Kill the wasp nest. Keep the teeth." | Label: Dead Pixel Harmonix

## Read first
- `PLAN.md` — 11-phase build plan with Self-Review Gates
- `HANDOFF.md` — locked decisions, authority order, cold-start brief
- `design/CANONICAL-UI.md` — UI spec and reference

## Current status
See HANDOFF.md for current phase, what's shipped, and what's next. Don't duplicate that point-in-time state here — it goes stale.

## Locked decisions
- 6 knobs: Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, Reap Mix
- 4-band crossover at 3.8 / 5.5 / 8 kHz
- Delta Listen: ghost icon in header, glows red when active
- Wasp Meter labels: BITE / PLASTIC / WASP / SAND / AIR / ICE
- Continuous resize, aspect-locked 1.54:1 (1000x650 min → 1600x1040 max)
- Native JUCE rendering, no WebView
- Aesthetic: Spectre Codex v2 — deep black + toxic neon green + scanlines
- Pricing: $29 launch / $49 normal / $69 bundle

## Key files
- `CMakeLists.txt` — build config
- `Source/Theme/Palette.h` — 55-token color system
- `Source/DSP/` — BandSplitter, TransientDetector, DynamicReducer, ReapMixer, InputTrim
- `Source/DSP/Tuning.h` — **every constant that decides how it sounds.** Re-voice here, nowhere else.
- `tools/oklch-to-srgb.py` — color conversion utility
- `tests/passthrough_test.cpp` — transparency gate
- `tests/dsp_test.cpp` — Phase 4 DSP gate

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
