#pragma once

#include <chrono>
#include <string>
#include "Log.h"

// Timer class (similar to UE5's timing utilities)
class FTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double>;
    
    FTimer();
    
    // Reset the timer
    void Reset();
    
    // Get elapsed time in seconds
    double GetElapsedTime() const;
    
    // Get elapsed time in milliseconds
    double GetElapsedTimeMS() const;
    
    // Check if a specific time has elapsed
    bool HasTimeElapsed(double seconds) const;
    
private:
    TimePoint startTime;
};

// Frame timer - tracks frame delta time and FPS
class FFrameTimer {
public:
    FFrameTimer();
    
    // Call at the start of each frame
    void Tick();
    
    // Get delta time (time since last frame) in seconds
    double GetDeltaTime() const { return deltaTime; }
    
    // Get delta time in milliseconds
    double GetDeltaTimeMS() const { return deltaTime * 1000.0; }
    
    // Get FPS (frames per second)
    float GetFPS() const { return fps; }
    
    // Get smoothed FPS (averaged over multiple frames)
    float GetSmoothedFPS() const { return smoothedFPS; }
    
    // Get total time since start
    double GetTotalTime() const { return totalTime; }
    
    // Get frame count
    uint64_t GetFrameCount() const { return frameCount; }
    
    // Set target FPS (for frame limiting)
    void SetTargetFPS(float targetFPS) { this->targetFPS = targetFPS; }
    float GetTargetFPS() const { return targetFPS; }
    
    // Enable/disable frame limiting
    void SetFrameLimiting(bool enabled) { bFrameLimiting = enabled; }
    bool IsFrameLimitingEnabled() const { return bFrameLimiting; }
    
    // Limit frame rate (call at end of frame)
    void LimitFrameRate();
    
    // Get statistics string
    std::string GetStatsString() const;

private:
    void UpdateFPS();
    
    double deltaTime;
    double totalTime;
    float fps;
    float smoothedFPS;
    uint64_t frameCount;
    
    float targetFPS;
    bool bFrameLimiting;
    
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    std::chrono::high_resolution_clock::time_point startTime;
    
    // FPS smoothing
    static constexpr int FPS_SMOOTHING_SAMPLES = 60;
    double fpsHistory[FPS_SMOOTHING_SAMPLES];
    int fpsHistoryIndex;
};

// Scoped timer - measures time in a scope (RAII)
class FScopedTimer {
public:
    FScopedTimer(const char* name, bool logOnDestroy = true);
    ~FScopedTimer();
    
    double GetElapsedTime() const;
    
private:
    const char* timerName;
    bool bLogOnDestroy;
    std::chrono::high_resolution_clock::time_point startTime;
};

// Performance counter - high precision timing
class FPerformanceCounter {
public:
    FPerformanceCounter();
    
    void Start();
    void Stop();
    
    double GetElapsedTime() const;
    double GetElapsedTimeMS() const;
    double GetElapsedTimeUS() const; // microseconds
    
    bool IsRunning() const { return bRunning; }
    
private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    bool bRunning;
};

// Global frame timer instance (similar to GFrameTime in UE5)
extern FFrameTimer* GFrameTimer;

// Convenience macros (similar to UE5)
#define SCOPED_TIMER(Name) FScopedTimer _scopedTimer(Name, true)
#define SCOPED_TIMER_SILENT(Name) FScopedTimer _scopedTimer(Name, false)

// Time utility functions
namespace TimeUtils {
    // Sleep for specified milliseconds
    void SleepMS(uint32_t milliseconds);
    
    // Get current time in seconds since epoch
    double GetTimeSinceEpoch();
    
    // Format time duration to string
    std::string FormatDuration(double seconds);
}

