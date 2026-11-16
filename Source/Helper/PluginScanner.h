#pragma once

#include <juce_core/juce_core.h>

// Lists installed plugin names by looking at the standard plugin folders.
// It never loads the plugins (fast, can't crash), it only reads file names,
// so the coach can say "use Serum + Valhalla" instead of generic advice.
class PluginScanner
{
public:
    void startScan (juce::ThreadPool& pool);
    juce::StringArray getNames() const;

private:
    static juce::StringArray scan();

    mutable juce::CriticalSection lock;
    juce::StringArray names;
};
