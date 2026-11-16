#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class DawBotEditor : public juce::AudioProcessorEditor, private juce::ChangeListener
{
public:
    explicit DawBotEditor (DawBotProcessor&);
    ~DawBotEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    struct Interface;
    std::unique_ptr<Interface> ui;
    DawBotProcessor& proc;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DawBotEditor)
};
