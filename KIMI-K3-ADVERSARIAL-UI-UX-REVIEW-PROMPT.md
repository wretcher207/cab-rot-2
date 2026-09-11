# Kimi K3 prompt: adversarial Cab Rot UI/UX review

You are reviewing the current UI/UX and visual design of **Cab Rot**, a native JUCE audio plugin by Dead Pixel Harmonix. Be adversarial. Treat the current interface as a release candidate that must earn the right to ship, not as a student project that deserves encouragement.

Work from the repository and the current in-host screenshot:

- Repository: `C:\Users\wretc\workspace\cab-rot`
- Current screenshot: `C:\Users\wretc\Pictures\Screenshots\86aa0a58-6304-48fd-9b75-c1d23f109d3d.png`
- Start with `HANDOFF.md`, `design/CANONICAL-UI.md`, `design/FACELIFT-REPORT.md`, `Source/PluginEditor.cpp`, and the components under `Source/UI/`.
- Inspect the processor and parameter wiring when needed to verify whether visible controls and displays actually do what they imply.

Cab Rot is a focused dynamic harshness controller for high-gain amp-sim guitars. It is supposed to remove 2–12 kHz fizz while preserving pick attack. The product should feel serious, useful, and premium, in the quality class of Kush Audio, Brainworx, and iZotope. Use those companies as a bar for product confidence, hierarchy, legibility, interaction quality, information design, and finish. Do not imitate one of their skins.

## Visual references and why they matter

Three reference screenshots are in `C:\Users\wretc\workspace\cab-rot\references\`. Their filenames describe their roles. Inspect all three before judging Cab Rot:

- `kush-control-layout-depth.webp`: This is the target for depth, physical presence, and cumulative craft. The surface feels tangible enough that you can almost smell the hardware. It achieves that through material variation, lighting, wear, recesses, layered panels, dimensional controls, tiny indicators, purposeful asymmetry, and many small details that support the larger composition. Do not copy the UBK-2 or automatically turn Cab Rot into faux vintage hardware. Determine how Cab Rot can achieve the same degree of sensory conviction and authored detail in its own Dead Pixel Harmonix identity.
- `gullfoss-visualization.jpg`: This is the target for a graphical display that explains the processing. The display makes it possible to understand what the signal has been doing, what the processor is doing now, and where the behavior is headed without losing orientation. Study its continuity, history, scales, boundaries, input/output context, and the relationship between the graph and its controls. Use those principles to decide what Cab Rot's central display should communicate about detected harshness, active reduction, frequency, persistence, and recovery.
- `soothe-2-detail.jpg`: This is the target for functional density without visual chaos. A large amount of analysis, control, selection, comparison, and metering is present, yet the primary task remains legible. Study its grouping, hierarchy, progressive disclosure, direct manipulation, selected-band context, compact secondary controls, and use of the visualization as part of the control surface rather than as decoration.

Treat these references as three complementary standards, not three skins to blend together. Extract the design principles behind them, decide which principles belong in Cab Rot, reject those that do not, and produce one original, coherent visual and interaction language. Explicitly compare the current interface against each reference's relevant strength. The existing flat-monochrome design spec is not sacred: if its strict hairlines, uniform darkness, minimal material variation, or refusal of dimensional treatment prevents Cab Rot from reaching the desired depth and clarity, say so and recommend a better system.

The current branch is a working Phase 4 DSP build after a monochrome Dead Pixel Design facelift. The six main knobs are connected to real DSP. Do not assume the rest of the screen is equally real. Verify it. Known current-state facts include:

- The spectral display and reduction curve use placeholder data rather than live audio analysis.
- The `66.1%` Fizz Amount display is static.
- The CPU value and IN/OUT meters are static.
- The LIVE indicator breathes decoratively rather than communicating a meaningful processing event.
- The six amp-profile buttons update the mode parameter, but the six distinct mode behaviors are not implemented yet.
- Delta Listen, A/B, and oversampling have clickable UI and parameters, but their labeled audio behavior is not implemented yet.
- The footer always says `PROCESSING`.

Do not let the roadmap excuse what is on screen now. If the interface presents something as live, selectable, measurable, or operational, it must either be truthful and useful in the current build or it should not be there yet.

## Your assignment

Perform a ruthless UI/UX and product-design review of the plugin as it exists now.

Interrogate every visible element:

- What does it communicate?
- What user decision or action does it support?
- Is it functional, truthful, discoverable, and legible?
- Does its visual weight match its importance?
- If it is decorative, does it strengthen the product concept enough to earn the space?
- If clicked, dragged, scrolled, hovered, or right-clicked, does it behave as a professional audio-plugin user would expect?

Pay special attention to the large central graphic. A waveform, spectrum, curve, meter, or animation is not justified merely because audio plugins often have one. Determine what this display should represent for a dynamic harshness controller, what data would make it genuinely useful while dialing in the effect, and whether interaction would improve it. If the current graph cannot justify its footprint, say so and replace the idea with something better.

Actively hunt for:

- dead clicks and controls that change state without changing the sound
- decorative or fabricated telemetry
- fake precision
- unclear control names, values, units, defaults, and relationships
- weak hierarchy or large areas that consume attention without helping the mix decision
- poor feedback for bypass, Delta Listen, mode changes, A/B state, oversampling, clipping, silence, and active gain reduction
- controls that look disabled when they are merely inactive
- tiny text, weak contrast, cramped hit targets, ambiguous icons, and interaction states that disappear in a dark interface
- host-window and resizing problems at the documented minimum, default, and maximum sizes
- missing keyboard, mouse-wheel, double-click reset, context-menu, tooltip, focus, automation, and accessibility behavior expected from a serious JUCE plugin
- visual choices that feel like an austere wireframe, developer UI, or generic “dark audio plugin” rather than a finished Dead Pixel Harmonix product
- features or visual complexity that should be removed instead of polished

Judge the product as a guitarist and producer using it in a real session: audio is playing, attention is limited, comparisons must be fast, and the UI must make it obvious what Cab Rot is detecting, what it is removing, and whether the result is better without forcing the user to decode a science project.

## Deliverable

Lead with an unvarnished verdict: does this currently look and behave like a premium commercial plugin, and would you trust it in a paid release?

Then provide:

1. A concise explanation of the product and primary user task as the interface currently communicates them.
2. An inventory of every visible UI element, stating its apparent purpose, actual implementation state, whether it earns its place, and the required action: keep, fix, replace, hide until real, or remove.
3. The most damaging UI/UX failures, ranked by severity and release impact. Cite screenshots and source files for factual claims.
4. A dead-click and false-feedback audit. Distinguish fully functional controls, parameter-only controls, static displays, decorative motion, and genuinely dead elements.
5. A critique of composition, hierarchy, typography, spacing, contrast, density, control affordance, feedback, resizing, and accessibility.
6. A specific recommendation for the central visualization: what it should show, why a producer needs it, how it responds to audio and controls, and what its idle, active, silence, overload, and unavailable states should be.
7. A reference synthesis explaining what Cab Rot should learn from Kush's sensory depth, Gullfoss's intelligible processing history, and Soothe's disciplined functional density, plus what it must not borrow from each.
8. One coherent redesign direction with a strong point of view. Describe the information architecture, visual hierarchy, interaction model, material and depth system, central display, and signature visual language in enough detail that a designer or JUCE developer could execute it. Do not give me a mood-board paragraph or a menu of vague options.
9. A prioritized plan split into: release blockers, functional depth, premium visual craft, and final polish. Call out dependencies on unfinished DSP work instead of proposing fake UI around missing data.
10. A short “cut list” of anything that should disappear because it is redundant, dishonest, arbitrary, or not worth its pixels.

Be concrete and opinionated. Name exact controls and regions. Explain why each problem matters during real mixing. Avoid generic advice such as “improve spacing,” “add polish,” or “make it more modern” unless you specify exactly what changes and what that solves. Do not soften the assessment to protect the existing work.
