#include "ObjectHierarchyPanel.h"
#include "../../Core/Log.h"
#include "../../Core/Object/UObject.h"
#include <sstream>

namespace UI {

ObjectHierarchyPanel::ObjectHierarchyPanel() {
    SetTitle("Object Hierarchy");
    SetBounds(FRect(420, 10, 350, 400));
    SetVisible(false); // Oculto por defecto
}

void ObjectHierarchyPanel::Render() {
    if (!IsVisible()) return;
    
    // Implementación básica - muestra lista de objetos
    static int logCounter = 0;
    if (++logCounter % 180 == 0) { // Log cada 3 segundos
        std::stringstream ss;
        ss << "=== Object Hierarchy ===\n";
        ss << "Objects: " << objectList.size() << "\n";
        
        for (size_t i = 0; i < objectList.size() && i < 10; i++) {
            if (objectList[i]) {
                ss << "  [" << objectList[i]->GetUniqueID() << "] " 
                   << objectList[i]->GetName() << "\n";
            }
        }
        
        if (objectList.size() > 10) {
            ss << "  ... and " << (objectList.size() - 10) << " more\n";
        }
        
        if (selectedObject) {
            ss << "\nSelected: [" << selectedObject->GetUniqueID() << "] " 
               << selectedObject->GetName() << "\n";
        }
        
        UE_LOG_VERBOSE(LogCategories::Core, "%s", ss.str().c_str());
    }
}

void ObjectHierarchyPanel::Update(float deltaTime) {
    // Update logic
}

void ObjectHierarchyPanel::UpdateObjectList(const std::vector<UObject*>& objects) {
    objectList = objects;
}

} // namespace UI

