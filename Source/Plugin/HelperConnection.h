#pragma once

#include <juce_events/juce_events.h>
#include "Shared/Protocol.h"
#include <functional>

// Plugin-side link to the DawBot Helper app. Reconnects automatically.
class HelperConnection : public juce::InterprocessConnection,
                         private juce::Timer
{
public:
    HelperConnection() : juce::InterprocessConnection (true, dawbot::protocol::magic) {}

    ~HelperConnection() override
    {
        onMessage = nullptr;
        onConnectionChanged = nullptr;
        stopTimer();
        disconnect();
    }

    // Call from the message thread (the editor does this when it opens).
    void start()
    {
        if (! isTimerRunning())
        {
            startTimer (1500);
            timerCallback();
        }
    }

    bool send (const juce::var& message)
    {
        return isConnected() && sendMessage (dawbot::protocol::encode (message));
    }

    static bool launchHelperApp()
    {
       #if JUCE_MAC
        juce::File app ("/Applications/DawBot Helper.app");
        if (! app.exists())
            app = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Applications/DawBot Helper.app");
       #elif JUCE_WINDOWS
        auto app = juce::File::getSpecialLocation (juce::File::globalApplicationsDirectory)
                       .getChildFile ("DawBot").getChildFile ("DawBot Helper.exe");
       #else
        juce::File app;
       #endif
        return app.exists() && app.startAsProcess();
    }

    std::function<void (const juce::var&)> onMessage;
    std::function<void (bool connected)>   onConnectionChanged;

private:
    void timerCallback() override
    {
        if (! isConnected())
            connectToSocket ("127.0.0.1", dawbot::protocol::helperPort, 250);
    }

    void connectionMade() override { if (onConnectionChanged) onConnectionChanged (true); }
    void connectionLost() override { if (onConnectionChanged) onConnectionChanged (false); }

    void messageReceived (const juce::MemoryBlock& block) override
    {
        if (onMessage)
            onMessage (dawbot::protocol::decode (block));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelperConnection)
};
