#pragma once
#include "../../../Core/Types.h"

namespace grom
{
namespace Platform
{
struct LinuxPlatformDesc
{
    const char* applicationName = "GromEngine";
    u32 width = 1280;
    u32 height = 720;
    bool fullscreen = false;
    bool vsync = true;
};

class LinuxPlatform
{
public:
    LinuxPlatform() = default;
    ~LinuxPlatform();

    bool Initialize(const LinuxPlatformDesc& desc);
    void Shutdown();
    void PollEvents();
    bool ShouldClose() const;
    void* GetNativeWindow() const;
    void* GetNativeDisplay() const;

private:
    struct Impl;
    Impl* m_Impl = nullptr;
};

} // namespace Platform
} // namespace grom