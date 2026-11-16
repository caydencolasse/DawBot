#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Shared/MidiClip.h"
#include <optional>

// Shows a mini piano roll of the AI-generated clip. Drag it into the DAW
// and it lands as a regular MIDI clip (works in Ableton, Logic, FL, Cubase, Studio One...).
class MidiDragTile : public juce::Component
{
public:
    MidiDragTile()
    {
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
    }

    void setClip (const std::optional<dawbot::MidiClip>& newClip)
    {
        clip = newClip;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto accent = juce::Colour (0xfffa684a);
        auto b = getLocalBounds().toFloat().reduced (1.0f);

        g.setColour (juce::Colour (0xff242427));
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (accent);
        g.drawRoundedRectangle (b, 8.0f, 1.5f);

        if (! clip.has_value())
            return;

        auto area = b.reduced (10.0f);
        auto header = area.removeFromTop (18.0f);

        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
        g.drawText (clip->name.isEmpty() ? "DawBot clip" : clip->name, header, juce::Justification::centredLeft);

        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawText ("Drag me onto a MIDI track", header, juce::Justification::centredRight);

        area.removeFromTop (4.0f);

        int lo = 127, hi = 0;
        for (const auto& n : clip->notes) { lo = juce::jmin (lo, n.pitch); hi = juce::jmax (hi, n.pitch); }

        const auto len   = (float) clip->lengthBeats();
        const auto range = (float) juce::jmax (1, hi - lo + 1);
        const auto rowH  = area.getHeight() / range;

        for (const auto& n : clip->notes)
        {
            const float x = area.getX() + (float) n.startBeats / len * area.getWidth();
            const float w = juce::jmax (2.0f, (float) n.lengthBeats / len * area.getWidth() - 1.0f);
            const float y = area.getBottom() - (float) (n.pitch - lo + 1) * rowH;

            g.setColour (accent.withAlpha (0.45f + 0.55f * (float) n.velocity / 127.0f));
            g.fillRoundedRectangle (x, y, w, juce::jmax (2.0f, rowH - 1.0f), 2.0f);
        }
    }

    void mouseDown (const juce::MouseEvent&) override { dragStarted = false; }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! clip.has_value() || dragStarted || e.getDistanceFromDragStart() < 5)
            return;

        dragStarted = true;
        const auto file = clip->writeToTempFile();

        if (file.existsAsFile())
        {
            juce::StringArray files;
            files.add (file.getFullPathName());
            juce::DragAndDropContainer::performExternalDragDropOfFiles (files, false, this);
        }
    }

private:
    std::optional<dawbot::MidiClip> clip;
    bool dragStarted = false;
};
