# UI System - Sistema de Interfaz Gráfica

## 📋 Descripción

Sistema de interfaz gráfica base para el motor, diseñado para facilitar la visualización y gestión del motor de forma visual e intuitiva.

## 🎯 Propósito

El sistema UI permite:
- **Visualización**: Ver información del motor (FPS, stats, objetos)
- **Gestión**: Interactuar con objetos y sistemas del motor
- **Debug**: Panel de debug overlay para desarrollo
- **Extensibilidad**: Fácil agregar nuevos paneles y widgets

## 🔧 Componentes

### 1. UIBase (`UIBase.h/cpp`)
Sistema base para widgets, paneles y ventanas:
- `IWidget`: Clase base para todos los widgets
- `IPanel`: Panel que puede contener widgets
- `IWindow`: Ventana con título y contenido
- `FRect`: Rectángulo para layout
- `FColor`: Color RGBA
- `EWidgetState`: Estados de widgets (Normal, Hovered, Pressed, Disabled)

### 2. UIManager (`UIManager.h/cpp`)
Gestor principal de la interfaz:
- Singleton para acceso global
- Registro y gestión de paneles/ventanas
- Control de visibilidad
- Update y Render centralizados

**Uso**:
```cpp
// Inicializar
UI::UIManager::Get().Initialize();

// Registrar panel
auto panel = std::make_shared<UI::MyPanel>();
UI::UIManager::Get().RegisterPanel("MyPanel", panel);

// Mostrar/Ocultar
UI::UIManager::Get().ShowPanel("MyPanel");
UI::UIManager::Get().HidePanel("MyPanel");
UI::UIManager::Get().TogglePanel("MyPanel");

// En main loop
UI::UIManager::Get().Update(deltaTime);
UI::UIManager::Get().Render();
```

### 3. Paneles Incluidos

#### DebugOverlay
Overlay de debug (siempre visible, opcionalmente):
- FPS actual
- Delta time
- Frame count
- Total time
- Posición de cámara
- Tamaño de Render Queue

#### StatsPanel
Panel de estadísticas detalladas:
- FPS
- Delta time
- Frame count
- Total time

#### ObjectHierarchyPanel
Panel para mostrar jerarquía de objetos UObject:
- Lista de objetos
- Selección de objetos
- Información de objetos seleccionados

## 🚀 Crear un Nuevo Panel

```cpp
// MyCustomPanel.h
#pragma once
#include "UIBase.h"

namespace UI {
    class MyCustomPanel : public IPanel {
    public:
        MyCustomPanel();
        virtual ~MyCustomPanel() = default;
        
        virtual void Render() override;
        virtual void Update(float deltaTime) override;
        
        // Métodos personalizados
        void SetData(const std::string& data);
        
    private:
        std::string myData;
    };
}

// MyCustomPanel.cpp
#include "MyCustomPanel.h"
#include "../../Core/Log.h"

namespace UI {
    MyCustomPanel::MyCustomPanel() {
        SetBounds(FRect(10, 10, 400, 300));
        SetVisible(false);
    }
    
    void MyCustomPanel::Render() {
        if (!IsVisible()) return;
        
        // Renderizar panel (por ahora, logging básico)
        UE_LOG_INFO(LogCategories::Core, "MyCustomPanel: %s", myData.c_str());
    }
    
    void MyCustomPanel::Update(float deltaTime) {
        // Actualizar lógica
    }
    
    void MyCustomPanel::SetData(const std::string& data) {
        myData = data;
    }
}
```

## 📝 Integración en main.cpp

```cpp
#include "UI/UIManager.h"
#include "UI/Panels/DebugOverlay.h"

// En initVulkan() o mainLoop()
UI::UIManager::Get().Initialize();
UI::UIManager::Get().RegisterPanel("DebugOverlay", 
                                   std::make_shared<UI::DebugOverlay>());

// En mainLoop()
UI::UIManager::Get().Update(deltaTime);
UI::UIManager::Get().Render();

// En cleanup()
UI::UIManager::Get().Shutdown();
```

## 🎨 Renderizado Actual

**Nota**: El sistema actual usa logging básico para mostrar información. En el futuro, esto se integrará con un renderer de UI real como:
- **Dear ImGui**: Popular para overlays de debug
- **Nuklear**: Ligero y header-only
- **Custom Vulkan Renderer**: Renderer personalizado con Vulkan

## 🔮 Futuras Mejoras

- [ ] Integrar Dear ImGui para renderizado visual real
- [ ] Property Inspector panel (editar propiedades de UObject)
- [ ] Console panel (comandos del motor)
- [ ] Asset Browser panel
- [ ] Scene Outliner mejorado
- [ ] Toolbar y menús
- [ ] Temas y estilos personalizables
- [ ] Drag & Drop support
- [ ] Layout persistence (guardar posiciones de ventanas)

## ⌨️ Controles

- **TAB**: Toggle UI visibility (mostrar/ocultar paneles)
- **ESC**: Lock/Unlock mouse (cuando UI está visible)

## 📚 Arquitectura

```
UIManager (Singleton)
├── Panels (unordered_map)
│   ├── DebugOverlay
│   └── Custom Panels...
└── Windows (unordered_map)
    ├── StatsPanel
    ├── ObjectHierarchyPanel
    └── Custom Windows...
```

Cada panel/ventana:
- Puede contener widgets
- Tiene su propio Update() y Render()
- Controla su propia visibilidad
- Puede tener estados (hovered, pressed, etc.)

