#include "Overlay.h"
#include "ScreenCapture.h"
#include <cmath>

namespace dawbot
{
namespace
{
    const juce::Colour accent (0xff7c5cff);
}

Overlay::Overlay()
{
    setOpaque (false);
    setInterceptsMouseClicks (false, false);
    setAlwaysOnTop (true);
}

Overlay::~Overlay()
{
    stopTimer();
}

void Overlay::present (juce::Rectangle<int> screenArea, std::vector<OverlayStep> newSteps, int index)
{
    steps = std::move (newSteps);
    setBounds (screenArea);

    if (! isOnDesktop())
    {
        addToDesktop (juce::ComponentPeer::windowIsTemporary | juce::ComponentPeer::windowIgnoresMouseClicks);

        if (auto* peer = getPeer())
            capture::excludeWindowFromCapture (peer->getNativeHandle());
    }

    showStep (index);
}

void Overlay::showStep (int index)
{
    if (steps.empty())
    {
        hide();
        return;
    }

    current = juce::jlimit (0, (int) steps.size() - 1, index);
    const bool hasTarget = steps[(size_t) current].target.has_value();

    setVisible (hasTarget);

    if (hasTarget)
        startTimerHz (30);
    else
        stopTimer();

    repaint();
}

void Overlay::hide()
{
    stopTimer();
    setVisible (false);
}

void Overlay::timerCallback()
{
    phase += 0.18f;

    if (current < (int) steps.size() && steps[(size_t) current].target)
        repaint (steps[(size_t) current].target->expanded (40));
}

void Overlay::paint (juce::Graphics& g)
{
    if (current >= (int) steps.size() || ! steps[(size_t) current].target)
        return;

    const auto& step = steps[(size_t) current];
    const auto target = step.target->toFloat().expanded (6.0f);
    const float pulse = 0.5f + 0.5f * std::sin (phase);

    // Highlight
    g.setColour (accent.withAlpha (0.12f + 0.12f * pulse));
    g.fillRoundedRectangle (target, 8.0f);
    g.setColour (accent.withAlpha (0.95f));
    g.drawRoundedRectangle (target, 8.0f, 3.0f);
    g.setColour (accent.withAlpha (0.4f * (1.0f - pulse)));
    g.drawRoundedRectangle (target.expanded (6.0f + 14.0f * pulse), 12.0f, 2.0f);

    // Callout
    juce::AttributedString text;
    text.append ("Step " + juce::String (current + 1) + "  ", juce::Font (juce::FontOptions (15.0f).withStyle ("Bold")), accent.brighter (0.6f));
    text.append (step.text, juce::Font (juce::FontOptions (15.0f)), juce::Colours::white);
    text.setWordWrap (juce::AttributedString::byWord);

    juce::TextLayout layout;
    layout.createLayout (text, 340.0f);

    const float bubbleW = layout.getWidth() + 24.0f;
    const float bubbleH = layout.getHeight() + 20.0f;
    const auto bounds = getLocalBounds().toFloat().reduced (8.0f);

    float x = juce::jlimit (bounds.getX(), bounds.getRight() - bubbleW, target.getCentreX() - bubbleW * 0.5f);
    float y = target.getBottom() + 14.0f;
    if (y + bubbleH > bounds.getBottom())
        y = target.getY() - 14.0f - bubbleH;
    y = juce::jlimit (bounds.getY(), bounds.getBottom() - bubbleH, y);

    const juce::Rectangle<float> bubble (x, y, bubbleW, bubbleH);

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (bubble.translated (0.0f, 3.0f), 10.0f);
    g.setColour (juce::Colour (0xf0151a26));
    g.fillRoundedRectangle (bubble, 10.0f);
    g.setColour (accent);
    g.drawRoundedRectangle (bubble, 10.0f, 1.5f);

    layout.draw (g, bubble.reduced (12.0f, 10.0f));
}
}
