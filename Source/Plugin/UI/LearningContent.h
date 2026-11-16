#pragma once
#include <juce_core/juce_core.h>
#include <vector>
namespace dawbot::ui
{
struct Article { juce::String title, summary, category, iconName, body; };
inline const std::vector<Article>& guides()
{
    static const std::vector<Article> content {
        { "The art of vocal chops", "Explore how slicing, rhythm and repetition can turn a phrase into an instrument.", "Vocals", "mic",
          "Turn a phrase into an instrument.\n\nA vocal chop is a small piece of a vocal recording, used as a rhythmic or melodic element. The creative decisions happen in the slice, the timing, and the space around it.\n\n01 / Find a useful moment\n\nListen for a clear syllable, a strong attack, or a texture that catches your ear. A small selection makes it easier to hear how repetition changes the phrase.\n\n02 / Make rhythm the starting point\n\nTry different lengths and leave room between repetitions. The gaps can be as important as the sound itself.\n\n03 / Shape the tone\n\nTry a small pitch change, then compare it with the original. Add a little delay or reverb and listen in the context of your track." },
        { "Shape a warmer bass", "Understand oscillators, filters and the role of movement in a bass sound.", "Sound design", "keys",
          "Start with one sound.\n\nChoose a simple oscillator and play a short bass phrase. Listen to the low end before adding effects.\n\nShape the brightness.\n\nUse a low-pass filter to reduce upper harmonics. Move the cutoff slowly and compare how clearly the bass speaks in the mix.\n\nAdd movement with purpose.\n\nTry a subtle filter envelope. Keep the level consistent when comparing settings so louder does not automatically seem better." },
        { "Make room in your mix", "A practical introduction to space, balance and frequency.", "Mixing", "book",
          "Balance comes first.\n\nStart by adjusting levels. Choose which element should lead the section and listen to the supporting parts around it.\n\nMake one change at a time.\n\nIf two sounds compete, try adjusting the arrangement before reaching for another effect. Compare in context, and keep a version you can return to." },
        { "Stock tools, new possibilities", "Learn the building blocks before adding more plugins.", "Sound design", "book",
          "Get to know the tools you have.\n\nChoose one stock instrument and explore its oscillator, filter and envelope. Make several versions of a sound, changing only one control at a time.\n\nAsk with context.\n\nTell dawbot your DAW, version and the instruments you own. Describe the result you want and ask for a stock-tool approach." }
    }; return content;
}
inline juce::StringArray lessonTitles() { return { "Choose your phrase", "Find the slice", "Build a rhythm", "Add movement", "Review your result" }; }
inline juce::StringArray lessonBodies()
{
    return { "Choose a short vocal recording you have permission to use. Listen for a phrase with a clear beginning and a sound you want to explore.",
             juce::String::fromUTF8("Listen to your reference and choose a small phrase with a clear beginning. Start simple—you can change the length as you hear it repeat."),
             "Repeat your slice in a short pattern. Leave a few gaps, then vary one repetition. Compare the rhythm with the drums in your session.",
             "Try a small change in pitch or filter tone. Add delay or reverb sparingly, and compare with the dry sound.",
             "Listen in the full track. Adjust the timing and level, then save a version you can return to. Ask dawbot for help adapting the process to your DAW." };
}
}
