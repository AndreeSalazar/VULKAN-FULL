#pragma once

#include "../UIBase.h"
#include <string>
#include <vector>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace UI {

// ============================================================================
// ContentBrowserPanel - Explorador de contenido/assets (estilo UE5)
// ============================================================================

class ContentBrowserPanel : public IWindow {
public:
    ContentBrowserPanel();
    virtual ~ContentBrowserPanel() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Configurar directorio raíz
    void SetRootDirectory(const std::string& path);
    std::string GetRootDirectory() const { return rootDirectory; }
    
    // Navegación
    void NavigateToDirectory(const std::string& path);
    void NavigateUp();
    
    // Filtros
    void SetFileFilter(const std::string& filter) { fileFilter = filter; }
    std::string GetFileFilter() const { return fileFilter; }

private:
    std::string rootDirectory;
    std::string currentDirectory;
    std::string fileFilter = "*"; // Por defecto, todos los archivos
    
    std::vector<fs::directory_entry> currentFiles;
    std::vector<fs::directory_entry> currentDirectories;
    
    void RefreshDirectoryContents();
    void RenderFileList();
    void RenderDirectoryTree();
    void RenderToolbar();
    
    enum class ViewMode {
        List,
        Grid,
        Details
    };
    ViewMode viewMode = ViewMode::Grid;
};

} // namespace UI

