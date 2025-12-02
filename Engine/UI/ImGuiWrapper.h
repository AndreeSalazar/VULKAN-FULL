#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
#include <memory>

// Forward declarations
struct ImGuiContext;

// Include ImGui headers in cpp, not header
#include <cstdint>

// ============================================================================
// ImGuiWrapper - Wrapper para Dear ImGui con Vulkan
// ============================================================================

namespace UI {

// Enums simples para la API pública
enum class ImGuiStyleType {
    Dark = 0,
    Light = 1,
    Classic = 2
};

enum ImGuiWindowFlags {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
    ImGuiWindowFlags_NoScrollbar = 1 << 3,
    ImGuiWindowFlags_NoCollapse = 1 << 4,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 5
};

enum ImGuiCond {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

class ImGuiWrapper {
public:
    static ImGuiWrapper& Get();
    
    // Inicialización y shutdown
    bool Initialize(GLFWwindow* window, VkInstance instance, VkPhysicalDevice physicalDevice,
                    VkDevice device, VkQueue graphicsQueue, uint32_t queueFamilyIndex,
                    VkRenderPass renderPass, VkDescriptorPool descriptorPool,
                    uint32_t minImageCount = 2, uint32_t imageCount = 2);
    
    void Shutdown();
    
        // Frame lifecycle
        void NewFrame();
        void PrepareRender(); // Call ImGui::Render() to prepare draw data
        void Render(VkCommandBuffer commandBuffer);
        void PostRender(); // Called after frame ends, safe to update textures here
    
    // Utilidades
    bool IsInitialized() const { return bInitialized; }
    ImGuiContext* GetContext() const { return imguiContext; }
    
    // Configuración
    void SetStyle(ImGuiStyleType style); // Dark, Light, Classic, etc.
    void SetStyleUE5(); // Estilo personalizado inspirado en Unreal Engine 5
    void EnableDocking(bool enable);
    void SetIniFilename(const std::string& filename);
    
    // Helpers para widgets comunes
    static bool Button(const char* label, float width = 0.0f, float height = 0.0f);
    static void Text(const char* fmt, ...);
    static void TextColored(float r, float g, float b, float a, const char* fmt, ...);
    static bool InputText(const char* label, std::string& buffer);
    static bool InputFloat3(const char* label, float* values);
    static bool Checkbox(const char* label, bool* value);
    static void Separator();
    static void SameLine(float offset = 0.0f, float spacing = -1.0f);
    
    // Window helpers
    static bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    static void End();
    static void BeginChild(const char* str_id, float size_x = 0.0f, float size_y = 0.0f, bool border = false);
    static void EndChild();
    
    // Layout helpers
    static void SetNextWindowPos(float x, float y, int cond = 0);
    static void SetNextWindowSize(float width, float height, int cond = 0);
    
    // Font management
    void LoadFont(const std::string& path, float size);

private:
    ImGuiWrapper() = default;
    ~ImGuiWrapper();
    ImGuiWrapper(const ImGuiWrapper&) = delete;
    ImGuiWrapper& operator=(const ImGuiWrapper&) = delete;
    
    bool bInitialized = false;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    
    // ImGui context
    ImGuiContext* imguiContext = nullptr;
};

} // namespace UI

