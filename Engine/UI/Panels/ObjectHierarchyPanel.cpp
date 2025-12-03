#include "ObjectHierarchyPanel.h"
#include "../../Core/Log.h"
#include "../../Core/Object/UObject.h"
#include <sstream>

namespace UI {

ObjectHierarchyPanel::ObjectHierarchyPanel() {
    SetTitle("Object Hierarchy");
    SetResizable(true);
    SetMovable(true);
    // Visible por defecto para que aparezca inmediatamente
}

void ObjectHierarchyPanel::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void ObjectHierarchyPanel::Update(float deltaTime) {
    // Update logic
}

void ObjectHierarchyPanel::UpdateObjectList(const std::vector<UObject*>& objects) {
    objectList = objects;
}

} // namespace UI
