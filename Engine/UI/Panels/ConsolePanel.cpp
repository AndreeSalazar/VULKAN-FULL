#include "ConsolePanel.h"
#include "../../Core/Log.h"
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
    
    // TODO: Implementar renderizado con eGUI (Rust)
    // Por ahora es un stub - no renderiza nada
}

void ConsolePanel::Update(float deltaTime) {
    // Update logic
}

void ConsolePanel::AddLog(const std::string& message, ELogLevel level) {
    logs.push_back(LogEntry(message, level, 0.0f)); // Timestamp placeholder
    
    if (logs.size() > MAX_LOG_ENTRIES) {
        logs.pop_front();
    }
    
    if (bAutoScroll) {
        bScrollToBottom = true;
    }
}

void ConsolePanel::ClearLogs() {
    logs.clear();
}

void ConsolePanel::RenderToolbar() {
    // TODO: Implementar con eGUI
}

void ConsolePanel::RenderLogList() {
    // TODO: Implementar con eGUI
}

void ConsolePanel::RenderCommandInput() {
    // TODO: Implementar con eGUI
}

void ConsolePanel::GetLogColor(ELogLevel level, float& r, float& g, float& b, float& a) {
    switch (level) {
        case ELogLevel::Info:
            r = 1.0f; g = 1.0f; b = 1.0f; a = 1.0f;
            break;
        case ELogLevel::Warning:
            r = 1.0f; g = 1.0f; b = 0.0f; a = 1.0f;
            break;
        case ELogLevel::Error:
            r = 1.0f; g = 0.0f; b = 0.0f; a = 1.0f;
            break;
        case ELogLevel::Verbose:
            r = 0.7f; g = 0.7f; b = 0.7f; a = 1.0f;
            break;
    }
}

} // namespace UI
