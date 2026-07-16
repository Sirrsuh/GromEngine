#include <UI/UIPanel.h>

namespace grom
{

UIPanel::UIPanel()
{
    m_Style.Background = GColor(30, 30, 30, 200);
}

UIPanel::~UIPanel()
{
}

void UIPanel::Render(UICanvas* canvas)
{
    if (!m_Visible) return;

    UIElement::Render(canvas);
}

}
