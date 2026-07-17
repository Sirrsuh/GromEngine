#pragma once
#include "../Platform.h"
#include <Core/Container.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace grom {

class LinuxWindow : public Window
{
public:
    LinuxWindow() = default;
    ~LinuxWindow() override;

    bool Create(const WindowDesc& desc) override;
    void Destroy() override;
    void Show() override;
    void Hide() override;
    void SetTitle(const GString& title) override;
    void SetSize(u32 width, u32 height) override;
    void* GetNativeHandle() const override { return m_Window; }

    bool ShouldClose() const override;
    void PollEvents() override;

private:
    void* m_Display = nullptr;
    void* m_Window = nullptr;
    bool m_ShouldClose = false;
};

class LinuxPlatform : public Platform
{
public:
    LinuxPlatform() = default;
    ~LinuxPlatform() override;

    bool Initialize() override;
    void Shutdown() override;
    Window* CreateWindow(const WindowDesc& desc) override;
    void PollEvents() override;
    double GetTime() const override;
    void Sleep(u32 ms) override;

    static LinuxPlatform* Create();

private:
    void* m_Display = nullptr;
    TArray<LinuxWindow*> m_Windows;
};

} // namespace grom