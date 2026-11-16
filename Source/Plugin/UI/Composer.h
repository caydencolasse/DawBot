#pragma once
#include "Components.h"
namespace dawbot::ui
{
class Composer : public juce::Component, public Measured, public juce::FileDragAndDropTarget,
                 private juce::KeyListener
{
public:
    Composer();
    using juce::Component::keyPressed;
    int heightForWidth (int) const override { return references.isEmpty() ? 156 : 232; }
    void paint (juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag (const juce::StringArray&) override { return true; }
    void filesDropped (const juce::StringArray&, int, int) override;
    std::function<void()> onSend, onLayout;
    std::function<void(const juce::String&)> onDraft;
    Field input { "Message", "", true };
    Action send;
    juce::String getReferenceNames() const;
    void clearReferences();
private:
    bool keyPressed (const juce::KeyPress&, juce::Component*) override;
    void chooseFiles();
    Action attach, clear;
    juce::StringArray references;
    juce::String feedback;
    std::unique_ptr<juce::FileChooser> chooser;
};
}
