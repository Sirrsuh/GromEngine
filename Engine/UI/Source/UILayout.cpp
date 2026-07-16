#include <UI/UILayout.h>

namespace grom
{

void UILayout::ApplyHorizontal(UIElement* parent, const UILayoutParams& params)
{
    if (!parent) return;

    const TArray<UIElement*>& children = parent->GetChildren();
    if (children.IsEmpty()) return;

    f32 totalWidth = 0.0f;
    f32 maxHeight = 0.0f;
    for (usize i = 0; i < children.Size(); ++i)
    {
        if (!children[i]->IsVisible()) continue;
        GVec2 s = children[i]->GetAbsoluteSize();
        totalWidth += s.x;
        if (s.y > maxHeight) maxHeight = s.y;
    }
    totalWidth += params.Spacing * static_cast<f32>(children.Size() - 1);

    f32 offsetX = params.Padding.x;
    for (usize i = 0; i < children.Size(); ++i)
    {
        if (!children[i]->IsVisible()) continue;
        GVec2 s = children[i]->GetAbsoluteSize();
        children[i]->SetPosition(GVec2(offsetX, params.Padding.y));
        offsetX += s.x + params.Spacing;
    }
}

void UILayout::ApplyVertical(UIElement* parent, const UILayoutParams& params)
{
    if (!parent) return;

    const TArray<UIElement*>& children = parent->GetChildren();
    if (children.IsEmpty()) return;

    f32 offsetY = params.Padding.y;
    for (usize i = 0; i < children.Size(); ++i)
    {
        if (!children[i]->IsVisible()) continue;
        GVec2 s = children[i]->GetAbsoluteSize();
        children[i]->SetPosition(GVec2(params.Padding.x, offsetY));
        offsetY += s.y + params.Spacing;
    }
}

void UILayout::ApplyGrid(UIElement* parent, const UILayoutParams& params)
{
    if (!parent || params.GridColumns == 0) return;

    const TArray<UIElement*>& children = parent->GetChildren();
    if (children.IsEmpty()) return;

    u32 col = 0;
    u32 row = 0;
    f32 cellW = params.CellSize.x;
    f32 cellH = params.CellSize.y;

    for (usize i = 0; i < children.Size(); ++i)
    {
        if (!children[i]->IsVisible()) continue;

        f32 x = params.Padding.x + static_cast<f32>(col) * (cellW + params.Spacing);
        f32 y = params.Padding.y + static_cast<f32>(row) * (cellH + params.Spacing);
        children[i]->SetPosition(GVec2(x, y));

        ++col;
        if (col >= params.GridColumns)
        {
            col = 0;
            ++row;
        }
    }
}

void UILayout::Apply(UIElement* parent, const UILayoutParams& params)
{
    switch (params.Type)
    {
    case EUILayoutType::Horizontal:
        ApplyHorizontal(parent, params);
        break;
    case EUILayoutType::Vertical:
        ApplyVertical(parent, params);
        break;
    case EUILayoutType::Grid:
        ApplyGrid(parent, params);
        break;
    default:
        break;
    }
}

}
