#include "AIClient.h"
#include "CoachPrompt.h"
#include "Shared/Protocol.h"

using dawbot::protocol::makeObject;

namespace
{
juce::String buildUserText (const CoachRequest& r)
{
    const auto& c = r.context;
    juce::String s;

    s << "SESSION CONTEXT\n";
    s << "- DAW: " << c["host"].toString() << " (plugin format: " << c["pluginFormat"].toString() << ")\n";
    s << "- OS: " << c["os"].toString() << "\n";
    s << "- Tempo: " << juce::String ((double) c["bpm"], 1) << " BPM, time signature " << c["timeSignature"].toString()
      << ", transport " << ((bool) c["isPlaying"] ? "playing" : "stopped") << "\n";

    if (r.jpeg.getSize() > 0)
        s << "- Screenshot attached: " << r.imageWidth << "x" << r.imageHeight << " px. Target boxes must use this pixel space.\n";
    else
        s << "- No screenshot attached: every target must be null.\n";

    auto plugins = r.installedPlugins;
    if (plugins.size() > 400)
        plugins.removeRange (400, plugins.size() - 400);

    s << "- Installed plugins (" << r.installedPlugins.size() << "): "
      << (plugins.isEmpty() ? juce::String ("unknown") : plugins.joinIntoString (", ")) << "\n\n";

    s << "USER QUESTION\n" << r.question;
    return s;
}

juce::String authHeader (const HelperConfig& cfg)
{
    if (cfg.authToken.isNotEmpty())
        return "Authorization: Bearer " + cfg.authToken;

    return cfg.provider == "openai" ? "Authorization: Bearer " + cfg.apiKey
                                    : "x-api-key: " + cfg.apiKey;
}

juce::var postJson (const juce::String& url, const juce::var& body, const juce::String& headers, juce::String& error)
{
    int status = 0;

    auto options = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inPostData)
                       .withExtraHeaders (headers)
                       .withConnectionTimeoutMs (90'000)
                       .withStatusCode (&status);

    auto stream = juce::URL (url).withPOSTData (juce::JSON::toString (body, true)).createInputStream (options);

    if (stream == nullptr)
    {
        error = "Couldn't reach " + url + ". Check your internet connection.";
        return {};
    }

    const auto text = stream->readEntireStreamAsString();

    if (status < 200 || status >= 300)
    {
        const auto parsed = juce::JSON::parse (text);
        auto message = parsed["error"]["message"].toString();
        if (message.isEmpty()) message = text.substring (0, 400);
        error = "AI service returned HTTP " + juce::String (status) + ": " + message;
        return {};
    }

    return juce::JSON::parse (text);
}

// --- Anthropic Messages API (Claude) ---------------------------------------
CoachResult askAnthropic (const HelperConfig& cfg, const CoachRequest& req)
{
    juce::Array<juce::var> messages;
    for (const auto& h : req.history)
        messages.add (makeObject ({ { "role", h["role"] }, { "content", h["text"] } }));

    juce::Array<juce::var> content;
    if (req.jpeg.getSize() > 0)
        content.add (makeObject ({
            { "type", "image" },
            { "source", makeObject ({ { "type", "base64" },
                                      { "media_type", "image/jpeg" },
                                      { "data", juce::Base64::toBase64 (req.jpeg.getData(), req.jpeg.getSize()) } }) } }));

    content.add (makeObject ({ { "type", "text" }, { "text", buildUserText (req) } }));
    messages.add (makeObject ({ { "role", "user" }, { "content", content } }));

    // Forcing a tool call gives us schema-shaped JSON back.
    const auto tool = makeObject ({
        { "name", "coach_response" },
        { "description", "Return the coaching answer, on-screen steps, optional MIDI clip and plugin chain." },
        { "input_schema", dawbot::coach::responseSchema() } });

    juce::Array<juce::var> tools;
    tools.add (tool);

    const auto body = makeObject ({
        { "model", cfg.model },
        { "max_tokens", 4096 },
        { "system", dawbot::coach::systemPrompt() },
        { "tools", tools },
        { "tool_choice", makeObject ({ { "type", "tool" }, { "name", "coach_response" } }) },
        { "messages", messages } });

    const auto headers = "Content-Type: application/json\r\nanthropic-version: 2023-06-01\r\n" + authHeader (cfg);

    CoachResult result;
    const auto response = postJson (cfg.effectiveBaseUrl() + "/v1/messages", body, headers, result.error);
    if (result.error.isNotEmpty())
        return result;

    if (auto* blocks = response["content"].getArray())
        for (const auto& block : *blocks)
            if (block["type"].toString() == "tool_use")
            {
                result.reply = block["input"];
                return result;
            }

    result.error = "The model didn't return a structured answer.";
    return result;
}

// --- OpenAI Chat Completions API -------------------------------------------
CoachResult askOpenAI (const HelperConfig& cfg, const CoachRequest& req)
{
    juce::Array<juce::var> messages;
    messages.add (makeObject ({ { "role", "system" }, { "content", dawbot::coach::systemPrompt() } }));

    for (const auto& h : req.history)
        messages.add (makeObject ({ { "role", h["role"] }, { "content", h["text"] } }));

    juce::Array<juce::var> content;
    content.add (makeObject ({ { "type", "text" }, { "text", buildUserText (req) } }));

    if (req.jpeg.getSize() > 0)
        content.add (makeObject ({
            { "type", "image_url" },
            { "image_url", makeObject ({ { "url", "data:image/jpeg;base64," + juce::Base64::toBase64 (req.jpeg.getData(), req.jpeg.getSize()) },
                                         { "detail", "high" } }) } }));

    messages.add (makeObject ({ { "role", "user" }, { "content", content } }));

    const auto body = makeObject ({
        { "model", cfg.model },
        { "messages", messages },
        { "max_completion_tokens", 4096 },
        { "response_format", makeObject ({
              { "type", "json_schema" },
              { "json_schema", makeObject ({ { "name", "coach_response" },
                                             { "strict", true },
                                             { "schema", dawbot::coach::responseSchema() } }) } }) } });

    const auto headers = "Content-Type: application/json\r\n" + authHeader (cfg);

    CoachResult result;
    const auto response = postJson (cfg.effectiveBaseUrl() + "/v1/chat/completions", body, headers, result.error);
    if (result.error.isNotEmpty())
        return result;

    const auto message = response["choices"][0]["message"];
    const auto refusal = message["refusal"].toString();

    if (refusal.isNotEmpty())
    {
        result.error = refusal;
        return result;
    }

    result.reply = juce::JSON::parse (message["content"].toString());
    if (! result.reply.isObject())
        result.error = "The model didn't return a structured answer.";

    return result;
}
}

CoachResult AIClient::ask (const HelperConfig& cfg, const CoachRequest& req)
{
    if (cfg.apiKey.isEmpty() && cfg.authToken.isEmpty())
        return { {}, "No API key set. Open DawBot Helper and add your Anthropic or OpenAI key." };

    return cfg.provider == "openai" ? askOpenAI (cfg, req) : askAnthropic (cfg, req);
}
