#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace dawbot::protocol;

DawBotProcessor::DawBotProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    helper.onMessage = [this] (const juce::var& m) { handleHelperMessage (m); };

    helper.onConnectionChanged = [this] (bool connected)
    {
        if (! connected && waiting)
        {
            waiting = false;
            statusText = {};
            pendingRequestId = {};
            lastError = "Lost the connection to DawBot Helper. Your question is ready to retry.";
            if (workspace.snapshot()["draft"].toString().isEmpty()) workspace.setDraft (lastQuestion);
        }
        sendChangeMessage();
    };
}

DawBotProcessor::~DawBotProcessor()
{
    cancelPendingUpdate();
    helper.onMessage = nullptr;
    helper.onConnectionChanged = nullptr;
}

bool DawBotProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return ! out.isDisabled() && out == layouts.getMainInputChannelSet();
}

void DawBotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Audio passes through untouched.
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (auto b = pos->getBpm())
                bpm.store (*b);

            if (auto ts = pos->getTimeSignature())
            {
                timeSigNum.store (ts->numerator);
                timeSigDen.store (ts->denominator);
            }

            isPlaying.store (pos->getIsPlaying());
        }
    }
}

juce::AudioProcessorEditor* DawBotProcessor::createEditor()
{
    return new DawBotEditor (*this);
}

// ---------------------------------------------------------------------------

juce::var DawBotProcessor::buildSessionContext() const
{
    const auto prefs = workspace.snapshot()["preferences"];

    return makeObject ({
        { "host",          getHostName() },
        { "pluginFormat",  juce::String (juce::AudioProcessor::getWrapperTypeDescription (wrapperType)) },
        { "os",            prefs["os"].toString().isEmpty() ? juce::SystemStats::getOperatingSystemName() : prefs["os"].toString() },
        { "preferences",   prefs },
        { "bpm",           bpm.load() },
        { "timeSignature", juce::String (timeSigNum.load()) + "/" + juce::String (timeSigDen.load()) },
        { "isPlaying",     isPlaying.load() }
    });
}

void DawBotProcessor::launchHelper()
{
    if (! HelperConnection::launchHelperApp())
        lastError = "I couldn't find DawBot Helper. Install it in your Applications folder, then open it.";

    sendChangeMessage();
}

bool DawBotProcessor::ask (const juce::String& rawQuestion, bool showOnScreen)
{
    const auto question = rawQuestion.trim();
    if (question.isEmpty() || waiting || isUpdatePending())
        return false;

    if (! helper.isConnected())
    {
        lastError = "DawBot Helper is offline. Launch it, then retry your question.";
        sendChangeMessage();
        return false;
    }

    pendingRequestId = juce::Uuid().toString();
    waiting = true;
    lastError = {};
    lastQuestion = question;
    statusText = showOnScreen ? "Looking at your screen..." : "Thinking...";

    // Send bounded text memory on every request so a helper reconnect or a DAW
    // session reload can continue the visible conversation without stale memory.
    juce::Array<juce::var> memory;
    for (int i = juce::jmax (0, entries.size() - 12); i < entries.size(); ++i)
        memory.add (makeObject ({ { "role", entries[i].role == "you" ? "user" : "assistant" },
                                 { "text", entries[i].text.substring (0, 8000) } }));

    if (! helper.send (makeObject ({
        { "type",         type::ask },
        { "id",           pendingRequestId },
        { "question",     question },
        { "showOnScreen", showOnScreen },
        { "context",      buildSessionContext() },
        { "history",      memory }
    })))
    {
        waiting = false;
        pendingRequestId = {};
        statusText = {};
        lastError = "The question could not be sent. Please try again.";
        sendChangeMessage();
        return false;
    }
    if (entries.isEmpty() || entries.getLast().role != "you" || entries.getLast().text != question)
        entries.add ({ "you", question });
    storeConversation();

    sendChangeMessage();
    return true;
}

void DawBotProcessor::showStep (int index)
{
    if (steps.isEmpty())
        return;

    currentStep = juce::jlimit (0, steps.size() - 1, index);
    helper.send (makeObject ({ { "type", type::showStep }, { "index", currentStep } }));
    sendChangeMessage();
}

void DawBotProcessor::hideOverlay()
{
    helper.send (makeObject ({ { "type", type::hideOverlay } }));
}

void DawBotProcessor::resetChat()
{
    waiting = false;
    pendingRequestId = {};
    statusText = {};
    lastError = {};
    conversationId = juce::Uuid().toString();
    entries.clear();
    steps.clear();
    currentStep = 0;
    midiClip.reset();
    workspace.setDraft ({});
    storeConversation();
    helper.send (makeObject ({ { "type", type::resetChat } }));
    hideOverlay();
    sendChangeMessage();
}

void DawBotProcessor::handleHelperMessage (const juce::var& msg)
{
    const auto t  = msg["type"].toString();
    const auto id = msg["id"].toString();
    // Replies from a reset/restored conversation must not overwrite the new one.
    if (pendingRequestId.isEmpty() || id != pendingRequestId) return;

    if (t == type::status)
    {
        if (id == pendingRequestId)
            statusText = msg["text"].toString();
    }
    else if (t == type::reply)
    {
        waiting = false;
        statusText = {};
        entries.add ({ "dawbot", formatReply (msg) });
        pendingRequestId = {};
        midiClip.reset();

        steps.clear();
        if (auto* s = msg["steps"].getArray())
            steps.addArray (*s);
        currentStep = 0;

        if (msg["midi"].isObject())
        {
            auto clip = dawbot::MidiClip::fromVar (msg["midi"], bpm.load(), timeSigNum.load(), timeSigDen.load());
            if (clip.isValid())
                midiClip = std::move (clip);
        }
        storeConversation();
    }
    else if (t == type::error)
    {
        waiting = false;
        statusText = {};
        pendingRequestId = {};
        lastError = msg["message"].toString();
        if (workspace.snapshot()["draft"].toString().isEmpty()) workspace.setDraft (lastQuestion);
    }

    sendChangeMessage();
}

juce::String DawBotProcessor::getHostName() const
{
    const auto selected = workspace.snapshot()["preferences"]["daw"].toString();
    if (selected.isNotEmpty()) return selected;
    const juce::PluginHostType host;
    const juce::String detected (host.getHostDescription());
    return detected.isEmpty() || detected == "Unknown" ? "Choose your DAW" : detected;
}

void DawBotProcessor::storeConversation()
{
    juce::Array<juce::var> data;
    while (entries.size() > 20) entries.remove (0);
    for (const auto& e : entries) data.add (makeObject ({ { "role", e.role }, { "text", e.text } }));
    workspace.setConversation (data, conversationId);
    updateHostDisplay (ChangeDetails().withNonParameterStateChanged (true));
}

void DawBotProcessor::clearHistory()
{
    resetChat();
    workspace.clearHistory();
    sendChangeMessage();
}

void DawBotProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    const juce::ScopedLock sl (hostStateLock);
    if (pendingHostState.getSize() > 0) dest = pendingHostState;
    else workspace.write (dest);
}
void DawBotProcessor::setStateInformation (const void* data, int size)
{
    // Hosts may restore off the message thread. Validate now, then replace the
    // workspace and invalidate requests together on the message thread. Queued
    // replies can only touch the old workspace until that atomic transition.
    dawbot::WorkspaceState incoming;
    if (! incoming.restore (data, size)) return;
    juce::MemoryBlock validated; incoming.write (validated);
    { const juce::ScopedLock sl (hostStateLock); pendingHostState = std::move (validated); }
    triggerAsyncUpdate();
}
void DawBotProcessor::handleAsyncUpdate()
{
    waiting = false;
    pendingRequestId = {}; lastError = {}; statusText = {};
    entries.clear(); steps.clear(); midiClip.reset(); currentStep = 0;
    juce::var restored;
    {
        const juce::ScopedLock sl (hostStateLock);
        if (pendingHostState.getSize() == 0) return;
        workspace.restore (pendingHostState.getData(), (int) pendingHostState.getSize());
        pendingHostState.reset();
        restored = workspace.snapshot();
    }
    ++workspaceRestoreCount;
    conversationId = restored["conversationId"].toString();
    if (conversationId.isEmpty()) conversationId = juce::Uuid().toString();
    if (auto* data = restored["conversation"].getArray())
        for (const auto& e : *data) entries.add ({ e["role"].toString(), e["text"].toString() });
    helper.send (makeObject ({ { "type", type::resetChat } }));
    hideOverlay();
    sendChangeMessage();
}

juce::String DawBotProcessor::formatReply (const juce::var& reply)
{
    juce::String s = reply["answer"].toString();

    if (auto* st = reply["steps"].getArray(); st != nullptr && ! st->isEmpty())
    {
        s << "\n\nSteps:";
        int i = 1;
        for (const auto& step : *st)
            s << "\n" << i++ << ". " << step["instruction"].toString()
              << ((bool) step["hasTarget"] ? "  [highlighted on screen]" : "");
    }

    if (auto* chain = reply["plugin_chain"].getArray(); chain != nullptr && ! chain->isEmpty())
    {
        s << "\n\nSignal chain:";
        for (const auto& p : *chain)
        {
            s << "\n- " << p["plugin"].toString();
            if (! (bool) p["installed"])
                s << " (not installed)";
            s << ": " << p["purpose"].toString();

            const auto settings = p["settings"].toString();
            if (settings.isNotEmpty())
                s << "\n    " << settings;
        }
    }

    if (reply["midi"].isObject())
        s << "\n\nMIDI clip ready: drag it from the tile below onto a MIDI track.";

    return s;
}

// ---------------------------------------------------------------------------
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DawBotProcessor();
}
