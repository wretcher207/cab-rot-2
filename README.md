# Cab Rot

> Kill the wasp nest. Keep the teeth.

A focused dynamic harshness controller for high-gain amp-sim guitars. Removes
2 kHz to 12 kHz fizz without neutering pick attack, using four-band detection
and transient-protected reduction. Six provisional amp-profile modes now alter
the DSP and switch with 300 ms derivative ramps.

Dead Pixel Harmonix. JUCE 8 / VST3 / Standalone.

## Status

Phase 4 DSP and the Tier 3 UI truth pass are implemented. The verified baseline
through `24fa790` has real reduction telemetry, Fizz amount, peak meters, CPU,
LIVE and footer state; working Delta Listen and A/B snapshots; honest knob
defaults and units; six real provisional modes; and no fake spectrum.
Passthrough is 3/3 and the DSP suite is 37/37 with the machine-dependent CPU
benchmark skipped. The five-target Release build is clean at 0 warnings and 0
errors.

The mode voicings still need David's by-ear approval. Oversampling and its
control remain deferred. Current default, minimum, and maximum UI captures are
in `design/screenshots/ui-truth-final-*.png` (gitignored build evidence).

## Start here

**New to this project?** Read [HANDOFF.md](HANDOFF.md) first, then [PLAN.md](PLAN.md).

- [HANDOFF.md](HANDOFF.md): onboarding, locked decisions, current state, first commands
- [PLAN.md](PLAN.md): 11-phase build plan with self-review gates, architecture, risk register
- [design/CANONICAL-UI.md](design/CANONICAL-UI.md): current geometry, visual system, and truthful component states
- Original product spec: `c:\Users\david\workspace\second-mind\wiki\sources\2026-05-05-cab-rot-plugin-spec.md`
