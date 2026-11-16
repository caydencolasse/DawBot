#include "UI/EditorInterface.h"
#include "Shared/Protocol.h"
using namespace dawbot::ui;
namespace
{
class Brand : public juce::Button
{
public:
    Brand (std::function<void()> action) : Button ("Home") { onClick = std::move (action); setTooltip ("Go to dawbot home"); }
    void paintButton (juce::Graphics& g, bool, bool) override
    {
        g.drawImageWithin (logo(), 0, 3, 42, 42, juce::RectanglePlacement::centred);
        g.setColour (foreground); g.setFont (font (21, true)); g.drawText ("dawbot", 58, 0, 230, 28, juce::Justification::centredLeft);
        g.setColour (muted); g.setFont (font (13)); g.drawText ("Your studio companion", 58, 29, 230, 20, juce::Justification::centredLeft);
        if (hasKeyboardFocus (true)) { g.setColour (accent); g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1), 8, 1); }
    }
};
class Welcome : public juce::Component, public Measured
{
public:
    int heightForWidth (int width) const override { return width < 900 ? 76 : 92; }
    void paint (juce::Graphics& g) override
    {
        const bool small = getWidth() < 900;
        const int size = small ? 64 : 92, x = size + 24;
        g.drawImageWithin (logo(), 0, 0, size, size, juce::RectanglePlacement::centred);
        g.setColour (foreground); g.setFont (font (small ? 28.0f : 38.0f, true));
        g.drawText ("What are we making today?", x, small ? 0 : 10, getWidth()-x, 48, juce::Justification::centredLeft);
        g.setColour (muted); g.setFont (font (16));
        g.drawText ("Find your sound. Learn your DAW. Keep creating.", x, small ? 48 : 60, getWidth()-x, 24, juce::Justification::centredLeft);
    }
};
juce::Component* findName (juce::Component& parent, const juce::String& name)
{
    if (parent.getName() == name && dynamic_cast<juce::TextEditor*> (&parent) != nullptr) return &parent;
    for (auto* child : parent.getChildren()) if (auto* found = findName (*child, name)) return found;
    return nullptr;
}
bool isSettings (DawBotScreen s)
{
    return s == DawBotScreen::settings || s == DawBotScreen::tools || s == DawBotScreen::privacy || s == DawBotScreen::help || s == DawBotScreen::clearHistory;
}
}

DawBotEditor::DawBotEditor (DawBotProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    ui = std::make_unique<Interface> (*this, p);
    setResizable (true, true); setResizeLimits (800, 640, 1600, 1400); setSize (1200, 1000);
    proc.addChangeListener (this); proc.startHelperConnection(); ui->render();
}
DawBotEditor::~DawBotEditor() { proc.removeChangeListener (this); setLookAndFeel (nullptr); ui.reset(); }
void DawBotEditor::paint (juce::Graphics& g) { g.fillAll (background); g.setColour (border); g.drawHorizontalLine (79, 0, (float) getWidth()); }
void DawBotEditor::resized() { if (ui) ui->layout(); }
void DawBotEditor::changeListenerCallback (juce::ChangeBroadcaster*) { ui->refresh(); }

DawBotEditor::Interface::Interface (DawBotEditor& e, DawBotProcessor& p)
    : editor (e), proc (p), host ("Choose your DAW", [this] { navigate (DawBotScreen::settings); }),
      history ("History", [this] { navigate (DawBotScreen::history); }, Tone::ghost, "history"),
      settings ("Settings", [this] { navigate (DawBotScreen::settings); }, Tone::ghost, "settings")
{
    editor.setLookAndFeel (&lookAndFeel);
    observedRestoreCount = proc.getWorkspaceRestoreCount();
    brand = std::make_unique<Brand> ([this] { navigate (DawBotScreen::home); });
    for (auto* c : std::initializer_list<juce::Component*> { brand.get(), &host, &history, &settings, &viewport }) editor.addAndMakeVisible (c);
    viewport.setScrollBarsShown (true, false); viewport.setScrollBarThickness (8);
    composer.input.setText (proc.getWorkspace().snapshot()["draft"].toString(), false);
    composer.onDraft = [this] (const juce::String& draft) { proc.getWorkspace().setDraft (draft); changed(); };
    composer.onSend = [this] { submit(); };
    composer.onLayout = [this] { layout(); };
    if (! proc.getEntries().isEmpty()) screen = DawBotScreen::chat;
}
DawBotEditor::Interface::~Interface() { cancelPendingUpdate(); viewport.setViewedComponent (nullptr, false); }
void DawBotEditor::Interface::layout()
{
    const int w = editor.getWidth();
    brand->setBounds (24, 16, juce::jmax (240, w - 460), 48);
    host.setBounds (w - 400, 18, 224, 44); history.setBounds (w - 156, 18, 44, 44); settings.setBounds (w - 100, 18, 44, 44);
    const bool nav = screen != DawBotScreen::home && ! isSettings (screen);
    if (navigation) { navigation->setVisible (nav); navigation->setBounds (24, 90, juce::jmin (820, w-48), 44); }
    const int top = nav ? 144 : 80;
    const bool chat = screen == DawBotScreen::chat;
    const int composerHeight = composer.heightForWidth (w - 72);
    viewport.setBounds (0, top, w, editor.getHeight() - top - (chat ? composerHeight + 44 : 0));
    if (chat) composer.setBounds (36, editor.getHeight() - composerHeight - 24, w - 72, composerHeight);
    if (body)
    {
        // Reserve the scrollbar gutter consistently to prevent layout oscillation.
        const int contentWidth = w - 8;
        body->setSize (contentWidth, juce::jmax (viewport.getHeight(), body->heightForWidth (contentWidth)));
    }
    const bool nowCompact = w < 1000;
    if (nowCompact != compact) { compact = nowCompact; if (screen == DawBotScreen::home) triggerAsyncUpdate(); }
}
void DawBotEditor::Interface::navigate (DawBotScreen next)
{
    screen = next; filter.clear(); category.clear(); message.clear();
    viewport.setViewPosition (0, 0); triggerAsyncUpdate();
}
void DawBotEditor::Interface::changed() { proc.updateHostDisplay (juce::AudioProcessor::ChangeDetails().withNonParameterStateChanged (true)); }
void DawBotEditor::Interface::setPreference (const juce::String& key, const juce::var& value)
{
    auto prefs = proc.getWorkspace().snapshot()["preferences"];
    prefs.getDynamicObject()->setProperty (key, value); proc.getWorkspace().setPreferences (prefs); changed();
}
void DawBotEditor::Interface::refresh()
{
    host.setButtonText (proc.getHostName());
    composer.send.setEnabled (! proc.isWaiting());
    if (helperStatus != nullptr && helperDetails != nullptr)
    {
        helperStatus->setText (proc.isHelperConnected() ? "Helper connected" : "Helper offline");
        helperDetails->setText (proc.getLastError().isNotEmpty() ? proc.getLastError() : "AI answers, screen guidance and MIDI generation run in the DawBot Helper app.");
        layout();
    }
    const auto draft = proc.getWorkspace().snapshot()["draft"].toString();
    if (composer.input.getText() != draft) composer.input.setText (draft, false);
    if (observedRestoreCount != proc.getWorkspaceRestoreCount())
    {
        observedRestoreCount = proc.getWorkspaceRestoreCount(); message.clear(); composer.clearReferences();
        if (screen == DawBotScreen::home && ! proc.getEntries().isEmpty()) screen = DawBotScreen::chat;
        else if (screen == DawBotScreen::chat && proc.getEntries().isEmpty()) screen = DawBotScreen::home;
        triggerAsyncUpdate();
    }
    if (screen == DawBotScreen::chat || screen == DawBotScreen::home) triggerAsyncUpdate();
}
void DawBotEditor::Interface::submit()
{
    const auto question = composer.input.getText().trim();
    if (question.isEmpty() || proc.isWaiting()) return;
    const bool screenGuide = (bool) proc.getWorkspace().snapshot()["preferences"]["screenGuidance"];
    if (proc.ask (question, screenGuide)) { composer.input.clear(); proc.getWorkspace().setDraft ({}); }
    screen = DawBotScreen::chat; triggerAsyncUpdate();
}
void DawBotEditor::Interface::queueQuestion (const juce::String& question)
{
    composer.input.setText (question); proc.getWorkspace().setDraft (question);
    screen = proc.getEntries().isEmpty() ? DawBotScreen::home : DawBotScreen::chat;
    focusName = "Message"; focusPosition = question.length(); triggerAsyncUpdate();
}
void DawBotEditor::Interface::render()
{
    const auto oldPosition = viewport.getViewPosition();
    if (auto* focused = dynamic_cast<juce::TextEditor*> (juce::Component::getCurrentlyFocusedComponent()))
        if (editor.isParentOf (focused)) { focusName = focused->getName(); focusPosition = focused->getCaretPosition(); }
    helperStatus = nullptr; helperDetails = nullptr;
    composer.setVisible (false); editor.removeChildComponent (&composer);
    viewport.setViewedComponent (nullptr, false); body.reset(); navigation.reset();
    body = std::make_unique<Stack> (20, compact ? 24 : 36);
    viewport.setViewedComponent (body.get(), false);
    auto nav = std::make_unique<Grid> (5, 12);
    const std::pair<const char*, DawBotScreen> destinations[] = {
        { "Chat", DawBotScreen::home }, { "Your Projects", DawBotScreen::projects }, { "Guides", DawBotScreen::guides },
        { "Tutorials", DawBotScreen::tutorials }, { "How-to", DawBotScreen::howto } };
    for (const auto& [label, dest] : destinations)
        nav->add<Action> (label, [this, dest] { navigate (dest == DawBotScreen::home && ! proc.getEntries().isEmpty() ? DawBotScreen::chat : dest); }, screen == dest || (dest == DawBotScreen::home && screen == DawBotScreen::chat) ? Tone::selected : Tone::ghost);
    navigation = std::move (nav); editor.addAndMakeVisible (*navigation);
    host.setButtonText (proc.getHostName());
    if (message.isNotEmpty()) notice ("Workspace", message);
    switch (screen)
    {
        case DawBotScreen::home: showHome(); break;
        case DawBotScreen::chat: showChat(); break;
        case DawBotScreen::projects: showProjects(); break;
        case DawBotScreen::project: showProject(); break;
        case DawBotScreen::newProject: showProjectForm (false); break;
        case DawBotScreen::renameProject: showProjectForm (true); break;
        case DawBotScreen::saveProject: showSaveProject(); break;
        case DawBotScreen::history: showHistory(); break;
        case DawBotScreen::guides: case DawBotScreen::tutorials: case DawBotScreen::howto: showLearning(); break;
        case DawBotScreen::article: showArticle(); break;
        case DawBotScreen::lesson: showLesson(); break;
        case DawBotScreen::settings: case DawBotScreen::tools: case DawBotScreen::privacy: showSettings(); break;
        case DawBotScreen::help: showHelp(); break;
        case DawBotScreen::saved:
            heading (savedTitle, "Saved in this DAW session"); body->add<Text> (savedText, 16, muted);
            action (*body, "Copy text", [this] { juce::SystemClipboard::copyTextToClipboard (savedText); }); break;
        case DawBotScreen::deleteProject:
            heading ("Delete this project?", selectedProject()["name"].toString());
            notice ("This cannot be undone", "This removes the project and its saved answers from this plugin instance. Files on your computer remain in place.", true);
            action (*body, "Keep project", [this] { navigate (DawBotScreen::project); }, Tone::primary);
            action (*body, "Delete project", [this] { proc.getWorkspace().deleteProject (selectedId); changed(); navigate (DawBotScreen::projects); }); break;
        case DawBotScreen::clearHistory:
            heading ("Clear your conversation history?");
            notice ("This cannot be undone", "Saved project answers remain in place. The current chat and saved history will be cleared.", true);
            action (*body, "Keep history", [this] { navigate (DawBotScreen::privacy); }, Tone::primary);
            action (*body, "Clear history", [this] { proc.clearHistory(); composer.clearReferences(); composer.input.clear(); changed(); navigate (DawBotScreen::home); }); break;
    }
    layout(); viewport.setViewPosition (oldPosition);
    if (focusName.isNotEmpty() && editor.isShowing())
        if (auto* c = dynamic_cast<juce::TextEditor*> (findName (editor, focusName))) { c->grabKeyboardFocus(); c->setCaretPosition (focusPosition); }
    focusName.clear(); editor.repaint();
}
void DawBotEditor::Interface::heading (const juce::String& title, const juce::String& subtitle)
{
    auto& group = body->add<Stack> (8); group.add<Text> (title, 28, foreground, true);
    if (subtitle.isNotEmpty()) group.add<Text> (subtitle, 16, muted);
}
void DawBotEditor::Interface::notice (const juce::String& title, const juce::String& description, bool problem)
{
    auto& group = body->add<Stack> (6, 20, true); group.panelColour = problem ? surface : accentTint;
    group.add<Text> (title, 16, problem ? error : foreground, true); group.add<Text> (description, 13, muted);
}
Action& DawBotEditor::Interface::action (Stack& parent, const juce::String& label, std::function<void()> callback, Tone tone)
{
    return parent.add<Action> (label, std::move (callback), tone);
}
Field& DawBotEditor::Interface::field (Stack& parent, const juce::String& label, const juce::String& value, bool multiline)
{
    auto& group = parent.add<Stack> (8); group.add<Text> (label, 13, muted); return group.add<Field> (label, value, multiline);
}
void DawBotEditor::Interface::search (const juce::String& placeholder)
{
    auto& input = body->add<Field> ("Search", filter); input.setTextToShowWhenEmpty (placeholder, muted);
    input.onTextChange = [this, ptr = &input] { filter = ptr->getText(); focusName = "Search"; focusPosition = ptr->getCaretPosition(); triggerAsyncUpdate(); };
}
void DawBotEditor::Interface::showHome()
{
    body->add<Welcome>();
    auto& prompts = body->add<Grid> (2, 20);
    const juce::StringArray titles { "How do I make a Fred again.. style vocal stutter?", "How do I get that classic chipmunk vocal sound?", juce::String::fromUTF8("Which synth should I use—or can I use stock?"), "How do I automate a filter in my DAW?" };
    const juce::StringArray categories { "SOUND DESIGN", "VOCAL EFFECTS", "PLUGIN ADVICE", "DAW SHORTCUTS" }, icons { "wave", "mic", "keys", "command" };
    for (int i = 0; i < 4; ++i) prompts.add<Card> (titles[i], categories[i], icons[i], [this, q = titles[i]] { queueQuestion (q); }, Card::Kind::prompt);
    body->borrow (composer); body->add<Text> ("Tips tailored to your DAW and the tools you have.", 13, muted);
    body->add<Text> ("Your creative workspace", 21, foreground, true);
    auto& modules = body->add<Grid> (4, 16);
    const juce::StringArray labels { "Your Projects", "Guides", "Tutorials", "How-to" }, descriptions { "Pick up where you left off", "Explore production techniques", "Follow a lesson", "Get a quick walkthrough" }, moduleIcons { "folder", "book", "play", "help" };
    const DawBotScreen dest[] { DawBotScreen::projects, DawBotScreen::guides, DawBotScreen::tutorials, DawBotScreen::howto };
    for (int i = 0; i < 4; ++i) modules.add<Card> (labels[i], descriptions[i], moduleIcons[i], [this, d = dest[i]] { navigate (d); }, compact ? Card::Kind::compactModule : Card::Kind::module);
}
void DawBotEditor::Interface::showChat()
{
    heading ("Your conversation", proc.getHostName());
    auto& controls = body->add<Grid> (3, 12);
    action (controls, "New chat", [this] { proc.resetChat(); composer.clearReferences(); composer.input.clear(); navigate (DawBotScreen::home); });
    action (controls, "Save to project", [this] { startSave ("DawBot conversation", transcript()); }, Tone::primary).setEnabled (! proc.getEntries().isEmpty());
    action (controls, "Copy conversation", [this] { juce::SystemClipboard::copyTextToClipboard (transcript()); }).setEnabled (! proc.getEntries().isEmpty());
    if (proc.getLastError().isNotEmpty()) { notice ("Your question is ready to retry", proc.getLastError(), true); action (*body, "Launch Helper", [this] { proc.launchHelper(); }); }
    if (proc.getEntries().isEmpty()) body->add<Text> ("Ask a production question to start the conversation.", 16, muted);
    for (const auto& entry : proc.getEntries())
    {
        auto& card = body->add<Stack> (12, 24, true); card.panelColour = entry.role == "you" ? raised : surface;
        card.add<Text> (entry.role == "you" ? "You" : "dawbot", 16, foreground, true); card.add<Text> (entry.text, 16, entry.role == "you" ? foreground : muted);
    }
    if (proc.isWaiting()) notice ("Working on your question", proc.getStatusText());
    if (proc.getNumSteps() > 0)
    {
        auto& panel = body->add<Stack> (12, 20, true);
        panel.add<Text> ("Step " + juce::String (proc.getCurrentStep()+1) + " of " + juce::String (proc.getNumSteps()), 13, accent);
        panel.add<Text> (proc.getStepText (proc.getCurrentStep()), 16);
        auto& row = panel.add<Grid> (3, 12);
        action (row, "Previous", [this] { proc.showStep (proc.getCurrentStep()-1); }).setEnabled (proc.getCurrentStep() > 0);
        action (row, "Next step", [this] { proc.showStep (proc.getCurrentStep()+1); }, Tone::primary).setEnabled (proc.getCurrentStep()+1 < proc.getNumSteps());
        action (row, "Hide overlay", [this] { proc.hideOverlay(); });
    }
    if (proc.getMidiClip().has_value()) { midi.setClip (proc.getMidiClip()); midi.setSize (100, 120); body->borrow (midi); }
    editor.addAndMakeVisible (composer); composer.send.setEnabled (! proc.isWaiting());
}
juce::String DawBotEditor::Interface::transcript() const
{
    juce::String text; for (const auto& e : proc.getEntries()) text << (e.role == "you" ? "You: " : "dawbot: ") << e.text << "\n\n"; return text;
}
