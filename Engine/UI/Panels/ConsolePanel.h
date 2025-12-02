#pragma once

#include "../UIBase.h"
#include <string>
#include <vector>
#include <deque>

// Forward declaration - ImVec4 viene de imgui.h
struct ImVec4;

namespace UI {

// ============================================================================
// ConsolePanel - Panel de consola para logs y comandos (estilo UE5)
// ============================================================================

enum class ELogLevel {
    Info,
    Warning,
    Error,
    Verbose
};

struct LogEntry {
    std::string message;
    ELogLevel level;
    float timestamp;
    
    LogEntry(const std::string& msg, ELogLevel lvl, float ts) 
        : message(msg), level(lvl), timestamp(ts) {}
};

class ConsolePanel : public IWindow {
public:
    ConsolePanel();
    virtual ~ConsolePanel() = default;
    
    virtual void Render() override;
    virtual void Update(float deltaTime) override;
    
    // Agregar log
    void AddLog(const std::string& message, ELogLevel level = ELogLevel::Info);
    void ClearLogs();
    
    // Filtros
    void SetShowInfo(bool show) { bShowInfo = show; }
    void SetShowWarnings(bool show) { bShowWarnings = show; }
    void SetShowErrors(bool show) { bShowErrors = show; }
    void SetShowVerbose(bool show) { bShowVerbose = show; }
    
    // Command input
    void SetCommandCallback(std::function<void(const std::string&)> callback) {
        commandCallback = callback;
    }

private:
    std::deque<LogEntry> logs;
    static const size_t MAX_LOG_ENTRIES = 1000;
    
    bool bShowInfo = true;
    bool bShowWarnings = true;
    bool bShowErrors = true;
    bool bShowVerbose = false;
    
    bool bAutoScroll = true;
    bool bScrollToBottom = false;
    
    char inputBuffer[256] = {0};
    std::vector<std::string> commandHistory;
    int historyPos = -1;
    
    std::function<void(const std::string&)> commandCallback;
    
    void RenderToolbar();
    void RenderLogList();
    void RenderCommandInput();
    // Retorna color como 4 floats (rgba) - ImVec4 se construye en .cpp
    void GetLogColor(ELogLevel level, float& r, float& g, float& b, float& a);
};

} // namespace UI

