#pragma once
#include "../../../Core/Types.h"

namespace grom
{
namespace Platform
{
struct macOSPlatformDesc
{
    const char* applicationName = "GromEngine";
    u32 width = 1280;
    u32 height = 720;
    bool fullscreen = false;
    bool vsync = true;
};

class macOSPlatform
{
public:
    macOSPlatform() = default;
    ~macOSPlatform();

    bool Initialize(const macOSPlatformDesc& desc);
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