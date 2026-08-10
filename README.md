# Cab Rot

> Kill the wasp nest. Keep the teeth.

A focused dynamic harshness controller for high-gain amp-sim guitars. Removes
2 kHz to 12 kHz fizz without neutering pick attack, using four-band detection
and transient-protected reduction. Six amp-profile modes are planned, but their
buttons stay hidden until they change the audio.

Dead Pixel Harmonix. JUCE 8 / VST3 / Standalone.

## Status

Phase 4 DSP and the Tier 2 UI truth pass are implemented. The verified baseline
through `ef092c7` has real reduction telemetry, Fizz amount, peak meters, CPU,
LIVE and footer state; working Delta Listen and A/B snapshots; honest knob
defaults and units; and no fake spectrum. Passthrough is 3/3 and the DSP suite
is 33/33 with the machine-dependent CPU benchmark skipped.

The provisional mode system is the next implementation gate. Oversampling and
its control remain deferred. Final default, minimum, and maximum UI screenshots
have not been approved yet.

## Start here

**New to this project?** Read [HANDOFF.md](HANDOFF.md) first, then [PLAN.md](PLAN.md).

- [HANDOFF.md](HANDOFF.md): onboarding, locked decisions, current state, first commands
- [PLAN.md](PLAN.md): 11-phase build plan with self-review gates, architecture, risk register
- [design/CANONICAL-UI.md](design/CANONICAL-UI.md): current geometry, visual system, and truthful component states
- [design/UI-FIX-SPEC.md](design/UI-FIX-SPEC.md): adversarial UI findings and implementation gates
- Original product spec: `c:\Users\david\workspace\second-mind\wiki\sources\2026-05-05-cab-rot-plugin-spec.md`
