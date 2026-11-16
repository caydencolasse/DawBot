// Compiled with -fobjc-arc (see CMakeLists.txt).
#include "ScreenCapture.h"

#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>
#include <memory>

namespace dawbot::capture
{
namespace
{
    // Heap-allocated so a late completion handler (after a timeout) never touches a dead stack frame.
    struct PendingCapture
    {
        dispatch_semaphore_t done = dispatch_semaphore_create (0);
        CGImageRef image = nullptr;
        juce::String error;

        ~PendingCapture() { if (image != nullptr) CGImageRelease (image); }
    };

    juce::Image toJuceImage (CGImageRef cgImage)
    {
        const auto w = CGImageGetWidth (cgImage);
        const auto h = CGImageGetHeight (cgImage);

        juce::Image image (juce::Image::ARGB, (int) w, (int) h, false, juce::SoftwareImageType());

        {
            juce::Image::BitmapData data (image, juce::Image::BitmapData::writeOnly);
            auto colourSpace = CGColorSpaceCreateDeviceRGB();
            // Matches JUCE's premultiplied BGRA layout on little-endian machines.
            auto ctx = CGBitmapContextCreate (data.data, w, h, 8, (size_t) data.lineStride, colourSpace,
                                              (CGBitmapInfo) kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
            CGColorSpaceRelease (colourSpace);

            if (ctx != nullptr)
            {
                CGContextDrawImage (ctx, CGRectMake (0, 0, (CGFloat) w, (CGFloat) h), cgImage);
                CGContextRelease (ctx);
            }
        }

        return image;
    }

    juce::String toJuceString (NSError* error, const char* fallback)
    {
        return error != nil ? juce::String::fromUTF8 (error.localizedDescription.UTF8String) : juce::String (fallback);
    }
}

bool hasPermission()                 { return CGPreflightScreenCaptureAccess(); }
void requestPermission()             { CGRequestScreenCaptureAccess(); }
void excludeWindowFromCapture (void*) {}

Result captureArea (juce::Rectangle<int> logicalArea, juce::Rectangle<int>)
{
    Result result;

    if (! CGPreflightScreenCaptureAccess())
    {
        result.error = "Screen Recording permission isn't granted. Open DawBot Helper and click \"Grant Screen Recording\".";
        return result;
    }

    if (@available (macOS 14.0, *))
    {
        // Which display is the user working on? (JUCE logical coords == Quartz global points.)
        CGDirectDisplayID displayID = CGMainDisplayID();
        CGDirectDisplayID found = 0;
        uint32_t count = 0;
        const auto centre = CGPointMake ((CGFloat) logicalArea.getCentreX(), (CGFloat) logicalArea.getCentreY());

        if (CGGetDisplaysWithPoint (centre, 1, &found, &count) == kCGErrorSuccess && count > 0)
            displayID = found;

        auto pending = std::make_shared<PendingCapture>();
        NSString* ownBundleID = NSBundle.mainBundle.bundleIdentifier;

        [SCShareableContent getShareableContentExcludingDesktopWindows: NO
                                                   onScreenWindowsOnly: YES
                                                     completionHandler: ^(SCShareableContent* content, NSError* error)
        {
            if (content == nil)
            {
                pending->error = toJuceString (error, "No shareable screen content.");
                dispatch_semaphore_signal (pending->done);
                return;
            }

            SCDisplay* target = nil;
            for (SCDisplay* d in content.displays)
                if (d.displayID == displayID)
                    target = d;

            if (target == nil)
                target = content.displays.firstObject;

            if (target == nil)
            {
                pending->error = "No display found.";
                dispatch_semaphore_signal (pending->done);
                return;
            }

            // Leave DawBot's own overlay out of the screenshot.
            NSMutableArray<SCRunningApplication*>* excluded = [NSMutableArray array];
            for (SCRunningApplication* app in content.applications)
                if (ownBundleID != nil && [app.bundleIdentifier isEqualToString: ownBundleID])
                    [excluded addObject: app];

            SCContentFilter* filter = [[SCContentFilter alloc] initWithDisplay: target
                                                         excludingApplications: excluded
                                                              exceptingWindows: @[]];

            SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
            const CGFloat scale = filter.pointPixelScale;
            config.width       = (size_t) (filter.contentRect.size.width  * scale);
            config.height      = (size_t) (filter.contentRect.size.height * scale);
            config.showsCursor = YES;

            [SCScreenshotManager captureImageWithFilter: filter
                                          configuration: config
                                      completionHandler: ^(CGImageRef image, NSError* captureError)
            {
                if (image != nullptr)
                    pending->image = CGImageRetain (image);
                else
                    pending->error = toJuceString (captureError, "Screen capture failed.");

                dispatch_semaphore_signal (pending->done);
            }];
        }];

        if (dispatch_semaphore_wait (pending->done, dispatch_time (DISPATCH_TIME_NOW, (int64_t) (5 * NSEC_PER_SEC))) != 0)
        {
            result.error = "Screen capture timed out.";
            return result;
        }

        if (pending->image == nullptr)
        {
            result.error = pending->error.isNotEmpty() ? pending->error : juce::String ("Screen capture failed.");
            return result;
        }

        result.image = toJuceImage (pending->image);
        return result;
    }

    result.error = "Screen capture needs macOS 14 (Sonoma) or newer.";
    return result;
}
}
