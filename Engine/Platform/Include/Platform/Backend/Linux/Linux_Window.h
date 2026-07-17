#pragma once
#include "../../../Platform/Platform_Window.h"

namespace grom {

class LinuxWindow : public Window
{
public:
    LinuxWindow() = default;
    ~LinuxWindow() override;

    bool Create(const WindowDesc& desc) override;
    void Destroy() override;
    void SetTitle(const GString& title) override;
    void SetSize(u32 width, u32 height) override;
    void* GetNativeHandle() const override;
    bool ShouldClose() const override;
    void PollEvents() override;

private:
    Display* m_Display = nullptr;
    Window m_Window = 0;
    Atom m_WMDeleteMessage = 0;
    bool m_ShouldClose = false;
};

} // namespace grom