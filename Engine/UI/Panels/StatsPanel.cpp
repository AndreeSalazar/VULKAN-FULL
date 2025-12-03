#include "StatsPanel.h"
#include "../../Core/Log.h"
#include <sstream>
#include <iomanip>

namespace UI {

StatsPanel::StatsPanel() {
    SetTitle("Statistics");
    SetResizable(true);
    SetMovable(true);
    // Visible por defecto para que aparezca inmediatamente
}

void StatsPanel::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
    // La UI se renderizará desde Rust cuando esté implementada
}

void StatsPanel::Update(float deltaTime) {
    // Update logic
}

void StatsPanel::UpdateStats(float fps, float deltaTime, uint64_t frameCount, float totalTime) {
    this->currentFPS = fps;
    this->deltaTime = deltaTime;
    this->frameCount = frameCount;
    this->totalTime = totalTime;
}

} // namespace UI
