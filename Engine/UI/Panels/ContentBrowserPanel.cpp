#include "ContentBrowserPanel.h"
#include "../../Core/Log.h"
#include <experimental/filesystem>
#include <algorithm>
namespace fs = std::experimental::filesystem;

namespace UI {

ContentBrowserPanel::ContentBrowserPanel() {
    SetTitle("Content Browser");
    SetResizable(true);
    SetMovable(true);
    
    // Directorio por defecto: directorio de assets del proyecto
    rootDirectory = "assets";
    currentDirectory = rootDirectory;
    
    // Crear directorio si no existe
    if (!fs::exists(rootDirectory)) {
        fs::create_directories(rootDirectory);
    }
    
    RefreshDirectoryContents();
}

void ContentBrowserPanel::Render() {
    if (!IsVisible()) return;
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void ContentBrowserPanel::Update(float deltaTime) {
    // Update logic
}

void ContentBrowserPanel::SetRootDirectory(const std::string& path) {
    rootDirectory = path;
    currentDirectory = path;
    RefreshDirectoryContents();
}

void ContentBrowserPanel::NavigateToDirectory(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        currentDirectory = path;
        RefreshDirectoryContents();
    }
}

void ContentBrowserPanel::NavigateUp() {
    if (currentDirectory != rootDirectory) {
        auto parent = fs::path(currentDirectory).parent_path();
        if (!parent.empty()) {
            NavigateToDirectory(parent.string());
        }
    }
}

void ContentBrowserPanel::RefreshDirectoryContents() {
    currentFiles.clear();
    currentDirectories.clear();
    
    try {
        if (fs::exists(currentDirectory) && fs::is_directory(currentDirectory)) {
            for (const auto& entry : fs::directory_iterator(currentDirectory)) {
                if (fs::is_directory(entry.path())) {
                    currentDirectories.push_back(entry);
                } else {
                    currentFiles.push_back(entry);
                }
            }
        }
    } catch (const std::exception& e) {
        UE_LOG_ERROR(LogCategories::UI, "Error refreshing directory contents: %s", e.what());
    }
}

void ContentBrowserPanel::RenderToolbar() {
    // TODO: Implementar con eGUI
}

void ContentBrowserPanel::RenderFileList() {
    // TODO: Implementar con eGUI
}

void ContentBrowserPanel::RenderDirectoryTree() {
    // TODO: Implementar con eGUI
}

} // namespace UI
