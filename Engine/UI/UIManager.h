#pragma once

#include "UIBase.h"
#include <unordered_map>
#include <memory>

// ============================================================================
// UIManager - Gestor principal de la interfaz gráfica
// Similar a Unreal Engine's Slate/SlateUI system
// ============================================================================

namespace UI {

class UIManager {
public:
    static UIManager& Get();
    
    // Inicialización y shutdown
    void Initialize();
    void Shutdown();
    
    // Actualización y renderizado
    void Update(float deltaTime);
    void Render();
    
    // Gestión de paneles/ventanas
    void RegisterPanel(const std::string& name, std::shared_ptr<IPanel> panel);
    void UnregisterPanel(const std::string& name);
    std::shared_ptr<IPanel> GetPanel(const std::string& name);
    
    // Gestión de ventanas
    void RegisterWindow(const std::string& name, std::shared_ptr<IWindow> window);
    void UnregisterWindow(const std::string& name);
    std::shared_ptr<IWindow> GetWindow(const std::string& name);
    
    // Visibilidad
    void ShowPanel(const std::string& name);
    void HidePanel(const std::string& name);
    void TogglePanel(const std::string& name);
    
    void ShowWindow(const std::string& name);
    void HideWindow(const std::string& name);
    void ToggleWindow(const std::string& name);
    
    // Estados
    bool IsInitialized() const { return bInitialized; }
    bool IsPanelVisible(const std::string& name) const;
    bool IsWindowVisible(const std::string& name) const;
    
    // Debug overlay (siempre visible)
    void SetShowDebugOverlay(bool show) { bShowDebugOverlay = show; }
    bool IsShowDebugOverlay() const { return bShowDebugOverlay; }

private:
    UIManager() = default;
    ~UIManager();
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;
    
    bool bInitialized = false;
    bool bShowDebugOverlay = true;
    
    std::unordered_map<std::string, std::shared_ptr<IPanel>> panels;
    std::unordered_map<std::string, std::shared_ptr<IWindow>> windows;
};

} // namespace UI

