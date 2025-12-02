#pragma once

#include "../UIBase.h"
#include "../../Core/Object/UObject.h"
#include <string>
#include <vector>

namespace UI {

// ============================================================================
// DetailsPanel - Panel de propiedades/editables (estilo UE5)
// Muestra y permite editar propiedades de objetos seleccionados
// ============================================================================

class DetailsPanel : public IWindow {
public:
    DetailsPanel();
    virtual ~DetailsPanel() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Seleccionar objeto para mostrar propiedades
    void SetSelectedObject(UObject* obj) { selectedObject = obj; }
    UObject* GetSelectedObject() const { return selectedObject; }
    
    // Agregar propiedad personalizada
    void AddProperty(const std::string& name, const std::string& value);
    void ClearProperties();

private:
    UObject* selectedObject = nullptr;
    
    struct Property {
        std::string name;
        std::string value;
        std::string type;
    };
    std::vector<Property> customProperties;
    
    void RenderObjectProperties(UObject* obj);
    void RenderPropertyField(const std::string& name, const std::string& value);
};

} // namespace UI

