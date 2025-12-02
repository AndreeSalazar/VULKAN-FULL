#include "ConsolePanel.h"
#include "../ImGuiWrapper.h"
#include "../../Core/Log.h"
#include <imgui.h>
#include <chrono>

namespace UI {

ConsolePanel::ConsolePanel() {
    SetTitle("Output Log");
    SetResizable(true);
    SetMovable(true);
    
    // Agregar algunos logs de ejemplo
    AddLog("Console initialized", ELogLevel::Info);
    AddLog("Ready for commands", ELogLevel::Info);
}

void ConsolePanel::Render() {
    if (!IsVisible()) return;
    
    ImGuiWrapper& imgui = ImGuiWrapper::Get();
    if (!imgui.IsInitialized()) return;
    
    // Establecer contexto de ImGui
    ImGui::SetCurrentContext(imgui.GetContext());
    
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Output Log", &open, ImGuiWindowFlags_None)) {
        // Toolbar con filtros
        RenderToolbar();
        
        ImGui::Separator();
        
        // Área de logs
        ImGui::BeginChild("LogArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 10), false);
        RenderLogList();
        ImGui::EndChild();
        
        // Input de comandos
        ImGui::Separator();
        RenderCommandInput();
    }
    ImGui::End();
    
    if (!open) {
        SetVisible(false);
    }
}

void ConsolePanel::Update(float deltaTime) {
    // Update logic
}

void ConsolePanel::RenderToolbar() {
    // Botones de filtro
    ImGui::Checkbox("Info", &bShowInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warnings", &bShowWarnings);
    ImGui::SameLine();
    ImGui::Checkbox("Errors", &bShowErrors);
    ImGui::SameLine();
    ImGui::Checkbox("Verbose", &bShowVerbose);
    
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    if (ImGui::Button("Clear")) {
        ClearLogs();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &bAutoScroll);
}

void ConsolePanel::RenderLogList() {
    for (const auto& log : logs) {
        // Aplicar filtros
        bool shouldShow = false;
        switch (log.level) {
            case ELogLevel::Info: shouldShow = bShowInfo; break;
            case ELogLevel::Warning: shouldShow = bShowWarnings; break;
            case ELogLevel::Error: shouldShow = bShowErrors; break;
            case ELogLevel::Verbose: shouldShow = bShowVerbose; break;
        }
        
        if (!shouldShow) continue;
        
        // Color según nivel
        float r, g, b, a;
        GetLogColor(log.level, r, g, b, a);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, a));
        
        // Mostrar log
        ImGui::Text("[%.2f] %s", log.timestamp, log.message.c_str());
        
        ImGui::PopStyleColor();
    }
    
    // Auto-scroll
    if (bAutoScroll || bScrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        bScrollToBottom = false;
    }
}

void ConsolePanel::RenderCommandInput() {
    ImGui::Text(">");
    ImGui::SameLine();
    
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | 
                                 ImGuiInputTextFlags_CallbackCompletion |
                                 ImGuiInputTextFlags_CallbackHistory;
    
    if (ImGui::InputText("##CommandInput", inputBuffer, sizeof(inputBuffer), flags)) {
        std::string command = inputBuffer;
        if (!command.empty()) {
            AddLog("> " + command, ELogLevel::Info);
            
            // Ejecutar comando
            if (commandCallback) {
                commandCallback(command);
            }
            
            // Agregar a historial
            commandHistory.push_back(command);
            historyPos = -1;
            
            // Limpiar input
            memset(inputBuffer, 0, sizeof(inputBuffer));
        }
        
        ImGui::SetKeyboardFocusHere(-1); // Mantener foco en input
    }
}

void ConsolePanel::AddLog(const std::string& message, ELogLevel level) {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    float timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0f;
    
    logs.push_back(LogEntry(message, level, timestamp));
    
    // Limitar tamaño
    if (logs.size() > MAX_LOG_ENTRIES) {
        logs.pop_front();
    }
    
    bScrollToBottom = true;
}

void ConsolePanel::ClearLogs() {
    logs.clear();
    AddLog("Log cleared", ELogLevel::Info);
}

void ConsolePanel::GetLogColor(ELogLevel level, float& r, float& g, float& b, float& a) {
    switch (level) {
        case ELogLevel::Info:
            r = 1.0f; g = 1.0f; b = 1.0f; a = 1.0f;      // White
            break;
        case ELogLevel::Warning:
            r = 1.0f; g = 1.0f; b = 0.0f; a = 1.0f;      // Yellow
            break;
        case ELogLevel::Error:
            r = 1.0f; g = 0.0f; b = 0.0f; a = 1.0f;      // Red
            break;
        case ELogLevel::Verbose:
            r = 0.7f; g = 0.7f; b = 0.7f; a = 1.0f;      // Gray
            break;
        default:
            r = 1.0f; g = 1.0f; b = 1.0f; a = 1.0f;      // White
            break;
    }
}

} // namespace UI

