#pragma once
#include "../PluginEditor.h"
#include "../MidiDragTile.h"
#include "Components.h"
#include "Composer.h"
#include "LearningContent.h"

enum class DawBotScreen { home, chat, projects, project, newProject, renameProject, deleteProject,
                         saveProject, saved, guides, tutorials, howto, article, lesson,
                         history, settings, tools, privacy, clearHistory, help };

struct DawBotEditor::Interface : private juce::AsyncUpdater
{
    Interface (DawBotEditor&, DawBotProcessor&);
    ~Interface() override;
    void layout();
    void refresh();
    void navigate (DawBotScreen);
    void render();
    void submit();
    void queueQuestion (const juce::String&);
    void changed();
    void setPreference (const juce::String&, const juce::var&);
    void startSave (juce::String title, juce::String text);
    void showHome();
    void showChat();
    void showProjects();
    void showProject();
    void showProjectForm (bool rename);
    void showSaveProject();
    void showHistory();
    void showLearning();
    void showArticle();
    void showLesson();
    void showSettings();
    void showHelp();
    void heading (const juce::String&, const juce::String& = {});
    void notice (const juce::String&, const juce::String&, bool problem = false);
    void search (const juce::String&);
    dawbot::ui::Action& action (dawbot::ui::Stack&, const juce::String&, std::function<void()>, dawbot::ui::Tone = dawbot::ui::Tone::secondary);
    dawbot::ui::Field& field (dawbot::ui::Stack&, const juce::String&, const juce::String&, bool multiline = false);
    juce::var selectedProject() const;
    juce::String transcript() const;
    void handleAsyncUpdate() override { render(); }

    DawBotEditor& editor;
    DawBotProcessor& proc;
    dawbot::ui::LookAndFeel lookAndFeel;
    std::unique_ptr<juce::Button> brand;
    dawbot::ui::Action host, history, settings;
    juce::Viewport viewport;
    std::unique_ptr<dawbot::ui::Stack> body, navigation;
    dawbot::ui::Composer composer;
    dawbot::ui::Text* helperStatus = nullptr;
    dawbot::ui::Text* helperDetails = nullptr;
    MidiDragTile midi;
    DawBotScreen screen = DawBotScreen::home;
    juce::String filter, category, selectedId, savedTitle, savedText, pendingTitle, pendingText, message;
    int articleIndex = 0, lessonStep = 0, focusPosition = 0;
    bool pendingSave = false, compact = false;
    juce::uint64 observedRestoreCount = 0;
    juce::String focusName;
};
