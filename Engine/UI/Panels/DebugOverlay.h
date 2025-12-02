#pragma once

#include "../UIBase.h"

// ============================================================================
// DebugOverlay - Overlay de debug (FPS, stats, etc.)
// ============================================================================

namespace UI {

class DebugOverlay : public IPanel {
public:
    DebugOverlay();
    virtual ~DebugOverlay() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Configuración
    void SetShowFPS(bool show) { bShowFPS = show; }
    void SetShowStats(bool show) { bShowStats = show; }
    void SetShowCameraInfo(bool show) { bShowCameraInfo = show; }
    void SetShowRenderQueueInfo(bool show) { bShowRenderQueueInfo = show; }
    
    // Datos a mostrar
    void SetFPS(float fps) { currentFPS = fps; }
    void SetDeltaTime(float dt) { deltaTime = dt; }
    void SetFrameCount(uint64_t count) { frameCount = count; }
    void SetTotalTime(float time) { totalTime = time; }
    void SetCameraPosition(float x, float y, float z);
    void SetRenderQueueSize(size_t size) { renderQueueSize = size; }

private:
    // Visibility flags
    bool bShowFPS = true;
    bool bShowStats = true;
    bool bShowCameraInfo = true;
    bool bShowRenderQueueInfo = true;
    
    // Data
    float currentFPS = 0.0f;
    float deltaTime = 0.0f;
    uint64_t frameCount = 0;
    float totalTime = 0.0f;
    float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 0.0f;
    size_t renderQueueSize = 0;
    
    // Position
    float overlayX = 10.0f;
    float overlayY = 10.0f;
};

} // namespace UI

