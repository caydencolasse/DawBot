#include "ScreenCapture.h"

// Linux / other: chat + MIDI work, screen capture isn't implemented yet.
namespace dawbot::capture
{
bool hasPermission()                  { return false; }
void requestPermission()              {}
void excludeWindowFromCapture (void*) {}

Result captureArea (juce::Rectangle<int>, juce::Rectangle<int>)
{
    return { {}, "Screen capture isn't supported on this platform yet." };
}
}
