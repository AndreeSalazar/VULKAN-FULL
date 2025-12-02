#include "ContentBrowserPanel.h"
#include "../ImGuiWrapper.h"
#include "../../Core/Log.h"
#include <imgui.h>
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
    
    ImGuiWrapper& imgui = ImGuiWrapper::Get();
    if (!imgui.IsInitialized()) return;
    
    // Establecer contexto de ImGui
    ImGui::SetCurrentContext(imgui.GetContext());
    
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Content Browser", &open, ImGuiWindowFlags_None)) {
        // Toolbar
        RenderToolbar();
        
        ImGui::Separator();
        
        // Breadcrumb navigation
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Path: %s", currentDirectory.c_str());
        
        ImGui::Separator();
        
        // Área principal: lista de archivos
        if (ImGui::BeginChild("ContentArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false)) {
            RenderFileList();
        }
        ImGui::EndChild();
        
        // Status bar
        ImGui::Separator();
        ImGui::Text("%zu items", currentFiles.size() + currentDirectories.size());
    }
    ImGui::End();
    
    if (!open) {
        SetVisible(false);
    }
}

void ContentBrowserPanel::Update(float deltaTime) {
    // Update logic
}

void ContentBrowserPanel::RenderToolbar() {
    // Botones de navegación
    if (ImGui::Button("<")) {
        NavigateUp();
    }
    ImGui::SameLine();
    if (ImGui::Button(">")) {
        // Forward navigation (implementar historial si necesario)
    }
    ImGui::SameLine();
    if (ImGui::Button("Home")) {
        NavigateToDirectory(rootDirectory);
    }
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Modo de vista
    const char* viewModes[] = { "List", "Grid", "Details" };
    int currentMode = static_cast<int>(viewMode);
    if (ImGui::Combo("View", &currentMode, viewModes, 3)) {
        viewMode = static_cast<ViewMode>(currentMode);
    }
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Filtro de archivos
    char filterBuffer[256] = {0};
    strncpy(filterBuffer, fileFilter.c_str(), sizeof(filterBuffer) - 1);
    if (ImGui::InputText("Filter", filterBuffer, sizeof(filterBuffer))) {
        fileFilter = filterBuffer;
        RefreshDirectoryContents();
    }
}

void ContentBrowserPanel::RenderFileList() {
    // Renderizar directorios primero
    for (const auto& entry : currentDirectories) {
        std::string name = entry.path().filename().string();
        
        ImGui::Bullet();
        ImGui::SameLine();
        
        if (ImGui::Selectable(("📁 " + name).c_str(), false)) {
            NavigateToDirectory(entry.path().string());
        }
    }
    
    // Renderizar archivos
    for (const auto& entry : currentFiles) {
        std::string name = entry.path().filename().string();
        std::string ext = entry.path().extension().string();
        
        // Icono según extensión
        const char* icon = "📄";
        if (ext == ".lua") icon = "📜";
        else if (ext == ".png" || ext == ".jpg") icon = "🖼️";
        else if (ext == ".glsl" || ext == ".vert" || ext == ".frag") icon = "🔷";
        
        ImGui::Bullet();
        ImGui::SameLine();
        
        if (ImGui::Selectable((icon + std::string(" ") + name).c_str(), false)) {
            // Seleccionar archivo (evento)
            UE_LOG_VERBOSE(LogCategories::UI, "Selected file: %s", entry.path().string().c_str());
        }
        
        // Tooltip con info del archivo
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Path: %s", entry.path().string().c_str());
            try {
                ImGui::Text("Size: %lu bytes", static_cast<unsigned long>(fs::file_size(entry.path())));
            } catch (...) {
                ImGui::Text("Size: Unknown");
            }
            ImGui::EndTooltip();
        }
    }
}

void ContentBrowserPanel::SetRootDirectory(const std::string& path) {
    rootDirectory = path;
    if (fs::exists(rootDirectory)) {
        currentDirectory = rootDirectory;
        RefreshDirectoryContents();
    }
}

void ContentBrowserPanel::NavigateToDirectory(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        currentDirectory = path;
        RefreshDirectoryContents();
    }
}

void ContentBrowserPanel::NavigateUp() {
    fs::path parent = fs::path(currentDirectory).parent_path();
    if (!parent.empty() && fs::exists(parent)) {
        NavigateToDirectory(parent.string());
    }
}

void ContentBrowserPanel::RefreshDirectoryContents() {
    currentFiles.clear();
    currentDirectories.clear();
    
    if (!fs::exists(currentDirectory)) {
        return;
    }
    
    try {
        for (const auto& entry : fs::directory_iterator(currentDirectory)) {
            if (fs::is_directory(entry.path())) {
                currentDirectories.push_back(entry);
            } else {
                // Aplicar filtro
                if (fileFilter == "*" || entry.path().extension().string() == fileFilter) {
                    currentFiles.push_back(entry);
                }
            }
        }
        
        // Ordenar
        std::sort(currentDirectories.begin(), currentDirectories.end(),
                  [](const auto& a, const auto& b) {
                      return a.path().filename() < b.path().filename();
                  });
        std::sort(currentFiles.begin(), currentFiles.end(),
                  [](const auto& a, const auto& b) {
                      return a.path().filename() < b.path().filename();
                  });
    } catch (const fs::filesystem_error& e) {
        UE_LOG_ERROR(LogCategories::UI, "Error reading directory: %s", e.what());
    }
}

} // namespace UI

