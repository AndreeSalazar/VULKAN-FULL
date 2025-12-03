#include "UIManager.h"
#include "../Core/Log.h"

namespace UI {

UIManager& UIManager::Get() {
    static UIManager instance;
    return instance;
}

UIManager::~UIManager() {
    Shutdown();
}

void UIManager::Initialize() {
    if (bInitialized) {
        UE_LOG_WARNING(LogCategories::Core, "UIManager already initialized");
        return;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Initializing UIManager...");
    
    // Los paneles se registrarán desde fuera (para evitar dependencias circulares)
    // RegisterPanel("DebugOverlay", ...) se hace desde main.cpp
    
    bInitialized = true;
    UE_LOG_INFO(LogCategories::Core, "UIManager initialized successfully");
}

void UIManager::Shutdown() {
    if (!bInitialized) {
        return;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Shutting down UIManager...");
    
    panels.clear();
    windows.clear();
    
    bInitialized = false;
    UE_LOG_INFO(LogCategories::Core, "UIManager shutdown complete");
}

void UIManager::Update(float deltaTime) {
    if (!bInitialized) return;
    
    // Update all panels
    for (auto& [name, panel] : panels) {
        if (panel && panel->IsVisible()) {
            panel->Update(deltaTime);
        }
    }
    
    // Update all windows
    for (auto& [name, window] : windows) {
        if (window && window->IsVisible()) {
            window->Update(deltaTime);
        }
    }
}

void UIManager::Render() {
    if (!bInitialized) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora solo renderizamos paneles si eGUI está inicializado
    
    // Render MenuBar FIRST (must be before any windows)
    auto menuBar = GetPanel("MenuBar");
    if (menuBar && menuBar->IsVisible()) {
        menuBar->Render();
    }
    
    // Render all windows (paneles estilo UE5)
    for (auto& [name, window] : windows) {
        if (window && window->IsVisible()) {
            window->Render();
        }
    }
    
    // Render debug overlay (on top, siempre visible)
    if (bShowDebugOverlay) {
        auto overlay = GetPanel("DebugOverlay");
        if (overlay) {
            overlay->Render();
        }
    }
    
    // Render StatusBar LAST (must be after all windows)
    auto statusBar = GetPanel("StatusBar");
    if (statusBar && statusBar->IsVisible()) {
        statusBar->Render();
    }
}

void UIManager::RegisterPanel(const std::string& name, std::shared_ptr<IPanel> panel) {
    if (!panel) {
        UE_LOG_WARNING(LogCategories::Core, "Attempting to register null panel: %s", name.c_str());
        return;
    }
    
    panels[name] = panel;
    UE_LOG_VERBOSE(LogCategories::Core, "Registered UI Panel: %s", name.c_str());
}

void UIManager::UnregisterPanel(const std::string& name) {
    auto it = panels.find(name);
    if (it != panels.end()) {
        panels.erase(it);
        UE_LOG_VERBOSE(LogCategories::Core, "Unregistered UI Panel: %s", name.c_str());
    }
}

std::shared_ptr<IPanel> UIManager::GetPanel(const std::string& name) {
    auto it = panels.find(name);
    if (it != panels.end()) {
        return it->second;
    }
    return nullptr;
}

void UIManager::RegisterWindow(const std::string& name, std::shared_ptr<IWindow> window) {
    if (!window) {
        UE_LOG_WARNING(LogCategories::Core, "Attempting to register null window: %s", name.c_str());
        return;
    }
    
    windows[name] = window;
    UE_LOG_VERBOSE(LogCategories::Core, "Registered UI Window: %s", name.c_str());
}

void UIManager::UnregisterWindow(const std::string& name) {
    auto it = windows.find(name);
    if (it != windows.end()) {
        windows.erase(it);
        UE_LOG_VERBOSE(LogCategories::Core, "Unregistered UI Window: %s", name.c_str());
    }
}

std::shared_ptr<IWindow> UIManager::GetWindow(const std::string& name) {
    auto it = windows.find(name);
    if (it != windows.end()) {
        return it->second;
    }
    return nullptr;
}

void UIManager::ShowPanel(const std::string& name) {
    auto panel = GetPanel(name);
    if (panel) {
        panel->SetVisible(true);
    }
}

void UIManager::HidePanel(const std::string& name) {
    auto panel = GetPanel(name);
    if (panel) {
        panel->SetVisible(false);
    }
}

void UIManager::TogglePanel(const std::string& name) {
    auto panel = GetPanel(name);
    if (panel) {
        panel->SetVisible(!panel->IsVisible());
    }
}

void UIManager::ShowWindow(const std::string& name) {
    auto window = GetWindow(name);
    if (window) {
        window->SetVisible(true);
    }
}

void UIManager::HideWindow(const std::string& name) {
    auto window = GetWindow(name);
    if (window) {
        window->SetVisible(false);
    }
}

void UIManager::ToggleWindow(const std::string& name) {
    auto window = GetWindow(name);
    if (window) {
        window->SetVisible(!window->IsVisible());
    }
}

bool UIManager::IsPanelVisible(const std::string& name) const {
    auto it = panels.find(name);
    if (it != panels.end()) {
        return it->second->IsVisible();
    }
    return false;
}

bool UIManager::IsWindowVisible(const std::string& name) const {
    auto it = windows.find(name);
    if (it != windows.end()) {
        return it->second->IsVisible();
    }
    return false;
}

} // namespace UI

