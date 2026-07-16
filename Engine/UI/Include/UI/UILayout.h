#pragma once
#include <UI/UIElement.h>

namespace grom
{

struct UILayoutParams
{
    EUILayoutType Type = EUILayoutType::None;
    f32 Spacing = 0.0f;
    GVec2 Padding = { 0.0f, 0.0f };
    GVec2 CellSize = { 0.0f, 0.0f };
    u32 GridColumns = 1;
};

class UILayout
{
public:
    static void ApplyHorizontal(UIElement* parent, const UILayoutParams& params);
    static void ApplyVertical(UIElement* parent, const UILayoutParams& params);
    static void ApplyGrid(UIElement* parent, const UILayoutParams& params);
    static void Apply(UIElement* parent, const UILayoutParams& params);
};

}
