#include <UI/UIText.h>

namespace grom
{

UIText::UIText()
    : m_TextColor(GColor::White())
    , m_TextSize(14.0f)
    , m_Alignment(ETextAlignment::Left)
{
    m_Size = GVec2(200.0f, 20.0f);
}

UIText::~UIText()
{
}

void UIText::Render(UICanvas* canvas)
{
    if (!m_Visible) return;

    UIElement::Render(canvas);
}

}
