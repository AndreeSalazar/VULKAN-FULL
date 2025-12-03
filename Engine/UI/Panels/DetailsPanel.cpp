#include "DetailsPanel.h"
#include "../../Core/Log.h"
#include "../../Core/Object/UObject.h"
#include "../../Core/Object/UClass.h"

namespace UI {

DetailsPanel::DetailsPanel() {
    SetTitle("Details");
    SetResizable(true);
    SetMovable(true);
}

void DetailsPanel::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void DetailsPanel::Update(float deltaTime) {
    // Update logic
}

void DetailsPanel::AddProperty(const std::string& name, const std::string& value) {
    Property prop;
    prop.name = name;
    prop.value = value;
    prop.type = "string";
    customProperties.push_back(prop);
}

void DetailsPanel::ClearProperties() {
    customProperties.clear();
}

void DetailsPanel::RenderObjectProperties(UObject* obj) {
    // TODO: Implementar con eGUI
}

void DetailsPanel::RenderPropertyField(const std::string& name, const std::string& value) {
    // TODO: Implementar con eGUI
}

} // namespace UI
