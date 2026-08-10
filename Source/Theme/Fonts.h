#pragma once

#include <juce_graphics/juce_graphics.h>

namespace cabrot::theme
{
// Single-source typeface and Font helpers. PluginEditor + LookAndFeel +
// any UI atom should pull fonts from here, never construct Font directly
// from a system family name.
//
// Three faces, per the Dead Pixel Design brand kit:
//   DPD Display     - wordmark and display only. Uppercase, weight 400,
//                      0.06em to 0.10em tracking, up to 0.32em on the
//                      wordmark. Never synthetically bolded.
//   Inter           - body and controls. 400 / 500, sentence case.
//   JetBrains Mono  - metadata, short uppercase labels at ~0.18em
//                      tracking, and every numeric readout.
//
// JUCE's extra kerning factor is stated as a fraction of the font height,
// which maps 1:1 onto CSS-style em tracking.

class Fonts
{
public:
    // Load typefaces once on first use. Cached for the lifetime of the
    // application. Safe to call from any thread - JUCE's Typeface class
    // is reference-counted.
    static juce::Typeface::Ptr dpdDisplay();  // DPD Display Regular
    static juce::Typeface::Ptr inter();       // Inter Regular
    static juce::Typeface::Ptr interMedium(); // Inter Medium
    static juce::Typeface::Ptr mono();        // JetBrains Mono Regular

    // Pre-baked slots.
    static juce::Font wordmark();                    // 24 px DPD Display, 0.30em
    static juce::Font display (float heightPx,
                               float trackingEm = 0.06f);
    static juce::Font body (float heightPx, bool medium = false);
    static juce::Font mono (float heightPx, float trackingEm = 0.0f);
    static juce::Font monoLabel (float heightPx = 10.0f);  // 0.18em tracking

    // Scaled variants for the resizable layout. `scale` should be the
    // editor height divided by the canonical 780. Floor values keep chrome
    // legible at minimum size.
    static juce::Font wordmarkScaled   (float scale);
    static juce::Font displayScaled    (float scale, float refPx, float floorPx);
    static juce::Font monoScaled       (float scale, float refPx, float floorPx,
                                        float trackingEm = 0.0f);
    static juce::Font monoLabelScaled  (float scale, float refPx = 10.0f,
                                        float floorPx = 8.5f);
};
} // namespace cabrot::theme
