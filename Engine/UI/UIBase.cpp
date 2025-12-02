#include "UIBase.h"

namespace UI {

void IPanel::AddWidget(std::shared_ptr<IWidget> widget) {
    if (widget) {
        widgets.push_back(widget);
    }
}

void IPanel::RemoveWidget(std::shared_ptr<IWidget> widget) {
    widgets.erase(
        std::remove_if(widgets.begin(), widgets.end(),
            [&widget](const std::shared_ptr<IWidget>& w) {
                return w == widget;
            }),
        widgets.end()
    );
}

void IPanel::ClearWidgets() {
    widgets.clear();
}

void IPanel::Render() {
    if (!IsVisible()) return;
    
    // Render all child widgets
    for (auto& widget : widgets) {
        if (widget && widget->IsVisible()) {
            widget->Render();
        }
    }
}

void IPanel::Update(float deltaTime) {
    if (!IsEnabled()) return;
    
    // Update all child widgets
    for (auto& widget : widgets) {
        if (widget && widget->IsEnabled()) {
            widget->Update(deltaTime);
        }
    }
}

} // namespace UI

