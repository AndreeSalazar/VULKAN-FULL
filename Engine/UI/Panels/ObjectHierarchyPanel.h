#pragma once

#include "../UIBase.h"
#include "../../Core/Object/UObject.h"
#include <vector>

// ============================================================================
// ObjectHierarchyPanel - Panel para mostrar jerarquía de objetos UObject
// ============================================================================

namespace UI {

class ObjectHierarchyPanel : public IWindow {
public:
    ObjectHierarchyPanel();
    virtual ~ObjectHierarchyPanel() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Actualizar lista de objetos
    void UpdateObjectList(const std::vector<class UObject*>& objects);
    
    // Selección
    void SetSelectedObject(class UObject* obj) { selectedObject = obj; }
    class UObject* GetSelectedObject() const { return selectedObject; }
    
    // Callback cuando se selecciona un objeto
    void SetOnObjectSelected(std::function<void(class UObject*)> callback) {
        onObjectSelected = callback;
    }

private:
    std::vector<class UObject*> objectList;
    class UObject* selectedObject = nullptr;
    std::function<void(class UObject*)> onObjectSelected;
};

} // namespace UI

