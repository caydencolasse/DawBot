#include "Config.h"
#include "Shared/Protocol.h"

using dawbot::protocol::makeObject;

juce::String HelperConfig::defaultModelFor (const juce::String& provider)
{
    // Change these as newer models ship; both are editable in the Helper window.
    return provider == "openai" ? "gpt-5" : "claude-sonnet-5-5";
}

juce::String HelperConfig::effectiveBaseUrl() const
{
    if (baseUrl.isNotEmpty())
        return baseUrl.trimCharactersAtEnd ("/");

    return provider == "openai" ? "https://api.openai.com" : "https://api.anthropic.com";
}

juce::File HelperConfig::file()
{
   #if JUCE_MAC
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Application Support");
   #else
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #endif
    return base.getChildFile ("DawBot").getChildFile ("config.json");
}

HelperConfig HelperConfig::load()
{
    HelperConfig c;
    const auto json = juce::JSON::parse (file());

    if (json.isObject())
    {
        c.provider     = json.getProperty ("provider", c.provider).toString();
        c.model        = json.getProperty ("model", defaultModelFor (c.provider)).toString();
        c.apiKey       = json.getProperty ("apiKey", "").toString();
        c.baseUrl      = json.getProperty ("baseUrl", "").toString();
        c.authToken    = json.getProperty ("authToken", "").toString();
        c.maxImageEdge = juce::jlimit (640, 2048, (int) json.getProperty ("maxImageEdge", c.maxImageEdge));
    }

    // Environment variables are handy during development.
    if (c.apiKey.isEmpty())
        c.apiKey = juce::SystemStats::getEnvironmentVariable (c.provider == "openai" ? "OPENAI_API_KEY" : "ANTHROPIC_API_KEY", {});

    return c;
}

void HelperConfig::save() const
{
    // NOTE: stored in plain text for Phase 1 dev builds. For release, keep keys server-side
    // (proxy mode) and store only the DawBot token, ideally in the OS keychain.
    auto f = file();
    f.getParentDirectory().createDirectory();

    const auto json = makeObject ({
        { "provider",     provider },
        { "model",        model },
        { "apiKey",       apiKey },
        { "baseUrl",      baseUrl },
        { "authToken",    authToken },
        { "maxImageEdge", maxImageEdge }
    });

    f.replaceWithText (juce::JSON::toString (json));
}
