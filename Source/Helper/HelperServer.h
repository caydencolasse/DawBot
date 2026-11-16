#pragma once

#include <juce_events/juce_events.h>
#include "AIClient.h"
#include "Config.h"
#include "Overlay.h"
#include "PluginScanner.h"
#include <functional>

struct HelperServices
{
    HelperConfig&     config;
    dawbot::Overlay&  overlay;
    PluginScanner&    scanner;
    juce::ThreadPool& pool;
    std::function<void (const juce::String&)> log;
};

class HelperServer;

// One per connected DawBot plugin instance.
class ClientConnection : public juce::InterprocessConnection
{
public:
    explicit ClientConnection (HelperServer&);
    ~ClientConnection() override;

    void connectionMade() override;
    void connectionLost() override;
    void messageReceived (const juce::MemoryBlock&) override;

private:
    void handleAsk (const juce::var& message);
    void finishAsk (const juce::String& id, const CoachRequest&, const CoachResult&,
                    juce::Rectangle<int> screenArea, const juce::String& captureError);
    void sendJson (const juce::var& message);

    HelperServer& server;
    juce::Array<juce::var> history;
    bool busy = false;

    JUCE_DECLARE_WEAK_REFERENCEABLE (ClientConnection)
};

// Listens on 127.0.0.1 only, so nothing outside this computer can connect.
class HelperServer : public juce::InterprocessConnectionServer
{
public:
    explicit HelperServer (HelperServices& s) : services (s) {}
    ~HelperServer() override;

    bool start();
    int  getNumConnections() const;
    void removeConnection (ClientConnection*);

    HelperServices& services;

protected:
    juce::InterprocessConnection* createConnectionObject() override;

private:
    mutable juce::CriticalSection lock;
    juce::OwnedArray<ClientConnection> connections;
};
