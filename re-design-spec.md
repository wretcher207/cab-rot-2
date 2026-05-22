# TECHNICAL RE-DESIGN SPECIFICATION: SUNDER
**Document Version:** 1.0.0  
**Project Classification:** Premium Audio Processor (Spectral Cab Soother / Resonance Attenuator)  
**Target Execution Environment:** AI Developer Assistant (Claude Code / Cursor)

---

## 1. VISION & DESIGN MANIFESTO: "SCIENTIFIC LUXURY"

SUNDER is a high-end, professional spectral resonance attenuation plugin designed specifically to tame harsh mid-and-high frequency "fizz," "bite," and digital brittleness in electric guitar cabinet simulations and raw tracks. 

This document serves as an absolute, non-negotiable architectural blueprint to completely purge "vibecoded AI slop" (clunky 3D widgets, comic-book boxes, neon grids, and amateur terminology) and replace it with a **Scientific Luxury** aesthetic. 

### Core Pillars:
1. **Understated Restraint:** No heavy bounding boxes, no fake plastic or brushed metal textures, no simulated shadows. Layout hierarchy is achieved exclusively through **negative space, typography, and precise structural alignment**.
2. **Clinical Language:** Replace slang with industrial, confident DSP terms.
3. **Fluid Motion:** Real-time visualizers must be heavily smoothed, organic, and anti-aliased. The reduction visualization should feel like a luxurious, heavy silk curtain or liquid mercury shifting, never an unstable, flickering pixelated line chart.
4. **Strict Color Discipline:** A monolithic, ultra-dark UI accented by a single high-end metallic or warm tone.

---

## 2. BRANDING, TERMINOLOGY, & VALUE MAPPING

All legacy references to "The Crypt," "Fizz," and generic controls are deprecated immediately.

### 2.1 UI Nomenclature & Lexicon
| Legacy Term | Premium Replacement | Description / DSP Context |
| :--- | :--- | :--- |
| **The Crypt (Plugin Name)** | **SUNDER** | Verb: *To split, sever, or break apart.* Implies surgical separation of harsh anomalies from the core signal. |
| **Fizz Amount** | **SUPPRESSION** | The primary threshold/depth offset scaling parameter for the dynamic spectral filter bank. |
| **Fizz Freq Range** | **TARGET FOCUS** | Defines the high and low boundaries ($f_{min}$ and $f_{max}$) of the spectral analysis zone. |
| **Knob / Box Borders** | **Negative Space / Microlines** | Removal of all standard container shapes. Structural zones are isolated via raw whitespace or 1px strokes with $\leq 12\%$ opacity. |

---

## 3. UI ARCHITECTURE & VISUAL SPECIFICATION

### 3.1 Dimensions & Layout Geometry
* **Default Window Size:** 960px (Width) x 580px (Height)
* **Aspect Ratio:** Fixed 48:29 (or resizable with proportional layout anchoring).
* **Grid Layout:** 3 distinct structural areas running top-to-bottom:
  1. **Header Bar (48px):** Minimal branding on left, utility menus (Preset, Bypass, Over-sampling) on right.
  2. **The Spectral Canvas (360px):** Central visualization zone taking up the entire width minus margins.
  3. **Control Dock (172px):** Single horizontal row containing the core encoders, grouped logically from input/detection to output/ballistics.

### 3.2 Typography Hierarchy
Only professional, highly legible sans-serif or technical monospace typefaces are permitted (e.g., *Inter*, *SF Pro Display*, or *JetBrains Mono* for data fields).
* **Primary Branding (Sunder):** 16pt, Tracking: +0.2em, Uppercase, Semi-Bold.
* **Section / Parameter Headers:** 10pt, Tracking: +0.1em, Uppercase, Muted Silver.
* **Value Indicators:** 14pt, Monospace variant (to avoid digit-jitter when turning controls), White.
* **Fine Print / Scales:** 8pt, Regular, Desaturated Gray.

### 3.3 Color Palette (The 3-Color Monolithic Rule)
```
[Background: Deep Charcoal Slate]  --> #0C0C0E
[Surfaces / Inactive Elements]    --> #16161A
[Typography & Secondary Lines]   --> #8E8E93
[Active Accent: Imperial Amber]   --> #D4A359 (or Slate Blue #5B84B1)
```
* **Canvas Gridlines:** 0.5px thickness, color `#1C1C21`. No sharp harsh white/gray lines. Gridlines should be almost imperceptible, disappearing when the user shifts focus away from the graph.

### 3.4 Interactive Controls: The Minimalist Vector Encoder
Completely eliminate traditional knob bitmaps. Encoders must be rendered as raw vector paths:
1. **Center:** The current numerical value with unit string (e.g., `-3.4 dB`, `2.4 kHz`) rendered in clean typography. No moving pointers.
2. **Outer Perimeter:** A thin (1.5px) track circle (`#16161A`).
3. **Active Arc:** A precision vector arc in `Imperial Amber` (`#D4A359`) that traces the parameter value from minimum to current position.

---

## 4. PHASED IMPLEMENTATION & STRICT GATE CHECKS

This implementation is divided into **5 sequential phases**. 
* **CRITICAL INSTRUCTION FOR CLAUDE CODE:** You are strictly forbidden from writing code for a subsequent phase until the current phase has passed its **Visual/Functional Gate Check**. 
* Every gate check requires rendering, logging, or compile verification with **zero placeholders**.

---

### PHASE 1: DESCRIPTOR, COMPONENT TREE & STATE ENGINE
**Objective:** Establish the data models, parameter declarations, and framework infrastructure without any styling or layout.

#### Tasks:
1. Declare the plugin processor class and state container (e.g., `AudioProcessorValueTreeState` in JUCE, or equivalent state engine).
2. Instantiate the primary parameter definitions with explicit step sizes, defaults, and skew factors:
   * `suppression` (0.0 to 100.0%, Default: 0.0%)
   * `target_focus_low` (200 Hz to 5.0 kHz, Skewed Logarithmic, Default: 2.0 kHz)
   * `target_focus_high` (5.0 kHz to 20.0 kHz, Skewed Logarithmic, Default: 12.0 kHz)
   * `selectivity` (0.1 to 10.0, Default: 1.0) — *Controls the Q-factor profile of the spectral bands.*
   * `recovery` (5 ms to 500 ms, Logarithmic, Default: 50 ms) — *The release ballistics.*
3. Wire up basic text-only parameter dumping to console or standard stdout to verify binding.

#### 🛑 GATE CHECK 1 — FUNCTIONAL VERIFICATION
* **Execution:** Run a automated compile test and initialize the state engine.
* **Pass Criteria:** 1. The code compiles with zero warnings related to parameters.
  2. Modifying a parameter via text input prints the exact structural update to the log.
  3. No UI code has been written yet. Only raw logic.

---

### PHASE 2: UI STRUCTURAL WIREFRAME & WIRE-BOXES
**Objective:** Construct the layout engine and layout boundaries. Absolutely zero colors, typography tweaks, gradients, or animations are allowed here.

#### Tasks:
1. Implement the parent window resize and layout bounds calculations using explicit proportional fractions or layout grids.
2. Partition the window into the three defined areas: Header Bar, Spectral Canvas, and Control Dock.
3. Draw raw, solid white 1px border outlines around these three primary bounds to visually prove structural accuracy.
4. Position text placeholders exactly where the branding, graph, and encoders will live.

#### 🛑 GATE CHECK 2 — VISUAL LAYOUT VERIFICATION
* **Execution:** Launch the plugin UI window or capture an image snapshot of the layout.
* **Pass Criteria:**
  1. Take a screenshot. The layout must exactly match the specified dimensions (960x580).
  2. Verify that the areas do not overlap or jitter when the window bounds change.
  3. **Visual Proof Required:** Confirm that the empty blocks are proportionally balanced with generous negative space. Do not proceed if elements feel crowded.

---

### PHASE 3: METALLIC MONOLITH STYLING & VECTOR COMPONENT BUILD
**Objective:** Transform the raw wireframe into the "Scientific Luxury" styling paradigm.

#### Tasks:
1. Implement the global styling sheet/draw instructions:
   * Set window background to solid matte `#0C0C0E`.
   * Apply typography rules (Inter/SF Pro, correct tracking, and weights).
2. Code the custom **Minimalist Vector Encoder** component drawing routine:
   * Draw the center value text without any enclosing box.
   * Draw the fine background track circle.
   * Draw the active `Imperial Amber` (`#D4A359`) arc based on current parameter percentage.
3. Replace the rough text layout from Phase 2 with these custom vector encoders. Arrange them horizontally in the Control Dock with strict, wide spacing.

#### 🛑 GATE CHECK 3 — UI RENDERING AUDIT
* **Execution:** Open the plugin GUI interface.
* **Pass Criteria:**
  1. Visually check that there are **zero clunky rectangles or boxes** around the dials.
  2. Confirm typography rendering is razor sharp, values do not clip, and the vector arcs trace accurately from 0% to 100% when interacted with.
  3. The background color must be perfectly flat and deep charcoal; ensure no default system styling bleeds through.

---

### PHASE 4: THE SPECTRAL CANVAS & REAL-TIME GRAPH
**Objective:** Implement the real-time smoothed frequency analysis and the downward reduction "veil" visualization.

#### Tasks:
1. Implement a lightweight FFT analyzer block (e.g., 2048 or 4096 bins) running on the incoming audio signal.
2. Apply a temporal smoothing factor ($lpha pprox 0.85$ to $0.95$) to the FFT magnitudes to eliminate frantic flickering.
3. Draw the background grid: 4 horizontal decibel lines and 5 vertical frequency lines (e.g., 200Hz, 1kHz, 2kHz, 5kHz, 10kHz) using ultra-faint 0.5px lines (`#1C1C21`).
4. **The Reduction Curve Rendering:** Map the attenuation data across the spectrum. Render this as a solid filled polygon shifting downwards from the top reference ceiling (`0 dB`). The fill should use an elegant gradient going from `#D4A359` with $25\%$ opacity at the top edge to $0\%$ opacity at its lowest peak, mimicking a translucent curtain.

#### 🛑 GATE CHECK 4 — VISUAL DYNAMICS VERIFICATION
* **Execution:** Feed a harsh guitar signal or white noise loop through the plugin.
* **Pass Criteria:**
  1. The background lines must stay faint and not create visual clutter.
  2. The input spectrum curve must look completely smooth and anti-aliased, moving fluidly like liquid.
  3. When `Suppression` is turned up, the downward attenuation curve must gracefully drape over the problematic frequencies. No jagged stairs, pixelations, or twitchy rendering artifacts are permitted.

---

### PHASE 5: REFINEMENT, SUBTLETIES, & PERFORMANCE OPTIMIZATION
**Objective:** Final polish, interactive optimization, and elimination of unnecessary rendering overhead.

#### Tasks:
1. Add subtle interactive highlights: when a user hovers over an encoder, its center text changes color from muted silver to bright white, and its active arc glows slightly brighter.
2. Implement frame-rate limiting or dirty-region rendering on the Spectral Canvas to guarantee the UI consumes less than $5\%$ of total CPU thread capacity.
3. Conduct an aggressive code cleanup: delete any leftover debugging draw commands, old layout borders from Phase 2, and any commented-out code.

#### 🛑 GATE CHECK 5 — FINAL QUALITY ASSURANCE
* **Execution:** Profile the final build under full processing load inside a DAW or host test harness.
* **Pass Criteria:**
  1. UI runs at a locked, stable 60 FPS (or matches native monitor refresh rate) without causing audio dropouts or spiking CPU.
  2. Run a final asset audit: double check that **zero texture files or image bitmaps** are loaded—everything must be perfectly generated using native vector drawing routines.
  3. The plugin looks clean, minimal, expensive, and functions exactly like a premium surgical tool.

---

## 5. CODE CAPABILITIES & ENFORCEMENT RULES FOR CLAUDE CODE

When editing files, follow these strict execution guidelines:
* **NO COMPLEX CONTAINERS:** If you are about to write a standard border draw command around a group of controls, **STOP**. Use an extra 20px of layout margin whitespace instead.
* **MONOSPACE VALUE SAFEGUARD:** Ensure any string that rapidly changes (like decibel or frequency readouts) uses a monospace font layout to avoid the text shaking left and right as numbers change width.
* **ZERO VIBE-CODING:** Every layout coordinate must be calculated using clear mathematical formulas or relative anchors based on the root window width/height. No magic numbers (e.g., `x = 243; y = 112;`) allowed.