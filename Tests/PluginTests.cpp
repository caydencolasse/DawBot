#include "Plugin/PluginProcessor.h"
#include "Plugin/PluginEditor.h"
#include "Plugin/UI/Composer.h"
#include <iostream>

namespace
{
int failures = 0, checks = 0;
void expect (bool condition, const char* message)
{
    ++checks;
    if (! condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
void pump (int ms = 40) { juce::MessageManager::getInstance()->runDispatchLoopUntil (ms); }
juce::Component* named (juce::Component& root, const juce::String& name)
{
    if (root.getName() == name && (dynamic_cast<juce::Button*> (&root) != nullptr || dynamic_cast<juce::TextEditor*> (&root) != nullptr)) return &root;
    for (auto* child : root.getChildren())
        if (child->isVisible()) if (auto* found = named (*child, name)) return found;
    return nullptr;
}
void click (juce::Component& root, const juce::String& name)
{
    auto* button = dynamic_cast<juce::Button*> (named (root, name));
    expect (button != nullptr && button->isEnabled(), ("Button available: " + name).toRawUTF8());
    if (button != nullptr && button->isEnabled()) button->triggerClick();
    pump();
}
void type (juce::Component& root, const juce::String& name, const juce::String& text)
{
    auto* field = dynamic_cast<juce::TextEditor*> (named (root, name));
    expect (field != nullptr, ("Field available: " + name).toRawUTF8());
    if (field != nullptr) field->setText (text);
    pump();
}
void screenshot (juce::Component& editor, const juce::File& directory, const juce::String& name)
{
    if (directory == juce::File()) return;
    directory.createDirectory();
    auto file = directory.getChildFile (name + ".png"); file.deleteFile();
    juce::FileOutputStream output (file);
    expect (output.openedOk() && juce::PNGImageFormat().writeImageToStream (editor.createComponentSnapshot (editor.getLocalBounds()), output), "Native screenshot renders");
}
void workspaceTests()
{
    using dawbot::protocol::makeObject;
    dawbot::WorkspaceState state;
    expect (state.createProject ("  ").isEmpty(), "Empty project names are rejected");
    const auto id = state.createProject (" Night Drive ");
    expect (id.isNotEmpty() && state.snapshot()["projects"][0]["name"].toString() == "Night Drive", "Project names are trimmed");
    expect (state.renameProject (id, "After Hours"), "Existing projects can be renamed");
    expect (! state.renameProject (id, " "), "Empty rename keeps the existing project");
    expect (state.saveToProject (id, "Vocal chops", "Slice a short phrase, then vary the rhythm."), "Save an answer to a project");
    expect (! state.saveToProject (id, "Too long", juce::String::repeatedString ("a", 20001)), "Oversized saved answers are rejected, never silently truncated");
    auto clone = state.snapshot(); clone["projects"][0].getDynamicObject()->setProperty ("name", "changed");
    expect (state.snapshot()["projects"][0]["name"].toString() == "After Hours", "Snapshots cannot mutate workspace state");
    state.setDraft ("Keep this draft"); state.setLessonProgress (3);
    auto prefs = state.snapshot()["preferences"]; prefs.getDynamicObject()->setProperty ("daw", "Ableton Live 12"); state.setPreferences (prefs);
    juce::Array<juce::var> entries { makeObject ({ { "role", "you" }, { "text", "How do I slice a vocal?" } }), makeObject ({ { "role", "dawbot" }, { "text", "Start with a short phrase." } }) };
    state.setConversation (entries, "conversation-1");
    juce::MemoryBlock bytes; state.write (bytes); dawbot::WorkspaceState restored;
    expect (restored.restore (bytes.getData(), (int) bytes.getSize()), "Host state round-trips");
    expect (restored.snapshot()["projects"][0]["items"].size() == 1 && (int) restored.snapshot()["lessonProgress"] == 3, "Saved answers and tutorial progress survive reload");
    expect (restored.snapshot()["draft"].toString() == "Keep this draft" && restored.snapshot()["preferences"]["daw"].toString() == "Ableton Live 12", "Draft and DAW preferences survive reload");
    auto bad = dawbot::protocol::encode (makeObject ({ { "version", 99 } }));
    expect (! restored.restore (bad.getData(), (int) bad.getSize()), "Unsupported state versions are rejected");
    expect (! restored.restore ("no-json", 7) && restored.snapshot()["projects"].size() == 1, "Malformed state cannot overwrite current work");
    prefs.getDynamicObject()->setProperty ("saveHistory", false); state.setPreferences (prefs); state.write (bytes);
    expect (restored.restore (bytes.getData(), (int) bytes.getSize()), "Private state round-trips");
    expect (restored.snapshot()["history"].size() == 0 && restored.snapshot()["conversation"].size() == 0 && restored.snapshot()["draft"].toString().isEmpty(), "Disabled history excludes unsaved conversation and draft from host state");
    expect (restored.snapshot()["projects"].size() == 1, "Privacy preference keeps explicitly saved answers");
    state.clearHistory(); expect (state.snapshot()["projects"].size() == 1, "Clearing chat history preserves projects");
    expect (state.deleteProject (id) && ! state.deleteProject (id), "Delete removes only an existing project");

    dawbot::WorkspaceState large;
    juce::Array<juce::var> longChat;
    const auto longText = juce::String::repeatedString ("\"", 8000);
    for (int i = 0; i < 60; ++i) longChat.add (makeObject ({ { "role", i % 2 ? "dawbot" : "you" }, { "text", juce::String (i) + longText } }));
    for (int i = 0; i < 20; ++i) large.setConversation (longChat, juce::String (i));
    large.write (bytes);
    expect (bytes.getSize() <= 4 * 1024 * 1024, "Serialized history stays within the loader budget");
    expect (restored.restore (bytes.getData(), (int) bytes.getSize()), "Large workspaces remain loadable");
    const auto recent = restored.snapshot()["conversation"];
    expect (recent.size() > 0 && recent[0]["text"].toString().startsWith ("40"), "Bounded chat keeps the most recent twenty messages");

    auto nearLimit = large.snapshot();
    juce::Array<juce::var> projects;
    for (int p = 0; p < 3; ++p)
    {
        juce::Array<juce::var> items;
        for (int i = 0; i < (p == 2 ? 16 : 18); ++i)
            items.add (makeObject ({ { "title", "Note" }, { "text", juce::String::repeatedString ("x", 20000) } }));
        projects.add (makeObject ({ { "id", juce::String (p) }, { "name", "A" }, { "items", items } }));
    }
    auto filler = makeObject ({ { "title", "Padding" }, { "text", "" } });
    projects.getReference (2).getDynamicObject()->getProperty ("items").getArray()->add (filler);
    const auto padding = 1024 * 1024 - 40 - (int) dawbot::protocol::encode (projects).getSize();
    expect (padding > 0 && padding < 20000, "Storage boundary fixture fits one saved answer");
    filler.getDynamicObject()->setProperty ("text", juce::String::repeatedString ("x", padding));
    nearLimit.getDynamicObject()->setProperty ("projects", projects);
    nearLimit.getDynamicObject()->setProperty ("history", juce::Array<juce::var>());
    bytes = dawbot::protocol::encode (nearLimit);
    expect (restored.restore (bytes.getData(), (int) bytes.getSize()), "Near-capacity project storage loads");
    expect (restored.createProject (juce::String::repeatedString ("n", 80)).isEmpty(), "Creating a project cannot exceed the storage budget");
    expect (! restored.renameProject ("0", juce::String::repeatedString ("n", 80)), "Renaming cannot exceed the storage budget");
    restored.write (bytes); expect (state.restore (bytes.getData(), (int) bytes.getSize()), "Rejected project mutations leave a loadable workspace");
}
class TestPeer : public juce::InterprocessConnection
{
public:
    TestPeer() : InterprocessConnection (true, dawbot::protocol::magic) {}
    ~TestPeer() override { disconnect(); }
    juce::Array<juce::var> messages;
    void messageReceived (const juce::MemoryBlock& block) override { messages.add (dawbot::protocol::decode (block)); }
    void connectionMade() override {}
    void connectionLost() override {}
    juce::var lastAsk() const
    {
        for (int i = messages.size(); --i >= 0;) if (messages[i]["type"].toString() == "ask") return messages[i];
        return {};
    }
    void reply (const juce::var& value) { expect (sendMessage (dawbot::protocol::encode (value)), "Fake helper sends its reply"); }
};
class TestHelper : public juce::InterprocessConnectionServer
{
public:
    std::atomic<TestPeer*> peer { nullptr };
    ~TestHelper() override { stop(); delete peer.exchange (nullptr); }
    juce::InterprocessConnection* createConnectionObject() override
    {
        auto* next = new TestPeer(); peer.store (next); return next;
    }
};
template <typename Predicate> bool waitFor (Predicate predicate)
{
    for (int i = 0; i < 200; ++i) { if (predicate()) return true; pump (10); }
    return predicate();
}
void connectionTests (const juce::File& captures)
{
    using dawbot::protocol::makeObject;
    TestHelper server;
    if (! server.beginWaitingForSocket (dawbot::protocol::helperPort, "127.0.0.1"))
    { expect (false, "Test helper port must be free (close DawBot Helper before testing)"); return; }
    DawBotProcessor processor;
    processor.startHelperConnection();
    if (! waitFor ([&] { return processor.isHelperConnected() && server.peer.load() != nullptr; }))
    { expect (false, "Processor connects to the local test helper"); return; }
    auto& peer = *server.peer.load();
    auto prefs = processor.getWorkspace().snapshot()["preferences"];
    prefs.getDynamicObject()->setProperty ("daw", "Ableton Live 12"); prefs.getDynamicObject()->setProperty ("budget", "Under $100");
    processor.getWorkspace().setPreferences (prefs);
    expect (processor.ask ("Show me vocal stutter steps", false), "Connected submission is accepted");
    expect (waitFor ([&] { return peer.lastAsk().isObject(); }), "Question arrives through real localhost IPC");
    const auto first = peer.lastAsk();
    expect (first["context"]["host"].toString() == "Ableton Live 12" && first["context"]["preferences"]["budget"].toString() == "Under $100", "Helper receives DAW and tool preferences");
    peer.reply (makeObject ({ { "type", "reply" }, { "id", "wrong-id" }, { "answer", "Stale answer" } })); pump();
    expect (processor.isWaiting() && processor.getEntries().size() == 1, "Mismatched replies cannot overwrite pending chat");
    processor.resetChat();
    peer.reply (makeObject ({ { "type", "reply" }, { "id", first["id"] }, { "answer", "Old answer" } })); pump();
    expect (! processor.isWaiting() && processor.getEntries().isEmpty(), "New chat invalidates replies already in flight");
    expect (processor.ask ("How do I make this stutter?", false), "New chat can send another question");
    expect (waitFor ([&] { return peer.lastAsk()["id"] != first["id"]; }), "New request gets a new ID");
    const auto second = peer.lastAsk();
    juce::Array<juce::var> steps { makeObject ({ { "instruction", "Choose a short phrase and find a clean slice." }, { "hasTarget", false } }),
                                 makeObject ({ { "instruction", "Repeat the slice and leave space between hits." }, { "hasTarget", false } }) };
    juce::Array<juce::var> notes { makeObject ({ { "pitch", 60 }, { "start_beats", 0.0 }, { "length_beats", 0.5 }, { "velocity", 100 } }) };
    auto midi = makeObject ({ { "name", "Stutter rhythm" }, { "notes", notes }, { "bars", 1 } });
    peer.reply (makeObject ({ { "type", "reply" }, { "id", second["id"] }, { "answer", juce::String::fromUTF8 ("Web-sourced answer\nStart with a short vocal slice.\n\nWeb source: Ableton manual — https://www.ableton.com/en/manual/") }, { "steps", steps }, { "midi", midi } }));
    expect (waitFor ([&] { return ! processor.isWaiting(); }), "Matching answer finishes waiting state");
    expect (processor.getNumSteps() == 2 && processor.getMidiClip().has_value(), "Live replies retain screen steps and MIDI output");
    {
        DawBotEditor editor (processor); editor.setVisible (true); pump();
        expect (named (editor, "Open web source: Ableton manual") != nullptr, "Web citations have clickable source actions");
        screenshot (editor, captures, "chat-answer"); click (editor, "Next step");
        expect (processor.getCurrentStep() == 1, "Native step control advances the existing helper flow");
    }
    juce::MemoryBlock saved; processor.getStateInformation (saved); processor.setStateInformation (saved.getData(), (int) saved.getSize()); pump();
    expect (processor.ask ("Now make it warmer", false), "Restored chat accepts a follow-up");
    expect (waitFor ([&] { return peer.lastAsk()["id"] != second["id"]; }), "Restored follow-up reaches helper");
    const auto third = peer.lastAsk();
    expect (third["history"].size() == 2, "Restored follow-up supplies prior text to helper");
    peer.reply (makeObject ({ { "type", "reply" }, { "id", third["id"] }, { "answer", "Try softening the high end with a filter." } })); pump();
    expect (! processor.getMidiClip().has_value() && processor.getNumSteps() == 0, "A plain answer clears stale MIDI and steps");
    expect (processor.ask ("Try a delay", false), "Further question is accepted");
    expect (waitFor ([&] { return peer.lastAsk()["id"] != third["id"]; }), "Further question reaches helper");
    peer.reply (makeObject ({ { "type", "error" }, { "id", peer.lastAsk()["id"] }, { "message", "Test service unavailable" } })); pump();
    expect (! processor.isWaiting() && processor.getWorkspace().snapshot()["draft"].toString() == "Try a delay", "Helper errors restore the retry draft");
    processor.getWorkspace().setDraft ({});
    expect (processor.ask ("Try another effect", false), "Retry after helper error is possible");
    expect (waitFor ([&] { return peer.lastAsk()["question"].toString() == "Try another effect"; }), "Retry reaches helper");
    processor.getWorkspace().setDraft ("My next idea is already typed");
    peer.reply (makeObject ({ { "type", "error" }, { "id", peer.lastAsk()["id"] }, { "message", "Test error" } })); pump();
    expect (processor.getWorkspace().snapshot()["draft"].toString() == "My next idea is already typed", "Helper error preserves a newer draft");
    expect (processor.ask ("Old session question", false), "Request begins before session restore");
    expect (waitFor ([&] { return peer.lastAsk()["question"].toString() == "Old session question"; }), "Old-session request reaches helper");
    peer.reply (makeObject ({ { "type", "reply" }, { "id", peer.lastAsk()["id"] }, { "answer", "Must not enter restored workspace" } }));
    juce::Thread::sleep (40); // Let IPC queue delivery without pumping its message callback.
    dawbot::WorkspaceState incoming; incoming.setDraft ("New session draft"); juce::MemoryBlock fresh; incoming.write (fresh);
    processor.setStateInformation (fresh.getData(), (int) fresh.getSize());
    processor.getStateInformation (saved);
    expect (dawbot::protocol::decode (saved)["draft"].toString() == "New session draft", "Host reads queued state before asynchronous UI restore");
    pump();
    expect (processor.getEntries().isEmpty() && processor.getWorkspace().snapshot()["conversation"].size() == 0, "Queued old reply cannot replace a restored conversation");
}
void attachmentTests()
{
    dawbot::ui::Composer composer;
    juce::TemporaryFile reference (".wav"); reference.getFile().replaceWithText ("local reference fixture");
    composer.filesDropped ({ reference.getFile().getFullPathName() }, 0, 0);
    expect (composer.getReferenceNames() == reference.getFile().getFileName(), "Supported local references can be added");
    composer.filesDropped ({ reference.getFile().getFullPathName() }, 0, 0);
    expect (composer.getReferenceNames() == reference.getFile().getFileName(), "Duplicate references are ignored");
    juce::TemporaryFile invalid (".exe"); invalid.getFile().replaceWithText ("invalid fixture");
    composer.filesDropped ({ invalid.getFile().getFullPathName() }, 0, 0);
    expect (! composer.getReferenceNames().contains (invalid.getFile().getFileName()), "Unsupported references are rejected");
    click (composer, "Remove references"); expect (composer.getReferenceNames().isEmpty(), "References can be removed");
}
void uiTests (const juce::File& captures)
{
    DawBotProcessor processor;
    DawBotEditor editor (processor); editor.setVisible (true); pump();
    expect (named (editor, "Your Projects") && named (editor, "Guides") && named (editor, "Tutorials") && named (editor, "How-to"), "Home provides all four module entry points");
    expect (named (editor, "Prefer stock plugins") == nullptr, "Stock preference belongs only in settings");
    screenshot (editor, captures, "home-1200");
    editor.setSize (800, 760); pump(); screenshot (editor, captures, "home-800");
    expect (named (editor, "Send message") != nullptr && named (editor, "Attach files") != nullptr, "Compact composer retains its actions");
    editor.setSize (1200, 1000); pump();
    click (editor, "How do I make a Fred again.. style vocal stutter?");
    auto* input = dynamic_cast<juce::TextEditor*> (named (editor, "Message"));
    expect (input != nullptr && input->getText().contains ("vocal stutter"), "Suggested prompt populates composer");
    type (editor, "Message", "Help me shape a warmer bass"); click (editor, "Send message");
    input = dynamic_cast<juce::TextEditor*> (named (editor, "Message"));
    expect (input != nullptr && input->getText() == "Help me shape a warmer bass", "Offline submission retains draft");
    expect (processor.getEntries().isEmpty(), "Offline submission does not fabricate a sent message");
    screenshot (editor, captures, "chat-offline");
    click (editor, "Your Projects"); click (editor, "New project"); type (editor, "Project name", "Night Drive");
    editor.setSize (800, 760); pump();
    auto* nameField = dynamic_cast<juce::TextEditor*> (named (editor, "Project name"));
    expect (nameField != nullptr && nameField->getText() == "Night Drive", "Resizing preserves an unsaved project name");
    editor.setSize (1200, 1000); pump(); click (editor, "Create project");
    expect (processor.getWorkspace().snapshot()["projects"].size() == 1, "Project form creates a project");
    click (editor, "Guides"); screenshot (editor, captures, "guides");
    click (editor, "The art of vocal chops"); click (editor, "Save guide"); click (editor, "Night Drive");
    expect (processor.getWorkspace().snapshot()["projects"][0]["items"].size() == 1, "Save guide flow stores real guide text");
    screenshot (editor, captures, "project-detail");
    click (editor, "Tutorials"); click (editor, "Your first vocal stutter");
    auto* lastStep = dynamic_cast<juce::Button*> (named (editor, "5  Review your result"));
    expect (lastStep != nullptr && ! lastStep->isEnabled(), "Tutorial cannot mark unvisited later steps complete");
    click (editor, "Mark step complete");
    expect ((int) processor.getWorkspace().snapshot()["lessonProgress"] == 1, "Tutorial progress advances after completing a step");
    screenshot (editor, captures, "tutorial-step");
    click (editor, "Settings"); click (editor, "My tools");
    expect (named (editor, "Prefer stock plugins") != nullptr, "Tool settings expose stock preference");
    type (editor, "Typical plugin budget", "Under $100"); editor.setSize (800, 760); pump(); editor.setSize (1200, 1000); pump(); click (editor, "Save preferences");
    expect (processor.getWorkspace().snapshot()["preferences"]["budget"].toString() == "Under $100", "Tool preferences persist");
    screenshot (editor, captures, "settings-tools");
    dawbot::WorkspaceState settingsRestore;
    auto restoredPrefs = settingsRestore.snapshot()["preferences"]; restoredPrefs.getDynamicObject()->setProperty ("budget", "Stock only"); settingsRestore.setPreferences (restoredPrefs);
    juce::MemoryBlock settingsBytes; settingsRestore.write (settingsBytes); processor.setStateInformation (settingsBytes.getData(), (int) settingsBytes.getSize()); pump();
    auto* restoredBudget = dynamic_cast<juce::TextEditor*> (named (editor, "Typical plugin budget"));
    expect (restoredBudget != nullptr && restoredBudget->getText() == "Stock only", "Session restore refreshes open settings fields");
    click (editor, "Home");
    dawbot::WorkspaceState restored; restored.setDraft ("Restored session draft"); juce::MemoryBlock bytes; restored.write (bytes);
    processor.setStateInformation (bytes.getData(), (int) bytes.getSize()); pump();
    input = dynamic_cast<juce::TextEditor*> (named (editor, "Message"));
    expect (input != nullptr && input->getText() == "Restored session draft", "Loading a session refreshes an already open composer");
}
}
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    const auto captures = argc > 1 ? juce::File (argv[1]) : juce::File();
    workspaceTests(); uiTests (captures); attachmentTests(); connectionTests (captures);
    std::cout << (failures == 0 ? "PASS" : "FAIL") << ": " << checks << " native checks (" << failures << " failures)\n";
    return failures == 0 ? 0 : 1;
}
