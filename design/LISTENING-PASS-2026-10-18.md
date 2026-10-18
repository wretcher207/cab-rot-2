# Cab Rot listening pass, 2026-10-18

David's by-ear check of the six provisional amp modes and twelve provisional
factory presets. This is the gate before Phase 9. Values come from
`Source/DSP/ModeConfig.h` and `Source/Presets/FactoryPresets.h`.

Verdicts: **Keep** (ship as is), **Tweak** (right idea, wrong amount, say
what), **Kill** (wrong, replace it).

## Setup

- Build under test: `fix/ui-truth` at `f352a4c` (DSP code unchanged since `e614a26`).
- Source tracks used: _fill in_
- Oversampling: OFF unless noted.

## Modes

Each mode is compared against 5150, with the six knobs left where they are.
What the code claims, in plain words:

| Mode | What it should do, compared to 5150 | Verdict | Notes |
|---|---|---|---|
| 5150 | The reference voicing. | | |
| Recto | Grabs fizz earlier (2 dB), cuts 15% deeper, clamps 20% faster. | | |
| HM-2 | Grabs a little earlier, cuts 30% deeper, and the WASP band (5.5-8 kHz) 50% deeper. Slightly less pick protection. | | |
| Djent | Fastest clamp of all six, more pick protection, cuts 10% deeper. Air Rot's shelf comes in a little later. | | |
| Blackened | Grabs earliest of all (3 dB), cuts 25% deeper, clamps 30% slower. Air Rot's shelf comes in sooner. | | |
| Sludge | Gentlest cuts (10% shallower), slowest clamp, less pick protection. Air Rot's shelf comes in soonest. | | |

## Presets

Loaded from CRYPT (footer) by double-clicking a name. Presets do not change
input/output trim or oversampling, so the level should not jump.

| # | Preset | Mode | Verdict | Notes |
|---|---|---|---|---|
| 1 | 5150 Wasp Coffin | 5150 | | |
| 2 | Plastic IR Burial | 5150 | | |
| 3 | Recto Sandpaper Mercy | Recto | | |
| 4 | Djent Razor Tax | Djent | | |
| 5 | HM-2 Without Regret | HM-2 | | |
| 6 | Blackened Ice Removal | Blackened | | |
| 7 | Sludge Blanket Lift | Sludge | | |
| 8 | Deathcore Dentist | Djent | | |
| 9 | Bedroom Amp Sim Rescue | 5150 | | |
| 10 | Bus Glue Fizz Net | 5150 | | |
| 11 | Lead Guitar Glass Cage | 5150 | | |
| 12 | Raw Demo Salvage | Recto | | |

## Outcome

_Filled in after the pass._
