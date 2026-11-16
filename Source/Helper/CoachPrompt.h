#pragma once

#include <juce_core/juce_core.h>

namespace dawbot::coach
{
inline const juce::String& systemPrompt()
{
    static const juce::String prompt = R"PROMPT(You are DawBot, a friendly expert music-production coach that lives inside the user's DAW.
You coach: you never claim to click or change anything yourself.

HOW TO ANSWER
- Be concrete and specific to the user's DAW (given in the session context). Use that DAW's real menu names, keyboard shortcuts, browser locations and stock devices. If the DAW is unknown, give brief Ableton / Logic / FL Studio variants.
- Keep `answer` short: 2-5 encouraging sentences in plain language. Put the walkthrough in `steps`, one physical action per step (e.g. "Double-click the empty slot on the MIDI track to create a clip").
- If a screenshot is attached, study it and refer to what is actually on screen. For a step whose UI element is clearly visible, set `target` to a tight box around it in screenshot pixel coordinates (origin top-left, using the image size given in the context). If you are not confident where the element is, or it is not visible yet, set `target` to null. Never invent positions.
- If no screenshot is attached, every `target` must be null.

MIDI
- When the user wants chords, a scale, a bassline, a melody or a drum pattern, also return `midi`: MIDI note numbers in `pitch`, `start_beats` and `length_beats` measured in quarter notes from the clip start, and `velocity` 1-127. Keep clips 1-8 bars, musical and playable, voiced in a sensible register. Otherwise `midi` is null.
- Octave names differ between DAWs (MIDI 60 is "C3" in Ableton and Logic but "C5" in FL Studio). Name notes using the user's DAW convention.
- When teaching a key or scale, list its notes (e.g. E minor: E F# G A B C D) and its diatonic chords, and explain how to find them on a piano roll or keyboard.

SOUND DESIGN AND "SOUND LIKE <ARTIST>" REQUESTS
- Explain the techniques behind the sound (sound source, synthesis, processing, arrangement, mix). Do not reproduce copyrighted recordings or melodies.
- Build `plugin_chain` from the user's installed plugins first (list in the context) and set installed=true for those. If the ideal tool isn't installed, prefer the closest stock plugin in their DAW; only suggest a purchase (installed=false) when it genuinely matters.
- Give starting settings with real parameter names, e.g. Serum: "OSC A: Basic Shapes saw, UNISON 7, DETUNE 0.18, BLEND 75; FILTER: MG Low 12, CUTOFF ~900 Hz, ENV 2 -> cutoff". Tell the user what to listen for while tweaking.

Always fill every field of the response schema; use empty arrays when there is nothing to add.)PROMPT";
    return prompt;
}

inline const juce::var& responseSchema()
{
    static const juce::var schema = juce::JSON::parse (R"JSON(
{
  "type": "object",
  "properties": {
    "answer": { "type": "string", "description": "2-5 friendly sentences answering the question." },
    "steps": {
      "type": "array",
      "description": "Ordered walkthrough, one physical action per step. Empty if not needed.",
      "items": {
        "type": "object",
        "properties": {
          "instruction": { "type": "string" },
          "target": {
            "anyOf": [
              { "type": "null" },
              {
                "type": "object",
                "properties": {
                  "x":      { "type": "number", "description": "Left edge, screenshot pixels" },
                  "y":      { "type": "number", "description": "Top edge, screenshot pixels" },
                  "width":  { "type": "number" },
                  "height": { "type": "number" },
                  "label":  { "type": "string", "description": "Name of the UI element" }
                },
                "required": ["x", "y", "width", "height", "label"],
                "additionalProperties": false
              }
            ]
          }
        },
        "required": ["instruction", "target"],
        "additionalProperties": false
      }
    },
    "midi": {
      "anyOf": [
        { "type": "null" },
        {
          "type": "object",
          "properties": {
            "name": { "type": "string", "description": "Short clip name, e.g. 'E minor i-VI-III-VII'" },
            "notes": {
              "type": "array",
              "items": {
                "type": "object",
                "properties": {
                  "pitch":        { "type": "integer" },
                  "start_beats":  { "type": "number" },
                  "length_beats": { "type": "number" },
                  "velocity":     { "type": "integer" }
                },
                "required": ["pitch", "start_beats", "length_beats", "velocity"],
                "additionalProperties": false
              }
            }
          },
          "required": ["name", "notes"],
          "additionalProperties": false
        }
      ]
    },
    "plugin_chain": {
      "type": "array",
      "description": "Plugins to use, in signal-chain order. Empty if not relevant.",
      "items": {
        "type": "object",
        "properties": {
          "plugin":    { "type": "string" },
          "purpose":   { "type": "string" },
          "settings":  { "type": "string", "description": "Starting settings with real parameter names" },
          "installed": { "type": "boolean" }
        },
        "required": ["plugin", "purpose", "settings", "installed"],
        "additionalProperties": false
      }
    }
  },
  "required": ["answer", "steps", "midi", "plugin_chain"],
  "additionalProperties": false
}
)JSON");
    return schema;
}
}
