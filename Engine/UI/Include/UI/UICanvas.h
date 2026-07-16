#pragma once
#include <UI/UIWidget.h>
#include <RHI/RHI.h>

namespace grom
{

class UICanvas : public UIElement
{
public:
    UICanvas();
    virtual ~UICanvas() override;

    bool Initialize(Device* device, u32 width, u32 height);
    void Resize(u32 width, u32 height);
    void Render(Device* device);
    virtual void Update(f32 deltaTime) override;

    void DispatchEvent(const GVec2& mousePos, bool mouseDown, bool mouseUp);

    UIElement* HitTest(const GVec2& point);

    Texture* GetRenderTarget() const { return m_RenderTarget; }
    u32 GetWidth() const { return m_Width; }
    u32 GetHeight() const { return m_Height; }

private:
    void DrawQuad(Device* device, f32 x, f32 y, f32 w, f32 h, const GColor& color);

    Texture* m_RenderTarget;
    Texture* m_DepthStencil;
    Pipeline* m_Pipeline;
    Buffer* m_VertexBuffer;
    Buffer* m_IndexBuffer;
    Shader* m_VertexShader;
    Shader* m_PixelShader;
    u32 m_Width;
    u32 m_Height;
    bool m_Initialized;

    UIWidget* m_HoveredWidget;
    UIWidget* m_PressedWidget;
};

}
