#include <juce_gui_extra/juce_gui_extra.h>
#include "HelperServer.h"
#include "ScreenCapture.h"
#include "Shared/Protocol.h"

// ===========================================================================
// Settings + status window
// ===========================================================================
class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent (HelperServices& s, HelperServer& srv) : services (s), server (srv)
    {
        title.setText ("DawBot Helper", juce::dontSendNotification);
        title.setFont (juce::Font (juce::FontOptions (22.0f).withStyle ("Bold")));

        subtitle.setText ("Keep this running while you produce. It sees your screen when you ask "
                          "DawBot to \"show me\", talks to the AI, and draws the on-screen highlights.",
                          juce::dontSendNotification);
        subtitle.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));

        provider.addItem ("Anthropic (Claude)", 1);
        provider.addItem ("OpenAI", 2);
        provider.setSelectedId (services.config.provider == "openai" ? 2 : 1, juce::dontSendNotification);
        provider.onChange = [this]
        {
            const auto now  = providerId();
            const auto prev = now == "openai" ? juce::String ("anthropic") : juce::String ("openai");
            if (model.isEmpty() || model.getText() == HelperConfig::defaultModelFor (prev))
                model.setText (HelperConfig::defaultModelFor (now));
            updateProviderFields();
        };

        model.setText (services.config.model);
        apiKey.setPasswordCharacter ((juce::juce_wchar) 0x2022);
        apiKey.setText (services.config.apiKey);
        apiKey.setTextToShowWhenEmpty ("Anthropic key (Claude only)", juce::Colours::grey);
        authToken.setPasswordCharacter ((juce::juce_wchar) 0x2022);
        authToken.setText (services.config.authToken);
        authToken.setTextToShowWhenEmpty ("DawBot server token, not an OpenAI key", juce::Colours::grey);
        baseUrl.setText (services.config.baseUrl);
        baseUrl.setTextToShowWhenEmpty ("Optional, e.g. https://api.dawbot.ai", juce::Colours::grey);

        saveButton.onClick = [this]
        {
            services.config.provider = providerId();
            services.config.model    = model.getText().trim();
            services.config.apiKey   = providerId() == "openai" ? juce::String() : apiKey.getText().trim();
            services.config.authToken = authToken.getText().trim();
            services.config.baseUrl  = baseUrl.getText().trim();
            services.config.save();
            appendLog ("Settings saved.");
        };

        grantButton.onClick = []
        {
            dawbot::capture::requestPermission();
           #if JUCE_MAC
            juce::URL ("x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture").launchInDefaultBrowser();
           #endif
        };

        logView.setMultiLine (true);
        logView.setReadOnly (true);
        logView.setScrollbarsShown (true);
        logView.setCaretVisible (false);
        logView.setFont (juce::Font (juce::FontOptions (12.0f)));

        for (auto* l : { &providerLabel, &modelLabel, &keyLabel, &tokenLabel, &urlLabel })
            l->setJustificationType (juce::Justification::centredRight);

        providerLabel.setText ("AI provider", juce::dontSendNotification);
        modelLabel.setText ("Model", juce::dontSendNotification);
        keyLabel.setText ("API key", juce::dontSendNotification);
        urlLabel.setText ("Base URL", juce::dontSendNotification);
        tokenLabel.setText ("DawBot token", juce::dontSendNotification);

        for (auto* c : std::initializer_list<juce::Component*> { &title, &subtitle, &providerLabel, &provider, &modelLabel, &model,
                                                                 &keyLabel, &apiKey, &tokenLabel, &authToken, &urlLabel, &baseUrl, &saveButton,
                                                                 &permission, &grantButton, &connections, &logView })
            addAndMakeVisible (c);

        updateProviderFields();
        setSize (540, 640);
        startTimer (1000);
        timerCallback();
    }

    void appendLog (const juce::String& line)
    {
        logView.moveCaretToEnd();
        logView.insertTextAtCaret (juce::Time::getCurrentTime().toString (false, true) + "  " + line + "\n");
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff0d1017));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (16);
        title.setBounds (r.removeFromTop (30));
        subtitle.setBounds (r.removeFromTop (40));
        r.removeFromTop (8);

        auto row = [&r] (juce::Label& label, juce::Component& field)
        {
            auto line = r.removeFromTop (28);
            label.setBounds (line.removeFromLeft (90));
            field.setBounds (line.reduced (6, 0));
            r.removeFromTop (6);
        };

        row (providerLabel, provider);
        row (modelLabel, model);
        row (keyLabel, apiKey);
        row (tokenLabel, authToken);
        row (urlLabel, baseUrl);

        saveButton.setBounds (r.removeFromTop (30).removeFromRight (120));
        r.removeFromTop (12);

        auto permRow = r.removeFromTop (28);
        if (grantButton.isVisible())
            grantButton.setBounds (permRow.removeFromRight (200));
        permission.setBounds (permRow);

        connections.setBounds (r.removeFromTop (26));
        r.removeFromTop (8);
        logView.setBounds (r);
    }

private:
    void updateProviderFields()
    {
        const bool openai = providerId() == "openai";
        apiKey.setEnabled (!openai);
        model.setEnabled (!openai); // OpenAI model selection is server-owned.
        baseUrl.setTextToShowWhenEmpty (openai ? "Required DawBot server, e.g. http://localhost:8787" : "Optional proxy URL", juce::Colours::grey);
    }

    juce::String providerId() const { return provider.getSelectedId() == 2 ? "openai" : "anthropic"; }

    void timerCallback() override
    {
        const bool allowed = dawbot::capture::hasPermission();
        permission.setText (allowed ? "Screen Recording: allowed" : "Screen Recording: not allowed yet", juce::dontSendNotification);
        permission.setColour (juce::Label::textColourId, allowed ? juce::Colours::lightgreen : juce::Colours::orange);

        if (grantButton.isVisible() == allowed)
        {
            grantButton.setVisible (! allowed);
            resized();
        }

        const int n = server.getNumConnections();
        connections.setText (juce::String (n) + (n == 1 ? " DawBot plugin connected" : " DawBot plugins connected"),
                             juce::dontSendNotification);
    }

    HelperServices& services;
    HelperServer&   server;

    juce::Label title, subtitle, providerLabel, modelLabel, keyLabel, tokenLabel, urlLabel, permission, connections;
    juce::ComboBox provider;
    juce::TextEditor model, apiKey, authToken, baseUrl, logView;
    juce::TextButton saveButton { "Save" }, grantButton { "Grant Screen Recording" };
};

// ===========================================================================
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow (const juce::String& name, juce::Component& content)
        : DocumentWindow (name, juce::Colour (0xff0d1017), DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);
        setContentNonOwned (&content, true);
        setResizable (true, false);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    // Closing just minimises: the helper must keep running for the plugin.
    void closeButtonPressed() override { setMinimised (true); }
};

// ===========================================================================
class DawBotHelperApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override          { return false; }

    void initialise (const juce::String&) override
    {
        config = HelperConfig::load();
        scanner.startScan (pool);
        overlay = std::make_unique<dawbot::Overlay>();

        services = std::make_unique<HelperServices> (HelperServices {
            config, *overlay, scanner, pool,
            [this] (const juce::String& line) { if (mainComponent != nullptr) mainComponent->appendLog (line); } });

        server        = std::make_unique<HelperServer> (*services);
        mainComponent = std::make_unique<MainComponent> (*services, *server);
        mainWindow    = std::make_unique<MainWindow> (getApplicationName(), *mainComponent);

        if (server->start())
            mainComponent->appendLog ("Listening for DawBot plugins on localhost:" + juce::String (dawbot::protocol::helperPort));
        else
            mainComponent->appendLog ("Couldn't open port " + juce::String (dawbot::protocol::helperPort) + ". Is another DawBot Helper already running?");

        if (config.apiKey.isEmpty() && config.authToken.isEmpty())
            mainComponent->appendLog ("Add your API key above to get started.");
    }

    void shutdown() override
    {
        server        = nullptr;
        mainWindow    = nullptr;
        mainComponent = nullptr;
        services      = nullptr;
        overlay       = nullptr;
    }

    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted (const juce::String&) override
    {
        if (mainWindow != nullptr)
        {
            mainWindow->setMinimised (false);
            mainWindow->toFront (true);
        }
    }

private:
    // Declaration order matters: the pool is destroyed before the scanner it writes to.
    HelperConfig     config;
    PluginScanner    scanner;
    juce::ThreadPool pool { 2 };

    std::unique_ptr<dawbot::Overlay> overlay;
    std::unique_ptr<HelperServices> services;
    std::unique_ptr<HelperServer>   server;
    std::unique_ptr<MainComponent>  mainComponent;
    std::unique_ptr<MainWindow>     mainWindow;
};

START_JUCE_APPLICATION (DawBotHelperApplication)
