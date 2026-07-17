#pragma once
#include "../../../Core/Types.h"

namespace grom
{
namespace Platform
{
class LinuxWindow
{
public:
    LinuxWindow() = default;
    ~LinuxWindow();

    bool Create(const char* title, u32 width, u32 height, bool fullscreen);
    void Destroy();
    void* GetNativeHandle() const;
    void SetTitle(const char* title);
    void SetFullscreen(bool fullscreen);
    void GetSize(u32& width, u32& height) const;
    bool ShouldClose() const;
    void PollEvents();
    void SwapBuffers();

private:
    struct Impl;
    Impl* m_Impl = nullptr;
};
} // namespace Platform
} // namespace grom