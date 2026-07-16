#pragma once

#include <Core/Types.h>
#include <Core/Container.h>

#ifndef GROM_PLATFORM_API
#define GROM_PLATFORM_API
#endif

namespace grom
{

using ResizeCallback = void(*)(u32, u32);

struct WindowDesc
{
    GString Title;
    u32 Width = 1280;
    u32 Height = 720;
    bool Fullscreen = false;
    bool VSync = true;
};

class Window
{
public:
    virtual ~Window() = default;

    virtual void* GetNativeHandle() = 0;
    virtual u32 GetWidth() = 0;
    virtual u32 GetHeight() = 0;
    virtual void SetTitle(const GString& title) = 0;
    virtual void Resize(u32 width, u32 height) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsFullscreen() = 0;
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void ProcessMessages() = 0;
    virtual bool IsClosed() = 0;
    virtual void SetResizeCallback(ResizeCallback callback) = 0;
};

GROM_PLATFORM_API Window* CreatePlatformWindow(const WindowDesc& desc);

} // namespace grom
