#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "MidiDragTile.h"

class DawBotEditor : public juce::AudioProcessorEditor,
                     private juce::ChangeListener
{
public:
    explicit DawBotEditor (DawBotProcessor&);
    ~DawBotEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override { refresh(); }
    void refresh();
    void submit();

    DawBotProcessor& proc;

    juce::Label        title, status, stepLabel;
    juce::TextButton   launchButton { "Launch Helper" }, newChatButton { "New chat" };
    juce::TextEditor   transcript, input;
    juce::TextButton   sendButton { "Ask" };
    juce::ToggleButton showOnScreen { "Show me on screen (uses screen capture)" };
    juce::TextButton   prevStep { "<" }, nextStep { ">" }, hideOverlayButton { "Hide" };
    MidiDragTile       midiTile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DawBotEditor)
};
