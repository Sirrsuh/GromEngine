#include <UI/UIWidget.h>

namespace grom
{

UIWidget::UIWidget()
    : m_State(EUIElementState::Normal)
    , m_Pressed(false)
{
}

UIWidget::~UIWidget()
{
}

void UIWidget::OnMouseEnter(const GVec2& pos)
{
    GROM_UNUSED(pos);
    if (m_Enabled)
    {
        m_State = EUIElementState::Hovered;
    }
}

void UIWidget::OnMouseLeave()
{
    m_State = EUIElementState::Normal;
    m_Pressed = false;
}

void UIWidget::OnMouseMove(const GVec2& pos)
{
    GROM_UNUSED(pos);
}

void UIWidget::OnMouseDown(const GVec2& pos)
{
    GROM_UNUSED(pos);
    if (m_Enabled)
    {
        m_State = EUIElementState::Pressed;
        m_Pressed = true;
        m_LastMousePos = pos;
    }
}

void UIWidget::OnMouseUp(const GVec2& pos)
{
    GROM_UNUSED(pos);
    if (m_Enabled)
    {
        m_State = EUIElementState::Hovered;
        m_Pressed = false;
    }
}

void UIWidget::OnClick()
{
}

void UIWidget::OnDrag(const GVec2& delta)
{
    GROM_UNUSED(delta);
}

}
