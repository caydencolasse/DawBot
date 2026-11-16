#include "PluginEditor.h"

namespace
{
    const juce::Colour background (0xff0d1017);
    const juce::Colour panel      (0xff141925);
    const juce::Colour accent     (0xff7c5cff);
    const juce::Colour userText   (0xff8ab4ff);
}

DawBotEditor::DawBotEditor (DawBotProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    title.setText ("DawBot", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (20.0f).withStyle ("Bold")));
    title.setColour (juce::Label::textColourId, juce::Colours::white);

    status.setJustificationType (juce::Justification::centredRight);
    status.setFont (juce::Font (juce::FontOptions (12.0f)));

    stepLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    stepLabel.setMinimumHorizontalScale (0.7f);

    transcript.setMultiLine (true);
    transcript.setReadOnly (true);
    transcript.setScrollbarsShown (true);
    transcript.setCaretVisible (false);
    transcript.setColour (juce::TextEditor::backgroundColourId, panel);
    transcript.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    transcript.setFont (juce::Font (juce::FontOptions (14.0f)));

    input.setTextToShowWhenEmpty ("Ask anything, e.g. \"Show me how to draw an E minor chord\"", juce::Colours::grey);
    input.setFont (juce::Font (juce::FontOptions (14.0f)));
    input.onReturnKey = [this] { submit(); };

    sendButton.setColour (juce::TextButton::buttonColourId, accent);
    sendButton.onClick = [this] { submit(); };

    showOnScreen.setToggleState (true, juce::dontSendNotification);

    launchButton.onClick      = [this] { proc.launchHelper(); };
    newChatButton.onClick     = [this] { proc.resetChat(); };
    prevStep.onClick          = [this] { proc.showStep (proc.getCurrentStep() - 1); };
    nextStep.onClick          = [this] { proc.showStep (proc.getCurrentStep() + 1); };
    hideOverlayButton.onClick = [this] { proc.hideOverlay(); };

    for (auto* c : std::initializer_list<juce::Component*> { &title, &status, &launchButton, &newChatButton,
                                                             &transcript, &input, &sendButton, &showOnScreen,
                                                             &prevStep, &nextStep, &hideOverlayButton, &stepLabel,
                                                             &midiTile })
        addAndMakeVisible (c);

    proc.addChangeListener (this);
    proc.startHelperConnection();

    setResizable (true, true);
    setResizeLimits (360, 480, 1200, 1600);
    setSize (440, 660);

    refresh();
}

DawBotEditor::~DawBotEditor()
{
    proc.removeChangeListener (this);
}

void DawBotEditor::submit()
{
    const auto text = input.getText();
    if (text.trim().isEmpty() || proc.isWaiting())
        return;

    input.clear();
    proc.ask (text, showOnScreen.getToggleState());
}

void DawBotEditor::refresh()
{
    const bool connected = proc.isHelperConnected();
    launchButton.setVisible (! connected);

    if (proc.isWaiting())
    {
        status.setText (proc.getStatusText(), juce::dontSendNotification);
        status.setColour (juce::Label::textColourId, accent.brighter());
    }
    else
    {
        status.setText (connected ? "Helper connected" : "Helper offline", juce::dontSendNotification);
        status.setColour (juce::Label::textColourId, connected ? juce::Colours::lightgreen : juce::Colours::orange);
    }

    // Transcript
    transcript.clear();
    const auto& entries = proc.getEntries();

    if (entries.isEmpty())
    {
        transcript.setColour (juce::TextEditor::textColourId, juce::Colours::white.withAlpha (0.7f));
        transcript.insertTextAtCaret ("Hey! I'm DawBot, your production coach.\n\n"
                                      "Try:\n"
                                      "- \"Show me how to draw an E minor chord progression\"\n"
                                      "- \"How do I load a piano instrument?\"\n"
                                      "- \"How do I get a supersaw lead like Martin Garrix in Serum?\"\n\n"
                                      "Keep \"Show me on screen\" on and I'll highlight where to click.");
    }

    for (const auto& e : entries)
    {
        const bool you = e.role == "you";
        transcript.setColour (juce::TextEditor::textColourId, you ? userText : juce::Colours::white);
        transcript.insertTextAtCaret ((you ? "You: " : "DawBot: ") + e.text + "\n\n");
    }

    transcript.moveCaretToEnd();

    // Step navigator
    const int numSteps = proc.getNumSteps();
    const bool hasSteps = numSteps > 0;

    for (auto* c : std::initializer_list<juce::Component*> { &prevStep, &nextStep, &hideOverlayButton, &stepLabel })
        c->setVisible (hasSteps);

    if (hasSteps)
    {
        const int cur = proc.getCurrentStep();
        stepLabel.setText ("Step " + juce::String (cur + 1) + "/" + juce::String (numSteps) + ": " + proc.getStepText (cur),
                           juce::dontSendNotification);
        prevStep.setEnabled (cur > 0);
        nextStep.setEnabled (cur < numSteps - 1);
    }

    // MIDI tile
    midiTile.setClip (proc.getMidiClip());
    midiTile.setVisible (proc.getMidiClip().has_value());

    sendButton.setEnabled (! proc.isWaiting());

    resized();
}

void DawBotEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);
}

void DawBotEditor::resized()
{
    auto r = getLocalBounds().reduced (10);

    auto top = r.removeFromTop (30);
    title.setBounds (top.removeFromLeft (90));
    if (launchButton.isVisible())
        launchButton.setBounds (top.removeFromRight (110).reduced (0, 2));
    newChatButton.setBounds (top.removeFromRight (74).reduced (2, 2));
    status.setBounds (top);

    r.removeFromTop (6);

    auto inputRow = r.removeFromBottom (34);
    sendButton.setBounds (inputRow.removeFromRight (64));
    inputRow.removeFromRight (6);
    input.setBounds (inputRow);

    r.removeFromBottom (4);
    showOnScreen.setBounds (r.removeFromBottom (24));

    if (midiTile.isVisible())
    {
        r.removeFromBottom (6);
        midiTile.setBounds (r.removeFromBottom (84));
    }

    if (stepLabel.isVisible())
    {
        r.removeFromBottom (6);
        auto row = r.removeFromBottom (30);
        prevStep.setBounds (row.removeFromLeft (30));
        hideOverlayButton.setBounds (row.removeFromRight (56));
        row.removeFromRight (4);
        nextStep.setBounds (row.removeFromRight (30));
        stepLabel.setBounds (row.reduced (4, 0));
    }

    r.removeFromBottom (6);
    transcript.setBounds (r);
}
