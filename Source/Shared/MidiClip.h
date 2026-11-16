#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace dawbot
{
    struct Note
    {
        int    pitch       = 60;   // MIDI note number
        double startBeats  = 0.0;  // quarter notes from clip start
        double lengthBeats = 1.0;
        int    velocity    = 100;
    };

    // A MIDI clip the AI generated (chords, scale, bassline, drums...).
    // The plugin turns it into a .mid file the user drags straight onto a DAW track.
    struct MidiClip
    {
        juce::String      name;
        double            bpm          = 120.0;
        int               timeSigNum   = 4;
        int               timeSigDen   = 4;
        std::vector<Note> notes;

        static MidiClip fromVar (const juce::var& v, double bpm, int timeSigNum, int timeSigDen);

        bool       isValid() const { return ! notes.empty(); }
        double     lengthBeats() const;              // rounded up to whole bars
        juce::File writeToTempFile() const;          // Standard MIDI File, type 1
    };
}
