#include "EditorInterface.h"
#include <DawBotAssets.h>
using namespace dawbot::ui;

juce::var DawBotEditor::Interface::selectedProject() const
{
    const auto state = proc.getWorkspace().snapshot();
    if (auto* projects = state["projects"].getArray())
        for (const auto& project : *projects) if (project["id"].toString() == selectedId) return project;
    return {};
}
void DawBotEditor::Interface::showProjects()
{
    heading ("Your Projects", "Keep your conversations and production notes together.");
    action (*body, "New project", [this] { pendingSave = false; navigate (DawBotScreen::newProject); }, Tone::primary);
    search (juce::String::fromUTF8("Search projects…"));
    const auto state = proc.getWorkspace().snapshot();
    int shown = 0;
    if (auto* projects = state["projects"].getArray())
        for (const auto& p : *projects)
        {
            const auto name = p["name"].toString(), id = p["id"].toString();
            if (! name.containsIgnoreCase (filter)) continue;
            ++shown;
            body->add<Card> (name, juce::String (p["items"].size()) + juce::String::fromUTF8(" saved answers · Saved in this DAW session"), "folder",
                [this, id] { selectedId = id; navigate (DawBotScreen::project); }, Card::Kind::row);
        }
    if (shown == 0) notice (filter.isEmpty() ? "Your first project starts here." : "No matching projects", filter.isEmpty() ? "Create a project, then save a useful conversation or guide to it." : "Try a shorter project name or clear your search.");
    notice ("A home for the process", "Projects are saved with this plugin instance in your DAW session. Keep your audio and DAW files in their usual location.");
}
void DawBotEditor::Interface::showProject()
{
    const auto p = selectedProject();
    if (! p.isObject()) { heading ("Project unavailable"); action (*body, "Back to projects", [this] { navigate (DawBotScreen::projects); }); return; }
    heading (p["name"].toString(), "Your saved answers and production notes");
    auto& controls = body->add<Grid> (3, 12);
    action (controls, "Ask about this project", [this, name = p["name"].toString()] { queueQuestion ("Help me develop an idea for my project, " + name + ". "); }, Tone::primary);
    action (controls, "Rename project", [this] { navigate (DawBotScreen::renameProject); });
    action (controls, "Delete project", [this] { navigate (DawBotScreen::deleteProject); }, Tone::ghost);
    if (auto* items = p["items"].getArray())
    {
        if (items->isEmpty()) notice ("Keep the good ideas", "Use Save to project from a conversation or guide to collect it here.");
        for (const auto& item : *items)
        {
            const auto title = item["title"].toString(), text = item["text"].toString();
            body->add<Card> (title, juce::String::fromUTF8("Saved answer · Read"), "book", [this, title, text] { savedTitle = title; savedText = text; navigate (DawBotScreen::saved); }, Card::Kind::row);
        }
    }
}
void DawBotEditor::Interface::showProjectForm (bool rename)
{
    heading (rename ? "Rename project" : "Make space for your next idea", "Give this collection a name you can return to.");
    auto& name = field (*body, "Project name", rename ? selectedProject()["name"].toString() : "");
    name.setTextToShowWhenEmpty ("e.g. Night Drive", muted);
    auto& buttons = body->add<Grid> (2, 12);
    action (buttons, rename ? "Save name" : "Create project", [this, ptr = &name, rename]
    {
        if (ptr->getText().trim().isEmpty()) { ptr->setTextToShowWhenEmpty ("Enter a project name", error); ptr->grabKeyboardFocus(); return; }
        if (rename && ! proc.getWorkspace().renameProject (selectedId, ptr->getText()))
        { message = "The name could not be saved. Your saved-project storage may be full."; triggerAsyncUpdate(); return; }
        if (! rename)
        {
            selectedId = proc.getWorkspace().createProject (ptr->getText());
            if (selectedId.isEmpty()) { message = "This instance has reached its project count or storage limit."; triggerAsyncUpdate(); return; }
            if (pendingSave && ! proc.getWorkspace().saveToProject (selectedId, pendingTitle, pendingText))
            { changed(); navigate (DawBotScreen::project); message = "Project created, but the answer could not be saved. Your saved projects have reached their storage limit."; return; }
        }
        pendingSave = false; changed(); navigate (DawBotScreen::project);
    }, Tone::primary);
    action (buttons, "Cancel", [this, rename] { navigate (rename ? DawBotScreen::project : DawBotScreen::projects); }, Tone::ghost);
}
void DawBotEditor::Interface::startSave (juce::String title, juce::String text)
{
    if (text.length() > 20000)
    { message = "This conversation is too long to save as one answer (20,000 characters). Use Copy conversation to keep the full text."; triggerAsyncUpdate(); return; }
    pendingTitle = std::move (title); pendingText = std::move (text); pendingSave = true; navigate (DawBotScreen::saveProject);
}
void DawBotEditor::Interface::showSaveProject()
{
    heading ("Save to a project", "Choose where to keep this answer.");
    action (*body, "Create a new project", [this] { navigate (DawBotScreen::newProject); }, Tone::primary);
    const auto state = proc.getWorkspace().snapshot();
    if (auto* projects = state["projects"].getArray())
        for (const auto& p : *projects)
        {
            const auto id = p["id"].toString();
            body->add<Card> (p["name"].toString(), "Save here", "folder", [this, id]
            {
                if (proc.getWorkspace().saveToProject (id, pendingTitle, pendingText))
                { selectedId = id; pendingSave = false; changed(); navigate (DawBotScreen::project); }
                else { message = "This project or your saved-project storage is full. Delete an unneeded project to free space."; triggerAsyncUpdate(); }
            }, Card::Kind::row);
        }
}
void DawBotEditor::Interface::showHistory()
{
    heading ("Conversation history", "Revisit the ideas you have explored in this plugin instance."); search (juce::String::fromUTF8("Search conversations…"));
    const auto state = proc.getWorkspace().snapshot(); int count = 0;
    if (auto* historyItems = state["history"].getArray())
        for (const auto& h : *historyItems)
        {
            const auto title = h["title"].toString(); if (! title.containsIgnoreCase (filter)) continue;
            juce::String text;
            if (auto* entries = h["entries"].getArray()) for (const auto& e : *entries) text << (e["role"].toString() == "you" ? "You: " : "dawbot: ") << e["text"].toString() << "\n\n";
            ++count; body->add<Card> (title, juce::String::fromUTF8("Saved conversation · Read"), "history", [this, title, text] { savedTitle = title; savedText = text; navigate (DawBotScreen::saved); }, Card::Kind::row);
        }
    if (count == 0) notice ("No conversations to show", "Ask a question or adjust Save conversation history in Privacy & data.");
}
void DawBotEditor::Interface::showLearning()
{
    const bool tutorial = screen == DawBotScreen::tutorials, howto = screen == DawBotScreen::howto;
    heading (tutorial ? "Learn by making" : howto ? "Get to the next step" : "Understand the technique",
             tutorial ? "Follow a focused lesson, one small step at a time." : howto ? "Quick answers for the task in front of you." : "Build knowledge you can bring into every session.");
    search (tutorial ? juce::String::fromUTF8("Find a tutorial…") : howto ? juce::String::fromUTF8("Search tasks and shortcuts…") : juce::String::fromUTF8("Search guides, sounds and production techniques…"));
    if (tutorial)
    {
        const int progress = (int) proc.getWorkspace().snapshot()["lessonProgress"];
        notice (progress == 5 ? "Completed: Your first vocal stutter" : "Your first vocal stutter", juce::String (progress) + " of 5 steps complete");
        if (juce::String ("Your first vocal stutter").containsIgnoreCase (filter))
            body->add<Card> ("Your first vocal stutter", "Take a short reference through slicing, repetition and finishing touches.", "play", [this, progress] { lessonStep = juce::jmin (4, progress); navigate (DawBotScreen::lesson); });
        else notice ("No matching tutorial", "Try vocal or stutter, or ask dawbot for a walkthrough.");
        return;
    }
    if (howto)
    {
        const juce::StringArray titles { "Create filter automation", "Find a keyboard shortcut", "Add a plugin to a track", "Export an audio snippet" };
        const juce::StringArray icons { "wave", "command", "keys", "wave" };
        int count = 0;
        for (int i = 0; i < titles.size(); ++i) if (titles[i].containsIgnoreCase (filter))
        {
            ++count; body->add<Card> (titles[i], "Ask for a walkthrough tailored to your DAW", icons[i], [this, task = titles[i]] { queueQuestion ("Show me how to " + task.toLowerCase() + " in " + proc.getHostName() + "."); }, Card::Kind::row);
        }
        if (count == 0) notice ("No matching task", "Describe the task in chat and include your DAW version.");
        notice ("Answers tailored to your setup", "Current context: " + proc.getHostName() + ". Change this in Settings."); return;
    }
    auto& filters = body->add<Grid> (4, 8);
    for (const juce::String name : { "All guides", "Sound design", "Vocals", "Mixing" })
        action (filters, name, [this, name] { category = name == "All guides" ? "" : name; triggerAsyncUpdate(); }, (category == name || (category.isEmpty() && name == "All guides")) ? Tone::selected : Tone::ghost);
    auto& cards = body->add<Grid> (2, 20); int count = 0;
    for (int i = 0; i < (int) guides().size(); ++i)
    {
        const auto& a = guides()[(size_t) i];
        if (! (a.title + " " + a.summary).containsIgnoreCase (filter) || (category.isNotEmpty() && a.category != category)) continue;
        ++count; cards.add<Card> (a.title, a.summary, a.iconName, [this, i] { articleIndex = i; navigate (DawBotScreen::article); });
    }
    if (count == 0) notice ("No matching guides", "Try a simpler phrase or choose All guides.");
}
void DawBotEditor::Interface::showArticle()
{
    const auto& a = guides()[(size_t) juce::jlimit (0, (int) guides().size()-1, articleIndex)];
    heading (a.title, juce::String::fromUTF8("Guide · ") + a.category); auto& row = body->add<Grid> (3, 12);
    action (row, "Save guide", [this, a] { startSave (a.title, a.body); }, Tone::primary);
    action (row, "Ask dawbot", [this, title = a.title] { queueQuestion ("Help me apply this technique in " + proc.getHostName() + ": " + title); });
    action (row, "Back to guides", [this] { navigate (DawBotScreen::guides); }, Tone::ghost);
    body->add<Text> (a.body, 16, muted);
    if (articleIndex == 0) action (*body, "Start the tutorial", [this] { lessonStep = 0; navigate (DawBotScreen::lesson); }, Tone::primary);
}
void DawBotEditor::Interface::showLesson()
{
    const int progress = (int) proc.getWorkspace().snapshot()["lessonProgress"];
    heading ("Your first vocal stutter", "Step " + juce::String (lessonStep+1) + juce::String::fromUTF8(" of 5 · Practice in your DAW"));
    auto& layout = body->add<Grid> (2, 28);
    auto& chapters = layout.add<Stack> (16, 24, true);
    chapters.add<Text> ("Your progress", 21, foreground, true); chapters.add<Text> (juce::String (progress) + " of 5 steps complete", 13, muted);
    const auto titles = lessonTitles();
    for (int i = 0; i < titles.size(); ++i)
        action (chapters, juce::String (i+1) + "  " + titles[i], [this, i] { lessonStep = i; triggerAsyncUpdate(); }, lessonStep == i ? Tone::selected : Tone::ghost).setEnabled (i <= progress);
    auto& current = layout.add<Stack> (20);
    current.add<Text> (titles[lessonStep], 28, foreground, true); current.add<Text> (lessonBodies()[lessonStep], 16, muted);
    action (current, lessonStep == 4 ? "Finish tutorial" : "Mark step complete", [this, progress]
    {
        proc.getWorkspace().setLessonProgress (juce::jmax (progress, lessonStep+1)); changed();
        if (lessonStep < 4) ++lessonStep; else navigate (DawBotScreen::tutorials);
        triggerAsyncUpdate();
    }, Tone::primary);
    action (current, "Ask about this step", [this, title = titles[lessonStep]] { queueQuestion ("Help me with this vocal stutter step in " + proc.getHostName() + ": " + title); });
}
void DawBotEditor::Interface::showSettings()
{
    const bool tools = screen == DawBotScreen::tools, privacy = screen == DawBotScreen::privacy;
    heading (tools ? "My tools" : privacy ? "Privacy & data" : "Settings", tools ? "Help dawbot recommend the tools that fit your setup." : privacy ? "Control what you share and what you keep." : "Make dawbot fit the way you work.");
    auto& tabs = body->add<Grid> (4, 12);
    for (const auto& [label, dest] : std::initializer_list<std::pair<const char*, DawBotScreen>> { { "General", DawBotScreen::settings }, { "My tools", DawBotScreen::tools }, { "Privacy & data", DawBotScreen::privacy }, { "Help", DawBotScreen::help } })
        action (tabs, label, [this, dest] { navigate (dest); }, screen == dest ? Tone::selected : Tone::ghost);
    const auto p = proc.getWorkspace().snapshot()["preferences"];
    auto toggle = [this, &p] (const char* label, const char* description, const char* key)
    {
        auto& row = body->add<Stack> (4, 20, true); row.add<Switch> (label, (bool) p[key], [this, key] (bool on) { setPreference (key, on); }); row.add<Text> (description, 13, muted);
    };
    if (tools)
    {
        toggle ("Prefer stock plugins", "Start with instruments and effects included in my DAW.", "preferStock");
        toggle ("Include paid alternatives", "Show paid options when they offer a useful difference.", "includePaid");
        auto& row = body->add<Grid> (2, 24);
        auto& budget = field (row, "Typical plugin budget", p["budget"].toString()); budget.setTextToShowWhenEmpty ("Free and stock first", muted);
        auto& experience = field (row, "Experience level", p["experience"].toString()); experience.setTextToShowWhenEmpty ("Getting comfortable", muted);
        auto& plugins = field (*body, "My plugin list", p["plugins"].toString(), true); plugins.setTextToShowWhenEmpty (juce::String::fromUTF8("Add the instruments and effects you own, one per line…"), muted);
        action (*body, "Save preferences", [this, b = &budget, e = &experience, pl = &plugins]
        { setPreference ("budget", b->getText()); setPreference ("experience", e->getText()); setPreference ("plugins", pl->getText()); message = "Your tools and recommendation preferences are saved."; triggerAsyncUpdate(); }, Tone::primary);
    }
    else if (privacy)
    {
        notice ("You choose what to share", "With screen guidance on, the helper sends a screenshot with your question. The helper also provides installed plugin names. Local reference files are not uploaded by this UI.");
        toggle ("Save conversation history", "Keep conversations in the plugin state saved by your DAW. Turning this off clears the history list and excludes unsaved chats from future saves.", "saveHistory");
        action (*body, "Review saved conversations", [this] { navigate (DawBotScreen::history); });
        action (*body, "Clear all history", [this] { navigate (DawBotScreen::clearHistory); });
        notice ("Saved project answers", "Answers you explicitly save to projects remain there until you delete the project. Previously saved DAW files and backups are unchanged.");
    }
    else
    {
        auto& row = body->add<Grid> (2, 24);
        auto& daw = field (row, "Default DAW and version", p["daw"].toString()); daw.setTextToShowWhenEmpty ("Automatic host detection", muted);
        auto& os = field (row, "Operating system", p["os"].toString()); os.setTextToShowWhenEmpty (juce::SystemStats::getOperatingSystemName(), muted);
        toggle ("Show me on screen", "Allow the helper to capture your screen for visual guidance when you send a question.", "screenGuidance");
        auto& status = body->add<Stack> (6, 20, true);
        helperStatus = &status.add<Text> (proc.isHelperConnected() ? "Helper connected" : "Helper offline", 16, foreground, true);
        helperDetails = &status.add<Text> (proc.getLastError().isNotEmpty() ? proc.getLastError() : "AI answers, screen guidance and MIDI generation run in the DawBot Helper app.", 13, muted);
        action (*body, "Launch Helper", [this] { proc.launchHelper(); });
        action (*body, "Save preferences", [this, d = &daw, o = &os]
        { setPreference ("daw", d->getText()); setPreference ("os", o->getText()); message = "Your DAW context is saved."; triggerAsyncUpdate(); }, Tone::primary);
    }
}
void DawBotEditor::Interface::showHelp()
{
    heading ("A little help, right here", "Get back to making music.");
    notice ("Start a conversation", "Choose your DAW in Settings, open DawBot Helper, then ask a question. Command/Ctrl + Enter sends; Enter adds a new line.");
    notice ("Bring a reference", "Use + or drop a supported file into the composer. This first UI increment keeps references local; audio and file analysis still need backend integration.");
    notice ("Keep an idea", "Save answers to a project, then save your DAW session to persist the workspace. Generated MIDI clips can still be dragged directly onto a MIDI track.");
    notice ("Type & credits", "Inter by the Inter Project Authors, distributed under the SIL Open Font License 1.1. The full license is included in this plugin.");
    action (*body, "Copy font license", [] { juce::SystemClipboard::copyTextToClipboard (juce::String::fromUTF8 (DawBotAssets::InterOFL_txt, DawBotAssets::InterOFL_txtSize)); });
    auto& row = body->add<Grid> (2, 12);
    action (row, "Open settings", [this] { navigate (DawBotScreen::settings); });
    action (row, "Back to chat", [this] { navigate (proc.getEntries().isEmpty() ? DawBotScreen::home : DawBotScreen::chat); }, Tone::primary);
}
