#include "Composer.h"
namespace dawbot::ui
{
Composer::Composer()
    : send (juce::String::fromUTF8 ("↑"), [this] { if (onSend) onSend(); }, Tone::primary),
      attach ("+", [this] { chooseFiles(); }),
      clear ("Remove references", [this] { clearReferences(); }, Tone::ghost)
{
    send.setName ("Send message"); send.setTooltip (juce::String::fromUTF8("Send text · Command/Ctrl + Enter"));
    attach.setName ("Attach files"); attach.setTooltip ("Add local audio, MIDI or file references");
    input.setTextToShowWhenEmpty (juce::String::fromUTF8("Ask dawbot anything, or drop an audio snippet…"), muted);
    input.getProperties().set ("borderless", true); input.setIndents (0, 2); input.addKeyListener (this);
    input.onTextChange = [this] { if (onDraft) onDraft (input.getText()); };
    for (auto* c : std::initializer_list<juce::Component*> { &input, &send, &attach, &clear }) addAndMakeVisible (c);
}
void Composer::paint (juce::Graphics& g)
{
    g.setColour (surface); g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1), 20);
    g.setColour (accent); g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1), 20, 2);
    g.setFont (font (13)); g.setColour (muted);
    g.drawText (juce::String::fromUTF8("Audio · MIDI · Files"), 84, getHeight() - 64, 240, 44, juce::Justification::centredLeft);
    if (! references.isEmpty())
    {
        g.drawFittedText (getReferenceNames(), 24, 88, getWidth() - 240, 24, juce::Justification::centredLeft, 1, 1.0f);
        g.drawFittedText (feedback.isNotEmpty() ? feedback : "References stay local. Only your text is sent; audio analysis is not connected yet.",
                         24, 116, getWidth() - 48, 40, juce::Justification::centredLeft, 2, 1.0f);
    }
    else if (feedback.isNotEmpty()) { g.setColour (error); g.drawText (feedback, 24, 67, getWidth() - 48, 22, juce::Justification::centredLeft); }
}
void Composer::resized()
{
    input.setBounds (24, 20, getWidth() - 48, 56);
    attach.setBounds (24, getHeight() - 64, 44, 44); send.setBounds (getWidth() - 72, getHeight() - 64, 48, 44);
    clear.setVisible (! references.isEmpty()); clear.setBounds (getWidth() - 212, 80, 188, 44);
}
void Composer::clearReferences()
{
    references.clear(); feedback.clear(); resized(); repaint(); if (onLayout) onLayout();
}
bool Composer::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    if (key.getKeyCode() == juce::KeyPress::returnKey && (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown()))
    { if (onSend) onSend(); return true; }
    return false;
}
void Composer::chooseFiles()
{
    chooser = std::make_unique<juce::FileChooser> ("Add a local reference", juce::File(), "*.wav;*.aif;*.aiff;*.mp3;*.flac;*.mid;*.midi;*.txt;*.pdf;*.png;*.jpg");
    juce::Component::SafePointer<Composer> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems,
        [safe] (const juce::FileChooser& c) { if (safe != nullptr) { juce::StringArray files; for (const auto& f : c.getResults()) files.add (f.getFullPathName()); safe->filesDropped (files, 0, 0); } });
}
void Composer::filesDropped (const juce::StringArray& files, int, int)
{
    feedback.clear();
    for (const auto& path : files)
    {
        const juce::File f (path);
        if (! f.existsAsFile() || f.getSize() > 50 * 1024 * 1024 || f.getSize() == 0 || ! f.hasFileExtension ("wav;aif;aiff;mp3;flac;mid;midi;txt;pdf;png;jpg"))
        { feedback = "Choose an audio, MIDI, image or document file under 50 MB."; continue; }
        if (references.size() >= 4) { feedback = "Up to four local references per question."; break; }
        references.addIfNotAlreadyThere (f.getFullPathName());
    }
    attach.setTooltip (feedback.isEmpty() ? "Add local references" : feedback); resized(); repaint(); if (onLayout) onLayout();
}
juce::String Composer::getReferenceNames() const
{
    juce::StringArray names; for (const auto& path : references) names.add (juce::File (path).getFileName()); return names.joinIntoString (", ");
}
}
