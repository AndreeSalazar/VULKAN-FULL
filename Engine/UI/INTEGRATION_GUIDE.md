# Guía de Integración: ImGui + Lua

## ✅ Estado de la Integración

### Completado
- ✅ **ImGui**: Descargado y estructura preparada
- ✅ **ImGuiWrapper**: Clase wrapper para ImGui creada
- ✅ **LuaUI**: Sistema básico de scripting Lua creado
- ✅ **CMakeLists.txt**: Configurado para ImGui y Lua
- ✅ **Ejemplos**: Panel de ejemplo con ImGui y script Lua de ejemplo

### Pendiente (Configuración)
- ⚠️ **Lua**: Requiere instalación o compilación manual
- ⚠️ **Bindings completos**: Necesita sol2 o bindings manuales
- ⚠️ **Integración en main.cpp**: Conectar ImGui con el render loop

---

## 📦 Instalación de Dependencias

### 1. ImGui (✅ Ya descargado)
```bash
# Ya está en Engine/ThirdParty/imgui
# No requiere instalación adicional
```

### 2. Lua

#### Opción A: Usar paquete del sistema (Recomendado)
```bash
# Ubuntu/Debian
sudo apt-get install liblua5.4-dev

# Fedora
sudo dnf install lua-devel

# Arch
sudo pacman -S lua
```

#### Opción B: Compilar desde fuente
```bash
cd Engine/ThirdParty
wget http://www.lua.org/ftp/lua-5.4.6.tar.gz
tar xzf lua-5.4.6.tar.gz
cd lua-5.4.6
make linux
# Los archivos .c estarán en src/
```

### 3. sol2 (✅ Ya descargado)
```bash
# Ya está en Engine/ThirdParty/sol2
# Es header-only, solo necesita include
```

---

## 🔧 Integración en vulkan_cube.cpp

### Paso 1: Inicializar ImGui

En `vulkan_cube.cpp`, después de crear el descriptor pool:

```cpp
#include "UI/ImGuiWrapper.h"

// En initVulkan() o createDescriptorPool()
void VulkanCube::initImGui() {
    UI::ImGuiWrapper& imgui = UI::ImGuiWrapper::Get();
    
    // Crear descriptor pool para ImGui
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.pPoolSizes = poolSizes;
    
    VkDescriptorPool imguiDescriptorPool;
    vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiDescriptorPool);
    
    // Inicializar ImGui
    imgui.Initialize(
        window,
        instance,
        physicalDevice,
        device,
        graphicsQueue,
        graphicsQueueFamilyIndex,
        renderPass,  // Necesita coincidir con tu render pass
        imguiDescriptorPool
    );
}
```

### Paso 2: Renderizar ImGui

En `recordCommandBuffer()` o donde renderizas:

```cpp
void VulkanCube::renderUI(VkCommandBuffer commandBuffer) {
    UI::ImGuiWrapper& imgui = UI::ImGuiWrapper::Get();
    if (imgui.IsInitialized()) {
        imgui.Render(commandBuffer);
    }
}
```

En `drawFrame()`:

```cpp
void VulkanCube::drawFrame() {
    // ... código existente ...
    
    // Antes de vkCmdEndRenderPass
    renderUI(commandBuffers[currentFrame]);
    
    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    
    // ... resto del código ...
}
```

### Paso 3: Actualizar ImGui cada frame

En `mainLoop()` de `main.cpp`, antes de renderizar:

```cpp
// En main loop
UI::ImGuiWrapper& imgui = UI::ImGuiWrapper::Get();
if (imgui.IsInitialized()) {
    imgui.NewFrame();
    
    // Renderizar tu UI aquí
    auto debugOverlay = std::dynamic_pointer_cast<UI::DebugOverlay_ImGui>(
        UI::UIManager::Get().GetPanel("DebugOverlay_ImGui")
    );
    if (debugOverlay) {
        debugOverlay->Render();
    }
}
```

---

## 📜 Integración de Lua

### Paso 1: Inicializar Lua

En `main.cpp`, en `initVulkan()` o al inicio:

```cpp
#include "UI/Scripting/LuaUI.h"

// En initVulkan()
UI::LuaUI::Get().Initialize();

// Cargar scripts
UI::LuaUI::Get().LoadScript("scripts/ui/example_panel.lua");
```

### Paso 2: Ejecutar funciones Lua

```cpp
// En main loop, después de NewFrame()
UI::LuaUI::Get().CallFunction("ExamplePanel_OnRender");
```

### Paso 3: Registrar funciones C++ para Lua

En `LuaUI.cpp`, en `RegisterEngineBindings()`:

```cpp
void LuaUI::RegisterEngineBindings() {
    if (!luaState) return;
    
    // Crear tabla Engine
    lua_newtable(luaState);
    
    // Engine.GetFPS()
    lua_pushcfunction(luaState, [](lua_State* L) -> int {
        float fps = FFrameTimer::GetGlobal()->GetFPS();
        lua_pushnumber(L, fps);
        return 1;
    });
    lua_setfield(luaState, -2, "GetFPS");
    
    // Engine.GetFrameCount()
    lua_pushcfunction(luaState, [](lua_State* L) -> int {
        uint64_t count = FFrameTimer::GetGlobal()->GetFrameCount();
        lua_pushinteger(L, count);
        return 1;
    });
    lua_setfield(luaState, -2, "GetFrameCount");
    
    lua_setglobal(luaState, "Engine");
}
```

---

## 🎨 Ejemplo Completo de Uso

### C++ Side (main.cpp)
```cpp
#include "UI/ImGuiWrapper.h"
#include "UI/UIManager.h"
#include "UI/Scripting/LuaUI.h"
#include "UI/Panels/DebugOverlay_ImGui.h"

void initUI() {
    // Inicializar ImGui (después de Vulkan)
    UI::ImGuiWrapper::Get().Initialize(...);
    
    // Inicializar Lua
    UI::LuaUI::Get().Initialize();
    
    // Registrar paneles
    UI::UIManager::Get().RegisterPanel("DebugOverlay_ImGui",
        std::make_shared<UI::DebugOverlay_ImGui>());
    
    // Cargar scripts Lua
    UI::LuaUI::Get().LoadScript("scripts/ui/example_panel.lua");
}

void renderUI() {
    UI::ImGuiWrapper& imgui = UI::ImGuiWrapper::Get();
    imgui.NewFrame();
    
    // Renderizar paneles C++
    UI::UIManager::Get().Render();
    
    // Ejecutar scripts Lua
    UI::LuaUI::Get().CallFunction("ExamplePanel_OnRender");
    
    // Render se hace en drawFrame() con imgui.Render()
}
```

### Lua Side (scripts/ui/example_panel.lua)
```lua
function ExamplePanel_OnRender()
    if UI:Begin("Lua Panel") then
        local fps = Engine:GetFPS()
        UI:Text("FPS: %.1f", fps)
        
        if UI:Button("Click Me") then
            print("Button clicked!")
        end
    end
    UI:End()
end
```

---

## 🔍 Troubleshooting

### Error: "imgui.h: No such file"
- Verifica que `Engine/ThirdParty/imgui` existe
- Verifica `CMakeLists.txt` incluye `${IMGUI_ROOT}`

### Error: "lua.h: No such file"
- Instala Lua: `sudo apt-get install liblua5.4-dev`
- O compila desde fuente (ver arriba)

### ImGui no se renderiza
- Verifica que `imgui.NewFrame()` se llama antes de `Begin()`
- Verifica que `imgui.Render()` se llama después de `NewFrame()`
- Verifica que el render pass es compatible

### Lua scripts no funcionan
- Verifica que Lua está compilado e incluido
- Verifica que los bindings están registrados
- Revisa logs para errores de Lua

---

## 📚 Recursos

- **ImGui Documentation**: https://github.com/ocornut/imgui
- **ImGui Vulkan Backend**: `Engine/ThirdParty/imgui/backends/imgui_impl_vulkan.h`
- **Lua Manual**: https://www.lua.org/manual/5.4/
- **sol2 Documentation**: https://thephd.github.io/sol2/

---

## ✅ Próximos Pasos

1. ✅ Integrar ImGui en `vulkan_cube.cpp`
2. ✅ Completar bindings Lua con sol2
3. ✅ Crear más paneles de ejemplo
4. ✅ Sistema de hot-reload para scripts Lua
5. ✅ Editor visual para crear UI

