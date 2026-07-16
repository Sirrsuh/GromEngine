#include <UI/UIElement.h>
#include <UI/UICanvas.h>

namespace grom
{

u64 UIElement::s_NextID = 1;

UIElement::UIElement()
    : m_ID(s_NextID++)
    , m_Position(0.0f, 0.0f)
    , m_Size(100.0f, 100.0f)
    , m_AnchorMin(0.0f, 0.0f)
    , m_AnchorMax(1.0f, 1.0f)
    , m_Anchor(EUIAnchor::TopLeft)
    , m_Style{}
    , m_Parent(nullptr)
    , m_Visible(true)
    , m_Enabled(true)
{
}

UIElement::~UIElement()
{
    for (usize i = 0; i < m_Children.Size(); ++i)
    {
        m_Children[i]->m_Parent = nullptr;
    }
}

void UIElement::Render(UICanvas* canvas)
{
    if (!m_Visible) return;

    for (usize i = 0; i < m_Children.Size(); ++i)
    {
        m_Children[i]->Render(canvas);
    }
}

void UIElement::Update(f32 deltaTime)
{
    for (usize i = 0; i < m_Children.Size(); ++i)
    {
        m_Children[i]->Update(deltaTime);
    }
}

void UIElement::SetPosition(const GVec2& pos)
{
    m_Position = pos;
}

void UIElement::SetSize(const GVec2& size)
{
    m_Size = size;
}

GVec2 UIElement::GetAbsolutePosition() const
{
    if (m_Parent)
    {
        GVec2 parentPos = m_Parent->GetAbsolutePosition();
        return parentPos + m_Position;
    }
    return m_Position;
}

GVec2 UIElement::GetAbsoluteSize() const
{
    return m_Size;
}

void UIElement::AddChild(UIElement* child)
{
    if (child->m_Parent)
    {
        child->m_Parent->RemoveChild(child);
    }
    child->m_Parent = this;
    m_Children.Add(child);
}

void UIElement::RemoveChild(UIElement* child)
{
    for (usize i = 0; i < m_Children.Size(); ++i)
    {
        if (m_Children[i] == child)
        {
            child->m_Parent = nullptr;
            m_Children.RemoveAt(i);
            return;
        }
    }
}

bool UIElement::ContainsPoint(const GVec2& point)
{
    GVec2 absPos = GetAbsolutePosition();
    GVec2 absSize = GetAbsoluteSize();
    return point.x >= absPos.x && point.x <= absPos.x + absSize.x &&
           point.y >= absPos.y && point.y <= absPos.y + absSize.y;
}

}
