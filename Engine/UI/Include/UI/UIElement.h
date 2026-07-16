#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>

namespace grom
{

enum class EUIElementState { Normal, Hovered, Pressed, Disabled, Focused };

enum class EUIAnchor { TopLeft, TopCenter, TopRight, CenterLeft, Center, CenterRight, BottomLeft, BottomCenter, BottomRight, Stretch };

enum class EUILayoutType { None, Horizontal, Vertical, Grid, Absolute };

struct UIStyle
{
    GColor Background = GColor::Transparent();
    GColor Foreground = GColor::White();
    GColor BorderColor = GColor::Black();
    f32 BorderWidth = 0.0f;
    f32 CornerRadius = 0.0f;
    f32 Opacity = 1.0f;
};

class UICanvas;

class UIElement
{
public:
    UIElement();
    virtual ~UIElement();

    virtual void Render(UICanvas* canvas);
    virtual void Update(f32 deltaTime);

    void SetPosition(const GVec2& pos);
    void SetSize(const GVec2& size);
    GVec2 GetAbsolutePosition() const;
    GVec2 GetAbsoluteSize() const;

    void AddChild(UIElement* child);
    void RemoveChild(UIElement* child);
    UIElement* GetParent() const { return m_Parent; }
    const TArray<UIElement*>& GetChildren() const { return m_Children; }

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }
    void SetStyle(const UIStyle& style) { m_Style = style; }
    UIStyle& GetStyle() { return m_Style; }
    const UIStyle& GetStyle() const { return m_Style; }

    void SetName(const GString& name) { m_Name = name; }
    const GString& GetName() const { return m_Name; }

    virtual bool ContainsPoint(const GVec2& point);

    u64 GetID() const { return m_ID; }

protected:
    u64 m_ID;
    GVec2 m_Position;
    GVec2 m_Size;
    GVec2 m_AnchorMin;
    GVec2 m_AnchorMax;
    EUIAnchor m_Anchor;
    UIStyle m_Style;
    UIElement* m_Parent;
    TArray<UIElement*> m_Children;
    bool m_Visible;
    bool m_Enabled;
    GString m_Name;
    static u64 s_NextID;
};

}
