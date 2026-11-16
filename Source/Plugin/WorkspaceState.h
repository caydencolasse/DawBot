#pragma once
#include <juce_core/juce_core.h>

namespace dawbot
{
// Instance-scoped UI data, saved by the DAW. Never accessed from processBlock.
// Snapshots are deep copies so host state callbacks cannot race UI mutations.
class WorkspaceState
{
public:
    WorkspaceState();
    juce::var snapshot() const;
    bool restore (const void* data, int size);
    void write (juce::MemoryBlock&) const;
    void setPreferences (const juce::var&);
    void setDraft (const juce::String&);
    void setConversation (const juce::var& entries, const juce::String& id);
    void clearHistory();
    juce::String createProject (const juce::String& name);
    bool renameProject (const juce::String& id, const juce::String& name);
    bool deleteProject (const juce::String& id);
    bool saveToProject (const juce::String& id, const juce::String& title, const juce::String& text);
    void setLessonProgress (int);

private:
    static juce::var defaults();
    static juce::var cleanPreferences (const juce::var&);
    static juce::Array<juce::var> cleanEntries (const juce::var&);
    mutable juce::CriticalSection lock;
    juce::var state;
};
}
