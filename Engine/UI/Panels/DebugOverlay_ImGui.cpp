#include "DebugOverlay_ImGui.h"
#include "../ImGuiWrapper.h"
#include "../../Core/Log.h"
#include <sstream>
#include <iomanip>

namespace UI {

DebugOverlay_ImGui::DebugOverlay_ImGui() {
    SetBounds(FRect(10, 10, 300, 200));
}

void DebugOverlay_ImGui::Render() {
    if (!IsVisible() || !bShowOverlay) return;
    
    // Usar ImGui para renderizar
    ImGuiWrapper& imgui = ImGuiWrapper::Get();
    if (!imgui.IsInitialized()) {
        return;
    }
    
    // Establecer contexto de ImGui
    ImGui::SetCurrentContext(imgui.GetContext());
    
    // Establecer posición
    imgui.SetNextWindowPos(10, 10, ImGuiCond_FirstUseEver);
    imgui.SetNextWindowSize(300, 0, ImGuiCond_FirstUseEver);
    
    // Crear ventana
    bool open = true;
    if (imgui.Begin("Debug Overlay", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
        // FPS
        imgui.TextColored(0.0f, 1.0f, 0.0f, 1.0f, "FPS: %.1f", currentFPS);
        imgui.Text("Delta: %.3fms", deltaTime * 1000.0f);
        
        imgui.Separator();
        
        // Stats
        imgui.Text("Frames: %llu", (unsigned long long)frameCount);
        imgui.Text("Time: %.2fs", totalTime);
        
        imgui.Separator();
        
        // Camera
        imgui.TextColored(0.5f, 0.8f, 1.0f, 1.0f, "Camera:");
        imgui.Text("  Pos: (%.2f, %.2f, %.2f)", cameraX, cameraY, cameraZ);
        
        imgui.Separator();
        
        // Render Queue
        imgui.Text("Render Queue: %zu commands", renderQueueSize);
    }
    imgui.End();
    
    bShowOverlay = open;
}

void DebugOverlay_ImGui::Update(float deltaTime) {
    // Update logic if needed
}

} // namespace UI

