# 🦀 Plan de Integración: eGUI (Rust) como Frontend UI

## 🎯 Visión General

**Arquitectura Propuesta:**
```
┌─────────────────────────────────────────────────┐
│         Motor Gráfico (C++/Vulkan)              │
│  - Rendering Engine                             │
│  - Core Systems                                 │
│  - Game Logic                                   │
└────────────┬────────────────────────────────────┘
             │ FFI Bridge (C ABI)
             │
┌────────────▼────────────────────────────────────┐
│         UI Frontend (Rust/eGUI)                 │
│  - Immediate Mode GUI                           │
│  - MenuBar, StatusBar, Panels                   │
│  - Eventos de usuario                           │
└────────────┬────────────────────────────────────┘
             │
┌────────────▼────────────────────────────────────┐
│         Render Backend (Rust/ash)               │
│  - Vulkan bindings                              │
│  - Renderizado de UI                            │
│  - Texturas, buffers, pipelines                 │
└─────────────────────────────────────────────────┘
```

## ✅ Ventajas de eGUI sobre ImGui

1. **Lenguaje Moderno**: Rust ofrece seguridad de memoria sin GC
2. **Mejor Integración con Vulkan**: eGUI está diseñado para Rust/Vulkan
3. **Sin Problemas de Texturas**: Manejo más robusto del ciclo de vida
4. **FFI Robusto**: Rust tiene excelente soporte para FFI con C/C++
5. **Ecosistema Activo**: eGUI es activamente mantenido
6. **Estilo Moderno**: UI más moderna por defecto

## ⚠️ Consideraciones

1. **FFI Overhead**: Comunicación entre C++ y Rust (mínimo, pero existe)
2. **Build System**: Necesitas Rust toolchain + CMake
3. **Curva de Aprendizaje**: Aprender Rust (pero vale la pena)
4. **Dos Lenguajes**: Más complejidad en el proyecto

## 🔧 Arquitectura de Integración

### Opción 1: UI en Rust, Render en C++ (Recomendado)

```
C++ Engine Core
    ↓ (FFI)
Rust UI Layer (eGUI)
    ↓ (Comandos de render)
C++ Vulkan Renderer
    ↓
GPU
```

**Flujo:**
1. **C++ Engine** actualiza estado del motor
2. **Rust UI** (eGUI) genera comandos de dibujo UI
3. **C++ Vulkan** renderiza la UI (texturas/buffers compartidos)

### Opción 2: Todo Renderizado en Rust

```
C++ Engine Core
    ↓ (FFI)
Rust UI + Render (eGUI + ash)
    ↓
GPU
```

**Ventajas:**
- Más simple (un solo render path)
- Mejor integración eGUI/Vulkan

**Desventajas:**
- Necesitas portar el renderer a Rust o usar ash (wrapper de Vulkan)

## 📋 Plan de Implementación

### Fase 1: Setup FFI Bridge

#### 1.1 Instalar Rust y dependencias

```bash
# Instalar Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Crear librería Rust
cd Engine
cargo new --lib RustUI
cd RustUI
```

#### 1.2 Configurar Cargo.toml

```toml
[package]
name = "engine-ui"
version = "0.1.0"
edition = "2021"

[lib]
crate-type = ["cdylib", "staticlib"]  # Compilar como biblioteca compartida

[dependencies]
egui = "0.24"
# egui_ash (backend de Vulkan para eGUI)
# cxx = "1.0"  # Bridge C++ <-> Rust
# cbindgen = "0.24"  # Generar headers C
```

### Fase 2: FFI Functions (Rust → C++)

#### 2.1 Exponer funciones desde Rust

```rust
// Engine/RustUI/src/lib.rs

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

// Structs compartidos (deben coincidir en C++ y Rust)
#[repr(C)]
pub struct EngineState {
    pub fps: f32,
    pub frame_time: f32,
    // ... otros campos
}

// Inicializar eGUI
#[no_mangle]
pub extern "C" fn egui_init(window_handle: *mut c_void, vulkan_instance: *mut c_void) -> bool {
    // Inicializar eGUI con el contexto de Vulkan
    true
}

// Renderizar frame UI
#[no_mangle]
pub extern "C" fn egui_render(
    state: *const EngineState,
    command_buffer: *mut c_void,  // VkCommandBuffer como puntero
) -> bool {
    // eGUI genera comandos de dibujo
    // Retorna true si hay UI para renderizar
    true
}

// Procesar eventos de input
#[no_mangle]
pub extern "C" fn egui_handle_event(event_type: u32, x: f32, y: f32) {
    // Procesar mouse/keyboard events
}
```

#### 2.2 Generar headers C con cbindgen

```bash
cargo install cbindgen
cbindgen --config cbindgen.toml --crate engine-ui --output ../include/engine_ui.h
```

### Fase 3: Integración en C++

#### 3.1 Header C generado (engine_ui.h)

```cpp
// Engine/include/engine_ui.h (generado automáticamente)

extern "C" {
    struct EngineState {
        float fps;
        float frame_time;
    };
    
    bool egui_init(void* window_handle, void* vulkan_instance);
    bool egui_render(const EngineState* state, void* command_buffer);
    void egui_handle_event(uint32_t event_type, float x, float y);
}
```

#### 3.2 Wrapper C++ (Engine/UI/EGUIWrapper.cpp)

```cpp
// Engine/UI/EGUIWrapper.h
#pragma once

#include "../include/engine_ui.h"  // Header generado por cbindgen
#include <vulkan/vulkan.h>

namespace UI {
    class EGUIWrapper {
    public:
        static EGUIWrapper& Get();
        
        bool Initialize(GLFWwindow* window, VkInstance instance, ...);
        void Render(VkCommandBuffer commandBuffer, float fps, float frameTime);
        void HandleEvent(uint32_t type, float x, float y);
        
    private:
        EngineState engineState;
        bool bInitialized = false;
    };
}
```

#### 3.3 Integración en main.cpp

```cpp
// Source/main.cpp

#include "Engine/UI/EGUIWrapper.h"

void mainLoop() {
    // ...
    
    // Actualizar estado del motor
    UI::EGUIWrapper::Get().UpdateEngineState(
        frameTimer.GetFPS(),
        deltaTime
    );
    
    // Renderizar UI (Rust genera comandos, C++ los ejecuta)
    UI::EGUIWrapper::Get().Render(commandBuffer);
    
    // ...
}
```

### Fase 4: CMake Integration

```cmake
# CMakeLists.txt

# Rust integration
find_program(CARGO cargo)
if(CARGO)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/libengine_ui.a
        COMMAND cargo build --release --manifest-path ${CMAKE_SOURCE_DIR}/Engine/RustUI/Cargo.toml
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Engine/RustUI
    )
    add_library(engine_ui STATIC IMPORTED)
    set_target_properties(engine_ui PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libengine_ui.a
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE engine_ui)
endif()
```

## 🎨 Ventajas para el Estilo UE5

1. **UI Más Moderna**: eGUI tiene un estilo más moderno por defecto
2. **Mejor Manejo de Texturas**: Sin los problemas que tenemos con ImGui
3. **Rust Safety**: Menos bugs de memoria en el código UI
4. **Modularidad**: UI separada del core del motor

## 📊 Comparación: ImGui vs eGUI

| Aspecto | ImGui (C++) | eGUI (Rust) |
|---------|-------------|-------------|
| **Lenguaje** | C++ | Rust |
| **Seguridad de Memoria** | Manual | Automática |
| **Integración Vulkan** | Compleja | Más simple |
| **Problemas de Texturas** | Sí (crash) | No (mejor manejo) |
| **Estilo por Defecto** | Básico | Más moderno |
| **FFI Overhead** | N/A | Mínimo |
| **Curva de Aprendizaje** | Baja | Media (Rust) |

## 🚀 Roadmap Sugerido

### Semana 1-2: Setup y Proof of Concept
- [ ] Instalar Rust y herramientas
- [ ] Crear proyecto Rust básico
- [ ] Implementar FFI básico (Rust ↔ C++)
- [ ] Mostrar un widget simple (botón)

### Semana 3-4: Integración Básica
- [ ] Integrar eGUI con Vulkan
- [ ] Renderizar UI básica en pantalla
- [ ] Procesar eventos de input

### Semana 5-6: Migración de Paneles
- [ ] Migrar MenuBar a Rust/eGUI
- [ ] Migrar StatusBar a Rust/eGUI
- [ ] Migrar StatsPanel a Rust/eGUI

### Semana 7-8: Sistema Completo
- [ ] Migrar todos los paneles
- [ ] Estilo UE5 con eGUI
- [ ] Testing y optimización

## 🔗 Recursos

- **eGUI**: https://github.com/emilk/egui
- **eGUI Book**: https://www.egui.rs/
- **cbindgen**: https://github.com/eqrion/cbindgen
- **cxx**: https://cxx.rs/ (bridge C++ ↔ Rust)
- **ash**: https://github.com/MaikKlein/ash (Vulkan bindings para Rust)

## 💡 Conclusión

**Recomendación**: Sí, vale la pena intentar eGUI si:
- ✅ Quieres resolver los problemas de texturas de ImGui
- ✅ Estás dispuesto a aprender Rust
- ✅ Quieres separar UI del core del motor
- ✅ Buscas un estilo más moderno

**Alternativa**: Si prefieres quedarte con C++:
- Continuar con ImGui y resolver el problema de texturas
- O considerar otras opciones en C++ (Nuklear, etc.)

---

**¿Seguimos con eGUI o prefieres otra opción?**

