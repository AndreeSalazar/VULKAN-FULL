# Guía de Integración: Sistema UI estilo Unreal Engine 5

## ✅ Estado Actual del Sistema

### Completado
- ✅ **ImGui**: Integrado con Vulkan backend
- ✅ **ImGuiWrapper**: Clase wrapper completa para ImGui
- ✅ **UIManager**: Sistema de gestión de paneles y ventanas
- ✅ **Estilo UE5**: Colores oscuros, acentos azules, espaciado mejorado
- ✅ **Paneles implementados**:
  - DebugOverlay (overlay de debug)
  - StatsPanel (estadísticas)
  - ObjectHierarchy (jerarquía de objetos)
  - ViewportPanel (vista del renderizado)
  - DetailsPanel (propiedades de objetos)
  - ContentBrowserPanel (explorador de archivos)
  - ConsolePanel (consola de logs)
- ✅ **Integración en main.cpp**: Loop completo funcional
- ✅ **CMakeLists.txt**: Configurado correctamente

### Problemas Conocidos (⚠️ Necesita Solución)
- ⚠️ **Texturas de fuentes**: La UI no aparece inmediatamente porque las texturas de fuentes no se actualizan correctamente
  - **Causa**: `ImGui_ImplVulkan_UpdateTexture()` requiere operaciones fuera del render pass
  - **Estado actual**: Se evita renderizar cuando hay texturas pendientes para evitar crashes
  - **Impacto**: La UI puede tardar varios frames en aparecer o no aparecer
  - **Solución pendiente**: Implementar actualización de texturas después del render pass

### Pendiente para Estilo UE5 Completo
- ⚠️ **Docking System**: Ventanas acoplables/dockables
  - ImGui soporta docking con `ImGuiConfigFlags_DockingEnable`
  - Requiere habilitar en `ImGuiIO::ConfigFlags`
  - Necesita configuración de layout inicial
- ⚠️ **Multi-Viewport**: Ventanas independientes
  - Requiere `ImGuiConfigFlags_ViewportsEnable`
  - Necesita soporte de plataforma adicional
- ⚠️ **Tab System**: Sistema de tabs para ventanas múltiples
  - Necesita implementar manejo de tabs personalizado
  - O usar sistema de docking nativo de ImGui
- ⚠️ **Layout System**: Sistema de layouts guardables/cargables
  - Layouts por defecto para diferentes modos (Editor, Game, etc.)
  - Guardar/cargar layouts desde archivo .ini
- ⚠️ **Toolbar/Menu Bar**: Barra de herramientas superior
  - Menú File/Edit/View/Window/Help
  - Toolbar con herramientas comunes
- ⚠️ **Status Bar**: Barra de estado inferior
  - Información del sistema
  - Indicadores de estado
- ⚠️ **Asset Browser mejorado**: Explorador de assets más robusto
  - Vista de árbol y lista
  - Filtros y búsqueda
  - Preview de assets
- ⚠️ **Scene Hierarchy mejorado**: Jerarquía de escena
  - Drag & drop para reordenar
  - Multi-selección
  - Context menu
- ⚠️ **Details Panel mejorado**: Panel de propiedades
  - Categorías colapsables
  - Edición inline de valores
  - Validación de propiedades
- ⚠️ **Viewport mejorado**: Vista del renderizado
  - Controles de cámara integrados
  - Gizmos para manipular objetos
  - Overlay de información

---

## 🔧 Configuración Actual

### Estructura del Loop Principal
```cpp
// En main.cpp, mainLoop()
while (!glfwWindowShouldClose(window)) {
    // 1. Actualizar input
    InputManager::Get().Update();
    
    // 2. ImGui: Nuevo frame
    UI::ImGuiWrapper::Get().NewFrame();
    
    // 3. Actualizar datos de UI
    // (FPS, stats, etc.)
    
    // 4. Actualizar lógica de UI
    UI::UIManager::Get().Update(deltaTime);
    
    // 5. Renderizar widgets de ImGui (crear ventanas)
    UI::UIManager::Get().Render();
    
    // 6. Preparar datos de renderizado
    UI::ImGuiWrapper::Get().PrepareRender(); // Llama ImGui::Render()
    
    // 7. Renderizar frame (incluye ImGui)
    cube.drawFrame(); // Dentro: UI::ImGuiWrapper::Get().Render(commandBuffer)
    
    // 8. Post-render (para actualizar texturas en el futuro)
    UI::ImGuiWrapper::Get().PostRender();
}
```

### Estilo UE5 Aplicado
- **Colores**: Fondos oscuros (#1C1C1C, #141414), acentos azules (#42A0FB)
- **Espaciado**: Generoso y consistente
- **Bordes**: Sutiles, redondeo moderado
- **Tipografía**: Mejorada para legibilidad

---

## 🚀 Próximos Pasos para Completar Estilo UE5

### 1. Habilitar Docking (Alta Prioridad)
```cpp
// En ImGuiWrapper::Initialize()
ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // ¡DESCOMENTAR ESTO!

// Verificar que la versión de ImGui soporta docking
// Si no está disponible, puede ser necesario actualizar ImGui
```

**Archivo**: `Engine/UI/ImGuiWrapper.cpp`, línea ~38

### 2. Configurar Layout Inicial
```cpp
// Crear función para configurar layout por defecto
void ImGuiWrapper::SetupDefaultLayout() {
    // Configurar ventanas en posiciones específicas
    // Usar ImGui::SetNextWindowPos() y SetNextWindowSize()
    // O usar docking para organizar automáticamente
}
```

**Archivo nuevo**: `Engine/UI/LayoutManager.cpp`

### 3. Implementar Toolbar/Menu Bar
```cpp
// En UIManager::Render() o nuevo panel
void MenuBar::Render() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) { /* ... */ }
            if (ImGui::MenuItem("Open Scene")) { /* ... */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { /* ... */ }
            ImGui::EndMenu();
        }
        // ... más menús
        ImGui::EndMainMenuBar();
    }
}
```

**Archivo nuevo**: `Engine/UI/Panels/MenuBar.cpp`

### 4. Implementar Status Bar
```cpp
// Panel simple en la parte inferior
void StatusBar::Render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - 30));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 30));
    
    ImGui::Begin("StatusBar", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("Ready");
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("FPS: %.1f", GetFPS());
    
    ImGui::End();
}
```

**Archivo nuevo**: `Engine/UI/Panels/StatusBar.cpp`

### 5. Mejorar Viewport con Controles
```cpp
// En ViewportPanel::Render()
void ViewportPanel::Render() {
    ImGui::Begin("Viewport");
    
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    
    // Renderizar el viewport aquí
    // Agregar controles de cámara
    // Agregar gizmos
    
    ImGui::End();
}
```

**Archivo**: `Engine/UI/Panels/ViewportPanel.cpp`

### 6. Sistema de Layouts Guardables
```cpp
class LayoutManager {
public:
    void SaveLayout(const std::string& name);
    void LoadLayout(const std::string& name);
    void LoadDefaultLayout();
};
```

**Archivo nuevo**: `Engine/UI/LayoutManager.cpp`

---

## 🐛 Resolver Problema de Texturas

### Problema Actual
Las texturas de fuentes no se cargan correctamente porque:
1. `ImGui_ImplVulkan_UpdateTexture()` requiere operaciones fuera del render pass
2. `RenderDrawData()` intenta actualizar texturas dentro del render pass
3. Actualmente se evita renderizar para prevenir crashes

### Soluciones Posibles

#### Opción 1: Actualizar texturas después del frame
```cpp
// En PostRender(), después de que termine el render pass
void ImGuiWrapper::PostRender() {
    // Esperar a que el device esté idle
    // Luego actualizar texturas pendientes
    // Esto requiere acceso a VkDevice
}
```

#### Opción 2: Usar command buffer separado
```cpp
// Crear un command buffer dedicado para actualizar texturas
// Ejecutarlo antes del render pass principal
```

#### Opción 3: Forzar actualización en inicialización
```cpp
// En Initialize(), después de crear el pipeline
// Forzar la actualización de texturas una vez
// Antes de comenzar el loop principal
```

### Implementación Sugerida (Opción 3)
Modificar `ImGuiWrapper::Initialize()` para forzar la actualización inicial de texturas:
```cpp
bool ImGuiWrapper::Initialize(...) {
    // ... código existente ...
    
    // Después de inicializar, forzar actualización de texturas
    ImGui::SetCurrentContext(imguiContext);
    ImGui::NewFrame();
    ImGui::EndFrame();  // Esto debería construir las texturas
    
    // Ahora obtener draw data y actualizar texturas
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData && drawData->Textures) {
        for (ImTextureData* tex : *drawData->Textures) {
            if (tex->Status != ImTextureStatus_OK) {
                ImGui_ImplVulkan_UpdateTexture(tex);
            }
        }
    }
    
    // Limpiar el frame de prueba
    ImGui::EndFrame();
    
    return true;
}
```

---

## 📋 Checklist de Implementación

### Fase 1: Resolver Problemas Básicos
- [ ] **Resolver problema de texturas** (CRÍTICO)
  - Implementar actualización forzada en inicialización
  - O implementar actualización post-frame segura
- [ ] **Verificar que UI aparece correctamente**
- [ ] **Asegurar que no hay crashes**

### Fase 2: Docking y Layouts
- [ ] Habilitar `ImGuiConfigFlags_DockingEnable`
- [ ] Crear `LayoutManager` para manejar layouts
- [ ] Implementar layout por defecto
- [ ] Sistema de guardar/cargar layouts
- [ ] Layouts presets (Editor, Game, Minimal)

### Fase 3: Componentes UE5
- [ ] MenuBar (File, Edit, View, Window, Help)
- [ ] Toolbar (herramientas comunes)
- [ ] StatusBar (info del sistema)
- [ ] Mejorar Viewport (controles, gizmos)
- [ ] Mejorar ContentBrowser (filtros, búsqueda)
- [ ] Mejorar Hierarchy (drag & drop, multi-select)
- [ ] Mejorar Details (categorías, validación)

### Fase 4: Features Avanzadas
- [ ] Multi-viewport support
- [ ] Tab system personalizado
- [ ] Hotkeys configurables
- [ ] Themes personalizables
- [ ] Plugin system para paneles
- [ ] Hot-reload de layouts

---

## 📚 Referencias y Recursos

### Documentación
- **ImGui**: https://github.com/ocornut/imgui
- **ImGui Docking**: https://github.com/ocornut/imgui/wiki/Docking
- **ImGui Vulkan Backend**: `Engine/ThirdParty/imgui/backends/imgui_impl_vulkan.h`
- **Unreal Engine UI Style Guide**: Referencias visuales de UE5

### Archivos Clave del Proyecto
- `Engine/UI/ImGuiWrapper.cpp`: Wrapper principal de ImGui
- `Engine/UI/UIManager.cpp`: Gestión de paneles y ventanas
- `Engine/UI/Panels/`: Todos los paneles implementados
- `Source/main.cpp`: Loop principal donde se integra todo

### Ejemplos de Código
- Ver `Engine/ThirdParty/imgui/examples/example_glfw_vulkan/` para ejemplos oficiales
- Ver paneles existentes como referencia para nuevos paneles

---

## 🔍 Troubleshooting

### La UI no aparece
1. **Verificar logs**: Buscar mensajes sobre texturas pendientes
2. **Verificar texturas**: `[ImGuiWrapper::Render] Texture X has pending status`
3. **Esperar varios frames**: Puede tardar en cargar texturas
4. **Verificar inicialización**: Asegurar que `ImGuiWrapper::Initialize()` se llama correctamente

### Crashes al renderizar
1. **Verificar render pass**: ImGui debe renderizarse dentro de un render pass activo
2. **Verificar texturas**: No intentar actualizar dentro del render pass
3. **Verificar command buffer**: Asegurar que es válido y está grabando

### Docking no funciona
1. **Verificar flag**: `ImGuiConfigFlags_DockingEnable` debe estar habilitado
2. **Verificar versión**: ImGui debe ser versión reciente (1.89+)
3. **Verificar que no se deshabilita**: Asegurar que el flag no se sobrescribe después

---

## ✅ Estado Final Objetivo

Un sistema UI completo estilo Unreal Engine 5 con:
- ✅ Docking de ventanas
- ✅ Layouts configurables
- ✅ Menu bar y toolbar
- ✅ Status bar
- ✅ Paneles mejorados (Viewport, Content Browser, Hierarchy, Details)
- ✅ Sistema de temas
- ✅ Hotkeys configurables
- ✅ Sin crashes ni problemas de texturas

---

**Última actualización**: Después de implementar estilo UE5 básico y resolver problemas de texturas  
**Próximo objetivo**: Habilitar docking y completar componentes principales
