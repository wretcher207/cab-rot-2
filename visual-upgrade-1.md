We are completely upgrading the visual fidelity of SUNDER based on a new "Physical Luxury Custom Hardware" reference design. We want to implement a highly tactile, recessed glass display and micro-machined metallic look using pure vector drawing routines, layered linear/radial gradients, and strategic drop-shadow drop-shadow operations. 

Do not use external image assets. Everything must be coded natively using our UI framework's drawing API (Graphics context, paths, gradients).

Please implement the following exact styling specifications:

## 1. MAIN CHASSIS & ENCLOSURE (The Frame)
- Background Base: Change from flat black to a deep obsidian/charcoal linear gradient: Top `#1B1D22` to Bottom `#0E1012`.
- Texture/Grain Simulation: If possible in our framework, overlay a very fine, low-opacity horizontal noise or multi-line stroke array to simulate a brushed dark-titanium/anodized aluminum finish.
- Inner Border/Bezel: The central display screen and control dock must look physically carved into the chassis. Draw a 2px inner inset border:
  * Top and Left stroke: 1px width, color `#3A3F48` (simulating light catching the upper bevel).
  * Bottom and Right stroke: 1px width, color `#050608` (simulating a deep recess shadow).

## 2. THE recessed SPECTRAL GLASS DISPLAY
- Screen Base: Solid dark charcoal background `#0B0C0E`.
- Glass Sheen Overlay: Draw a non-interactive, diagonal linear gradient across the entire screen area to simulate glass reflectivity. Start at Top-Left `(x:0, y:0)` with white at 4% opacity (`rgba(255,255,255,0.04)`) and fade out completely to 0% opacity by the center of the graph.
- Gridlines: Keep them incredibly faint. 0.5px thickness, color `#1B1D22`. They must sit *behind* the spectrum and reduction waves.

## 3. THE "LIQUID SILK" REDUCTION VEIL (The Kick in the Ass)
Instead of a flat, boring 2D shape, the attenuation curve needs to look like a deep, 3D smoky copper ribbon illuminated from behind. 
- Implement this by layering exactly **3 overlapping Bezier paths** for the reduction envelope:
  1. Base Layer (Back): Solid filled polygon from the 0dB ceiling down to the deepest reduction peak. Filled with a linear gradient running vertically: Top `#3E2715` (deep amber copper) fading down to `#0B0C0E` (screen background) at 0% opacity.
  2. Middle Layer (Thread Detail): A duplicate path but offset horizontally by +5 pixels and vertically by -2 pixels. Draw as a stroke only (1.5px width) using a bright copper color `#E59846` with 15% opacity to create a "threaded ribbon" texture.
  3. Front Layer (The Edge): The primary crisp reduction outline. 1.5px stroke width, using a glowing gradient from white/silver `#E2E8F0` at the shallow wings, morphing into a warm golden copper `#D4A359` at the center of the deep attenuation trough.

## 4. MICRO-KNURLED TACTILE ENCODERS (The Knobs)
Modify our custom encoder component to render a 3D-feeling cylindrical hardware dial:
- Knob Body: A solid circle filled with a radial gradient offset to the top-left to simulate spherical lighting. Center of gradient: `#32373F`, Edge of gradient: `#141619`.
- Knurling/Ridges: Draw a repeating series of tiny, 1px radiating tick-marks or a concentric outer ring with a dashed stroke pattern (`color: #0A0B0D`) around the perimeter of the knob body to look like a machined grip.
- Center Value Cap: A smaller concentric inner circle filled with `#1C1E22`. The numeric value text (e.g., "50%") sits dead-center, rendered in crisp white `#FFFFFF` with a subtle outer glow or drop shadow.
- The Parameter Arc Ring: The active value indicator track must sit in a tiny recessed groove *around* the base of the knob. It should be a fine vector arc using a vibrant copper-gold `#D4A359` with a soft blur/glow effect applied to simulate an under-knob LED ring.

## 5. HARDWARE STYLE MODE SELECTORS (The Amp Toggles)
- Convert the plain text amp names (5150, RECTO, etc.) into small physical toggle switches or push-buttons.
- Give each mode a small, circular 3D indentation button chassis (`#121316`).
- Active State: When a mode is selected (e.g., "5150"), draw a tiny 3px circular LED indicator dot directly to its left. The active dot must be a glowing amber `#D4A359` with a 2px radial blur to look emissive. The text for the active item should light up slightly brighter than the inactive items.

Please break this down into components and execute the layout updates systematically. Let's start with the Main Chassis and Recessed Screen borders first. Show me the code updates or build status for the chassis before moving to the knobs.