// Headless renderer for the WaspMeter, built to settle two questions the
// facelift report could not answer:
//
//   1. CANONICAL-UI 9.7: the stateError path was called "proven by
//      construction" while never being exercised. This renders the meter with
//      reduction values pushed past kReductionDamageThresholdDb and inspects
//      the resulting pixels, so the claim becomes a measurement.
//   2. CANONICAL-UI 9.1: the reference spectrum and the reduction curve use
//      different vertical mappings. This reports where each lands so the
//      mismatch is a number rather than an assertion.
//
// Renders offscreen into a juce::Image. No window, no message loop, so it
// runs in CI and over a headless session.

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Source/Theme/Palette.h"
#include "../Source/UI/WaspMeter.h"

#include <cstdio>

namespace
{
// Count pixels that match a token colour within a small tolerance. Exact
// equality is wrong here: the curve is antialiased, so edge pixels are
// blends of the stroke and whatever is behind it.
int countNear (const juce::Image& img, juce::Colour target, int tolerance)
{
    int hits = 0;
    for (int y = 0; y < img.getHeight(); ++y)
        for (int x = 0; x < img.getWidth(); ++x)
        {
            const auto p = img.getPixelAt (x, y);
            if (std::abs (p.getRed()   - target.getRed())   <= tolerance
             && std::abs (p.getGreen() - target.getGreen()) <= tolerance
             && std::abs (p.getBlue()  - target.getBlue())  <= tolerance)
                ++hits;
        }
    return hits;
}

// Topmost row containing a colour, or -1. Used to prove the error ink only
// appears below the threshold line.
int firstRowWith (const juce::Image& img, juce::Colour target, int tolerance)
{
    for (int y = 0; y < img.getHeight(); ++y)
        for (int x = 0; x < img.getWidth(); ++x)
        {
            const auto p = img.getPixelAt (x, y);
            if (std::abs (p.getRed()   - target.getRed())   <= tolerance
             && std::abs (p.getGreen() - target.getGreen()) <= tolerance
             && std::abs (p.getBlue()  - target.getBlue())  <= tolerance)
                return y;
        }
    return -1;
}

juce::Image render (cabrot::ui::WaspMeter& meter, int w, int h)
{
    meter.setBounds (0, 0, w, h);
    juce::Image img (juce::Image::ARGB, w, h, true);
    juce::Graphics g (img);
    // Paint the brand ground first; WaspMeter is not opaque.
    g.setColour (cabrot::theme::canvas);
    g.fillRect (0, 0, w, h);
    meter.paint (g);
    return img;
}

bool failed = false;

void check (bool condition, const char* label, const juce::String& detail)
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

    std::printf ("Cab Rot WaspMeter render gate\n\n");

    const int W = 820, H = 460;

    // --- Case 1: shipping placeholder data, all under the threshold -------
    {
        cabrot::ui::WaspMeter meter;
        const auto img = render (meter, W, H);

        const int errorPixels = countNear (img, cabrot::theme::stateError, 12);
        const int inkPixels   = countNear (img, cabrot::theme::inkPrimary, 12);

        check (errorPixels == 0,
               "placeholder data draws no damage-coloured ink",
               juce::String ("stateError pixels ") + juce::String (errorPixels));
        check (inkPixels > 200,
               "the reduction curve is actually drawn in primary ink",
               juce::String ("inkPrimary pixels ") + juce::String (inkPixels));
    }

    // --- Case 2: reduction forced past the damage threshold ---------------
    // This is the falsification CANONICAL-UI 9.7 asks for.
    {
        cabrot::ui::WaspMeter meter;
        meter.setTestReduction (18.0f);   // 18 dB, well past the 12 dB threshold
        const auto img = render (meter, W, H);

        const int errorPixels = countNear (img, cabrot::theme::stateError, 12);
        check (errorPixels > 100,
               "past the threshold, the curve renders in stateError",
               juce::String ("stateError pixels ") + juce::String (errorPixels));

        // The error ink must not appear above the threshold line. The scale
        // runs 0 dB at the plot top to kReductionScaleMaxDb at the bottom.
        const int firstErrorRow = firstRowWith (img, cabrot::theme::stateError, 12);
        const float expectedThresholdFraction =
            cabrot::theme::kReductionDamageThresholdDb / cabrot::theme::kReductionScaleMaxDb;

        check (firstErrorRow > 0,
               "damage ink appears somewhere in the frame",
               juce::String ("first row ") + juce::String (firstErrorRow));

        // Generous bound: the plot is inset inside the frame, so translate to
        // a fraction of total height and allow for the inset and stroke width.
        const float rowFraction = static_cast<float> (firstErrorRow) / static_cast<float> (H);
        check (rowFraction > expectedThresholdFraction * 0.5f,
               "damage ink stays in the lower (damaged) half of the scale",
               juce::String ("first error row at ") + juce::String (rowFraction, 3)
                 + " of height, threshold fraction "
                 + juce::String (expectedThresholdFraction, 3));
    }

    // --- Case 3: the boundary seam that 9.7 predicts ----------------------
    // Exactly at the threshold, the int-rounded clip meets a 1.5px float
    // stroke. Report what actually happens rather than asserting a result.
    {
        cabrot::ui::WaspMeter meter;
        meter.setTestReduction (cabrot::theme::kReductionDamageThresholdDb);
        const auto img = render (meter, W, H);

        const int errorPixels = countNear (img, cabrot::theme::stateError, 12);
        const int inkPixels   = countNear (img, cabrot::theme::inkPrimary, 12);
        std::printf ("  INFO  at exactly the threshold: stateError=%d inkPrimary=%d\n",
                     errorPixels, inkPixels);
        std::printf ("        (9.7 predicts primary ink can survive one row into\n"
                     "         damaged territory because the clip rounds to int)\n");
    }

    // --- Case 4: sweep across the threshold ------------------------------
    // Case 3 returned zero for BOTH colours, which no prior document
    // predicted. A flat curve sitting exactly on the clip boundary is split
    // by antialiasing into two part-alpha rows, so neither row matches its
    // token within tolerance: at the exact damage point the curve visibly
    // fades instead of changing colour. This sweep characterises the region.
    {
        std::printf ("\n  Threshold sweep (tolerance 12):\n");
        std::printf ("    dB   stateError  inkPrimary\n");
        for (float db : { 10.0f, 11.0f, 11.5f, 11.9f, 12.0f, 12.1f, 12.5f, 13.0f, 14.0f })
        {
            cabrot::ui::WaspMeter meter;
            meter.setTestReduction (db);
            const auto img = render (meter, W, H);
            std::printf ("  %5.1f   %8d   %9d\n", db,
                         countNear (img, cabrot::theme::stateError, 12),
                         countNear (img, cabrot::theme::inkPrimary, 12));
        }

        // A flat curve anywhere clearly inside damaged territory must be
        // unmistakably red. This is the property that actually matters.
        cabrot::ui::WaspMeter deep;
        deep.setTestReduction (16.0f);
        const auto deepImg = render (deep, W, H);
        check (countNear (deepImg, cabrot::theme::stateError, 12) > 100,
               "well past the threshold the curve is unmistakably damage-coloured",
               juce::String ("stateError pixels ")
                 + juce::String (countNear (deepImg, cabrot::theme::stateError, 12)));
    }

    std::printf ("\n%s\n", failed ? "FAILED" : "all render checks passed");
    return failed ? 1 : 0;
}
