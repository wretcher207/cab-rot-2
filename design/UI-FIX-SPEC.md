# Cab Rot UI Fix Spec

Executor: an agent working from this document alone. Author: adversarial
UI/UX review, 2026-08-10. Target branch: continue on `main` (post-facelift),
or cut `fix/ui-truth` from it.

## 0. Read first, non-negotiable

1. `HANDOFF.md` end to end. Build commands, machine traps (VS 2022 CMake
   path, Reaper VST3 lock, test env var) live there and are not repeated.
2. `design/CANONICAL-UI.md` for the geometry and token system. This spec
   changes data wiring and a few layout decisions; where it conflicts with
   CANONICAL-UI, this spec wins, and you must update CANONICAL-UI in the
   same commit so the docs never lie.
3. `PLAN.md` phases 5 and 7 for the mode system and oversampling context.

## 0.1 The governing rule

**Nothing on screen may claim to measure, monitor, detect, or switch unless
the claim is true in the current build.** If a capability is real, wire the
UI to it. If it is not real yet, hide the UI until the phase that ships it.
Fabricated telemetry and placeholder readouts are the failure mode this
spec exists to remove; do not reintroduce them in a new form.

Brand invariants that still bind every paint path: all colour through
`Source/Theme/Palette.h` tokens only (no hex literals anywhere else in
`Source/`), border radius 0, 1 px `rule` hairlines, no glow, no shadows,
scanlines only inside the spectral frame at <= 6%. `stateLive` is reserved
for genuinely-live indication, `stateError` for reduction past 12 dB.

Known-checked current state (do not re-verify, act on it):

- `WaspMeter` renders two hardcoded float arrays (`Source/UI/WaspMeter.h:52-70`). No timer, no repaint source.
- `Fizzreadout` hero number is the literal `66.1f` (`Source/UI/FizzReadout.h:24`).
- Header CPU value is the string literal `"4.2%"` (`Source/UI/HeaderBar.h:33`). Twin lives in `Source/UI/ThemeTest.cpp:115`.
- Footer IN/OUT meters are constructor constants 0.70 / 0.85 (`Source/UI/FooterBar.cpp:21-22`).
- `LivePill` breathes at 30 Hz unconditionally while visible (`Source/UI/atoms/LivePill.cpp`).
- Footer status is the constant `"V0.1.0 / PROCESSING"` (`Source/UI/FooterBar.cpp:12`).
- `mode`, `deltaListen`, `aOrB`, `oversampling` parameters exist in the APVTS layout but are read by no audio code (`Source/PluginProcessor.cpp:56-65` define them; `processChunk` never touches them).
- `getBandReductionDb()` already returns live per-band reduction dB from atomics written in `processChunk` (`Source/PluginProcessor.cpp:176, 325-328`). Four processed bands: BITE, PLASTIC, WASP, ICE.
- Reap Mix param default is 100 but `SpectreKnob` double-click returns 50 (`Source/UI/atoms/SpectreKnob.cpp`, `Source/PluginProcessor.cpp:47`).
- Knob value readouts have no unit (`KnobRow.cpp` passes no suffix).
- Six zone labels BITE/PLASTIC/WASP/SAND/AIR/ICE imply six bands and a linear frequency axis; the engine has four bands across 2.4k-12k.
- Auto Gain defaults ON in the DSP with no UI anywhere (`mixer.setAutoGain(wantsAuto)` in `updateDspParameters`).
- `KnobRow.cpp:9-18` constructor "seed" values are dead code; attachments own state.

## Tier 1 - Truth pass (no DSP work, deletions and hides)

Tasks, in commit order. One commit per task.

**T1.1 Remove the fake CPU readout.**
`HeaderBar.cpp`: delete the CPU label/value drawing and its layout slots
(locate `cpuLabelArea`, `cpuValueArea`). `HeaderBar.h`: delete the
`cpuValue` member. `ThemeTest.cpp`: delete the `4.2%` draw at line ~115.
The real CPU element returns in Tier 2.5; this commit only removes the lie.

**T1.2 Stop the meters pretending.**
`FooterBar.cpp` ctor: delete both `setLevel` calls and call
`inMeter.setVisible(false); outMeter.setVisible(false);`. Leave
`MeterPill` untouched. They return wired in Tier 2.4.

**T1.3 Fizz readout idle state.**
`FizzReadout`: replace the `66.1f` literal member with a nullable state.
When unwired, render `--` in the same hero treatment (inkPrimary number
slot, metadata-grey `%` suppressed) plus leave the `FIZZ AMOUNT` label.
Do not ship a number that did not come from the audio thread. Wired value
arrives in Tier 2.3.

**T1.4 Footer status becomes a state, not a slogan.**
`FooterBar.cpp`: replace the `kStatusText` constant with
`"V0.1.0"` always, plus a second segment whose text comes from a
`setStatusText(juce::String)` setter. Default segment: `"IDLE"`.
Real states land in Tier 2.6; the constant string goes now.

**T1.5 Kill dead seed code and the fake zones.**
`KnobRow.cpp`: delete the `kSeeds` default-value block; the attachments
set values. Keep the label strings only.
`WaspMeter.cpp` `paintLabels`: replace both arrays. Frequencies:
`1k, 2k, 4k, 8k, 12k, 20k` plotted LOG-spaced (formula in T2.2). Zones:
exactly four, `BITE, PLASTIC, WASP, ICE`, centered between the real
crossover boundaries 2.4k / 3.8k / 5.5k / 8k / 12k. Delete `SAND` and
`AIR` labels.

**T1.6 Hide the OS combo.**
`FooterBar`: `oversample.setVisible(false)` and skip its layout slot. The
parameter stays in the APVTS (automation lanes and state compatibility).
Do NOT delete the parameter or the ComboBoxAttachment; Phase 7 re-shows it
with a real `dsp::Oversampling` chain behind it.

Acceptance for Tier 1: five-target Release build 0/0; passthrough test
3/3; `CABROT_SKIP_CPU_BENCH=1` DSP test 12/12; `tools/visual-diff.ps1
-SkipBuild` screenshot shows no CPU text, no meter bars, `--` fizz number,
no OS combo, four zone labels, `V0.1.0 / IDLE` footer. The fake spectral
curve and reference shape are still present at this tier; they are
replaced wholesale in Tier 2.2, do not patch them.

## Tier 2 - Wire what already exists

All of the data these tasks display already lives on or near the audio
thread. Add one UI polling timer and read it.

**T2.1 One editor-owned timer.**
`CabRotEditor`: subclass `juce::Timer`, `startTimerHz(30)` in the ctor.
Each tick, read processor atomics once and push values into the children
via setters (`waspMeter.setBandReduction(...)`,
`fizzReadout.setValue(...)`, `footerBar.setLevels(...)`,
`headerBar.setCpu(...)`, `headerBar.setEngineLive(bool)`,
`footerBar.setStatus(...)`). Children repaint only when a value changed.
The `uiAnimation` parameter already exists: when false, stop the timer
and freeze all instruments at their last real values (never fake "0").

**T2.2 WaspMeter: live reduction columns on a log axis.**
This replaces every placeholder in `WaspMeter.h`. Pass
`CabRotProcessor&` into the ctor (editor owns the reference). Delete the
`reference[]` and `reductionDb[]` arrays and their paint methods.

- X axis: log frequency, 1 kHz..20 kHz. `x(f) = plot.x + plot.w * log10(f/1000.0) / log10(20.0)`. Frequency labels and ticks per T1.5, positioned with this formula.
- Content: four columns, one per processed band, spanning that band's horizontal extent on the log axis (boundaries 2.4k / 3.8k / 5.5k / 8k / 12k). Column height = current reduction dB on the existing 0..-24 dB vertical scale (`kReductionScaleMaxDb` already in `Palette.h`), drawn downward from the 0 dB top of the plot. Fill: `surface3`; top edge: 1.5 px `inkPrimary`. Any column deeper than `kReductionDamageThresholdDb` paints its top edge `stateError` below the threshold line instead (reuse the clipped-second-pass pattern from the deleted curve code).
- Peak hold: per-column thin residual line at the max over the last ~800 ms, 1 px `inkBody` at 60%.
- Silence: when processor reports no input (see T2.6 atomic), columns decay to zero and stay there. Empty frame with scale is a valid, professional resting state.
- Keep: frame, scanlines, dB scale, -12 dB full-strength damage guide, header strip. Rename header strip text to `GAIN REDUCTION` / `WASP METER`.
- The live input spectrum (FFT) is explicitly out of scope, Phase 6. No fake spectrum, no static fill.

**T2.3 FIZZ AMOUNT, real number.**
Definition (put this exact formula in code and in a comment):
`fizzPct = smoothed( min(1.0, maxBandReductionDb / kReductionDamageThresholdDb) ) * 100`.
Source: the same four band atomics. When the engine reports silence
(>1 s below -72 dBFS input), render the `--` idle state from T1.3. One
decimal, JetBrains Mono, `%` restored when showing a number.

**T2.4 IN/OUT meters, real levels.**
Processor: after `inputStage.process` and after `outputStage.process`,
compute per-block peak into two `std::atomic<float>` members
(`std::max` over abs samples is fine; keep it lock-free and branch-light).
Expose `getInputPeak()` / `getOutputPeak()`. `MeterPill`: convert to dB,
map -60..0 dB onto 0..1, decay in the UI at ~12 dB/s, `setVisible(true)`
again. Add a clip flag: if output peak >= 0 dBFS, hold a 1 px `stateError`
segment at the bar's right end for 1.5 s.

**T2.5 CPU, measured not imagined.**
Processor: wrap `processBlock` body with
`juce::Time::getHighResolutionTicks()`. EMA (alpha ~1/32) of
`elapsed / (numSamples / getSampleRate())` into `std::atomic<double>`.
Expose `getCpuPercent()`. Header: restore the CPU label/value (mono,
12.0f value in `inkBody`, label in `inkMeta`, same slot as before),
formatted `%.1f%%`, updated from the editor timer. If the editor is
closed the EMA just keeps running at the last measured rate; that is
honest.

**T2.6 LIVE and footer status become semantics.**
Processor: atomic smoothed input magnitude (reuse the input peak EMA).
`isEngineLive()` returns true when input has exceeded -72 dBFS within the
last second.
`LivePill`: delete the phase/Timer breathing. Render: `stateLive` square
when live, `inkDisabled` square when silent. Kill `LivePill`'s own timer;
the editor timer drives it.
`FooterBar` status segments: `PROCESSING` when live, `IDLE` when silent,
`CLIPPING` while the T2.4 clip flag is held. Priority: CLIPPING >
PROCESSING > IDLE.

**T2.7 Delta Listen, real.**
`processChunk`: when the `deltaListen` param is true, after the reducer
loop has filled `deltaBuffer`, copy `deltaBuffer * mixAmt` into `block`
and return before the mixer (skip mixer and output trim? No: still run
`outputStage` so output trim applies; skip only `mixer.process`).
Result: at Reap Mix 0, delta listen is silence (correct: nothing is
being removed); at 100 it is exactly the removed material. Two-line test
in `tests/dsp_test.cpp`: engage delta listen at Reap Mix 100, assert
output equals the measured delta path within tolerance; at mix 0 assert
near-zero floor. The ghost toggle already lights inkPrimary when on; no
UI change needed beyond the existing tooltip.

**T2.8 A/B, real snapshots.**
Processor members: `juce::ValueTree slotStateA, slotStateB;` (invalid =
never stored). Behaviour:
- First entry to B: `slotStateA = apvts.copyState()`, then
  `slotStateB = apvts.copyState()` (B starts as a copy of A, so the first
  compare is instant). Set the copy's `aOrB` param to 1.0 in `slotStateB`.
- Switching slots: snapshot the departing slot, then `apvts.replaceState`
  with the target slot's tree (with the `aOrB` value forced to the target).
- Persistence: `getStateInformation`: after building the APVTS xml, append
  `slotStateA`/`slotStateB` as child elements. `setStateInformation`:
  restore APVTS first, then read the children back, then
  `replaceState` from the slot matching the restored `aOrB`.
- Keep the existing footer A/B buttons and the lockstep
  ParameterAttachment; do not add a "copy A to B" control in this pass.
Test: in `tests/dsp_test.cpp`, set a knob, switch to B, set it differently,
switch back, assert both parameter restoration and that processing reflects
the restored value.

**T2.9 Knob honesty fixes.**
- `PluginEditor::wireAttachments`: after creating each SliderAttachment,
  read the parameter default and call
  `slider.setDoubleClickReturnValue(true, defaultVal)` where
  `defaultVal = slider.getNormalisableRange().convertFrom0to1(param->getDefaultValue())`. Fixes Reap Mix 100 vs the hardcoded 50 in `SpectreKnob.cpp` (delete the hardcoded call there).
- `KnobRow.cpp`: pass `"%"` as the `unitsSuffix` for all six knobs, so
  values read `50%` / `100%`.

**T2.10 Knob interaction emphasis.**
`SpectreLookAndFeel::drawRotarySlider`: when
`slider.isMouseOverOrDragging()`, stroke the value arc at 2.5 px instead
of 1.5 and draw the indicator at full `inkPrimary` 1.5 px. No colour
change, no glow. This is the entire hover/drag affordance and it is
enough.

Acceptance for Tier 2: all Tier 1 acceptance points, plus with audio
playing through the Standalone build the four columns move, the fizz
number moves, `--` returns on silence, meters track, CPU prints a real
value, LIVE dot goes dark on silence, delta listen is audibly the removed
material at mix 100 and silent at mix 0 (assert in dsp_test), A/B round
trip passes its test, build 0/0, both suites green.

## Tier 3 - Phase 5 mode system, provisional tunings

Implements PLAN Phase 5 so the AMP PROFILE grid stops being a dead
switch. Tunings below are provisional starting points; flag in your
summary that David validates all six by ear before release.

- New header `Source/DSP/ModeConfig.h`: struct
  `{ float thresholdOffsetDb; float ceilingScale; float attackScale; float edgeBias; float shelfStart; }` plus `static const ModeConfig kModes[6];` in index order matching the `modeNames` array (`5150, Recto, HM-2, Djent, Blackened, Sludge`).
- `updateDspParameters()`: read the mode index, fetch the config, apply:
  `thresholdDb += thresholdOffsetDb`, per-band ceilings
  `*= ceilingScale` (HM-2 scales WASP only: add a per-band scale array if
  simpler), `attackMs *= attackScale`, `edge = clamp(edge + edgeBias)`,
  and `kAirRotShelfStart` replaced per-mode. Ramp all five derivatives
  through existing `juce::SmoothedValue`s already feeding the reducers;
  mode changes must not click (add a sweep assertion mirroring the
  existing slew test with a mode change mid-buffer).
- Provisional table:

| Mode | thresholdOffsetDb | ceilingScale | attackScale | edgeBias | shelfStart |
|---|---|---|---|---|---|
| 5150 | 0 | 1.00 | 1.0 | 0.0 | 0.50 |
| Recto | -2 | 1.15 | 0.8 | 0.0 | 0.50 |
| HM-2 | -1 | 1.30 (WASP band 1.5) | 0.9 | -0.1 | 0.50 |
| Djent | -1 | 1.10 | 0.6 | +0.15 | 0.55 |
| Blackened | -3 | 1.25 | 1.3 | 0.0 | 0.45 |
| Sludge | 0 | 0.90 | 1.6 | -0.1 | 0.35 |

- Mode buttons need no visual change; the radio group, borders and text
  states already match spec. Grid stays as-is this pass.

## Tier 4 - Explicitly deferred (do NOT touch in this pass)

- **FFT input spectrum and reduction history trace** in the Wasp Meter:
  Phase 6 owns the 2048-pt FFT and lock-free FIFO. Until then the frame
  shows columns, scale, guides. Do not substitute any fake shape.
- **Oversampling 2x/4x**: Phase 7, with `dsp::Oversampling`, per-channel
  resampling and `setLatencySamples`. Combo stays hidden from T1.6 until
  then. Re-show it in the Phase 7 commit.
- **The recessed-well depth system and right-rail restructure** from the
  review's redesign direction: visual craft pass after Tier 2 lands and is
  screenshot-reviewed by David. Do not mix it into these commits.
- **Plugin-side bypass button**: declined. Host bypass suffices for v0.1;
  if added later it is a new additive APVTS param, not a UI toggle.
- **Input/output trim controls**: params already drive DSP
  (`inputGain`/`outputGain`). UI for them is Phase 8 advanced-overlay
  scope, not this pass.
- **Auto Gain UI**: leave the ON default; the parameter is honest (it does
  what it says), just hidden. Surfaces in the Phase 8 overlay. Note it in
  the summary so David can override.

## Gates (all must pass before you report done)

1. Five-target Release build, 0 warnings, 0 errors (see HANDOFF for the
   exact VS 2022 commands).
2. `CabRot_PassthroughTest.exe` exits 0 (3/3).
3. `CABROT_SKIP_CPU_BENCH=1 CabRot_DspTest.exe` exits 0, 12/12 plus the
   new delta-listen and A/B assertions.
4. `grep -rnE "0x[0-9A-Fa-f]{6}|#[0-9A-Fa-f]{6}|fromString" Source/`
   hits only `Source/Theme/Palette.h`.
5. `grep -rn "RoundedRectangle" Source/` and
   `grep -rni "glow\|dropshadow" Source/` return nothing.
6. `grep -rn "66.1\|4.2%\|0.70f); \|SCANNING" Source/` returns nothing.
7. Screenshots via `tools/visual-diff.ps1 -SkipBuild` at default, min
   (1000x680) and max (1600x1070): no clipping, four zone labels, log-spaced
   frequency row, no OS combo, `--` fizz readout on a silent transport.
8. Update `design/CANONICAL-UI.md` (WaspMeter section, footer section,
   fizz section) and `HANDOFF.md` (changelog + current state) in the final
   commit so the docs match the shipped screen.

## Commit discipline

One commit per task above (T1.1..T3), ordered. Commit messages state what
was faked, what it now reads from, or what was hidden and which phase
re-shows it. No commit may leave the tree with a fabricated readout it
did not have before; the whole point of the pass is that every screenshot
after Tier 1 is a true one.
