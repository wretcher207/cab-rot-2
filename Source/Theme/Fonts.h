#pragma once

#include <juce_graphics/juce_graphics.h>

namespace cabrot::theme
{
// Single-source typeface and Font helpers. PluginEditor + LookAndFeel +
// any UI atom should pull fonts from here, never construct Font directly
// from a system family name. Phase 1 gate: zero hits for `juce::Font (juce::FontOptions`
// in Source/UI when scanned.
//
// All sizes track the typography ladder in design/CANONICAL-UI.md §3.

class Fonts
{
public:
    // Load typefaces once on first use. Cached for the lifetime of the
    // application. Safe to call from any thread - JUCE's Typeface class
    // is reference-counted.
    static juce::Typeface::Ptr displayBold();   // Space Grotesk Bold
    static juce::Typeface::Ptr displayMedium(); // Space Grotesk Medium
    static juce::Typeface::Ptr displayRegular();// Space Grotesk Regular
    static juce::Typeface::Ptr monoRegular();   // JetBrains Mono Regular
    static juce::Typeface::Ptr monoBold();      // JetBrains Mono Bold

    // Pre-baked Font objects matching the CANONICAL-UI typography slots.
    // Heights are in pixels; tracking values translate to JUCE's
    // extra-kerning factor (em-based, like CSS letter-spacing).
    static juce::Font displayTitle();   // 24 px, -0.02 em
    static juce::Font heroNum();        // 80 px, -0.05 em
    static juce::Font body();           // 16 px, normal
    static juce::Font uiChrome();       // 11 px, +0.30 em, mono medium
    static juce::Font monoData();       // 14 px, +0.10 em, mono regular

    // Scaled variants for resizable layouts. `scale` should be the editor
    // height divided by the canonical 780. Values clamp internally to keep
    // tiny chrome readable.
    static juce::Font displayTitleScaled (float scale);
    static juce::Font heroNumScaled      (float scale);
    static juce::Font uiChromeScaled     (float scale);
    static juce::Font monoDataScaled     (float scale);
};
} // namespace cabrot::theme
