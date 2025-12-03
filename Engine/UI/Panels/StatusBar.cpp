#include "StatusBar.h"
#include "../../Core/Log.h"
#include <cstdio>

namespace UI {

StatusBar::StatusBar() : currentStatus("Ready"), currentFPS(0.0f), currentFrameTime(0.0f) {
    SetVisible(true); // StatusBar siempre visible
}

void StatusBar::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void StatusBar::Update(float deltaTime) {
    // StatusBar actualiza su información desde el exterior
}

void StatusBar::SetStatus(const char* status) {
    if (status) {
        currentStatus = status;
    }
}

void StatusBar::SetFPS(float fps) {
    currentFPS = fps;
}

void StatusBar::SetFrameTime(float frameTime) {
    currentFrameTime = frameTime;
}

} // namespace UI
