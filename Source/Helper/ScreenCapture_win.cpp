#include "ScreenCapture.h"

#ifndef NOMINMAX
 #define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace dawbot::capture
{
bool hasPermission()     { return true; }
void requestPermission() {}

void excludeWindowFromCapture (void* nativeHandle)
{
    // WDA_EXCLUDEFROMCAPTURE (0x11) needs Windows 10 2004+; silently ignored on older systems.
    if (nativeHandle != nullptr)
        SetWindowDisplayAffinity ((HWND) nativeHandle, 0x00000011);
}

Result captureArea (juce::Rectangle<int>, juce::Rectangle<int> physicalArea)
{
    Result result;
    const int w = physicalArea.getWidth();
    const int h = physicalArea.getHeight();

    if (w <= 0 || h <= 0)
    {
        result.error = "Invalid display area.";
        return result;
    }

    HDC screenDC = GetDC (nullptr);
    HDC memDC    = CreateCompatibleDC (screenDC);

    BITMAPINFO bmi {};
    bmi.bmiHeader.biSize        = sizeof (BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h;   // top-down rows
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection (screenDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

    if (dib != nullptr && bits != nullptr)
    {
        auto previous = SelectObject (memDC, dib);

        if (BitBlt (memDC, 0, 0, w, h, screenDC, physicalArea.getX(), physicalArea.getY(), SRCCOPY | CAPTUREBLT))
        {
            juce::Image image (juce::Image::ARGB, w, h, false, juce::SoftwareImageType());

            {
                juce::Image::BitmapData data (image, juce::Image::BitmapData::writeOnly);

                for (int y = 0; y < h; ++y)
                {
                    auto* src = static_cast<const juce::uint8*> (bits) + (size_t) y * (size_t) w * 4;
                    auto* dst = data.getLinePointer (y);

                    for (int x = 0; x < w; ++x)
                    {
                        dst[x * 4 + 0] = src[x * 4 + 0];   // B
                        dst[x * 4 + 1] = src[x * 4 + 1];   // G
                        dst[x * 4 + 2] = src[x * 4 + 2];   // R
                        dst[x * 4 + 3] = 255;              // A
                    }
                }
            }

            result.image = image;
        }
        else
        {
            result.error = "Screen capture failed (BitBlt).";
        }

        SelectObject (memDC, previous);
        DeleteObject (dib);
    }
    else
    {
        result.error = "Couldn't allocate the capture bitmap.";
    }

    DeleteDC (memDC);
    ReleaseDC (nullptr, screenDC);
    return result;
}
}
