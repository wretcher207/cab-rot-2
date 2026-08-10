// Headless editor gate. Constructs the real CabRotProcessor and its editor,
// lays it out at both resize extremes, and asserts the things design/
// CANONICAL-UI.md claims about layout and parameter wiring.
//
// Why this exists: sections 9.2, 9.4 and 9.5 make claims about the editor
// (reachable window sizes, fabricated readouts, parameters with UI and no DSP
// consumer) that had only ever been checked by reading the source. The plugin
// editor is the actual public surface a user touches, so it should be
// exercised, not inspected.
//
// No window is shown and no message loop runs, so this works headless.

#include <juce_audio_processors/juce_audio_processors.h>

#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"

#include <cstdio>

namespace
{
bool failed = false;

void check (bool condition, const char* label, const juce::String& detail = {})
{
    std::printf ("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (detail.isNotEmpty())
        std::printf ("        %s\n", detail.toRawUTF8());
    if (! condition)
        failed = true;
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    std::printf ("Cab Rot editor gate\n\n");

    cabrot::CabRotProcessor processor;

    // --- The editor must construct at all --------------------------------
    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    check (editor != nullptr, "the plugin creates an editor");
    if (editor == nullptr)
    {
        std::printf ("\nFAILED\n");
        return 1;
    }

    // --- CANONICAL-UI section 1: default size and aspect -----------------
    check (editor->getWidth() == 1200 && editor->getHeight() == 780,
           "default size is the documented 1200 x 780",
           juce::String (editor->getWidth()) + " x " + juce::String (editor->getHeight()));

    const double documentedRatio = 1200.0 / 780.0;
    const double actualRatio = static_cast<double> (editor->getWidth())
                             / static_cast<double> (editor->getHeight());
    check (std::abs (actualRatio - documentedRatio) < 0.001,
           "aspect ratio matches the locked 1.5385",
           juce::String (actualRatio, 4));

    // --- CANONICAL-UI section 9.2: the reachable resize extremes ----------
    // The doc says 1000x650 and 1600x1040 are reachable and that the earlier
    // 1000x680 / 1600x1070 figures were not. Resize and see what sticks.
    editor->setSize (1000, 650);
    check (editor->getWidth() == 1000 && editor->getHeight() == 650,
           "minimum extreme 1000 x 650 is reachable",
           juce::String (editor->getWidth()) + " x " + juce::String (editor->getHeight()));

    editor->setSize (1600, 1040);
    check (editor->getWidth() == 1600 && editor->getHeight() == 1040,
           "maximum extreme 1600 x 1040 is reachable",
           juce::String (editor->getWidth()) + " x " + juce::String (editor->getHeight()));

    // Every child must stay inside the editor at both extremes: this is the
    // "no clipping" claim the facelift report made about screenshots nobody
    // can see.
    auto childrenFitAt = [&editor] (int w, int h, juce::String& detail) -> bool
    {
        editor->setSize (w, h);
        const auto bounds = editor->getLocalBounds();
        for (int i = 0; i < editor->getNumChildComponents(); ++i)
        {
            auto* c = editor->getChildComponent (i);
            if (c == nullptr || ! c->isVisible())
                continue;
            if (! bounds.contains (c->getBounds()))
            {
                detail = "child " + juce::String (i) + " at "
                       + c->getBounds().toString() + " escapes " + bounds.toString();
                return false;
            }
        }
        return true;
    };

    juce::String detail;
    check (childrenFitAt (1000, 650, detail),
           "at minimum size every child stays inside the window", detail);
    detail.clear();
    check (childrenFitAt (1600, 1040, detail),
           "at maximum size every child stays inside the window", detail);
    detail.clear();
    check (childrenFitAt (1200, 780, detail),
           "at default size every child stays inside the window", detail);

    // --- Painting must not crash at any of the three sizes ---------------
    for (auto size : { juce::Point<int> (1000, 650),
                       juce::Point<int> (1200, 780),
                       juce::Point<int> (1600, 1040) })
    {
        editor->setSize (size.x, size.y);
        juce::Image img (juce::Image::ARGB, size.x, size.y, true);
        juce::Graphics g (img);
        editor->paintEntireComponent (g, true);
    }
    check (true, "the whole editor paints at min, default and max sizes");

    // --- CANONICAL-UI section 9.5: parameters with UI and no DSP consumer -
    // Assert the parameters exist and are host-visible. The doc's claim is
    // that they are wired to controls but read by no DSP; this half (that
    // they are real, automatable parameters) is what a host sees.
    auto& apvts = processor.getApvts();
    const char* wired[] = { "mode", "oversampling", "deltaListen", "aOrB" };
    for (auto* id : wired)
        check (apvts.getParameter (id) != nullptr,
               "parameter is exposed to the host",
               juce::String (id));

    // The six main knobs must all exist and accept automation.
    const char* knobs[] = { "fizzHunt", "edgePreserve", "cabSmooth",
                            "digitalSand", "airRot", "reapMix" };
    bool allKnobs = true;
    for (auto* id : knobs)
        if (apvts.getParameter (id) == nullptr)
            allKnobs = false;
    check (allKnobs, "all six documented knob parameters exist");

    // --- Mode is a 6-choice parameter, per locked decision ---------------
    if (auto* mode = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter ("mode")))
        check (mode->choices.size() == 6,
               "mode exposes the six documented amp profiles",
               mode->choices.joinIntoString (", "));
    else
        check (false, "mode is a choice parameter");

    if (auto* os = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter ("oversampling")))
        check (os->choices.size() == 3 && os->getIndex() == 0,
               "oversampling is Off / 2x / 4x, defaulting to Off",
               os->choices.joinIntoString (", ") + ", index " + juce::String (os->getIndex()));
    else
        check (false, "oversampling is a choice parameter");

    editor.reset();
    std::printf ("\n%s\n", failed ? "FAILED" : "all editor checks passed");
    return failed ? 1 : 0;
}
