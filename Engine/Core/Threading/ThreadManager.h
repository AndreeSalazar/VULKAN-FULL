#pragma once

#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

// ============================================================================
// ThreadManager - Gestión de threads del motor (Game Thread, Render Thread)
// Similar a Unreal Engine's threading system
// ============================================================================

enum class EThreadType {
    Game,
    Render
};

class ThreadManager {
public:
    static ThreadManager& Get();
    
    // Inicializar threads
    void Initialize();
    
    // Shutdown threads
    void Shutdown();
    
    // Verificar si estamos en un thread específico
    bool IsInGameThread() const;
    bool IsInRenderThread() const;
    
    // Ejecutar función en game thread
    void ExecuteInGameThread(std::function<void()> function);
    
    // Ejecutar función en render thread
    void ExecuteInRenderThread(std::function<void()> function);
    
    // Obtener IDs de threads
    std::thread::id GetGameThreadId() const { return gameThreadId; }
    std::thread::id GetRenderThreadId() const { return renderThreadId; }
    
    // Estados
    bool IsInitialized() const { return bInitialized; }
    bool IsShuttingDown() const { return bShuttingDown; }
    
    // Configurar callbacks de tick
    void SetGameThreadTickFunction(std::function<void(float)> func) {
        gameThreadTickFunction = func;
    }
    
    void SetRenderThreadTickFunction(std::function<void(float)> func) {
        renderThreadTickFunction = func;
    }
    
    // Configurar FPS targets
    void SetTargetGameFPS(float fps) { targetGameFPS = fps; }
    void SetTargetRenderFPS(float fps) { targetRenderFPS = fps; }

private:
    ThreadManager() = default;
    ~ThreadManager();
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    
    // Funciones de threads
    void GameThreadMain();
    void RenderThreadMain();
    
    // Callbacks (configurables)
    std::function<void(float)> gameThreadTickFunction;
    std::function<void(float)> renderThreadTickFunction;
    
    // Threads
    std::unique_ptr<std::thread> gameThread;
    std::unique_ptr<std::thread> renderThread;
    
    // Thread IDs
    std::thread::id gameThreadId;
    std::thread::id renderThreadId;
    std::thread::id mainThreadId;
    
    // Estados
    std::atomic<bool> bInitialized{false};
    std::atomic<bool> bShuttingDown{false};
    std::atomic<bool> bGameThreadRunning{false};
    std::atomic<bool> bRenderThreadRunning{false};
    
    // Sincronización
    std::mutex initMutex;
    std::condition_variable initCondition;
    std::atomic<bool> bGameThreadReady{false};
    std::atomic<bool> bRenderThreadReady{false};
    
    // Frame sync
    std::mutex frameMutex;
    std::condition_variable frameCondition;
    std::atomic<bool> bFrameReady{false};
    
    // FPS control
    std::atomic<float> targetGameFPS{60.0f};
    std::atomic<float> targetRenderFPS{60.0f};
};

// Macros útiles para verificar thread
#define CHECK_IS_IN_GAME_THREAD() \
    do { \
        if (!ThreadManager::Get().IsInGameThread()) { \
            UE_LOG_ERROR(LogCategories::Core, "Function called from wrong thread! Expected Game Thread"); \
            return; \
        } \
    } while(0)

#define CHECK_IS_IN_RENDER_THREAD() \
    do { \
        if (!ThreadManager::Get().IsInRenderThread()) { \
            UE_LOG_ERROR(LogCategories::Core, "Function called from wrong thread! Expected Render Thread"); \
            return; \
        } \
    } while(0)

#define ENSURE_GAME_THREAD() CHECK_IS_IN_GAME_THREAD()
#define ENSURE_RENDER_THREAD() CHECK_IS_IN_RENDER_THREAD()

