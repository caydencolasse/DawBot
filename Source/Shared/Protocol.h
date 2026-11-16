#pragma once

#include <juce_core/juce_core.h>
#include <initializer_list>
#include <utility>

// Messages between the DawBot plugin and the DawBot Helper app.
// Transport: juce::InterprocessConnection on a localhost-only socket, JSON payloads.
//
// plugin -> helper
//   { type:"ask", id, question, showOnScreen, context:{ host, pluginFormat, os, bpm, timeSignature, isPlaying } }
//   { type:"show_step", index }   { type:"hide_overlay" }   { type:"reset_chat" }
//
// helper -> plugin
//   { type:"status", id, text }
//   { type:"reply",  id, answer, steps:[{instruction, hasTarget}], midi:{name, notes:[...]}|null, plugin_chain:[...] }
//   { type:"error",  id, message }

namespace dawbot::protocol
{
    constexpr int          helperPort = 52817;
    constexpr juce::uint32 magic      = 0x44574254; // "DWBT"

    namespace type
    {
        inline constexpr const char* ask         = "ask";
        inline constexpr const char* showStep    = "show_step";
        inline constexpr const char* hideOverlay = "hide_overlay";
        inline constexpr const char* resetChat   = "reset_chat";
        inline constexpr const char* status      = "status";
        inline constexpr const char* reply       = "reply";
        inline constexpr const char* error       = "error";
    }

    inline juce::var makeObject (std::initializer_list<std::pair<const char*, juce::var>> props)
    {
        auto* o = new juce::DynamicObject();
        for (const auto& p : props)
            o->setProperty (p.first, p.second);
        return juce::var (o);
    }

    inline juce::MemoryBlock encode (const juce::var& v)
    {
        const auto s = juce::JSON::toString (v, true);
        return juce::MemoryBlock (s.toRawUTF8(), s.getNumBytesAsUTF8());
    }

    inline juce::var decode (const juce::MemoryBlock& block)
    {
        return juce::JSON::parse (block.toString());
    }
}
