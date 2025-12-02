#include "Timer.h"
#include <thread>
#include <cmath>
#include <sstream>
#include <iomanip>

// Global frame timer
FFrameTimer* GFrameTimer = nullptr;

// ============================================================================
// FTimer Implementation
// ============================================================================

FTimer::FTimer() {
    Reset();
}

void FTimer::Reset() {
    startTime = Clock::now();
}

double FTimer::GetElapsedTime() const {
    auto now = Clock::now();
    Duration elapsed = now - startTime;
    return elapsed.count();
}

double FTimer::GetElapsedTimeMS() const {
    return GetElapsedTime() * 1000.0;
}

bool FTimer::HasTimeElapsed(double seconds) const {
    return GetElapsedTime() >= seconds;
}

// ============================================================================
// FFrameTimer Implementation
// ============================================================================

FFrameTimer::FFrameTimer()
    : deltaTime(0.0)
    , totalTime(0.0)
    , fps(0.0f)
    , smoothedFPS(0.0f)
    , frameCount(0)
    , targetFPS(60.0f)
    , bFrameLimiting(false)
    , fpsHistoryIndex(0)
{
    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
    
    // Initialize FPS history
    for (int i = 0; i < FPS_SMOOTHING_SAMPLES; i++) {
        fpsHistory[i] = 0.0;
    }
}

void FFrameTimer::Tick() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    // Calculate delta time
    auto elapsed = std::chrono::duration<double>(currentTime - lastFrameTime);
    deltaTime = elapsed.count();
    
    // Clamp delta time to prevent huge spikes (e.g., when debugging)
    const double MAX_DELTA_TIME = 0.1; // 100ms max
    if (deltaTime > MAX_DELTA_TIME) {
        deltaTime = MAX_DELTA_TIME;
    }
    
    // Update total time
    auto totalElapsed = std::chrono::duration<double>(currentTime - startTime);
    totalTime = totalElapsed.count();
    
    // Update frame count
    frameCount++;
    
    // Update FPS
    UpdateFPS();
    
    // Update last frame time
    lastFrameTime = currentTime;
}

void FFrameTimer::UpdateFPS() {
    // Calculate current FPS
    if (deltaTime > 0.0) {
        fps = static_cast<float>(1.0 / deltaTime);
    } else {
        fps = 0.0f;
    }
    
    // Update FPS history for smoothing
    fpsHistory[fpsHistoryIndex] = fps;
    fpsHistoryIndex = (fpsHistoryIndex + 1) % FPS_SMOOTHING_SAMPLES;
    
    // Calculate smoothed FPS (average)
    double sum = 0.0;
    int validSamples = 0;
    for (int i = 0; i < FPS_SMOOTHING_SAMPLES; i++) {
        if (fpsHistory[i] > 0.0) {
            sum += fpsHistory[i];
            validSamples++;
        }
    }
    
    if (validSamples > 0) {
        smoothedFPS = static_cast<float>(sum / validSamples);
    } else {
        smoothedFPS = fps;
    }
}

void FFrameTimer::LimitFrameRate() {
    if (!bFrameLimiting || targetFPS <= 0.0f) {
        return;
    }
    
    double targetFrameTime = 1.0 / targetFPS;
    double elapsed = GetDeltaTime();
    
    if (elapsed < targetFrameTime) {
        double sleepTime = (targetFrameTime - elapsed) * 1000.0; // Convert to ms
        if (sleepTime > 0.0) {
            TimeUtils::SleepMS(static_cast<uint32_t>(sleepTime));
        }
    }
}

std::string FFrameTimer::GetStatsString() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "FPS: " << smoothedFPS 
       << " | Delta: " << (deltaTime * 1000.0) << "ms"
       << " | Frame: " << frameCount
       << " | Time: " << std::setprecision(2) << totalTime << "s";
    return ss.str();
}

// ============================================================================
// FScopedTimer Implementation
// ============================================================================

FScopedTimer::FScopedTimer(const char* name, bool logOnDestroy)
    : timerName(name)
    , bLogOnDestroy(logOnDestroy)
{
    startTime = std::chrono::high_resolution_clock::now();
}

FScopedTimer::~FScopedTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(endTime - startTime);
    
    if (bLogOnDestroy) {
        UE_LOG_VERBOSE(LogCategories::Core, "[Timer] %s: %.3f ms", timerName, elapsed.count());
    }
}

double FScopedTimer::GetElapsedTime() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double>(now - startTime);
    return elapsed.count();
}

// ============================================================================
// FPerformanceCounter Implementation
// ============================================================================

FPerformanceCounter::FPerformanceCounter()
    : bRunning(false)
{
}

void FPerformanceCounter::Start() {
    startTime = std::chrono::high_resolution_clock::now();
    bRunning = true;
}

void FPerformanceCounter::Stop() {
    if (bRunning) {
        endTime = std::chrono::high_resolution_clock::now();
        bRunning = false;
    }
}

double FPerformanceCounter::GetElapsedTime() const {
    auto end = bRunning ? std::chrono::high_resolution_clock::now() : endTime;
    auto elapsed = std::chrono::duration<double>(end - startTime);
    return elapsed.count();
}

double FPerformanceCounter::GetElapsedTimeMS() const {
    return GetElapsedTime() * 1000.0;
}

double FPerformanceCounter::GetElapsedTimeUS() const {
    auto end = bRunning ? std::chrono::high_resolution_clock::now() : endTime;
    auto elapsed = std::chrono::duration<double, std::micro>(end - startTime);
    return elapsed.count();
}

// ============================================================================
// TimeUtils Implementation
// ============================================================================

namespace TimeUtils {
    
void SleepMS(uint32_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double GetTimeSinceEpoch() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

std::string FormatDuration(double seconds) {
    std::stringstream ss;
    
    if (seconds < 0.001) {
        ss << std::fixed << std::setprecision(3) << (seconds * 1000000.0) << " us";
    } else if (seconds < 1.0) {
        ss << std::fixed << std::setprecision(3) << (seconds * 1000.0) << " ms";
    } else if (seconds < 60.0) {
        ss << std::fixed << std::setprecision(2) << seconds << " s";
    } else {
        int minutes = static_cast<int>(seconds / 60.0);
        double secs = fmod(seconds, 60.0);
        ss << minutes << "m " << std::fixed << std::setprecision(1) << secs << "s";
    }
    
    return ss.str();
}

} // namespace TimeUtils

