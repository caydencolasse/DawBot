#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <functional>
#include <vector>

namespace dawbot::ui
{
inline const juce::Colour background { 0xff161618 }, surface { 0xff242427 }, raised { 0xff303034 },
    border { 0xff45454b }, foreground { 0xfff8f7f2 }, muted { 0xffb9b8be }, accent { 0xfffa684a },
    accentTint { 0xff38241f }, error { 0xfff28c8c };
juce::Font font (float pixels, bool heading = false);
juce::Image logo();
std::unique_ptr<juce::Drawable> icon (const juce::String& name);

struct Measured { virtual ~Measured() = default; virtual int heightForWidth (int) const = 0; };
int heightFor (juce::Component&, int width);

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return font (16); }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return font (16); }
    void drawTextEditorOutline (juce::Graphics&, int, int, juce::TextEditor&) override;
    void fillTextEditorBackground (juce::Graphics&, int, int, juce::TextEditor&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class Text : public juce::Component, public Measured
{
public:
    Text (juce::String value, float size = 16, juce::Colour colour = foreground, bool strong = false);
    int heightForWidth (int width) const override;
    void paint (juce::Graphics&) override;
    void setText (const juce::String& value) { text = value; setName (text); repaint(); }
private:
    juce::TextLayout layout (int width) const;
    juce::String text;
    float size;
    juce::Colour colour;
    bool strong;
};

// The layout owns components, except an explicitly borrowed persistent composer.
class Stack : public juce::Component, public Measured
{
public:
    explicit Stack (int gap = 20, int padding = 0, bool panel = false);
    template <class T, class... Args> T& add (Args&&... args)
    {
        auto child = std::make_unique<T> (std::forward<Args> (args)...);
        auto& ref = *child;
        children.push_back (child.get()); addAndMakeVisible (ref); owned.push_back (std::move (child));
        return ref;
    }
    void borrow (juce::Component& child) { children.push_back (&child); addAndMakeVisible (child); }
    int heightForWidth (int) const override;
    void resized() override;
    void paint (juce::Graphics&) override;
    juce::Colour panelColour = surface;
private:
    int gap, padding;
    bool panel;
    std::vector<juce::Component*> children;
    std::vector<std::unique_ptr<juce::Component>> owned;
};

class Grid : public Stack
{
public:
    Grid (int numColumns = 2, int spacing = 20) : Stack (spacing), columns (numColumns), gap (spacing) {}
    int heightForWidth (int width) const override;
    void resized() override;
private:
    int countColumns (int width) const { return columns == 4 && width < 1000 ? 2 : columns; }
    int columns, gap;
};

enum class Tone { primary, secondary, ghost, selected };
class Action : public juce::Button, public Measured
{
public:
    Action (juce::String label, std::function<void()> action, Tone tone = Tone::secondary, juce::String iconName = {});
    int heightForWidth (int) const override { return 44; }
    void paintButton (juce::Graphics&, bool over, bool down) override;
    Tone tone;
private:
    std::unique_ptr<juce::Drawable> glyph;
};

class Card : public juce::Button, public Measured
{
public:
    enum class Kind { prompt, module, compactModule, article, row };
    Card (juce::String title, juce::String description, juce::String iconName, std::function<void()> action,
          Kind kind = Kind::article);
    int heightForWidth (int) const override;
    void paintButton (juce::Graphics&, bool over, bool down) override;
private:
    juce::String title, description;
    std::unique_ptr<juce::Drawable> glyph;
    Kind kind;
};

class Field : public juce::TextEditor, public Measured
{
public:
    Field (juce::String name, juce::String value, bool multiline = false);
    int heightForWidth (int) const override { return multiline ? 110 : 54; }
private:
    bool multiline;
};

class Switch : public juce::ToggleButton, public Measured
{
public:
    Switch (juce::String name, bool value, std::function<void(bool)> action);
    int heightForWidth (int) const override { return 48; }
};
}
