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
            entries.add ({ "dawbot", "Lost the connection to DawBot Helper. Reopen it and ask again." });
        }
        sendChangeMessage();
    };
}

DawBotProcessor::~DawBotProcessor()
{
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
    const juce::PluginHostType host;

    return makeObject ({
        { "host",          juce::String (host.getHostDescription()) },
        { "pluginFormat",  juce::String (juce::AudioProcessor::getWrapperTypeDescription (wrapperType)) },
        { "os",            juce::SystemStats::getOperatingSystemName() },
        { "bpm",           bpm.load() },
        { "timeSignature", juce::String (timeSigNum.load()) + "/" + juce::String (timeSigDen.load()) },
        { "isPlaying",     isPlaying.load() }
    });
}

void DawBotProcessor::launchHelper()
{
    if (! HelperConnection::launchHelperApp())
        entries.add ({ "dawbot", "I couldn't find DawBot Helper. Install it in your Applications folder, then open it." });

    sendChangeMessage();
}

void DawBotProcessor::ask (const juce::String& rawQuestion, bool showOnScreen)
{
    const auto question = rawQuestion.trim();
    if (question.isEmpty() || waiting)
        return;

    entries.add ({ "you", question });

    if (! helper.isConnected())
    {
        entries.add ({ "dawbot", "I can't reach DawBot Helper, which handles screen capture and the AI. "
                                 "Click \"Launch Helper\", then ask again." });
        sendChangeMessage();
        return;
    }

    pendingRequestId = juce::Uuid().toString();
    waiting = true;
    statusText = showOnScreen ? "Looking at your screen..." : "Thinking...";

    helper.send (makeObject ({
        { "type",         type::ask },
        { "id",           pendingRequestId },
        { "question",     question },
        { "showOnScreen", showOnScreen },
        { "context",      buildSessionContext() }
    }));

    sendChangeMessage();
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
    entries.clear();
    steps.clear();
    currentStep = 0;
    midiClip.reset();
    helper.send (makeObject ({ { "type", type::resetChat } }));
    hideOverlay();
    sendChangeMessage();
}

void DawBotProcessor::handleHelperMessage (const juce::var& msg)
{
    const auto t  = msg["type"].toString();
    const auto id = msg["id"].toString();

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
    }
    else if (t == type::error)
    {
        waiting = false;
        statusText = {};
        entries.add ({ "dawbot", "Something went wrong: " + msg["message"].toString() });
    }

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
