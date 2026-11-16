#include "MidiClip.h"
#include <cmath>

namespace dawbot
{
MidiClip MidiClip::fromVar (const juce::var& v, double bpm, int num, int den)
{
    MidiClip clip;
    clip.name       = v["name"].toString().trim();
    clip.bpm        = bpm > 0.0 ? bpm : 120.0;
    clip.timeSigNum = num > 0 ? num : 4;
    clip.timeSigDen = den > 0 ? den : 4;

    if (auto* arr = v["notes"].getArray())
    {
        for (const auto& n : *arr)
        {
            Note note;
            note.pitch       = juce::jlimit (0, 127, (int) n["pitch"]);
            note.startBeats  = juce::jmax (0.0, (double) n["start_beats"]);
            note.lengthBeats = juce::jmax (1.0 / 64.0, (double) n["length_beats"]);
            note.velocity    = n.hasProperty ("velocity") ? juce::jlimit (1, 127, (int) n["velocity"]) : 100;
            clip.notes.push_back (note);
        }
    }

    return clip;
}

double MidiClip::lengthBeats() const
{
    double end = 0.0;
    for (const auto& n : notes)
        end = juce::jmax (end, n.startBeats + n.lengthBeats);

    const double beatsPerBar = timeSigNum * (4.0 / timeSigDen);
    return juce::jmax (beatsPerBar, std::ceil (end / beatsPerBar) * beatsPerBar);
}

juce::File MidiClip::writeToTempFile() const
{
    constexpr int ppq = 960;

    juce::MidiMessageSequence seq;
    seq.addEvent (juce::MidiMessage::tempoMetaEvent ((int) std::round (60'000'000.0 / bpm)), 0.0);
    seq.addEvent (juce::MidiMessage::timeSignatureMetaEvent (timeSigNum, timeSigDen), 0.0);
    seq.addEvent (juce::MidiMessage::textMetaEvent (3, name.isEmpty() ? "DawBot" : name), 0.0);

    for (const auto& n : notes)
    {
        const double on  = n.startBeats * ppq;
        const double off = (n.startBeats + n.lengthBeats) * ppq;
        seq.addEvent (juce::MidiMessage::noteOn  (1, n.pitch, (juce::uint8) n.velocity), on);
        seq.addEvent (juce::MidiMessage::noteOff (1, n.pitch), off);
    }

    seq.updateMatchedPairs();

    juce::MidiFile file;
    file.setTicksPerQuarterNote (ppq);
    file.addTrack (seq);

    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("DawBot");
    dir.createDirectory();

    auto out = dir.getChildFile (juce::File::createLegalFileName (name.isEmpty() ? "DawBot Clip" : name) + ".mid");
    out.deleteFile();

    if (juce::FileOutputStream stream (out); stream.openedOk())
    {
        file.writeTo (stream);
        stream.flush();
    }

    return out;
}
}
