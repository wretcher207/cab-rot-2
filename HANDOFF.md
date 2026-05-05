# Cab Rot — Session Handoff

**Last updated**: 2026-05-05
**Project state**: Plan locked, repo scaffolded, no code written yet. Ready to execute Phase 0.

---

## TL;DR (read first)

You are picking up an audio plugin project for **Dead Pixel Harmonix**. Cab Rot is a JUCE 8 VST3/AU plugin that removes high-gain amp-sim fizz without killing pick attack. The plan exists, the UI design exists, all major decisions are locked. Next concrete action is **Phase 0**: scaffold a buildable empty VST3 in this directory and load it in Reaper.

The single source of truth is **[PLAN.md](PLAN.md)**. Read it end-to-end before doing anything. It has 11 phases (0–10), each with a Self-Review Gate that must pass before the next phase begins.

---

## What Cab Rot Is

A focused dynamic harshness controller for high-gain electric guitar amp sims. Targets the 2 kHz–12 kHz "wasp nest" range with 4 dynamic detection bands, transient-protected reduction window, 6 mode buttons (5150 / Recto / HM-2 / Djent / Blackened / Sludge), and a branded "Wasp Meter" spectral display. Tagline: **"Kill the wasp nest. Keep the teeth."**

Aesthetic: Spectre Codex v2 — deepest black + toxic green + Space Grotesk + JetBrains Mono + scanlines + ghost iconography. "A cursed piece of studio hardware dragged out of a server basement."

Pricing: $29 launch / $49 normal / $69 Gumroad bundle (plugin + 12 presets + 10 DI examples + Quickstart PDF + Reaper chains). Paid-only, no free version.

Build target: VST3 + AU, Windows-first, macOS as Phase 11 after v1.0 ships.

---

## Where the Truth Lives (authority order)

When sources disagree, the higher-authority document wins.

| Authority | Path | What it covers |
|---|---|---|
| 1 (highest) | [PLAN.md](PLAN.md) | Build phases, gates, architecture, locked decisions |
| 2 | `design/CANONICAL-UI.md` (TBD — Phase 1 deliverable) | Exact pixel measurements, color tokens per element, knob specs |
| 3 | [design/stitch-reference.html](design/stitch-reference.html) | Visual design source. Open in browser to see the target UI. |
| 4 (lowest) | `c:\Users\david\workspace\second-mind\wiki\sources\2026-05-05-cab-rot-plugin-spec.md` | Original product spec (ChatGPT-generated). Useful for DSP intent + marketing copy. Some details overridden by PLAN.md — see Gap Resolutions table. |

**Related second-mind context** (Obsidian wiki, read for background):
- `c:\Users\david\workspace\second-mind\wiki\entities\cab-rot.md` — entity page
- `c:\Users\david\workspace\second-mind\wiki\entities\dead-pixel-harmonix.md` — parent label
- `c:\Users\david\workspace\second-mind\wiki\concepts\spectre-codex-v2.md` — design system
- `c:\Users\david\workspace\second-mind\wiki\sources\2026-04-24-dead-pixel-harmonix-msv-1.md` — sibling product (MSV-1 channel strip), for context on DPH brand direction

---

## Locked Decisions (do not relitigate)

These were resolved with David in a Q&A round on 2026-05-05. They override anything in the spec or Stitch export that contradicts them.

| # | Decision | Rationale |
|---|---|---|
| 1 | **6 knobs** in bottom row: Fizz Hunt, Edge Preserve, Cab Smooth, Digital Sand, Air Rot, **Reap Mix** | Stitch was missing Reap Mix; spec calls for it as a first-class wet/dry control. Required for parallel-blend escape hatch. |
| 2 | **Oversampling Off / 2x / 4x** | Dropped 8x. Plugin only processes 2–12 kHz; 8x is overkill, costs ~5–6× CPU vs Off for negligible benefit. |
| 3 | **Delta Listen ghost icon in header**, click toggles persistently, glows red when active | Replaces Stitch's `sensors` icon. Spec says ghost-icon-toggle is mandatory demo feature. |
| 4 | **Wasp Meter labels: BITE / PLASTIC / WASP / SAND / AIR / ICE** between 2 kHz–12 kHz | Two label rows: numeric frequencies on top, named zones below. Spec wins over Stitch's generic 1k–20k labels. |
| 5 | **Stereo Link** lives in The Crypt advanced panel as "Stereo Behavior" (Linked / Partial / Dual Mono) | Lower-priority control, doesn't deserve front-panel real estate. |
| 6 | **Continuous resize**, aspect-locked at 1.54:1, min 1000×650, default 1200×780, max 1600×1040 | Native JUCE resizable, DPI-aware. |
| 7 | **Demo DI guitar tracked by David** on his own rig — 3 passages: chuggy 5150, HM-2 grind, raw bedroom phrase | Free, authentic, marketing angle ("this is what I tracked at home"). |
| 8 | **Paid-only from day 1**, $29 launch / $49 normal / $69 deluxe | No free version. Avoids cannibalization. Standard for serious indie plugins (Soothe2, Pro-Q 4). |
| 9 | **Windows-first ship**; macOS becomes Phase 11 after v1.0 validates | David's primary dev machine is Windows. macOS adds Apple Developer ID + notarization burden. |
| 10 | **JUCE 8 license**: develop on free Personal license through Phase 9; subscribe to Indie (~$40/month, verify 2026 pricing) before Phase 10 | Most indie path. Verify exact 2026 JUCE pricing/terms during Phase 0. |

**Render strategy**: Native JUCE Components + LookAndFeel. **Not WebView.** OKLCH→sRGB conversion happens once at build time via a Python script that emits `Source/Theme/Palette.h` with `constexpr juce::Colour` constants.

---

## Current State

### Done
- [x] Plan written and locked (PLAN.md)
- [x] Repo scaffolded at `c:\Users\david\workspace\cab-rot\`
- [x] Stitch UI reference copied to `design/stitch-reference.html`
- [x] README.md created
- [x] All 8 open questions resolved with David
- [x] Gap Resolutions table reflects locked answers (not recommendations)

### Not Done (no work has touched these yet)
- [ ] Phase 0: git init, JUCE submodule, CMakeLists.txt, empty VST3, load in Reaper
- [ ] Phase 1: design/CANONICAL-UI.md, OKLCH→sRGB script, SpectreLookAndFeel, theme test
- [ ] Phases 2–10: not started

### Next Concrete Action
Execute Phase 0 per PLAN.md. The Self-Review Gate 0 is:
1. `cmake --build` succeeds clean from fresh clone, no warnings
2. VST3 loads in Reaper without errors in `~/.reaper/reaper-debug.log`
3. Plugin passes audio through with measurable null vs bypass (sample-perfect)
4. `tools/visual-diff.ps1` produces a PNG of the plugin window
5. Repo is committed; `.gitignore` excludes `build/`, `*.vst3` in source tree, IDE files

Do not start Phase 1 until all five boxes are checked.

---

## Working Environment

- **OS**: Windows 11 Pro 10.0.26100
- **Primary working dir**: `c:\Users\david\workspace\cab-rot\`
- **Shell**: PowerShell (use PowerShell syntax: `$null` not `/dev/null`, `$env:VAR` not `$VAR`, backtick for line continuation). Bash also available via Bash tool.
- **DAW**: Reaper (David's primary). Used for plugin testing.
- **Compiler**: MSVC (Visual Studio 2022 Build Tools or full IDE). JUCE CMake works with both.
- **JUCE**: Not yet installed. Will be added as a git submodule in Phase 0. Target version: 8.x.
- **Python**: Available. Used in `tools/oklch-to-srgb.py` (Phase 1).

### Useful skills available in David's Claude Code setup
- **`juce-binary-data-gen`** — generates JUCE BinaryData.h/cpp from arbitrary binary resources (fonts, images). Will be used in Phase 1 to bundle Space Grotesk + JetBrains Mono into the plugin.
- **`juce-standalone-snapshot`** — builds a JUCE standalone plugin on Windows, launches it, captures a clean PNG of the window. Will be used in `tools/visual-diff.ps1` for phase-by-phase visual diffs against the Stitch reference.

These are documented at `~/.claude/skills/`. Future session can invoke them directly when relevant.

---

## David's Working Preferences (from `~/.claude/CLAUDE.md`)

These apply to every session globally. Do not violate them:

- **No em dashes** in any delivered prose (commit messages OK; user-facing copy not OK). Use commas, periods, parens, or colons.
- **Terse responses**. No trailing "here's what I did" summaries — the diff already shows it.
- **Work autonomously**. Don't ask permission for routine things (commits, pushes to feature branches, rebases, branch creation, local resets when tree is clean).
- **Ask only when destructive** — `git reset --hard` with uncommitted changes, force push to main, deleting branches with unmerged commits, `rm -rf` outside scratch dirs.
- **Honesty rules**: never fabricate file contents, command output, or API behavior. If unverified, say "I haven't verified" or "I don't know" — then verify or ask.
- **Stay on the path**: when David specifies a tool or approach (e.g. "use JUCE", "use the Stitch design"), use it. If a real blocker appears, STOP and report with alternatives. Don't pivot silently.
- **No lectures**: David knows what he's doing. Skip security/best-practices lectures unless asked.

---

## How to Boot a New Session

A fresh Claude Code session in this directory should:

1. **Read [PLAN.md](PLAN.md) end-to-end first.** Especially the Architecture, Repo Structure, and Phase 0 sections.
2. **Read this file (HANDOFF.md)** for the locked decisions and David's preferences.
3. **Skim [design/stitch-reference.html](design/stitch-reference.html)** in a browser if possible — the visual target is much clearer when seen than described.
4. **Check second-mind context** if needed: at minimum read `c:\Users\david\workspace\second-mind\wiki\sources\2026-05-05-cab-rot-plugin-spec.md` for the original DSP intent and marketing language.
5. **Confirm with David** that no decisions have changed since 2026-05-05 before starting work.
6. **Execute Phase 0**. Do not skip ahead.

### First commands to run (Phase 0 start)
```powershell
# from c:\Users\david\workspace\cab-rot\
git init
git submodule add https://github.com/juce-framework/JUCE.git JUCE
# then write CMakeLists.txt per Phase 0 in PLAN.md
```

---

## Risks and Gotchas

These are the things most likely to bite a new session:

- **OKLCH→sRGB drift** — the Stitch palette is in OKLCH. Use the standard `oklab → linear sRGB → sRGB` formula. JUCE's `juce::Colour` only takes sRGB. Get the conversion right once, bake into a header, never re-implement at runtime. Visual diff threshold in Phase 1 is 5% — if you exceed it, color math is wrong.
- **Linkwitz-Riley crossover phase summing** — Phase 4 explicit null test required. If the 4 bands don't sum back to flat within −60 dB, the crossover topology is wrong. Switch topologies before continuing — this is foundational.
- **APVTS parameter ID mismatches** — JUCE's `SliderAttachment` silently does nothing if the ID doesn't match the parameter. Phase 3 gate has an automation test specifically because this failure is silent.
- **Native Windows path quirks** — paths with spaces need quoting. CRLF line endings in source files cause CMake/MSVC warnings.
- **Reaper plugin scanner caching** — if Reaper isn't seeing a freshly built VST3, force-rescan via `Options → Preferences → Plug-ins → VST → Re-scan`. Don't assume "Reaper isn't loading my plugin" means the plugin is broken.
- **Native Windows Python and JUCE WSL paths** — `tools/oklch-to-srgb.py` runs in native Windows Python; don't call it via WSL. Output paths use Windows separators in the generated header.
- **JUCE 8 licensing**: the dev-on-free, ship-on-paid pattern is permitted by JUCE's terms as of 2024, but **verify 2026 terms during Phase 0** before assuming. Add the verify step to Phase 0's checklist.

---

## Out-of-Scope for v1.0 (do not build)

The plan deliberately excludes these. Resist scope creep — they go on the v1.1+ wishlist:

- AI tone matching
- Full spectrum editing (vs. the fixed 4-band detection)
- External sidechain input
- Cabinet IR loader
- Multiband saturation
- Cloud preset sync
- Plugin marketplace account system
- In-plugin chatbot or "Assistant"

Quote from spec: *"Keep the first beast lean. Let it bite before you teach it Latin."*

---

## Changelog

- **2026-05-05** — Plan locked. All 8 open questions resolved. Repo scaffolded. Handoff written.
