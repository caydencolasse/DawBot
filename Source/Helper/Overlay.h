#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>
#include <vector>

namespace dawbot
{
struct OverlayStep
{
    juce::String                         text;
    std::optional<juce::Rectangle<int>>  target;   // local to the overlay window
};

// Transparent, click-through, always-on-top window covering one display.
// Draws a pulsing highlight around the UI element the user should click, plus a callout.
class Overlay : public juce::Component,
                private juce::Timer
{
public:
    Overlay();
    ~Overlay() override;

    void present (juce::Rectangle<int> screenArea, std::vector<OverlayStep> steps, int index);
    void showStep (int index);
    void hide();

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    std::vector<OverlayStep> steps;
    int   current = 0;
    float phase   = 0.0f;
};
}
