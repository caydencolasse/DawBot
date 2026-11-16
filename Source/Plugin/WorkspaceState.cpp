#include "WorkspaceState.h"
#include "Shared/Protocol.h"

namespace dawbot
{
using protocol::makeObject;
namespace
{
juce::String bounded (const juce::var& v, int max = 200)
{
    return v.isString() ? v.toString().trim().substring (0, max) : juce::String();
}
bool flag (const juce::var& v, bool fallback) { return v.isBool() ? (bool) v : fallback; }
}

juce::var WorkspaceState::cleanPreferences (const juce::var& p)
{
    return makeObject ({ { "daw", bounded (p["daw"]) }, { "os", bounded (p["os"]) },
        { "preferStock", flag (p["preferStock"], true) }, { "includePaid", flag (p["includePaid"], false) },
        { "screenGuidance", flag (p["screenGuidance"], true) }, { "saveHistory", flag (p["saveHistory"], true) },
        { "budget", bounded (p["budget"]) }, { "experience", bounded (p["experience"]) },
        { "plugins", bounded (p["plugins"], 4000) } });
}
juce::var WorkspaceState::defaults()
{
    return makeObject ({ { "version", 1 }, { "preferences", cleanPreferences ({}) },
        { "projects", juce::Array<juce::var>() }, { "history", juce::Array<juce::var>() },
        { "conversation", juce::Array<juce::var>() }, { "conversationId", "" }, { "draft", "" }, { "lessonProgress", 0 } });
}
WorkspaceState::WorkspaceState() : state (defaults()) {}
juce::var WorkspaceState::snapshot() const { const juce::ScopedLock sl (lock); return state.clone(); }
void WorkspaceState::write (juce::MemoryBlock& dest) const
{
    auto copy = snapshot();
    if (! (bool) copy["preferences"]["saveHistory"])
    {
        copy.getDynamicObject()->setProperty ("conversation", juce::Array<juce::var>());
        copy.getDynamicObject()->setProperty ("conversationId", "");
        copy.getDynamicObject()->setProperty ("history", juce::Array<juce::var>());
        copy.getDynamicObject()->setProperty ("draft", "");
    }
    dest = protocol::encode (copy);
    // Explicitly saved projects have their own 1 MiB budget. Keep the newest
    // history that fits alongside them and the current chat in the 4 MiB state.
    auto* history = copy.getDynamicObject()->getProperty ("history").getArray();
    while (dest.getSize() > 4 * 1024 * 1024 && ! history->isEmpty())
    {
        history->removeLast();
        dest = protocol::encode (copy);
    }
}
juce::Array<juce::var> WorkspaceState::cleanEntries (const juce::var& data)
{
    juce::Array<juce::var> result;
    if (auto* entries = data.getArray())
        for (int i = juce::jmax (0, entries->size() - 20); i < entries->size(); ++i)
        {
            const auto& e = (*entries)[i];
            const auto role = bounded (e["role"]);
            if (role != "you" && role != "dawbot") continue;
            result.add (makeObject ({ { "role", role }, { "text", bounded (e["text"], 8000) } }));
        }
    return result;
}
bool WorkspaceState::restore (const void* data, int size)
{
    if (data == nullptr || size <= 0 || size > 4 * 1024 * 1024) return false;
    auto parsed = juce::JSON::parse (juce::String::fromUTF8 (static_cast<const char*> (data), size));
    if (! parsed.isObject() || ! parsed["version"].isInt() || (int) parsed["version"] != 1) return false;
    auto clean = defaults();
    auto* out = clean.getDynamicObject();
    auto prefs = cleanPreferences (parsed["preferences"]);
    out->setProperty ("preferences", prefs);
    if ((bool) prefs["saveHistory"])
    {
        out->setProperty ("draft", bounded (parsed["draft"], 8000));
        out->setProperty ("conversation", cleanEntries (parsed["conversation"]));
        out->setProperty ("conversationId", bounded (parsed["conversationId"]));
        juce::Array<juce::var> history;
        if (auto* source = parsed["history"].getArray())
            for (const auto& c : *source)
            {
                if (! c.isObject()) continue;
                history.add (makeObject ({ { "id", bounded (c["id"]) }, { "title", bounded (c["title"]) },
                    { "entries", cleanEntries (c["entries"]) } }));
                if (history.size() == 20) break;
            }
        out->setProperty ("history", history);
    }
    juce::Array<juce::var> projects;
    juce::StringArray ids;
    if (auto* source = parsed["projects"].getArray())
        for (const auto& p : *source)
        {
            const auto id = bounded (p["id"]), name = bounded (p["name"], 80);
            if (id.isEmpty() || name.isEmpty() || ids.contains (id)) continue;
            ids.add (id);
            juce::Array<juce::var> items;
            if (auto* rows = p["items"].getArray())
                for (const auto& row : *rows)
                {
                    if (! row.isObject()) continue;
                    items.add (makeObject ({ { "title", bounded (row["title"]) }, { "text", bounded (row["text"], 20000) } }));
                    if (items.size() == 20) break;
                }
            projects.add (makeObject ({ { "id", id }, { "name", name }, { "items", items } }));
            if (projects.size() == 20) break;
        }
    if (protocol::encode (projects).getSize() > 1024 * 1024) return false;
    out->setProperty ("projects", projects);
    out->setProperty ("lessonProgress", juce::jlimit (0, 5, (int) parsed["lessonProgress"]));
    const juce::ScopedLock sl (lock);
    state = clean;
    return true;
}
void WorkspaceState::setPreferences (const juce::var& p)
{
    const juce::ScopedLock sl (lock);
    state.getDynamicObject()->setProperty ("preferences", cleanPreferences (p));
    if (! (bool) state["preferences"]["saveHistory"])
        state.getDynamicObject()->setProperty ("history", juce::Array<juce::var>());
}
void WorkspaceState::setDraft (const juce::String& draft)
{
    const juce::ScopedLock sl (lock); state.getDynamicObject()->setProperty ("draft", draft.substring (0, 8000));
}
void WorkspaceState::setConversation (const juce::var& entries, const juce::String& id)
{
    const juce::ScopedLock sl (lock);
    auto clean = cleanEntries (entries);
    state.getDynamicObject()->setProperty ("conversation", clean);
    state.getDynamicObject()->setProperty ("conversationId", bounded (id));
    if (clean.isEmpty() || ! (bool) state["preferences"]["saveHistory"]) return;
    auto* history = state.getDynamicObject()->getProperty ("history").getArray();
    for (int i = history->size(); --i >= 0;)
        if ((*history)[i]["id"].toString() == id) history->remove (i);
    history->insert (0, makeObject ({ { "id", id }, { "title", clean[0]["text"].toString().substring (0, 100) }, { "entries", clean } }));
    while (history->size() > 20) history->removeLast();
}
void WorkspaceState::clearHistory()
{
    const juce::ScopedLock sl (lock);
    state.getDynamicObject()->setProperty ("history", juce::Array<juce::var>());
    state.getDynamicObject()->setProperty ("conversation", juce::Array<juce::var>());
    state.getDynamicObject()->setProperty ("conversationId", "");
    state.getDynamicObject()->setProperty ("draft", "");
}
juce::String WorkspaceState::createProject (const juce::String& rawName)
{
    const auto name = rawName.trim().substring (0, 80);
    if (name.isEmpty()) return {};
    const juce::ScopedLock sl (lock);
    auto* projects = state.getDynamicObject()->getProperty ("projects").getArray();
    if (projects->size() >= 20) return {};
    const auto id = juce::Uuid().toString();
    projects->insert (0, makeObject ({ { "id", id }, { "name", name }, { "items", juce::Array<juce::var>() } }));
    if (protocol::encode (*projects).getSize() > 1024 * 1024) { projects->remove (0); return {}; }
    return id;
}
bool WorkspaceState::renameProject (const juce::String& id, const juce::String& rawName)
{
    const auto name = rawName.trim().substring (0, 80);
    if (name.isEmpty()) return false;
    const juce::ScopedLock sl (lock);
    for (auto& p : *state.getDynamicObject()->getProperty ("projects").getArray())
        if (p["id"].toString() == id)
        {
            const auto previous = p["name"];
            p.getDynamicObject()->setProperty ("name", name);
            if (protocol::encode (state["projects"]).getSize() > 1024 * 1024)
            { p.getDynamicObject()->setProperty ("name", previous); return false; }
            return true;
        }
    return false;
}
bool WorkspaceState::deleteProject (const juce::String& id)
{
    const juce::ScopedLock sl (lock);
    auto* projects = state.getDynamicObject()->getProperty ("projects").getArray();
    for (int i = 0; i < projects->size(); ++i)
        if ((*projects)[i]["id"].toString() == id) { projects->remove (i); return true; }
    return false;
}
bool WorkspaceState::saveToProject (const juce::String& id, const juce::String& title, const juce::String& text)
{
    if (text.trim().isEmpty() || text.length() > 20000) return false;
    const juce::ScopedLock sl (lock);
    for (auto& p : *state.getDynamicObject()->getProperty ("projects").getArray())
        if (p["id"].toString() == id)
        {
            auto* items = p.getDynamicObject()->getProperty ("items").getArray();
            if (items->size() >= 20) return false;
            items->insert (0, makeObject ({ { "title", title.substring (0, 200) }, { "text", text } }));
            if (protocol::encode (state["projects"]).getSize() > 1024 * 1024)
            { items->remove (0); return false; }
            return true;
        }
    return false;
}
void WorkspaceState::setLessonProgress (int progress)
{
    const juce::ScopedLock sl (lock); state.getDynamicObject()->setProperty ("lessonProgress", juce::jlimit (0, 5, progress));
}
}
