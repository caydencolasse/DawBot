#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "HelperConnection.h"
#include "Shared/MidiClip.h"
#include "WorkspaceState.h"
#include <atomic>
#include <optional>

struct ChatEntry
{
    juce::String role;   // "you" or "dawbot"
    juce::String text;
};

// DawBot is a pass-through audio effect: it never touches the audio.
// It reads tempo / time signature from the host and talks to the Helper app.
class DawBotProcessor : public juce::AudioProcessor,
                        public juce::ChangeBroadcaster,
                        private juce::AsyncUpdater
{
public:
    DawBotProcessor();
    ~DawBotProcessor() override;

    // --- AudioProcessor ---------------------------------------------------
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                    { return true; }
    const juce::String getName() const override        { return JucePlugin_Name; }
    bool acceptsMidi() const override                  { return false; }
    bool producesMidi() const override                 { return false; }
    bool isMidiEffect() const override                 { return false; }
    double getTailLengthSeconds() const override       { return 0.0; }
    int getNumPrograms() override                      { return 1; }
    int getCurrentProgram() override                   { return 0; }
    void setCurrentProgram (int) override              {}
    const juce::String getProgramName (int) override   { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // --- Coaching API (message thread only) ------------------------------
    void startHelperConnection()               { helper.start(); }
    void launchHelper();
    bool ask (const juce::String& question, bool showOnScreen);
    void showStep (int index);
    void hideOverlay();
    void resetChat();
    void clearHistory();
    dawbot::WorkspaceState& getWorkspace() { return workspace; }
    juce::uint64 getWorkspaceRestoreCount() const { return workspaceRestoreCount; }
    juce::String getHostName() const;
    juce::String getLastError() const { return lastError; }

    bool isHelperConnected() const             { return helper.isConnected(); }
    bool isWaiting() const                     { return waiting; }
    juce::String getStatusText() const         { return statusText; }
    const juce::Array<ChatEntry>& getEntries() const { return entries; }
    int getNumSteps() const                    { return steps.size(); }
    int getCurrentStep() const                 { return currentStep; }
    juce::String getStepText (int index) const { return steps[index]["instruction"].toString(); }
    const std::optional<dawbot::MidiClip>& getMidiClip() const { return midiClip; }

private:
    void handleAsyncUpdate() override;
    void storeConversation();
    juce::var buildSessionContext() const;
    void handleHelperMessage (const juce::var& message);
    static juce::String formatReply (const juce::var& reply);

    HelperConnection helper;
    dawbot::WorkspaceState workspace;
    juce::CriticalSection hostStateLock;
    juce::MemoryBlock pendingHostState;
    juce::uint64 workspaceRestoreCount = 0;

    juce::Array<ChatEntry> entries;
    juce::Array<juce::var> steps;
    int currentStep = 0;
    std::optional<dawbot::MidiClip> midiClip;
    juce::String statusText, pendingRequestId;
    juce::String lastError, lastQuestion, conversationId { juce::Uuid().toString() };
    bool waiting = false;

    std::atomic<double> bpm        { 120.0 };
    std::atomic<int>    timeSigNum { 4 };
    std::atomic<int>    timeSigDen { 4 };
    std::atomic<bool>   isPlaying  { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DawBotProcessor)
};
