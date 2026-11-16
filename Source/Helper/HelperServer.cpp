#include "HelperServer.h"
#include "ScreenCapture.h"
#include "Shared/Protocol.h"
#include <cmath>

using namespace dawbot::protocol;

// ===========================================================================
ClientConnection::ClientConnection (HelperServer& s)
    : juce::InterprocessConnection (true, magic), server (s)
{
}

ClientConnection::~ClientConnection()
{
    disconnect();
}

void ClientConnection::connectionMade()
{
    server.services.log ("DawBot plugin connected.");
}

void ClientConnection::connectionLost()
{
    server.services.log ("DawBot plugin disconnected.");

    juce::WeakReference<ClientConnection> weak (this);
    auto* srv = &server;
    juce::MessageManager::callAsync ([weak, srv]
    {
        if (auto* self = weak.get())
            srv->removeConnection (self);
    });
}

void ClientConnection::sendJson (const juce::var& message)
{
    sendMessage (encode (message));
}

void ClientConnection::messageReceived (const juce::MemoryBlock& block)
{
    const auto msg = decode (block);
    const auto t = msg["type"].toString();
    auto& overlay = server.services.overlay;

    if      (t == type::ask)         handleAsk (msg);
    else if (t == type::showStep)    overlay.showStep ((int) msg["index"]);
    else if (t == type::hideOverlay) overlay.hide();
    else if (t == type::resetChat)   { ++generation; busy = false; history.clear(); overlay.hide(); }
}

void ClientConnection::handleAsk (const juce::var& msg)
{
    const auto id = msg["id"].toString();

    if (busy)
    {
        sendJson (makeObject ({ { "type", type::error }, { "id", id }, { "message", "I'm still working on your last question." } }));
        return;
    }

    auto& svc = server.services;
    const bool showOnScreen = (bool) msg["showOnScreen"];

    // Capture the display the user is working on (where the mouse is).
    const auto& displays = juce::Desktop::getInstance().getDisplays();
    const auto* display = displays.getDisplayForPoint (juce::Desktop::getMousePosition());
    if (display == nullptr)
        display = displays.getPrimaryDisplay();

    const auto logical  = display != nullptr ? display->totalArea : juce::Rectangle<int>();
    const auto physical = display != nullptr ? displays.logicalToPhysical (logical, display) : logical;

    if (showOnScreen)
        svc.overlay.hide();

    CoachRequest req;
    req.question         = msg["question"].toString();
    req.context          = msg["context"];
    req.installedPlugins = svc.scanner.getNames();
    req.history          = history;
    if (auto* memory = msg["history"].getArray())
    {
        req.history.clear();
        for (int i = juce::jmax (0, memory->size() - 12); i < memory->size(); ++i)
        {
            const auto role = (*memory)[i]["role"].toString();
            if (role == "user" || role == "assistant")
                req.history.add (makeObject ({ { "role", role }, { "text", (*memory)[i]["text"].toString().substring (0, 8000) } }));
        }
    }

    const auto cfg     = svc.config;   // copy for the worker thread
    const int maxEdge  = cfg.maxImageEdge;

    busy = true;
    sendJson (makeObject ({ { "type", type::status }, { "id", id },
                            { "text", showOnScreen ? "Looking at your screen..." : "Thinking..." } }));
    svc.log ("Q: " + req.question);

    juce::WeakReference<ClientConnection> weak (this);

    const auto requestGeneration = generation;
    svc.pool.addJob ([weak, requestGeneration, id, req, cfg, maxEdge, showOnScreen, logical, physical]() mutable
    {
        juce::String captureError;

        if (showOnScreen)
        {
            auto shot = dawbot::capture::captureArea (logical, physical);

            if (shot.image.isValid())
                req.jpeg = dawbot::capture::encodeJpeg (shot.image, maxEdge, req.imageWidth, req.imageHeight);
            else
                captureError = shot.error;
        }

        const auto result = AIClient::ask (cfg, req);

        juce::MessageManager::callAsync ([weak, requestGeneration, id, req, result, logical, captureError]
        {
            if (auto* self = weak.get())
                self->finishAsk (requestGeneration, id, req, result, logical, captureError);
        });
    });
}

void ClientConnection::finishAsk (juce::uint64 requestGeneration, const juce::String& id, const CoachRequest& req, const CoachResult& result,
                                  juce::Rectangle<int> screenArea, const juce::String& captureError)
{
    if (requestGeneration != generation) return;
    busy = false;
    auto& svc = server.services;

    if (! result.ok())
    {
        svc.log ("Error: " + result.error);
        sendJson (makeObject ({ { "type", type::error }, { "id", id }, { "message", result.error } }));
        return;
    }

    const auto& reply = result.reply;

    // Map the model's screenshot-pixel boxes onto the overlay window (which covers screenArea).
    std::vector<dawbot::OverlayStep> overlaySteps;
    juce::Array<juce::var> stepsOut;

    const double sx = req.imageWidth  > 0 ? screenArea.getWidth()  / (double) req.imageWidth  : 0.0;
    const double sy = req.imageHeight > 0 ? screenArea.getHeight() / (double) req.imageHeight : 0.0;

    if (auto* steps = reply["steps"].getArray())
    {
        for (const auto& s : *steps)
        {
            dawbot::OverlayStep step;
            step.text = s["instruction"].toString();

            const auto t = s["target"];
            if (t.isObject() && sx > 0.0 && sy > 0.0)
            {
                juce::Rectangle<int> r ((int) std::lround ((double) t["x"] * sx),
                                        (int) std::lround ((double) t["y"] * sy),
                                        juce::jmax (8, (int) std::lround ((double) t["width"]  * sx)),
                                        juce::jmax (8, (int) std::lround ((double) t["height"] * sy)));

                r = r.getIntersection (screenArea.withZeroOrigin());
                if (! r.isEmpty())
                    step.target = r;
            }

            stepsOut.add (makeObject ({ { "instruction", step.text }, { "hasTarget", step.target.has_value() } }));
            overlaySteps.push_back (std::move (step));
        }
    }

    svc.overlay.present (screenArea, std::move (overlaySteps), 0);

    auto answer = reply["answer"].toString();
    if (captureError.isNotEmpty())
        answer = "(I couldn't see your screen: " + captureError + ")\n\n" + answer;

    sendJson (makeObject ({
        { "type",         type::reply },
        { "id",           id },
        { "answer",       answer },
        { "steps",        stepsOut },
        { "midi",         reply["midi"] },
        { "plugin_chain", reply["plugin_chain"] } }));

    // Short text-only memory so follow-ups ("now make it darker") work.
    history.add (makeObject ({ { "role", "user" },      { "text", req.question } }));
    history.add (makeObject ({ { "role", "assistant" }, { "text", juce::JSON::toString (reply, true) } }));
    while (history.size() > 12)
        history.remove (0);

    svc.log ("A: " + reply["answer"].toString());
}

// ===========================================================================
HelperServer::~HelperServer()
{
    stop();
    const juce::ScopedLock sl (lock);
    connections.clear();
}

bool HelperServer::start()
{
    return beginWaitingForSocket (helperPort, "127.0.0.1");
}

int HelperServer::getNumConnections() const
{
    const juce::ScopedLock sl (lock);
    int n = 0;
    for (auto* c : connections)
        if (c->isConnected())
            ++n;
    return n;
}

void HelperServer::removeConnection (ClientConnection* c)
{
    const juce::ScopedLock sl (lock);
    connections.removeObject (c);
}

juce::InterprocessConnection* HelperServer::createConnectionObject()
{
    // Called on the server's socket thread.
    auto* c = new ClientConnection (*this);
    const juce::ScopedLock sl (lock);
    connections.add (c);
    return c;
}
