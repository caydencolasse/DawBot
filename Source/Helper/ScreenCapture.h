#pragma once

#include <juce_graphics/juce_graphics.h>

namespace dawbot::capture
{
    struct Result
    {
        juce::Image  image;
        juce::String error;
    };

    // macOS: Screen Recording permission (granted once, to DawBot Helper, not to each DAW).
    bool hasPermission();
    void requestPermission();

    // Captures one display. Blocking and thread-safe: call from a background thread.
    // logicalArea  = display bounds in JUCE (point) coordinates, used on macOS
    // physicalArea = same display in physical pixels, used on Windows
    Result captureArea (juce::Rectangle<int> logicalArea, juce::Rectangle<int> physicalArea);

    // Keeps our overlay out of screenshots (Windows; macOS excludes our app in the capture filter).
    void excludeWindowFromCapture (void* nativeWindowHandle);

    // Downscales to maxEdge on the long side and JPEG-encodes for the model.
    juce::MemoryBlock encodeJpeg (const juce::Image&, int maxEdge, int& outWidth, int& outHeight);
}
