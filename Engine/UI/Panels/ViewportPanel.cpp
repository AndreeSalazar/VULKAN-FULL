#include "ViewportPanel.h"
#include "../../Core/Log.h"

namespace UI {

ViewportPanel::ViewportPanel() {
    SetTitle("Viewport");
    SetResizable(true);
    SetMovable(false); // Viewport normalmente está fijo
}

void ViewportPanel::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void ViewportPanel::Update(float deltaTime) {
    // Update logic
}

} // namespace UI
