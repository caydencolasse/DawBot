#pragma once

#include <juce_core/juce_core.h>

// Helper settings, stored as JSON in the user's app-data folder.
//
// Dev mode:        provider + apiKey (your own Anthropic / OpenAI key).
// Production mode: baseUrl = https://api.dawbot.ai and authToken = the user's DawBot
//                  session token. The proxy (see /server) holds the real API keys and
//                  meters credits, so no provider keys ever ship inside the app.
struct HelperConfig
{
    juce::String provider = "anthropic";             // "anthropic" | "openai"
    juce::String model    = defaultModelFor ("anthropic");
    juce::String apiKey;
    juce::String baseUrl;                            // empty = provider default
    juce::String authToken;                          // DawBot account token (proxy mode)
    int          maxImageEdge = 1280;                // screenshot long edge sent to the model

    juce::String effectiveBaseUrl() const;

    static juce::String defaultModelFor (const juce::String& provider);
    static juce::File   file();
    static HelperConfig load();
    void save() const;
};
