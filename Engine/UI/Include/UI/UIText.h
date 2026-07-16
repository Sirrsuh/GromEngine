#pragma once
#include <UI/UIElement.h>

namespace grom
{

enum class ETextAlignment { Left, Center, Right };

class UIText : public UIElement
{
public:
    UIText();
    virtual ~UIText() override;

    virtual void Render(UICanvas* canvas) override;

    void SetText(const GString& text) { m_Text = text; }
    const GString& GetText() const { return m_Text; }

    void SetTextColor(const GColor& color) { m_TextColor = color; }
    const GColor& GetTextColor() const { return m_TextColor; }

    void SetTextSize(f32 size) { m_TextSize = size; }
    f32 GetTextSize() const { return m_TextSize; }

    void SetAlignment(ETextAlignment align) { m_Alignment = align; }
    ETextAlignment GetAlignment() const { return m_Alignment; }

private:
    GString m_Text;
    GColor m_TextColor;
    f32 m_TextSize;
    ETextAlignment m_Alignment;
};

}
