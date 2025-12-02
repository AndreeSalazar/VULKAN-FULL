#include "DebugOverlay.h"
#include "../../Core/Log.h"
#include <sstream>
#include <iomanip>

namespace UI {

DebugOverlay::DebugOverlay() {
    SetBounds(FRect(10, 10, 300, 200));
}

void DebugOverlay::Render() {
    if (!IsVisible()) return;
    
    // En una implementación real con ImGui o similar, aquí se renderizaría el overlay
    // Por ahora, solo logueamos la información
    // En el futuro, esto se integrará con un renderer de UI (ImGui, etc.)
    
    // Esta es una implementación básica - se puede mejorar con un renderer real
    static int logCounter = 0;
    if (++logCounter % 60 == 0) { // Log cada 60 frames
        std::stringstream ss;
        ss << "=== Debug Overlay ===\n";
        
        if (bShowFPS) {
            ss << "FPS: " << std::fixed << std::setprecision(2) << currentFPS << "\n";
            ss << "Delta: " << (deltaTime * 1000.0f) << "ms\n";
        }
        
        if (bShowStats) {
            ss << "Frames: " << frameCount << "\n";
            ss << "Time: " << std::fixed << std::setprecision(2) << totalTime << "s\n";
        }
        
        if (bShowCameraInfo) {
            ss << "Camera: (" << std::fixed << std::setprecision(2) 
               << cameraX << ", " << cameraY << ", " << cameraZ << ")\n";
        }
        
        if (bShowRenderQueueInfo) {
            ss << "Render Queue: " << renderQueueSize << " commands\n";
        }
        
        UE_LOG_VERBOSE(LogCategories::Core, "%s", ss.str().c_str());
    }
}

void DebugOverlay::Update(float deltaTime) {
    // Update logic if needed
}

void DebugOverlay::SetCameraPosition(float x, float y, float z) {
    cameraX = x;
    cameraY = y;
    cameraZ = z;
}

} // namespace UI

