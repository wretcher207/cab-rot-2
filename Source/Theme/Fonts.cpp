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

juce::Typeface::Ptr g_displayBold;
juce::Typeface::Ptr g_displayMedium;
juce::Typeface::Ptr g_displayRegular;
juce::Typeface::Ptr g_monoRegular;
juce::Typeface::Ptr g_monoBold;
}

juce::Typeface::Ptr Fonts::displayBold()
{
    return cached (g_displayBold, BinaryData::SpaceGroteskBold_ttf, BinaryData::SpaceGroteskBold_ttfSize);
}
juce::Typeface::Ptr Fonts::displayMedium()
{
    return cached (g_displayMedium, BinaryData::SpaceGroteskMedium_ttf, BinaryData::SpaceGroteskMedium_ttfSize);
}
juce::Typeface::Ptr Fonts::displayRegular()
{
    return cached (g_displayRegular, BinaryData::SpaceGroteskRegular_ttf, BinaryData::SpaceGroteskRegular_ttfSize);
}
juce::Typeface::Ptr Fonts::monoRegular()
{
    return cached (g_monoRegular, BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
}
juce::Typeface::Ptr Fonts::monoBold()
{
    return cached (g_monoBold, BinaryData::JetBrainsMonoBold_ttf, BinaryData::JetBrainsMonoBold_ttfSize);
}

juce::Font Fonts::displayTitle() { return withFace (displayBold(),    24.0f, -0.02f); }
juce::Font Fonts::heroNum()      { return withFace (displayBold(),    80.0f, -0.05f); }
juce::Font Fonts::body()         { return withFace (displayRegular(), 16.0f,  0.00f); }
juce::Font Fonts::uiChrome()     { return withFace (monoRegular(),    11.0f,  0.30f); }
juce::Font Fonts::monoData()     { return withFace (monoRegular(),    14.0f,  0.10f); }

juce::Font Fonts::displayTitleScaled (float scale)
{
    return withFace (displayBold(), juce::jlimit (16.0f, 36.0f, 24.0f * scale), -0.02f);
}
juce::Font Fonts::heroNumScaled (float scale)
{
    return withFace (displayBold(), juce::jlimit (56.0f, 120.0f, 80.0f * scale), -0.05f);
}
juce::Font Fonts::uiChromeScaled (float scale)
{
    return withFace (monoRegular(), juce::jlimit (9.0f, 14.0f, 11.0f * scale), 0.30f);
}
juce::Font Fonts::monoDataScaled (float scale)
{
    return withFace (monoRegular(), juce::jlimit (11.0f, 18.0f, 14.0f * scale), 0.10f);
}
} // namespace cabrot::theme
