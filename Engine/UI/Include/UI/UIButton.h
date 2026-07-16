#pragma once
#include <UI/UIWidget.h>

namespace grom
{

using UIClickCallback = void(*)(UIWidget*);

class UIButton : public UIWidget
{
public:
    UIButton();
    virtual ~UIButton() override;

    virtual void OnClick() override;

    void SetClickCallback(UIClickCallback callback) { m_ClickCallback = callback; }
    UIClickCallback GetClickCallback() const { return m_ClickCallback; }

    void SetNormalColor(const GColor& color) { m_NormalColor = color; }
    void SetHoverColor(const GColor& color) { m_HoverColor = color; }
    void SetPressedColor(const GColor& color) { m_PressedColor = color; }
    void SetDisabledColor(const GColor& color) { m_DisabledColor = color; }

    const GColor& GetNormalColor() const { return m_NormalColor; }
    const GColor& GetHoverColor() const { return m_HoverColor; }
    const GColor& GetPressedColor() const { return m_PressedColor; }
    const GColor& GetDisabledColor() const { return m_DisabledColor; }

    void SetLabel(const GString& label) { m_Label = label; }
    const GString& GetLabel() const { return m_Label; }

    virtual void Render(UICanvas* canvas) override;

private:
    UIClickCallback m_ClickCallback;
    GColor m_NormalColor;
    GColor m_HoverColor;
    GColor m_PressedColor;
    GColor m_DisabledColor;
    GString m_Label;
};

}
