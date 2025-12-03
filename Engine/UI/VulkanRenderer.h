#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "engine_ui_ffi.h"  // Para FontTextureData

namespace UI {

// ============================================================================
// VulkanRenderer - Renderer Vulkan para eGUI
// ============================================================================

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();
    
    // Inicialización
    bool Initialize(
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkQueue graphicsQueue,
        uint32_t graphicsQueueFamilyIndex,
        VkRenderPass renderPass,
        VkDescriptorPool descriptorPool,
        VkFormat imageFormat,
        VkSampleCountFlagBits msaaSamples
    );
    
    // Renderizado
    void Render(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t width, uint32_t height);
    
    // Actualizar textura de fuente después del frame (fuera del command buffer)
    // Debe llamarse después de vkQueueWaitIdle o después de que termine el frame
    void UpdateFontTextureIfNeeded();
    
    // Cleanup
    void Shutdown();
    
    bool IsInitialized() const { return bInitialized; }

private:
    bool bInitialized = false;
    
    // Vulkan resources
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex = 0;
    VkCommandPool commandPool = VK_NULL_HANDLE; // Para upload de texturas
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    
    // Pipeline para UI
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    
    // Buffers para vértices e índices (dinámicos)
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    size_t vertexBufferSize = 0;
    size_t indexBufferSize = 0;
    
    // Font texture
    VkImage fontImage = VK_NULL_HANDLE;
    VkDeviceMemory fontImageMemory = VK_NULL_HANDLE;
    VkImageView fontImageView = VK_NULL_HANDLE;
    VkSampler fontSampler = VK_NULL_HANDLE;
    VkDescriptorSet fontDescriptorSet = VK_NULL_HANDLE;
    uint32_t fontImageWidth = 0;
    uint32_t fontImageHeight = 0;
    
    // Font texture update queued (para actualizar después del frame)
    bool pendingFontTextureUpdate = false;
    FontTextureData pendingFontTextureData{};
    
    // Helper functions
    void createPipeline(VkFormat imageFormat, VkSampleCountFlagBits msaaSamples);
    void createFontTexture(uint32_t width = 0, uint32_t height = 0);  // Si width/height son 0, intenta obtener de eGUI
    void updateFontTexture(const void* pixels, uint32_t width, uint32_t height);
    void updateBuffers(const void* vertices, size_t vertexCount, const void* indices, size_t indexCount);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                     VkMemoryPropertyFlags properties, 
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void ensureBufferSize(VkBuffer& buffer, VkDeviceMemory& bufferMemory, 
                         size_t& currentSize, size_t requiredSize, 
                         VkBufferUsageFlags usage);
};

} // namespace UI


