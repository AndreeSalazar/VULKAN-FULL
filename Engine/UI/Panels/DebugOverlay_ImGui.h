#pragma once

#include "../UIBase.h"

// ============================================================================
// DebugOverlay_ImGui - Debug overlay usando ImGui
// ============================================================================

namespace UI {

class DebugOverlay_ImGui : public IPanel {
public:
    DebugOverlay_ImGui();
    virtual ~DebugOverlay_ImGui() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Datos a mostrar
    void SetFPS(float fps) { currentFPS = fps; }
    void SetDeltaTime(float dt) { deltaTime = dt; }
    void SetFrameCount(uint64_t count) { frameCount = count; }
    void SetTotalTime(float time) { totalTime = time; }
    void SetCameraPosition(float x, float y, float z) {
        cameraX = x; cameraY = y; cameraZ = z;
    }
    void SetRenderQueueSize(size_t size) { renderQueueSize = size; }

private:
    float currentFPS = 0.0f;
    float deltaTime = 0.0f;
    uint64_t frameCount = 0;
    float totalTime = 0.0f;
    float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 0.0f;
    size_t renderQueueSize = 0;
    bool bShowOverlay = true;
};

} // namespace UI

