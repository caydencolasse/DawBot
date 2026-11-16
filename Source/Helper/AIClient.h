#pragma once

#include <juce_core/juce_core.h>
#include "Config.h"

struct CoachRequest
{
    juce::String           question;
    juce::var              context;            // from the plugin: host, bpm, time sig...
    juce::StringArray      installedPlugins;
    juce::MemoryBlock      jpeg;               // empty = no screenshot
    int                    imageWidth  = 0;
    int                    imageHeight = 0;
    juce::Array<juce::var> history;            // [{role:"user"|"assistant", text}]
};

struct CoachResult
{
    juce::var    reply;   // object matching coach::responseSchema()
    juce::String error;
    bool ok() const { return error.isEmpty() && reply.isObject(); }
};

// Blocking: call from a background thread.
namespace AIClient
{
    CoachResult ask (const HelperConfig&, const CoachRequest&);
}
