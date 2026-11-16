#include "PluginScanner.h"

namespace
{
    bool isBundle (const juce::File& f)
    {
        return f.hasFileExtension ("component;vst3;vst;aaxplugin");
    }

    // Skip files nested inside another plugin bundle (e.g. Foo.vst3/Contents/...).
    bool isInsideBundle (const juce::File& f, const juce::File& root)
    {
        for (auto p = f.getParentDirectory(); p != root && p.isAChildOf (root); p = p.getParentDirectory())
            if (isBundle (p))
                return true;

        return false;
    }
}

void PluginScanner::startScan (juce::ThreadPool& pool)
{
    pool.addJob ([this]
    {
        auto found = scan();
        const juce::ScopedLock sl (lock);
        names = std::move (found);
    });
}

juce::StringArray PluginScanner::getNames() const
{
    const juce::ScopedLock sl (lock);
    return names;
}

juce::StringArray PluginScanner::scan()
{
    juce::Array<juce::File> roots;

   #if JUCE_MAC
    const auto home = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
    for (const auto& base : { juce::File ("/Library/Audio/Plug-Ins"), home.getChildFile ("Library/Audio/Plug-Ins") })
        for (const auto* sub : { "Components", "VST3", "VST" })
            roots.add (base.getChildFile (sub));
    roots.add (juce::File ("/Library/Application Support/Avid/Audio/Plug-Ins"));
   #elif JUCE_WINDOWS
    const auto programFiles = juce::File::getSpecialLocation (juce::File::globalApplicationsDirectory);
    roots.add (programFiles.getChildFile ("Common Files/VST3"));
    roots.add (programFiles.getChildFile ("Common Files/Avid/Audio/Plug-Ins"));
    roots.add (programFiles.getChildFile ("VSTPlugins"));
    roots.add (programFiles.getChildFile ("Steinberg/VSTPlugins"));
   #endif

    juce::StringArray result;

    for (const auto& root : roots)
    {
        if (! root.isDirectory())
            continue;

        for (const auto& entry : juce::RangedDirectoryIterator (root, true, "*.component;*.vst3;*.vst;*.aaxplugin;*.dll",
                                                                juce::File::findFilesAndDirectories))
        {
            const auto file = entry.getFile();
            if (! isInsideBundle (file, root))
                result.addIfNotAlreadyThere (file.getFileNameWithoutExtension(), true);
        }
    }

    result.sortNatural();
    return result;
}
