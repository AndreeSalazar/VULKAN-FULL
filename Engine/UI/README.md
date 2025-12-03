# UI System - Sistema de Interfaz Gráfica Unificado

## 📋 Descripción

Sistema de interfaz gráfica unificado para el motor Vulkan, combinando:
- **C++ Backend**: Wrappers, managers y lógica del motor
- **Rust Frontend**: eGUI para renderizado moderno de UI

## 🏗️ Estructura Unificada

```
Engine/UI/
├── Rust/                    # Frontend UI en Rust + eGUI
│   ├── Cargo.toml
│   ├── src/lib.rs          # Funciones FFI para eGUI
│   └── cbindgen.toml
│
├── Panels/                  # Paneles UI (C++ - lógica)
│   ├── MenuBar
│   ├── StatusBar
│   ├── StatsPanel
│   ├── ViewportPanel
│   ├── DetailsPanel
│   ├── ContentBrowserPanel
│   ├── ConsolePanel
│   ├── ObjectHierarchyPanel
│   └── DebugOverlay
│
├── Scripting/               # Scripting UI (Lua)
│   └── LuaUI
│
├── EGUIWrapper.h/cpp       # Bridge C++ ↔ Rust (FFI)
├── UIManager.h/cpp         # Gestor central de UI
├── UIBase.h/cpp            # Clases base (IPanel, IWindow, etc.)
└── README.md               # Este archivo
```

## 🎯 Arquitectura

```
┌─────────────────────────────────────────┐
│         C++ Engine Core                 │
│  (Rendering, Game Logic, Systems)       │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│      UI Manager (C++)                   │
│  - UIManager                            │
│  - Panels (lógica C++)                  │
└──────────────┬──────────────────────────┘
               │ FFI Bridge
┌──────────────▼──────────────────────────┐
│      eGUI Frontend (Rust)               │
│  - Engine/UI/Rust/src/lib.rs            │
│  - Renderizado con eGUI                 │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│      Vulkan Renderer (C++)              │
│  - Comandos de renderizado              │
└─────────────────────────────────────────┘
```

## 🔧 Componentes Principales

### 1. **EGUIWrapper** (`EGUIWrapper.h/cpp`)
Bridge C++ ↔ Rust que expone funciones FFI para comunicación entre ambos lenguajes.

```cpp
// Inicializar eGUI desde C++
UI::EGUIWrapper::Get().Initialize(window, instance, ...);

// Renderizar frame
UI::EGUIWrapper::Get().Render(commandBuffer);
```

### 2. **UIManager** (`UIManager.h/cpp`)
Gestor central que coordina todos los paneles y ventanas.

```cpp
// Registrar panel
UI::UIManager::Get().RegisterPanel("Stats", statsPanel);

// Renderizar
UI::UIManager::Get().Render();
```

### 3. **Paneles** (`Panels/`)
Cada panel contiene la lógica C++ y llama a eGUI (Rust) para renderizado.

### 4. **Rust Frontend** (`Rust/`)
Implementación de eGUI en Rust que expone funciones FFI.

## 🚀 Uso

### Inicialización
```cpp
// En main.cpp
UI::EGUIWrapper::Get().Initialize(...);
UI::UIManager::Get().Initialize();

// Registrar paneles
UI::UIManager::Get().RegisterPanel("MenuBar", menuBar);
UI::UIManager::Get().RegisterWindow("Viewport", viewport);
```

### Loop Principal
```cpp
// Cada frame
UI::EGUIWrapper::Get().NewFrame();
UI::EGUIWrapper::Get().UpdateEngineState(fps, deltaTime, ...);
UI::UIManager::Get().Update(deltaTime);
UI::UIManager::Get().Render();
UI::EGUIWrapper::Get().Render(commandBuffer);
```

## 📝 Paneles Disponibles

- **MenuBar**: Barra de menús principal
- **StatusBar**: Barra de estado inferior
- **StatsPanel**: Estadísticas de rendimiento
- **ViewportPanel**: Vista 3D principal
- **DetailsPanel**: Propiedades de objetos
- **ContentBrowserPanel**: Explorador de assets
- **ConsolePanel**: Consola de comandos y logs
- **ObjectHierarchyPanel**: Jerarquía de objetos
- **DebugOverlay**: Overlay de debug

## 🔄 Estado Actual

✅ **Completado:**
- Estructura unificada (C++ + Rust)
- Bridge FFI implementado
- Paneles creados (stubs por ahora)
- Integración con CMake

⏳ **Pendiente:**
- Implementar renderizado real con eGUI en Rust
- Migrar paneles de stubs a implementación completa
- Estilo UE5 completo

## 📚 Recursos

- [eGUI Documentation](https://www.egui.rs/)
- [Rust FFI Guide](https://doc.rust-lang.org/nomicon/ffi.html)
- Ver `EGUI_INTEGRATION_PLAN.md` para detalles técnicos
