// Standalone GUI app that hosts the ThemeTest component. Built only as
// a phase-1 visual-diff tool; not part of the plugin.

#include <juce_gui_basics/juce_gui_basics.h>

#include "ThemeTest.h"

namespace cabrot::ui
{
class ThemeTestWindow final : public juce::DocumentWindow
{
public:
    ThemeTestWindow()
        : juce::DocumentWindow ("Cab Rot — Theme Test",
                                juce::Colours::black,
                                juce::DocumentWindow::closeButton)
    {
        setUsingNativeTitleBar (true);
        setResizable (false, false);
        setContentOwned (new ThemeTest(), true);
        centreWithSize (1200, 780);
        setVisible (true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplicationBase::quit();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemeTestWindow)
};

class ThemeTestApp final : public juce::JUCEApplication
{
public:
    ThemeTestApp() = default;

    const juce::String getApplicationName()    override { return "Cab Rot Theme Test"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed()          override { return false; }

    void initialise (const juce::String&) override
    {
        window = std::make_unique<ThemeTestWindow>();
    }

    void shutdown() override { window.reset(); }

    void systemRequestedQuit() override { quit(); }
    void anotherInstanceStarted (const juce::String&) override {}

private:
    std::unique_ptr<ThemeTestWindow> window;
};
} // namespace cabrot::ui

START_JUCE_APPLICATION (cabrot::ui::ThemeTestApp)
