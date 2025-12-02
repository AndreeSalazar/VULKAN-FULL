#include "Log.h"
#include <cstdarg>
#include <ctime>

// Static member initialization
ELogVerbosity FLog::MinVerbosity = ELogVerbosity::Display;
bool FLog::bConsoleOutput = true;
bool FLog::bFileOutput = true;
std::ofstream FLog::LogFile;
std::mutex FLog::LogMutex;

void FLog::Initialize(const std::string& logFile) {
    if (bFileOutput) {
        LogFile.open(logFile, std::ios::out | std::ios::trunc);
        if (!LogFile.is_open()) {
            std::cerr << "Warning: Could not open log file: " << logFile << std::endl;
            bFileOutput = false;
        }
    }
    
    FLog::Info(LogCategories::Core::GetLogCategory(), "=== Engine Log Started ===");
}

void FLog::Shutdown() {
    FLog::Info(LogCategories::Core::GetLogCategory(), "=== Engine Log Ended ===");
    
    if (LogFile.is_open()) {
        LogFile.close();
    }
}

void FLog::Log(ELogVerbosity verbosity, const char* category, const char* format, ...) {
    if (verbosity <= MinVerbosity) {
        va_list args;
        va_start(args, format);
        InternalLog(verbosity, category, format, args);
        va_end(args);
    }
}

void FLog::InternalLog(ELogVerbosity verbosity, const char* category, const char* format, va_list args) {
    std::lock_guard<std::mutex> lock(LogMutex);
    
    std::string message = FormatString(format, args);
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm timeInfo;
    #ifdef _WIN32
    localtime_s(&timeInfo, &time_t);
    #else
    localtime_r(&time_t, &timeInfo);
    #endif
    
    // Format timestamp: [HH:MM:SS.mmm]
    char timeBuffer[64];
    std::snprintf(timeBuffer, sizeof(timeBuffer), "[%02d:%02d:%02d.%03lld]",
                  timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec,
                  static_cast<long long>(ms.count()));
    
    // Format log line: [Timestamp] Category: Verbosity: Message
    std::stringstream logLine;
    logLine << timeBuffer << " ";
    logLine << category << ": ";
    logLine << GetVerbosityString(verbosity) << ": ";
    logLine << message;
    
    std::string finalMessage = logLine.str();
    
    // Console output with colors
    if (bConsoleOutput) {
        std::ostream& out = (verbosity <= ELogVerbosity::Warning) ? std::cerr : std::cout;
        out << GetVerbosityColor(verbosity) << finalMessage << LogColors::Reset << std::endl;
    }
    
    // File output (no colors)
    if (bFileOutput && LogFile.is_open()) {
        LogFile << finalMessage << std::endl;
        LogFile.flush();
    }
    
    // Fatal logs should crash
    if (verbosity == ELogVerbosity::Fatal) {
        if (bFileOutput && LogFile.is_open()) {
            LogFile << "FATAL ERROR: Application will now terminate." << std::endl;
            LogFile.flush();
        }
        std::abort();
    }
}

std::string FLog::FormatString(const char* format, va_list args) {
    // Calculate required buffer size
    va_list argsCopy;
    va_copy(argsCopy, args);
    int size = std::vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);
    
    if (size < 0) {
        return std::string(format);
    }
    
    // Create buffer and format
    std::string buffer(size + 1, '\0');
    std::vsnprintf(&buffer[0], buffer.size(), format, args);
    buffer.resize(size);
    
    return buffer;
}

const char* FLog::GetVerbosityString(ELogVerbosity verbosity) {
    switch (verbosity) {
        case ELogVerbosity::Fatal: return "Fatal";
        case ELogVerbosity::Error: return "Error";
        case ELogVerbosity::Warning: return "Warning";
        case ELogVerbosity::Display: return "Display";
        case ELogVerbosity::Log: return "Log";
        case ELogVerbosity::Verbose: return "Verbose";
        case ELogVerbosity::VeryVerbose: return "VeryVerbose";
        default: return "Unknown";
    }
}

const char* FLog::GetVerbosityColor(ELogVerbosity verbosity) {
    switch (verbosity) {
        case ELogVerbosity::Fatal: return LogColors::Fatal;
        case ELogVerbosity::Error: return LogColors::Error;
        case ELogVerbosity::Warning: return LogColors::Warning;
        case ELogVerbosity::Display: return LogColors::Info;
        case ELogVerbosity::Log: return LogColors::Log;
        case ELogVerbosity::Verbose:
        case ELogVerbosity::VeryVerbose: return LogColors::Verbose;
        default: return LogColors::Reset;
    }
}

// Helper function implementations
void FLog::Fatal(const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    InternalLog(ELogVerbosity::Fatal, category, format, args);
    va_end(args);
}

void FLog::Error(const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    InternalLog(ELogVerbosity::Error, category, format, args);
    va_end(args);
}

void FLog::Warning(const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    InternalLog(ELogVerbosity::Warning, category, format, args);
    va_end(args);
}

void FLog::Display(const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    InternalLog(ELogVerbosity::Display, category, format, args);
    va_end(args);
}

void FLog::Info(const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    InternalLog(ELogVerbosity::Log, category, format, args);
    va_end(args);
}

void FLog::Verbose(const char* category, const char* format, ...) {
    va_list args;
    va_start(args, format);
    InternalLog(ELogVerbosity::Verbose, category, format, args);
    va_end(args);
}

