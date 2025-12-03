#pragma once

#include "UIBase.h"

namespace UI {

class MenuBar : public IPanel {
public:
    MenuBar();
    virtual ~MenuBar() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
private:
    void RenderFileMenu();
    void RenderEditMenu();
    void RenderViewMenu();
    void RenderWindowMenu();
    void RenderHelpMenu();
};

} // namespace UI

