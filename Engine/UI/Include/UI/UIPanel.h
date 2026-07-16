#pragma once
#include <UI/UIElement.h>

namespace grom
{

class UIPanel : public UIElement
{
public:
    UIPanel();
    virtual ~UIPanel() override;

    virtual void Render(UICanvas* canvas) override;
};

}
