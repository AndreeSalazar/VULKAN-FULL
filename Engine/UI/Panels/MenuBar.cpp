#include "MenuBar.h"
#include "../../Core/Log.h"
#include "UIManager.h"

namespace UI {

MenuBar::MenuBar() {
    SetVisible(true); // MenuBar siempre visible
}

void MenuBar::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void MenuBar::RenderFileMenu() {
    // TODO: Implementar con eGUI
}

void MenuBar::RenderEditMenu() {
    // TODO: Implementar con eGUI
}

void MenuBar::RenderViewMenu() {
    // TODO: Implementar con eGUI
}

void MenuBar::RenderWindowMenu() {
    // TODO: Implementar con eGUI
}

void MenuBar::RenderHelpMenu() {
    // TODO: Implementar con eGUI
}

void MenuBar::Update(float deltaTime) {
    // MenuBar no necesita actualización lógica
}

} // namespace UI
