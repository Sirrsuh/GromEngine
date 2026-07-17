#include <Platform/macOS/macOS_Window.h>
#include <Core/Assert.h>
#include <Core/Log.h>

#include <Cocoa/Cocoa.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

namespace grom::Platform
{

struct macOSWindow::Impl
{
    NSWindow* window = nullptr;
    NSView* contentView = nullptr;
    CAMetalLayer* metalLayer = nullptr;
    id<MTLDevice> device = nil;
    bool shouldClose = false;
    u32 width = 0;
    u32 height = 0;
};

macOSWindow::~macOSWindow()
{
    Destroy();
}

bool macOSWindow::Create(const char* title, u32 width, u32 height, bool fullscreen)
{
    m_Impl = new Impl();
    m_Impl->width = width;
    m_Impl->height = height;

    @autoreleasepool {
        NSRect frame = NSMakeRect(100, 100, static_cast<CGFloat>(width), static_cast<CGFloat>(height));
        
        NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
        if (fullscreen)
            styleMask |= NSFullScreenWindowMask;

        m_Impl->window = [[NSWindow alloc] initWithContentRect:frame
                                                      styleMask:styleMask
                                                        backing:NSBackingStoreBuffered
                                                          defer:NO];
        
        if (!m_Impl->window)
        {
            GROM_LOG_ERROR("Failed to create NSWindow");
            return false;
        }

        m_Impl->window.title = [NSString stringWithUTF8String:title];
        m_Impl->window.delegate = nil;
        m_Impl->contentView = m_Impl->window.contentView;
        m_Impl->contentView.wantsLayer = YES;

        // Create Metal device and layer
        m_Impl->device = MTLCreateSystemDefaultDevice();
        if (!m_Impl->device)
        {
            GROM_LOG_ERROR("Metal is not supported on this device");
            return false;
        }

        m_Impl->metalLayer = [CAMetalLayer layer];
        m_Impl->metalLayer.device = m_Impl->device;
        m_Impl->metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        m_Impl->metalLayer.frame = m_Impl->contentView.bounds;
        m_Impl->metalLayer.contentsScale = m_Impl->window.backingScaleFactor;
        
        [m_Impl->contentView.layer addSublayer:m_Impl->metalLayer];
    }

    GROM_LOG_INFO("Created macOS window: {}x{}", width, height);
    return true;
}

void macOSWindow::Destroy()
{
    if (m_Impl)
    {
        @autoreleasepool {
            if (m_Impl->metalLayer)
            {
                [m_Impl->metalLayer removeFromSuperlayer];
                m_Impl->metalLayer = nil;
            }
            if (m_Impl->device)
            {
                m_Impl->device = nil;
            }
            if (m_Impl->window)
            {
                [m_Impl->window close];
                m_Impl->window = nil;
            }
        }
        delete m_Impl;
        m_Impl = nullptr;
    }
}

void* macOSWindow::GetNativeHandle() const
{
    return m_Impl ? m_Impl->window : nullptr;
}

void macOSWindow::SetTitle(const char* title)
{
    if (m_Impl && m_Impl->window)
    {
        @autoreleasepool {
            m_Impl->window.title = [NSString stringWithUTF8String:title];
        }
    }
}

void macOSWindow::SetFullscreen(bool fullscreen)
{
    if (m_Impl && m_Impl->window)
    {
        @autoreleasepool {
            if (fullscreen)
                [m_Impl->window toggleFullScreen:nil];
            else if ([m_Impl->window.styleMask & NSFullScreenWindowMask])
                [m_Impl->window toggleFullScreen:nil];
        }
    }
}

void macOSWindow::GetSize(u32& width, u32& height) const
{
    if (m_Impl)
    {
        width = m_Impl->width;
        height = m_Impl->height;
    }
    else
    {
        width = 0;
        height = 0;
    }
}

bool macOSWindow::ShouldClose() const
{
    return m_Impl ? m_Impl->shouldClose : true;
}

void macOSWindow::PollEvents()
{
    // Events are handled by the main run loop
}

void macOSWindow::SwapBuffers()
{
    if (m_Impl && m_Impl->metalLayer)
    {
        @autoreleasepool {
            id<MTLDrawable> drawable = [m_Impl->metalLayer nextDrawable];
            if (drawable)
            {
                [drawable present];
            }
        }
    }
}

} // namespace grom::Platform