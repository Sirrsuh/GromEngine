#include <UI/UIButton.h>

namespace grom
{

UIButton::UIButton()
    : m_ClickCallback(nullptr)
    , m_NormalColor(GColor(60, 60, 60, 255))
    , m_HoverColor(GColor(80, 80, 80, 255))
    , m_PressedColor(GColor(40, 40, 40, 255))
    , m_DisabledColor(GColor(30, 30, 30, 128))
{
    m_Size = GVec2(120.0f, 30.0f);
    m_Style.Background = m_NormalColor;
}

UIButton::~UIButton()
{
}

void UIButton::OnClick()
{
    if (m_ClickCallback)
    {
        m_ClickCallback(this);
    }
}

void UIButton::Render(UICanvas* canvas)
{
    if (!m_Visible) return;

    switch (m_State)
    {
    case EUIElementState::Normal:
        m_Style.Background = m_NormalColor;
        break;
    case EUIElementState::Hovered:
        m_Style.Background = m_HoverColor;
        break;
    case EUIElementState::Pressed:
        m_Style.Background = m_PressedColor;
        break;
    case EUIElementState::Disabled:
        m_Style.Background = m_DisabledColor;
        break;
    default:
        m_Style.Background = m_NormalColor;
        break;
    }

    UIElement::Render(canvas);
}

}
