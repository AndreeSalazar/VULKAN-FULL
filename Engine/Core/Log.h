#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <mutex>

// Log categories (similar to UE_LOG categories)
#define DEFINE_LOG_CATEGORY_STATIC(CategoryName) \
    static const char* GetLogCategory() { return #CategoryName; }

// Log verbosity levels (similar to UE_LOG verbosity)
enum class ELogVerbosity : uint8_t {
    NoLogging = 0,
    Fatal,      // Always logs, crashes the game
    Error,      // Error level
    Warning,    // Warning level
    Display,    // Display level
    Log,        // Log level
    Verbose,    // Verbose level
    VeryVerbose // Very verbose level
};

// Color codes for terminal output
namespace LogColors {
    constexpr const char* Reset = "\033[0m";
    constexpr const char* Fatal = "\033[1;31m";  // Red bold
    constexpr const char* Error = "\033[31m";    // Red
    constexpr const char* Warning = "\033[33m";  // Yellow
    constexpr const char* Info = "\033[36m";     // Cyan
    constexpr const char* Log = "\033[37m";      // White
    constexpr const char* Verbose = "\033[90m";  // Dark gray
}

// Main logging class (similar to UE_LOG)
class FLog {
public:
    static void Initialize(const std::string& logFile = "Engine.log");
    static void Shutdown();
    
    // Set minimum verbosity level
    static void SetVerbosity(ELogVerbosity verbosity) { MinVerbosity = verbosity; }
    static ELogVerbosity GetVerbosity() { return MinVerbosity; }
    
    // Enable/disable console output
    static void SetConsoleOutput(bool enabled) { bConsoleOutput = enabled; }
    static void SetFileOutput(bool enabled) { bFileOutput = enabled; }
    
    // Log functions (similar to UE_LOG) - wrapper for InternalLog
    static void Log(ELogVerbosity verbosity, const char* category, const char* format, ...);
    
    // Helper functions (similar to UE_LOG)
    static void Fatal(const char* category, const char* format, ...);
    static void Error(const char* category, const char* format, ...);
    static void Warning(const char* category, const char* format, ...);
    static void Display(const char* category, const char* format, ...);
    static void Info(const char* category, const char* format, ...);
    static void Verbose(const char* category, const char* format, ...);

private:
    static void InternalLog(ELogVerbosity verbosity, const char* category, const char* format, va_list args);
    static std::string FormatString(const char* format, va_list args);
    static const char* GetVerbosityString(ELogVerbosity verbosity);
    static const char* GetVerbosityColor(ELogVerbosity verbosity);
    
    static ELogVerbosity MinVerbosity;
    static bool bConsoleOutput;
    static bool bFileOutput;
    static std::ofstream LogFile;
    static std::mutex LogMutex;
};

// UE_LOG style macros - These call the variadic functions directly
#define UE_LOG(Category, Verbosity, Format, ...) \
    FLog::Log(ELogVerbosity::Verbosity, Category::GetLogCategory(), Format, ##__VA_ARGS__)

#define UE_LOG_FATAL(Category, Format, ...) \
    FLog::Fatal(Category::GetLogCategory(), Format, ##__VA_ARGS__)

#define UE_LOG_ERROR(Category, Format, ...) \
    FLog::Error(Category::GetLogCategory(), Format, ##__VA_ARGS__)

#define UE_LOG_WARNING(Category, Format, ...) \
    FLog::Warning(Category::GetLogCategory(), Format, ##__VA_ARGS__)

#define UE_LOG_DISPLAY(Category, Format, ...) \
    FLog::Display(Category::GetLogCategory(), Format, ##__VA_ARGS__)

#define UE_LOG_INFO(Category, Format, ...) \
    FLog::Info(Category::GetLogCategory(), Format, ##__VA_ARGS__)

#define UE_LOG_VERBOSE(Category, Format, ...) \
    FLog::Verbose(Category::GetLogCategory(), Format, ##__VA_ARGS__)

// Log categories (similar to UE_LOG categories)
namespace LogCategories {
    struct Core {
        DEFINE_LOG_CATEGORY_STATIC(LogCore);
    };
    
    struct Render {
        DEFINE_LOG_CATEGORY_STATIC(LogRender);
    };
    
    struct RHI {
        DEFINE_LOG_CATEGORY_STATIC(LogRHI);
    };
    
    struct World {
        DEFINE_LOG_CATEGORY_STATIC(LogWorld);
    };
    
    struct Actor {
        DEFINE_LOG_CATEGORY_STATIC(LogActor);
    };
    
    struct Material {
        DEFINE_LOG_CATEGORY_STATIC(LogMaterial);
    };
    
    struct Blueprint {
        DEFINE_LOG_CATEGORY_STATIC(LogBlueprint);
    };
    
    struct Asset {
        DEFINE_LOG_CATEGORY_STATIC(LogAsset);
    };
    
    struct UI {
        DEFINE_LOG_CATEGORY_STATIC(LogUI);
    };
}

