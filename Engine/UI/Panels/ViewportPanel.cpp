#include "ViewportPanel.h"
#include "../ImGuiWrapper.h"
#include "../../Core/Log.h"
#include <imgui.h>

namespace UI {

ViewportPanel::ViewportPanel() {
    SetTitle("Viewport");
    SetResizable(true);
    SetMovable(false); // Viewport normalmente está fijo
}

void ViewportPanel::Render() {
    if (!IsVisible()) return;
    
    ImGuiWrapper& imgui = ImGuiWrapper::Get();
    if (!imgui.IsInitialized()) return;
    
    // Establecer contexto de ImGui (requerido antes de cualquier llamada ImGui)
    ImGui::SetCurrentContext(imgui.GetContext());
    
    // Crear ventana de viewport con flags estilo UE5
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    bool open = true;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Viewport", &open, flags)) {
        // Obtener tamaño del contenido disponible
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        if (contentSize.x > 0 && contentSize.y > 0) {
            viewportWidth = static_cast<uint32_t>(contentSize.x);
            viewportHeight = static_cast<uint32_t>(contentSize.y);
        }
        
        // Verificar si el mouse está sobre el viewport
        bMouseOverViewport = ImGui::IsWindowHovered();
        bViewportFocused = ImGui::IsWindowFocused();
        
        // Área donde se renderiza la escena
        // Por ahora mostramos un placeholder, luego se renderiza la escena aquí
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        // Dibujar fondo (placeholder para la escena renderizada)
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImU32 bgColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + viewportSize.x, cursorPos.y + viewportSize.y), bgColor);
        
        // Renderizar la escena aquí (si hay callback)
        if (renderCallback) {
            renderCallback();
        }
        
        // Toolbar del viewport (estilo UE5)
        if (ImGui::BeginChild("ViewportToolbar", ImVec2(0, 30), false)) {
            ImGui::Button("View");
            ImGui::SameLine();
            ImGui::Button("Camera");
            ImGui::SameLine();
            ImGui::Button("Show");
            ImGui::SameLine();
            ImGui::Checkbox("Grid", &bShowGrid);
            ImGui::SameLine();
            ImGui::Checkbox("Gizmos", &bShowGizmos);
        }
        ImGui::EndChild();
        
        // Info del viewport (esquina inferior derecha)
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + viewportSize.y - 30);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%dx%d", viewportWidth, viewportHeight);
    }
    ImGui::End();
    
    if (!open) {
        SetVisible(false);
    }
}

void ViewportPanel::Update(float deltaTime) {
    // Update logic
}

} // namespace UI

