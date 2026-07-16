#pragma once
#include <UI/UIElement.h>

namespace grom
{

class UIWidget : public UIElement
{
public:
    UIWidget();
    virtual ~UIWidget() override;

    virtual void OnMouseEnter(const GVec2& pos);
    virtual void OnMouseLeave();
    virtual void OnMouseMove(const GVec2& pos);
    virtual void OnMouseDown(const GVec2& pos);
    virtual void OnMouseUp(const GVec2& pos);
    virtual void OnClick();
    virtual void OnDrag(const GVec2& delta);

    EUIElementState GetState() const { return m_State; }
    bool IsHovered() const { return m_State == EUIElementState::Hovered; }
    bool IsPressed() const { return m_Pressed; }

protected:
    EUIElementState m_State;
    bool m_Pressed;
    GVec2 m_LastMousePos;
};

}
