#include "DetailsPanel.h"
#include "../ImGuiWrapper.h"
#include "../../Core/Log.h"
#include "../../Core/Object/UObject.h"
#include "../../Core/Object/UClass.h"
#include <imgui.h>

namespace UI {

DetailsPanel::DetailsPanel() {
    SetTitle("Details");
    SetResizable(true);
    SetMovable(true);
}

void DetailsPanel::Render() {
    if (!IsVisible()) return;
    
    ImGuiWrapper& imgui = ImGuiWrapper::Get();
    if (!imgui.IsInitialized()) return;
    
    // Establecer contexto de ImGui
    ImGui::SetCurrentContext(imgui.GetContext());
    
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Details", &open, ImGuiWindowFlags_None)) {
        if (selectedObject) {
            // Header con nombre del objeto
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", selectedObject->GetName().c_str());
            ImGui::Separator();
            
            // Información básica
            if (ImGui::CollapsingHeader("Basic Info", ImGuiTreeNodeFlags_DefaultOpen)) {
                RenderPropertyField("Name", selectedObject->GetName());
                RenderPropertyField("Class", selectedObject->GetClass() ? selectedObject->GetClass()->GetName() : "Unknown");
                RenderPropertyField("Flags", std::to_string(static_cast<int>(selectedObject->GetFlags())));
            }
            
            // Renderizar propiedades del objeto
            RenderObjectProperties(selectedObject);
            
            // Propiedades personalizadas
            if (!customProperties.empty()) {
                ImGui::Separator();
                if (ImGui::CollapsingHeader("Custom Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (auto& prop : customProperties) {
                        RenderPropertyField(prop.name, prop.value);
                    }
                }
            }
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object selected");
            ImGui::Text("Select an object in the World Outliner");
        }
    }
    ImGui::End();
    
    if (!open) {
        SetVisible(false);
    }
}

void DetailsPanel::Update(float deltaTime) {
    // Update logic
}

void DetailsPanel::RenderObjectProperties(UObject* obj) {
    if (!obj) return;
    
    // Usar reflexión si está disponible
    if (obj->GetClass()) {
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Aquí se podrían mostrar propiedades reflejadas
            // Por ahora solo mostramos información básica
            RenderPropertyField("Has Class", "Yes");
        }
    }
}

void DetailsPanel::RenderPropertyField(const std::string& name, const std::string& value) {
    ImGui::Text("%s:", name.c_str());
    ImGui::SameLine(150);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", value.c_str());
}

void DetailsPanel::AddProperty(const std::string& name, const std::string& value) {
    Property prop;
    prop.name = name;
    prop.value = value;
    customProperties.push_back(prop);
}

void DetailsPanel::ClearProperties() {
    customProperties.clear();
}

} // namespace UI

