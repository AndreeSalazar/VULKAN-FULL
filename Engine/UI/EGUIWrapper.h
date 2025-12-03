#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include "engine_ui_ffi.h"  // Incluir definición completa de EngineState
#include "VulkanRenderer.h"

namespace UI {

// ============================================================================
// EGUIWrapper - Wrapper C++ para eGUI (Rust)
// ============================================================================

class EGUIWrapper {
public:
    static EGUIWrapper& Get();
    
    // Inicialización
    bool Initialize(
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
    );
    
    // Ciclo de renderizado
    void NewFrame();
    void Render(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height);
    
    // Input handling
    void HandleMouseMove(float x, float y);
    void HandleMouseButton(uint32_t button, bool pressed);
    void HandleKey(uint32_t key, bool pressed);
    
    // Window management
    void SetScreenSize(uint32_t width, uint32_t height);
    
    // Estado del motor
    void UpdateEngineState(float fps, float frameTime, uint64_t frameCount, float totalTime);
    
    // Verificación
    bool IsInitialized() const { return bInitialized; }
    
    // Actualizar textura de fuente después del frame (fuera del command buffer)
    void UpdateFontTextureIfNeeded();
    
    // Cleanup
    void Shutdown();

private:
    EGUIWrapper() = default;
    ~EGUIWrapper() = default;
    EGUIWrapper(const EGUIWrapper&) = delete;
    EGUIWrapper& operator=(const EGUIWrapper&) = delete;
    
    bool bInitialized = false;
    EngineState* engineState = nullptr;
    std::unique_ptr<VulkanRenderer> renderer;
};

} // namespace UI

