#pragma once

#include "../UIBase.h"
#include <cstdint>

// Forward declarations
class VulkanCube;
struct GLFWwindow;

namespace UI {

// ============================================================================
// ViewportPanel - Panel principal de renderizado 3D (estilo UE5)
// ============================================================================

class ViewportPanel : public IWindow {
public:
    ViewportPanel();
    virtual ~ViewportPanel() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Configuración del viewport
    void SetSize(uint32_t width, uint32_t height) { viewportWidth = width; viewportHeight = height; }
    void GetSize(uint32_t& width, uint32_t& height) const { width = viewportWidth; height = viewportHeight; }
    
    // Controles
    void SetShowGrid(bool show) { bShowGrid = show; }
    void SetShowGizmos(bool show) { bShowGizmos = show; }
    
    // Callback para renderizado
    void SetRenderCallback(std::function<void()> callback) { renderCallback = callback; }
    
    // Mouse/Input handling en viewport
    bool IsMouseOverViewport() const { return bMouseOverViewport; }
    bool IsViewportFocused() const { return bViewportFocused; }

private:
    uint32_t viewportWidth = 1920;
    uint32_t viewportHeight = 1080;
    
    bool bShowGrid = true;
    bool bShowGizmos = true;
    bool bMouseOverViewport = false;
    bool bViewportFocused = false;
    
    std::function<void()> renderCallback;
    
    // ImGui texture ID para renderizar la escena
    // En Vulkan, esto sería un VkDescriptorSet
    void* viewportTextureID = nullptr;
};

} // namespace UI

