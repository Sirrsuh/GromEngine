#include <Platform/macOS/macOS_Platform.h>
#include <Core/Assert.h>
#include <Core/Log.h>

#include <Cocoa/Cocoa.h>
#include <ApplicationServices/ApplicationServices.h>

namespace grom::Platform
{

struct macOSPlatform::Impl
{
    NSApplication* app = nil;
    macOSWindow* window = nullptr;
    bool initialized = false;
    bool shouldClose = false;
};

macOSPlatform::~macOSPlatform()
{
    Shutdown();
}

bool macOSPlatform::Initialize(const macOSPlatformDesc& desc)
{
    m_Impl = new Impl();

    @autoreleasepool {
        // Initialize Cocoa application
        m_Impl->app = [NSApplication sharedApplication];
        [m_Impl->app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Create window
        m_Impl->window = new macOSWindow();
        if (!m_Impl->window->Create(desc.applicationName, desc.width, desc.height, desc.fullscreen))
        {
            GROM_LOG_ERROR("Failed to create macOS window");
            return false;
        }

        [m_Impl->app finishLaunching];
        m_Impl->initialized = true;
    }

    GROM_LOG_INFO("macOS platform initialized");
    return true;
}

void macOSPlatform::Shutdown()
{
    if (m_Impl)
    {
        @autoreleasepool {
            if (m_Impl->window)
            {
                m_Impl->window->Destroy();
                delete m_Impl->window;
                m_Impl->window = nullptr;
            }
        }
        delete m_Impl;
        m_Impl = nullptr;
    }
}

void macOSPlatform::PollEvents()
{
    if (!m_Impl || !m_Impl->initialized) return;

    @autoreleasepool {
        NSEvent* event = [m_Impl->app nextEventMatchingMask:NSAnyEventMask
                                                   untilDate:[NSDate distantPast]
                                                      inMode:NSDefaultRunLoopMode
                                                     dequeue:YES];
        while (event)
        {
            [m_Impl->app sendEvent:event];
            event = [m_Impl->app nextEventMatchingMask:NSAnyEventMask
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
        }
    }
}

bool macOSPlatform::ShouldClose() const
{
    return m_Impl ? m_Impl->shouldClose : true;
}

void* macOSPlatform::GetNativeWindow() const
{
    return m_Impl && m_Impl->window ? m_Impl->window->GetNativeHandle() : nullptr;
}

void* macOSPlatform::GetNativeDisplay() const
{
    // Return the Cocoa display
    return nullptr;
}

} // namespace grom::Platform