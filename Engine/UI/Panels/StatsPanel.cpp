#include "StatsPanel.h"
#include "../../Core/Log.h"
#include <sstream>
#include <iomanip>

namespace UI {

StatsPanel::StatsPanel() {
    SetTitle("Statistics");
    SetBounds(FRect(10, 250, 400, 300));
    SetVisible(false); // Oculto por defecto
}

void StatsPanel::Render() {
    if (!IsVisible()) return;
    
    // Implementación básica - se puede mejorar con renderer real
    static int logCounter = 0;
    if (++logCounter % 120 == 0) { // Log cada 2 segundos
        std::stringstream ss;
        ss << "=== Stats Panel ===\n";
        ss << "FPS: " << std::fixed << std::setprecision(2) << currentFPS << "\n";
        ss << "Delta Time: " << (deltaTime * 1000.0f) << "ms\n";
        ss << "Frame Count: " << frameCount << "\n";
        ss << "Total Time: " << std::fixed << std::setprecision(2) << totalTime << "s\n";
        
        UE_LOG_VERBOSE(LogCategories::Core, "%s", ss.str().c_str());
    }
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

