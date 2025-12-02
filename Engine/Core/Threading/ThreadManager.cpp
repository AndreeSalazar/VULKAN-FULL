#include "ThreadManager.h"
#include "RenderCommandQueue.h"
#include "../Log.h"
#include "../Timer.h"
#include <chrono>

ThreadManager& ThreadManager::Get() {
    static ThreadManager instance;
    return instance;
}

ThreadManager::~ThreadManager() {
    Shutdown();
}

void ThreadManager::Initialize() {
    if (bInitialized) {
        UE_LOG_WARNING(LogCategories::Core, "ThreadManager already initialized");
        return;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Initializing ThreadManager...");
    
    mainThreadId = std::this_thread::get_id();
    bShuttingDown = false;
    
    // Inicializar cola de comandos de renderizado
    RenderCommandQueue::Get();
    
    // Crear game thread
    bGameThreadRunning = true;
    bGameThreadReady = false;
    gameThread = std::make_unique<std::thread>(&ThreadManager::GameThreadMain, this);
    
    // Esperar a que game thread esté listo
    std::unique_lock<std::mutex> lock(initMutex);
    initCondition.wait(lock, [this] { return bGameThreadReady.load(); });
    gameThreadId = gameThread->get_id();
    
    UE_LOG_INFO(LogCategories::Core, "Game Thread started (ID: %zu)", 
                std::hash<std::thread::id>{}(gameThreadId));
    
    // Crear render thread
    bRenderThreadRunning = true;
    bRenderThreadReady = false;
    renderThread = std::make_unique<std::thread>(&ThreadManager::RenderThreadMain, this);
    
    // Esperar a que render thread esté listo
    initCondition.wait(lock, [this] { return bRenderThreadReady.load(); });
    renderThreadId = renderThread->get_id();
    
    UE_LOG_INFO(LogCategories::Core, "Render Thread started (ID: %zu)", 
                std::hash<std::thread::id>{}(renderThreadId));
    
    bInitialized = true;
    UE_LOG_INFO(LogCategories::Core, "ThreadManager initialized successfully");
}

void ThreadManager::Shutdown() {
    if (!bInitialized || bShuttingDown) {
        return;
    }
    
    UE_LOG_INFO(LogCategories::Core, "Shutting down ThreadManager...");
    
    bShuttingDown = true;
    
    // Shutdown render command queue
    RenderCommandQueue::Get().Shutdown();
    
    // Notificar threads para que salgan
    frameCondition.notify_all();
    initCondition.notify_all();
    
    // Esperar a que threads terminen
    if (gameThread && gameThread->joinable()) {
        gameThread->join();
        UE_LOG_INFO(LogCategories::Core, "Game Thread stopped");
    }
    
    if (renderThread && renderThread->joinable()) {
        renderThread->join();
        UE_LOG_INFO(LogCategories::Core, "Render Thread stopped");
    }
    
    bInitialized = false;
    UE_LOG_INFO(LogCategories::Core, "ThreadManager shutdown complete");
}

void ThreadManager::GameThreadMain() {
    // Marcar thread como listo
    {
        std::lock_guard<std::mutex> lock(initMutex);
        bGameThreadReady = true;
    }
    initCondition.notify_all();
    
    UE_LOG_INFO(LogCategories::Core, "Game Thread main loop started");
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (bGameThreadRunning && !bShuttingDown) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Clamp delta time (prevenir spikes)
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }
        
        // Ejecutar tick de game thread
        if (gameThreadTickFunction) {
            gameThreadTickFunction(deltaTime);
        }
        
        // Frame limiting
        float targetFrameTime = 1.0f / targetGameFPS.load();
        auto frameEnd = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(frameEnd - currentTime).count();
        
        if (elapsed < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - elapsed)
            );
        }
        
        // Notificar que frame está listo
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            bFrameReady = true;
        }
        frameCondition.notify_one();
    }
    
    bGameThreadRunning = false;
    UE_LOG_INFO(LogCategories::Core, "Game Thread main loop ended");
}

void ThreadManager::RenderThreadMain() {
    // Marcar thread como listo
    {
        std::lock_guard<std::mutex> lock(initMutex);
        bRenderThreadReady = true;
    }
    initCondition.notify_all();
    
    UE_LOG_INFO(LogCategories::Core, "Render Thread main loop started");
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (bRenderThreadRunning && !bShuttingDown) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Clamp delta time
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }
        
        // Ejecutar tick de render thread
        if (renderThreadTickFunction) {
            renderThreadTickFunction(deltaTime);
        }
        
        // Ejecutar comandos de renderizado
        RenderCommandQueue::Get().ExecuteAll();
        
        // Frame limiting
        float targetFrameTime = 1.0f / targetRenderFPS.load();
        auto frameEnd = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(frameEnd - currentTime).count();
        
        if (elapsed < targetFrameTime) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(targetFrameTime - elapsed)
            );
        }
    }
    
    bRenderThreadRunning = false;
    UE_LOG_INFO(LogCategories::Core, "Render Thread main loop ended");
}

bool ThreadManager::IsInGameThread() const {
    return std::this_thread::get_id() == gameThreadId;
}

bool ThreadManager::IsInRenderThread() const {
    return std::this_thread::get_id() == renderThreadId;
}

void ThreadManager::ExecuteInGameThread(std::function<void()> function) {
    if (IsInGameThread()) {
        function();
    } else {
        // Encolar para ejecutar en game thread
        // Por ahora, ejecutar inmediatamente si no estamos en game thread
        // (en implementación completa, usaría una cola de comandos de game thread)
        function();
    }
}

void ThreadManager::ExecuteInRenderThread(std::function<void()> function) {
    if (IsInRenderThread()) {
        function();
    } else {
        // Encolar en render command queue
        RenderCommandQueue::Get().Enqueue(ERenderCommandType::Custom, function);
    }
}

