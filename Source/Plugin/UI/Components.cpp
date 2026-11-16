#include "Components.h"
#include <DawBotAssets.h>

namespace dawbot::ui
{
juce::Font font (float pixels, bool heading)
{
    static auto regular = juce::Typeface::createSystemTypefaceFor (DawBotAssets::InterRegular_ttf, DawBotAssets::InterRegular_ttfSize);
    static auto semibold = juce::Typeface::createSystemTypefaceFor (DawBotAssets::InterSemiBold_ttf, DawBotAssets::InterSemiBold_ttfSize);
    return juce::Font (juce::FontOptions (heading ? semibold : regular).withPointHeight (pixels));
}
juce::Image logo() { return juce::ImageCache::getFromMemory (DawBotAssets::logo_png, DawBotAssets::logo_pngSize); }
std::unique_ptr<juce::Drawable> icon (const juce::String& name)
{
    int size = 0;
    const auto* bytes = DawBotAssets::getNamedResource ((name + "_svg").toRawUTF8(), size);
    return bytes != nullptr ? juce::Drawable::createFromImageData (bytes, (size_t) size) : nullptr;
}
int heightFor (juce::Component& c, int width)
{
    if (auto* measured = dynamic_cast<Measured*> (&c)) return measured->heightForWidth (width);
    return juce::jmax (1, c.getHeight());
}
LookAndFeel::LookAndFeel()
{
    setColour (juce::TextEditor::backgroundColourId, surface);
    setColour (juce::TextEditor::textColourId, foreground);
    setColour (juce::TextEditor::highlightColourId, accentTint);
    setColour (juce::TextEditor::highlightedTextColourId, foreground);
    setColour (juce::CaretComponent::caretColourId, accent);
    setColour (juce::Label::textColourId, foreground);
    setColour (juce::PopupMenu::backgroundColourId, surface);
    setColour (juce::PopupMenu::textColourId, foreground);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accentTint);
    setColour (juce::PopupMenu::highlightedTextColourId, accent);
    setColour (juce::ScrollBar::thumbColourId, border);
}
void LookAndFeel::fillTextEditorBackground (juce::Graphics& g, int w, int h, juce::TextEditor& t)
{
    g.setColour (t.findColour (juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle (juce::Rectangle<float> (0, 0, (float) w, (float) h), 12);
}
void LookAndFeel::drawTextEditorOutline (juce::Graphics& g, int w, int h, juce::TextEditor& t)
{
    if (t.getProperties()["borderless"]) return;
    g.setColour (t.hasKeyboardFocus (true) ? accent : border);
    g.drawRoundedRectangle (juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1), 12, 1);
}
void LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    g.setColour (foreground); g.setFont (font (16));
    g.drawText (b.getButtonText(), b.getLocalBounds().withTrimmedRight (64), juce::Justification::centredLeft);
    const auto track = juce::Rectangle<float> ((float) b.getWidth() - 48, 11, 44, 26);
    g.setColour (b.getToggleState() ? accent : raised); g.fillRoundedRectangle (track, 13);
    g.setColour (foreground); g.fillEllipse (track.getX() + (b.getToggleState() ? 21.0f : 3.0f), 14, 20, 20);
    if (b.hasKeyboardFocus (true) || over) { g.setColour (accent); g.drawRoundedRectangle (track.expanded (3), 16, 1); }
}
Text::Text (juce::String value, float s, juce::Colour c, bool bold)
    : text (std::move (value)), size (s), colour (c), strong (bold)
{
    setName (text); setAccessible (true); setInterceptsMouseClicks (false, false);
}
juce::TextLayout Text::layout (int width) const
{
    juce::AttributedString a; a.setJustification (juce::Justification::topLeft);
    a.setLineSpacing (4); a.append (text, font (size, strong), colour);
    juce::TextLayout l; l.createLayout (a, (float) juce::jmax (1, width)); return l;
}
int Text::heightForWidth (int width) const { return (int) std::ceil (layout (width).getHeight()) + 2; }
void Text::paint (juce::Graphics& g) { layout (getWidth()).draw (g, getLocalBounds().toFloat()); }

Stack::Stack (int spacing, int pad, bool hasPanel) : gap (spacing), padding (pad), panel (hasPanel) {}
int Stack::heightForWidth (int width) const
{
    int height = padding * 2;
    for (auto* c : children) height += heightFor (*c, width - padding * 2) + gap;
    return height - (children.empty() ? 0 : gap);
}
void Stack::resized()
{
    int y = padding;
    for (auto* c : children)
    {
        const int width = getWidth() - padding * 2;
        const int h = heightFor (*c, width);
        c->setBounds (padding, y, dynamic_cast<Action*> (c) != nullptr ? juce::jmin (288, width) : width, h);
        y += h + gap;
    }
}
void Stack::paint (juce::Graphics& g)
{
    if (panel) { g.setColour (panelColour); g.fillRoundedRectangle (getLocalBounds().toFloat(), 12); }
}
int Grid::heightForWidth (int width) const
{
    const int cols = countColumns (width), cellWidth = (width - gap * (cols - 1)) / cols;
    int total = 0;
    for (int i = 0; i < getNumChildComponents(); i += cols)
    {
        int h = 0; for (int j = i; j < juce::jmin (i + cols, getNumChildComponents()); ++j) h = juce::jmax (h, heightFor (*getChildComponent (j), cellWidth));
        total += h + gap;
    }
    return juce::jmax (0, total - gap);
}
void Grid::resized()
{
    const int cols = countColumns (getWidth()), cellWidth = (getWidth() - gap * (cols - 1)) / cols;
    int y = 0;
    for (int i = 0; i < getNumChildComponents(); i += cols)
    {
        int h = 0; for (int j = i; j < juce::jmin (i + cols, getNumChildComponents()); ++j) h = juce::jmax (h, heightFor (*getChildComponent (j), cellWidth));
        for (int j = i; j < juce::jmin (i + cols, getNumChildComponents()); ++j) getChildComponent (j)->setBounds ((j-i)*(cellWidth+gap), y, cellWidth, h);
        y += h + gap;
    }
}
Action::Action (juce::String label, std::function<void()> action, Tone t, juce::String iconName)
    : Button (label), tone (t), glyph (icon (iconName))
{
    setButtonText (label); onClick = std::move (action); setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setWantsKeyboardFocus (true); setTooltip (label);
}
void Action::paintButton (juce::Graphics& g, bool over, bool down)
{
    const auto b = getLocalBounds().toFloat().reduced (1);
    auto bg = tone == Tone::primary ? accent : tone == Tone::selected ? accentTint : surface;
    if (down) bg = bg.darker (.12f); else if (over) bg = bg.brighter (.08f);
    if (tone != Tone::ghost || over) { g.setColour (bg.withAlpha (isEnabled() ? 1.0f : .4f)); g.fillRoundedRectangle (b, 22); }
    if (tone == Tone::secondary || hasKeyboardFocus (true)) { g.setColour (hasKeyboardFocus (true) ? accent : border); g.drawRoundedRectangle (b, 22, 1); }
    g.setColour ((tone == Tone::primary ? background : tone == Tone::selected ? accent : foreground).withAlpha (isEnabled() ? 1.0f : .4f));
    g.setFont (font (16));
    if (glyph) glyph->drawWithin (g, b.withSizeKeepingCentre (24, 24), juce::RectanglePlacement::centred, isEnabled() ? 1.0f : .4f);
    else g.drawFittedText (getButtonText(), getLocalBounds().reduced (12, 2), juce::Justification::centred, 1, 1.0f);
}
Card::Card (juce::String t, juce::String d, juce::String iconName, std::function<void()> action, Kind k)
    : Button (t), title (std::move (t)), description (std::move (d)), glyph (icon (iconName)), kind (k)
{
    onClick = std::move (action); setWantsKeyboardFocus (true); setTooltip (title); setMouseCursor (juce::MouseCursor::PointingHandCursor);
}
int Card::heightForWidth (int width) const
{
    if (kind == Kind::prompt) return width < 400 ? 112 : 104;
    if (kind == Kind::module) return 150;
    if (kind == Kind::compactModule) return 76;
    if (kind == Kind::row) return 88;
    return 248;
}
void Card::paintButton (juce::Graphics& g, bool over, bool down)
{
    const auto b = getLocalBounds().toFloat().reduced (1);
    g.setColour (down ? raised : over ? surface.brighter (.05f) : surface); g.fillRoundedRectangle (b, 12);
    g.setColour (hasKeyboardFocus (true) || over ? accent : border); g.drawRoundedRectangle (b, 12, 1);
    auto r = getLocalBounds().reduced (20);
    const bool compact = kind == Kind::compactModule;
    if (kind == Kind::prompt || kind == Kind::row || compact)
    {
        if (glyph) glyph->drawWithin (g, r.removeFromLeft (28).withSizeKeepingCentre (28,28).toFloat(), juce::RectanglePlacement::centred, 1);
        r.removeFromLeft (16);
        if (kind == Kind::prompt)
        {
            g.setColour (muted); g.setFont (font (12)); g.drawText (description, r.removeFromTop (22), juce::Justification::centredLeft);
            g.setColour (foreground); g.setFont (font (16)); g.drawFittedText (title, r, juce::Justification::centredLeft, 3, 1.0f);
        }
        else
        {
            g.setColour (foreground); g.setFont (font (16)); g.drawText (title, r.removeFromTop (compact ? r.getHeight() : 24), juce::Justification::centredLeft);
            if (! compact) { g.setColour (muted); g.setFont (font (13)); g.drawFittedText (description, r, juce::Justification::centredLeft, 2, 1.0f); }
        }
        return;
    }
    if (glyph) glyph->drawWithin (g, r.removeFromTop (28).withWidth (28).toFloat(), juce::RectanglePlacement::centred, 1);
    r.removeFromTop (12); g.setColour (foreground); g.setFont (font (kind == Kind::article ? 21.0f : 16.0f, kind == Kind::article));
    g.drawFittedText (title, r.removeFromTop (kind == Kind::article ? 56 : 24), juce::Justification::topLeft, 2, 1.0f);
    r.removeFromTop (8); g.setColour (muted); g.setFont (font (kind == Kind::article ? 16.0f : 13.0f));
    g.drawFittedText (description, r, juce::Justification::topLeft, 4, 1.0f);
}
Field::Field (juce::String name, juce::String value, bool multi) : multiline (multi)
{
    setName (name); setTitle (name); setFont (font (16)); setMultiLine (multi, multi);
    setIndents (16, 15); setText (value, false); setSelectAllWhenFocused (! multi);
}
Switch::Switch (juce::String name, bool value, std::function<void(bool)> action) : ToggleButton (name)
{
    setName (name); setToggleState (value, juce::dontSendNotification);
    onClick = [this, action] { action (getToggleState()); };
}
}
