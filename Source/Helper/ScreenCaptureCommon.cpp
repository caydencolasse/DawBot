#include "ScreenCapture.h"

namespace dawbot::capture
{
juce::MemoryBlock encodeJpeg (const juce::Image& source, int maxEdge, int& outWidth, int& outHeight)
{
    auto image = source;
    const int longest = juce::jmax (image.getWidth(), image.getHeight());

    if (longest > maxEdge)
    {
        const double scale = maxEdge / (double) longest;
        image = image.rescaled (juce::jmax (1, juce::roundToInt (image.getWidth()  * scale)),
                                juce::jmax (1, juce::roundToInt (image.getHeight() * scale)),
                                juce::Graphics::highResamplingQuality);
    }

    outWidth  = image.getWidth();
    outHeight = image.getHeight();

    juce::JPEGImageFormat jpeg;
    jpeg.setQuality (0.8f);

    juce::MemoryOutputStream out;
    jpeg.writeImageToStream (image, out);
    return out.getMemoryBlock();
}
}
