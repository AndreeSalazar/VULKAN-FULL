#include "EGUIWrapper.h"
#include "../Core/Log.h"
// engine_ui_ffi.h ya está incluido en EGUIWrapper.h

namespace UI {

EGUIWrapper& EGUIWrapper::Get() {
    static EGUIWrapper instance;
    return instance;
}

bool EGUIWrapper::Initialize(
    GLFWwindow* window,
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkQueue graphicsQueue,
    uint32_t graphicsQueueFamilyIndex,
    VkRenderPass renderPass,
    VkDescriptorPool descriptorPool,
    uint32_t minImageCount,
    uint32_t imageCount
) {
    if (bInitialized) {
        UE_LOG_WARNING(LogCategories::UI, "EGUIWrapper already initialized");
        return true;
    }
    
    UE_LOG_INFO(LogCategories::UI, "Initializing eGUI (Rust)...");
    
    // Llamar a la función FFI de Rust
    bool success = egui_init(window, instance);
    
    if (success) {
        // Crear e inicializar renderer Vulkan
        renderer = std::make_unique<VulkanRenderer>();
        
        // Obtener formato del swapchain (necesitamos acceso al VulkanCube)
        // Por ahora usar VK_FORMAT_B8G8R8A8_UNORM como default
        VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        
        if (!renderer->Initialize(instance, physicalDevice, device, graphicsQueue,
                                  graphicsQueueFamilyIndex, renderPass, descriptorPool,
                                  imageFormat, msaaSamples)) {
            UE_LOG_ERROR(LogCategories::UI, "Failed to initialize VulkanRenderer");
            renderer.reset();
            return false;
        }
        
        bInitialized = true;
        UE_LOG_INFO(LogCategories::UI, "eGUI initialized successfully");
    } else {
        UE_LOG_ERROR(LogCategories::UI, "Failed to initialize eGUI");
    }
    
    return success;
}

void EGUIWrapper::NewFrame() {
    if (!bInitialized) return;
    egui_new_frame();
    
    // Mostrar demo UI (temporal - para testing)
    // TODO: Reemplazar con llamadas a paneles reales
    egui_show_demo();
}

void EGUIWrapper::Render(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height) {
    if (!bInitialized || !engineState) return;
    
    // Llamar a la función FFI de Rust para renderizar (genera datos de renderizado)
    // Esta función finaliza el frame y genera los meshes
    bool hasData = egui_render(engineState, commandBuffer);
    
    // SIEMPRE intentar renderizar, incluso si hasData es false
    // porque puede haber datos de un frame anterior
    if (renderer && renderer->IsInitialized()) {
        renderer->Render(commandBuffer, 0, width, height);
    }
}

void EGUIWrapper::UpdateEngineState(float fps, float frameTime, uint64_t frameCount, float totalTime) {
    // Usar estructura estática para el estado
    static EngineState staticState;
    staticState.fps = fps;
    staticState.frame_time = frameTime;
    staticState.frame_count = frameCount;
    staticState.total_time = totalTime;
    
    // Asignar puntero
    engineState = &staticState;
}

void EGUIWrapper::UpdateFontTextureIfNeeded() {
    if (renderer && renderer->IsInitialized()) {
        renderer->UpdateFontTextureIfNeeded();
    }
}

void EGUIWrapper::HandleMouseMove(float x, float y) {
    if (!bInitialized) return;
    // Botones actuales - se actualizan desde HandleMouseButton
    egui_handle_mouse_event(x, y, 0, false);
}

void EGUIWrapper::HandleMouseButton(uint32_t button, bool pressed) {
    if (!bInitialized) return;
    // Obtener posición actual del mouse desde GLFW
    egui_handle_mouse_event(0.0f, 0.0f, button, pressed);
}

void EGUIWrapper::HandleKey(uint32_t key, bool pressed) {
    if (!bInitialized) return;
    egui_handle_key_event(key, pressed, 0);
}

void EGUIWrapper::SetScreenSize(uint32_t width, uint32_t height) {
    if (!bInitialized) return;
    egui_set_screen_size(static_cast<float>(width), static_cast<float>(height));
}

void EGUIWrapper::Shutdown() {
    if (!bInitialized) return;
    
    UE_LOG_INFO(LogCategories::UI, "Shutting down eGUI...");
    
    // Shutdown renderer
    if (renderer) {
        renderer->Shutdown();
        renderer.reset();
    }
    
    egui_cleanup();
    bInitialized = false;
}

} // namespace UI

