#include "Fonts.h"
#include "BinaryData.h"

namespace cabrot::theme
{
namespace
{
// Slot writes are guarded so a future background pre-warm or audio-thread
// touch of a typeface can't race the message thread's first read. The lock
// is uncontended on the hot path (each slot is written exactly once), so
// SpinLock is appropriate over a full mutex.
juce::SpinLock& fontLock()
{
    static juce::SpinLock lock;
    return lock;
}

juce::Typeface::Ptr cached (juce::Typeface::Ptr& slot, const char* data, int size)
{
    {
        const juce::SpinLock::ScopedLockType guard (fontLock());
        if (slot != nullptr)
            return slot;
    }
    auto fresh = juce::Typeface::createSystemTypefaceFor (data, static_cast<size_t> (size));
    const juce::SpinLock::ScopedLockType guard (fontLock());
    if (slot == nullptr)
        slot = fresh;
    return slot;
}

juce::Font withFace (juce::Typeface::Ptr face, float pixels, float kerning)
{
    juce::FontOptions opts (pixels);
    if (face != nullptr)
        opts = opts.withTypeface (face);
    auto f = juce::Font (opts);
    if (kerning != 0.0f)
        f = f.withExtraKerningFactor (kerning);
    return f;
}

juce::Typeface::Ptr g_dpdDisplay;
juce::Typeface::Ptr g_inter;
juce::Typeface::Ptr g_interMedium;
juce::Typeface::Ptr g_mono;
}

juce::Typeface::Ptr Fonts::dpdDisplay()
{
    return cached (g_dpdDisplay, BinaryData::DPDDisplayRegular_ttf, BinaryData::DPDDisplayRegular_ttfSize);
}
juce::Typeface::Ptr Fonts::inter()
{
    return cached (g_inter, BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize);
}
juce::Typeface::Ptr Fonts::interMedium()
{
    return cached (g_interMedium, BinaryData::InterMedium_ttf, BinaryData::InterMedium_ttfSize);
}
juce::Typeface::Ptr Fonts::mono()
{
    return cached (g_mono, BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
}

juce::Font Fonts::wordmark() { return withFace (dpdDisplay(), 24.0f, 0.30f); }

juce::Font Fonts::display (float heightPx, float trackingEm)
{
    return withFace (dpdDisplay(), heightPx, trackingEm);
}

juce::Font Fonts::body (float heightPx, bool medium)
{
    return withFace (medium ? interMedium() : inter(), heightPx, 0.0f);
}

juce::Font Fonts::mono (float heightPx, float trackingEm)
{
    return withFace (mono(), heightPx, trackingEm);
}

juce::Font Fonts::monoLabel (float heightPx)
{
    return withFace (mono(), heightPx, 0.18f);
}

juce::Font Fonts::wordmarkScaled (float scale)
{
    return withFace (dpdDisplay(), juce::jlimit (18.0f, 34.0f, 24.0f * scale), 0.30f);
}
juce::Font Fonts::displayScaled (float scale, float refPx, float floorPx)
{
    return withFace (dpdDisplay(), juce::jmax (floorPx, refPx * scale), 0.06f);
}
juce::Font Fonts::monoScaled (float scale, float refPx, float floorPx, float trackingEm)
{
    return withFace (mono(), juce::jmax (floorPx, refPx * scale), trackingEm);
}
juce::Font Fonts::monoLabelScaled (float scale, float refPx, float floorPx)
{
    return withFace (mono(), juce::jmax (floorPx, refPx * scale), 0.18f);
}
} // namespace cabrot::theme
