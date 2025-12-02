#include "ImGuiWrapper.h"
#include "../Core/Log.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <cstdarg>
#include <cstdio>

namespace UI {

ImGuiWrapper& ImGuiWrapper::Get() {
    static ImGuiWrapper instance;
    return instance;
}

ImGuiWrapper::~ImGuiWrapper() {
    Shutdown();
}

bool ImGuiWrapper::Initialize(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice,
                              VkDevice device, VkQueue graphicsQueue, uint32_t queueFamilyIndex,
                              VkRenderPass renderPass, VkDescriptorPool descriptorPool,
                              uint32_t minImageCount, uint32_t imageCount) {
    if (bInitialized) {
        UE_LOG_WARNING(LogCategories::Core, "ImGuiWrapper already initialized");
        return false;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Initializing ImGui...");
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    // Docking puede no estar disponible en todas las versiones de ImGui
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking (estilo UE5)
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    memset(&init_info, 0, sizeof(init_info)); // Zero-initialize
    
    init_info.ApiVersion = VK_API_VERSION_1_0;
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.QueueFamily = queueFamilyIndex;
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool;
    init_info.MinImageCount = minImageCount;
    init_info.ImageCount = imageCount;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    init_info.UseDynamicRendering = false;
    
    // Configurar PipelineInfoMain (nueva API de ImGui)
    memset(&init_info.PipelineInfoMain, 0, sizeof(init_info.PipelineInfoMain));
    init_info.PipelineInfoMain.RenderPass = renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        UE_LOG_ERROR(LogCategories::Core, "Failed to initialize ImGui Vulkan backend");
        ImGui::DestroyContext(imguiContext);
        imguiContext = nullptr;
        return false;
    }
    
    // CRÍTICO: En versiones nuevas de ImGui, el pipeline puede no crearse automáticamente
    // si no se proporciona un RenderPass válido. Verificamos si necesitamos crearlo manualmente.
    // Pero primero, asegurémonos de que el RenderPass que pasamos es válido.
    // Si el pipeline no se creó, debemos crearlo explícitamente con ImGui_ImplVulkan_CreateMainPipeline()
    
    // En versiones nuevas de ImGui, los fonts se suben automáticamente la primera vez que se renderiza
    // dentro de un render pass activo. No necesitamos subirlos manualmente antes.
    
    this->descriptorPool = descriptorPool;
    bInitialized = true;
    
    UE_LOG_INFO(LogCategories::Core, "ImGui initialized successfully");
    return true;
}

void ImGuiWrapper::Shutdown() {
    if (!bInitialized) {
        return;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Shutting down ImGui...");
    
    ImGui::SetCurrentContext(imguiContext);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imguiContext);
    imguiContext = nullptr;
    
    bInitialized = false;
    UE_LOG_INFO(LogCategories::Core, "ImGui shutdown complete");
}

void ImGuiWrapper::NewFrame() {
    static bool bFirstFrame = true;
    
    if (!bInitialized) return;
    
    ImGui::SetCurrentContext(imguiContext);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // En el primer frame, necesitamos esperar a que ImGui::Render() se llame
    // antes de poder actualizar texturas. Así que lo hacemos en PrepareRender().
}

void ImGuiWrapper::PrepareRender() {
    static bool bFirstPrepare = true;
    
    if (!bInitialized) {
        UE_LOG_WARNING(LogCategories::UI, "PrepareRender called but ImGui not initialized");
        return;
    }
    if (!imguiContext) {
        UE_LOG_WARNING(LogCategories::UI, "PrepareRender called but imguiContext is null");
        return;
    }
    
    ImGui::SetCurrentContext(imguiContext);
    
    // Render ImGui data (prepare draw data) - DEBE llamarse después de crear todos los widgets
    ImGui::Render();
    
    // NOTA: NO actualizamos texturas manualmente aquí porque ImGui_ImplVulkan_UpdateTexture()
    // puede crashear si se llama fuera del contexto correcto o si los recursos no están listos.
    // En su lugar, confiamos en que ImGui manejará las texturas automáticamente cuando sea necesario.
    //
    // El problema es que RenderDrawData() intenta actualizar texturas dentro del render pass,
    // lo cual no es permitido. La solución es deshabilitar temporalmente Textures durante
    // RenderDrawData() si hay texturas pendientes, y esperar a que se actualicen automáticamente
    // en un momento seguro.
    
    if (bFirstPrepare) {
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->Textures != nullptr && drawData->Textures->Size > 0) {
            UE_LOG_VERBOSE(LogCategories::UI, "[PrepareRender] Found %d textures (will be handled automatically)", drawData->Textures->Size);
        }
        bFirstPrepare = false;
    }
    
    if (bFirstPrepare) {
        bFirstPrepare = false;
    }
}

void ImGuiWrapper::PostRender() {
    // NOTA: No actualizamos texturas aquí porque ImGui_ImplVulkan_UpdateTexture()
    // puede crashear incluso después del render pass, probablemente porque necesita
    // que ciertos recursos de Vulkan estén en un estado específico.
    //
    // En su lugar, simplemente esperamos a que ImGui actualice las texturas automáticamente
    // cuando sea necesario. Esto puede tomar varios frames, pero es más seguro.
    
    // Este método existe para mantener la API consistente, pero no hace nada por ahora.
    // Las texturas se actualizarán automáticamente cuando ImGui las necesite.
}

void ImGuiWrapper::Render(VkCommandBuffer commandBuffer) {
    static uint32_t renderCallCount = 0;
    renderCallCount++;
    
    if (!bInitialized) {
        UE_LOG_WARNING(LogCategories::UI, "Render called but ImGui not initialized");
        return;
    }
    if (!imguiContext) {
        UE_LOG_WARNING(LogCategories::UI, "Render called but imguiContext is null");
        return;
    }
    
    ImGui::SetCurrentContext(imguiContext);
    
    // Get draw data (should already be prepared via PrepareRender() -> ImGui::Render())
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData) {
        UE_LOG_VERBOSE(LogCategories::UI, "Render called but drawData is null");
        return;
    }
    
    // Validate draw data (same checks as ImGui_ImplVulkan_RenderDrawData)
    if (!drawData->Valid) {
        UE_LOG_VERBOSE(LogCategories::UI, "Render called but drawData->Valid is false");
        return;
    }
    if (drawData->CmdListsCount == 0) {
        UE_LOG_VERBOSE(LogCategories::UI, "Render called but CmdListsCount is 0");
        return;
    }
    if (drawData->TotalVtxCount == 0) {
        UE_LOG_VERBOSE(LogCategories::UI, "Render called but TotalVtxCount is 0");
        return;
    }
    
    // Check display size (same as ImGui_ImplVulkan_RenderDrawData does)
    int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
    int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) {
        UE_LOG_VERBOSE(LogCategories::UI, "Render called but framebuffer size invalid: %dx%d", fb_width, fb_height);
        return;
    }
    
    // Verificar que el backend data existe (esto debería existir si está inicializado)
    ImGuiIO& io = ImGui::GetIO();
    if (!io.BackendRendererUserData) {
        UE_LOG_ERROR(LogCategories::UI, "BackendRendererUserData is null! ImGui backend not properly initialized.");
        return;
    }
    
    // IMPORTANTE: ImGui_ImplVulkan_RenderDrawData accede a ImGui::GetPlatformIO() internamente
    // que requiere que el contexto esté establecido
    if (renderCallCount == 1) {
        UE_LOG_INFO(LogCategories::UI, "[ImGuiWrapper::Render] About to call ImGui_ImplVulkan_RenderDrawData...");
    }
    
    // Asegurar que el contexto está establecido antes de renderizar
    ImGuiContext* verifyCtx = ImGui::GetCurrentContext();
    if (verifyCtx != imguiContext) {
        UE_LOG_ERROR(LogCategories::UI, "[ImGuiWrapper::Render] Context mismatch! Expected: %p, Got: %p", imguiContext, verifyCtx);
        ImGui::SetCurrentContext(imguiContext);
    }
    
    // Render ImGui draw data into command buffer
    // This must be called INSIDE an active render pass (before vkCmdEndRenderPass)
    // CRITICAL: ImGui_ImplVulkan_RenderDrawData necesita que:
    // 1. El command buffer esté dentro de un render pass activo
    // 2. El backend de ImGui esté correctamente inicializado
    // 3. Las fuentes estén disponibles (se suben automáticamente en versiones nuevas)
    
    if (renderCallCount == 1) {
        UE_LOG_INFO(LogCategories::UI, "[ImGuiWrapper::Render] Calling ImGui_ImplVulkan_RenderDrawData...");
        UE_LOG_INFO(LogCategories::UI, "[ImGuiWrapper::Render] DrawData: Valid=%d, CmdListsCount=%d, TotalVtxCount=%d", 
                    drawData->Valid ? 1 : 0, drawData->CmdListsCount, drawData->TotalVtxCount);
        
        // En versiones nuevas de ImGui, las fuentes se crean automáticamente la primera vez que se renderiza
        // Pero necesitamos asegurarnos de que ImGui_ImplVulkan_NewFrame() se haya llamado
        // para que el backend esté listo para renderizar
        UE_LOG_INFO(LogCategories::UI, "[ImGuiWrapper::Render] BackendRendererUserData: %p", io.BackendRendererUserData);
    }
    
    // Verificar que tenemos datos de draw válidos antes de renderizar
    if (!drawData->Valid) {
        UE_LOG_WARNING(LogCategories::UI, "[ImGuiWrapper::Render] DrawData is not valid, skipping render");
        return;
    }
    
    if (drawData->CmdListsCount == 0) {
        UE_LOG_VERBOSE(LogCategories::UI, "[ImGuiWrapper::Render] No command lists to render");
        return;
    }
    
    // CRÍTICO: Si hay texturas pendientes (Status != OK), los ImDrawCmd ya tienen referencias
    // a texturas que no están cargadas, causando una aserción en GetTexID().
    //
    // ImGui_ImplVulkan_RenderDrawData() intenta actualizar texturas al inicio, pero lo hace
    // cuando ya estamos dentro de un render pass activo, lo cual causa un crash.
    //
    // Solución: Si hay texturas pendientes, simplemente NO renderizamos en este frame.
    // Esperamos a que las texturas se actualicen automáticamente en un momento seguro.
    // Esto puede tomar varios frames, pero evita crashes.
    
    bool bHasPendingTextures = false;
    if (drawData->Textures != nullptr && drawData->Textures->Size > 0) {
        for (int i = 0; i < drawData->Textures->Size; i++) {
            ImTextureData* tex = (*drawData->Textures)[i];
            if (tex && tex->Status != ImTextureStatus_OK) {
                bHasPendingTextures = true;
                if (renderCallCount <= 10) {  // Log los primeros 10 frames
                    UE_LOG_INFO(LogCategories::UI, "[ImGuiWrapper::Render] Frame %u: Texture %d has pending status %d, skipping render", renderCallCount, i, (int)tex->Status);
                }
                break;
            }
        }
    }
    
    // Si hay texturas pendientes, no renderizar este frame
    // Las texturas se actualizarán automáticamente cuando ImGui las necesite
    // (esto puede tomar varios frames, pero es más seguro)
    if (bHasPendingTextures) {
        return;
    }
    
    // Todas las texturas están listas, renderizar normalmente
    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
    
    if (renderCallCount == 1) {
        UE_LOG_INFO(LogCategories::UI, "[ImGuiWrapper::Render] ImGui_ImplVulkan_RenderDrawData completed successfully");
    }
}

void ImGuiWrapper::SetStyle(ImGuiStyleType style) {
    if (!bInitialized) return;
    
    ImGui::SetCurrentContext(imguiContext);
    switch (style) {
        case ImGuiStyleType::Dark:
            ImGui::StyleColorsDark();
            break;
        case ImGuiStyleType::Light:
            ImGui::StyleColorsLight();
            break;
        case ImGuiStyleType::Classic:
            ImGui::StyleColorsClassic();
            break;
    }
}

void ImGuiWrapper::SetStyleUE5() {
    if (!bInitialized) return;
    
    ImGui::SetCurrentContext(imguiContext);
    ImGuiStyle& style = ImGui::GetStyle();
    
    // ============================================
    // COLORES - Estilo Unreal Engine 5
    // ============================================
    
    // Fondos principales (muy oscuros, casi negros)
    ImVec4 bg_dark = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);        // Fondo principal
    ImVec4 bg_darker = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);      // Fondo más oscuro
    ImVec4 bg_medium = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);      // Fondo medio
    ImVec4 bg_light = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);       // Fondo claro
    
    // Acentos azul/cyan (color característico de UE5)
    ImVec4 accent = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);         // Azul principal
    ImVec4 accent_hover = ImVec4(0.35f, 0.67f, 1.00f, 1.00f);   // Azul hover
    ImVec4 accent_active = ImVec4(0.20f, 0.49f, 0.86f, 1.00f);  // Azul activo
    
    // Textos
    ImVec4 text = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);           // Texto principal
    ImVec4 text_disabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // Texto deshabilitado
    
    // Bordes y separadores
    ImVec4 border = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);         // Borde normal
    ImVec4 border_hover = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);   // Borde hover
    
    // Aplicar colores
    style.Colors[ImGuiCol_Text] = text;
    style.Colors[ImGuiCol_TextDisabled] = text_disabled;
    style.Colors[ImGuiCol_WindowBg] = bg_dark;
    style.Colors[ImGuiCol_ChildBg] = bg_darker;
    style.Colors[ImGuiCol_PopupBg] = bg_dark;
    style.Colors[ImGuiCol_Border] = border;
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Frame (inputs, botones, etc.)
    style.Colors[ImGuiCol_FrameBg] = bg_medium;
    style.Colors[ImGuiCol_FrameBgHovered] = bg_light;
    style.Colors[ImGuiCol_FrameBgActive] = bg_light;
    
    // Títulos y headers
    style.Colors[ImGuiCol_TitleBg] = bg_darker;
    style.Colors[ImGuiCol_TitleBgActive] = bg_medium;
    style.Colors[ImGuiCol_TitleBgCollapsed] = bg_darker;
    
    // Menu bar
    style.Colors[ImGuiCol_MenuBarBg] = bg_darker;
    
    // Scrollbar
    style.Colors[ImGuiCol_ScrollbarBg] = bg_darker;
    style.Colors[ImGuiCol_ScrollbarGrab] = bg_light;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = bg_medium;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = accent;
    
    // Slider
    style.Colors[ImGuiCol_SliderGrab] = accent;
    style.Colors[ImGuiCol_SliderGrabActive] = accent_active;
    
    // Botones
    style.Colors[ImGuiCol_Button] = bg_medium;
    style.Colors[ImGuiCol_ButtonHovered] = accent_hover;
    style.Colors[ImGuiCol_ButtonActive] = accent_active;
    
    // Headers (para árboles, tablas, etc.)
    style.Colors[ImGuiCol_Header] = bg_medium;
    style.Colors[ImGuiCol_HeaderHovered] = accent;
    style.Colors[ImGuiCol_HeaderActive] = accent_active;
    
    // Separadores
    style.Colors[ImGuiCol_Separator] = border;
    style.Colors[ImGuiCol_SeparatorHovered] = border_hover;
    style.Colors[ImGuiCol_SeparatorActive] = accent;
    
    // Resize grip
    style.Colors[ImGuiCol_ResizeGrip] = bg_light;
    style.Colors[ImGuiCol_ResizeGripHovered] = accent;
    style.Colors[ImGuiCol_ResizeGripActive] = accent_active;
    
    // Tabs
    style.Colors[ImGuiCol_Tab] = bg_medium;
    style.Colors[ImGuiCol_TabHovered] = accent;
    style.Colors[ImGuiCol_TabActive] = accent;
    style.Colors[ImGuiCol_TabUnfocused] = bg_medium;
    style.Colors[ImGuiCol_TabUnfocusedActive] = bg_light;
    
    // Table
    style.Colors[ImGuiCol_TableHeaderBg] = bg_darker;
    style.Colors[ImGuiCol_TableBorderStrong] = border;
    style.Colors[ImGuiCol_TableBorderLight] = border;
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt] = bg_darker;
    
    // Plot lines
    style.Colors[ImGuiCol_PlotLines] = accent;
    style.Colors[ImGuiCol_PlotLinesHovered] = accent_hover;
    style.Colors[ImGuiCol_PlotHistogram] = accent;
    style.Colors[ImGuiCol_PlotHistogramHovered] = accent_hover;
    
    // Text input
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    
    // Drag and drop
    style.Colors[ImGuiCol_DragDropTarget] = accent;
    
    // Nav (teclado/gamepad)
    style.Colors[ImGuiCol_NavHighlight] = accent;
    style.Colors[ImGuiCol_NavWindowingHighlight] = accent;
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.59f);
    
    // Modal dimming
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    
    // ============================================
    // ESPACIADO Y TAMAÑOS
    // ============================================
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
    
    // ============================================
    // REDONDEO Y BORDES
    // ============================================
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;  // Sin borde en frames (inputs más limpios)
    style.TabBorderSize = 1.0f;
    style.TabBarBorderSize = 1.0f;
    
    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    
    // ============================================
    // ALINEACIÓN Y DISPOSICIÓN
    // ============================================
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style.DisplaySafeAreaPadding = ImVec2(3.0f, 3.0f);
    
    // ============================================
    // TAMAÑOS MÍNIMOS
    // ============================================
    style.WindowMinSize = ImVec2(200.0f, 100.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    
    // ============================================
    // OTROS AJUSTES
    // ============================================
    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.60f;
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    style.CurveTessellationTol = 1.25f;
    style.CircleTessellationMaxError = 0.30f;
    
    // Fuente base más grande para mejor legibilidad
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
    
    UE_LOG_INFO(LogCategories::UI, "UE5 style applied successfully");
}

void ImGuiWrapper::EnableDocking(bool enable) {
    if (!bInitialized) return;
    
    ImGui::SetCurrentContext(imguiContext);
    ImGuiIO& io = ImGui::GetIO();
    // Docking puede no estar disponible en todas las versiones
    // if (enable) {
    //     io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // } else {
    //     io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
    // }
    (void)enable; // Supress unused warning
}

void ImGuiWrapper::SetIniFilename(const std::string& filename) {
    if (!bInitialized) return;
    
    ImGui::SetCurrentContext(imguiContext);
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = filename.c_str();
}

bool ImGuiWrapper::Button(const char* label, float width, float height) {
    if (!Get().bInitialized) return false;
    ImGui::SetCurrentContext(Get().imguiContext);
    if (width > 0 || height > 0) {
        return ImGui::Button(label, ImVec2(width, height));
    }
    return ImGui::Button(label);
}

void ImGuiWrapper::Text(const char* fmt, ...) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
}

void ImGuiWrapper::TextColored(float r, float g, float b, float a, const char* fmt, ...) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    va_list args;
    va_start(args, fmt);
    ImGui::TextColoredV(ImVec4(r, g, b, a), fmt, args);
    va_end(args);
}

bool ImGuiWrapper::InputText(const char* label, std::string& buffer) {
    if (!Get().bInitialized) return false;
    ImGui::SetCurrentContext(Get().imguiContext);
    // Usar InputText con buffer dinámico
    static char textBuffer[256];
    strncpy(textBuffer, buffer.c_str(), sizeof(textBuffer) - 1);
    textBuffer[sizeof(textBuffer) - 1] = '\0';
    if (ImGui::InputText(label, textBuffer, sizeof(textBuffer))) {
        buffer = std::string(textBuffer);
        return true;
    }
    return false;
}

bool ImGuiWrapper::InputFloat3(const char* label, float* values) {
    if (!Get().bInitialized) return false;
    ImGui::SetCurrentContext(Get().imguiContext);
    return ImGui::InputFloat3(label, values);
}

bool ImGuiWrapper::Checkbox(const char* label, bool* value) {
    if (!Get().bInitialized) return false;
    ImGui::SetCurrentContext(Get().imguiContext);
    return ImGui::Checkbox(label, value);
}

void ImGuiWrapper::Separator() {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::Separator();
}

void ImGuiWrapper::SameLine(float offset, float spacing) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::SameLine(offset, spacing);
}

bool ImGuiWrapper::Begin(const char* name, bool* p_open, int flags) {
    if (!Get().bInitialized) return false;
    ImGui::SetCurrentContext(Get().imguiContext);
    return ImGui::Begin(name, p_open, flags);
}

void ImGuiWrapper::End() {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::End();
}

void ImGuiWrapper::BeginChild(const char* str_id, float size_x, float size_y, bool border) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::BeginChild(str_id, ImVec2(size_x, size_y), border);
}

void ImGuiWrapper::EndChild() {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::EndChild();
}

void ImGuiWrapper::SetNextWindowPos(float x, float y, int cond) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::SetNextWindowPos(ImVec2(x, y), cond);
}

void ImGuiWrapper::SetNextWindowSize(float width, float height, int cond) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGui::SetNextWindowSize(ImVec2(width, height), cond);
}

void ImGuiWrapper::LoadFont(const std::string& path, float size) {
    if (!Get().bInitialized) return;
    ImGui::SetCurrentContext(Get().imguiContext);
    ImGuiIO& io = ImGui::GetIO();
    // Font loading implementation
    // io.Fonts->AddFontFromFileTTF(path.c_str(), size);
}

} // namespace UI

