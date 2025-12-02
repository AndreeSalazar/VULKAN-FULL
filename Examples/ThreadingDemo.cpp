#include "Core/Log.h"
#include "Core/Threading/ThreadManager.h"
#include "Core/Threading/RenderCommandQueue.h"
#include <thread>
#include <chrono>

// Programa de demostración del sistema de threading
int main() {
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "╔══════════════════════════════════════════════════════════╗");
    UE_LOG_INFO(LogCategories::Core, "║                                                          ║");
    UE_LOG_INFO(LogCategories::Core, "║   Threading System - Programa de Demostración            ║");
    UE_LOG_INFO(LogCategories::Core, "║                                                          ║");
    UE_LOG_INFO(LogCategories::Core, "╚══════════════════════════════════════════════════════════╝");
    UE_LOG_INFO(LogCategories::Core, "");
    
    auto& threadMgr = ThreadManager::Get();
    
    // Configurar callbacks
    int gameTickCount = 0;
    int renderTickCount = 0;
    
    threadMgr.SetGameThreadTickFunction([&gameTickCount](float deltaTime) {
        gameTickCount++;
        if (gameTickCount % 60 == 0) {
            UE_LOG_INFO(LogCategories::Core, "🎮 Game Thread Tick #%d (Delta: %.3fms)", 
                       gameTickCount, deltaTime * 1000.0f);
        }
    });
    
    threadMgr.SetRenderThreadTickFunction([&renderTickCount](float deltaTime) {
        renderTickCount++;
        if (renderTickCount % 60 == 0) {
            UE_LOG_INFO(LogCategories::Core, "🎨 Render Thread Tick #%d (Delta: %.3fms)", 
                       renderTickCount, deltaTime * 1000.0f);
        }
    });
    
    threadMgr.SetTargetGameFPS(60.0f);
    threadMgr.SetTargetRenderFPS(60.0f);
    
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🚀 Inicializando ThreadManager...");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    threadMgr.Initialize();
    
    // Demostrar RenderCommandQueue
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "📋 DEMOSTRACIÓN: RenderCommandQueue");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    auto& renderQueue = RenderCommandQueue::Get();
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Encolando 5 comandos de renderizado...");
    
    for (int i = 0; i < 5; i++) {
        renderQueue.Enqueue(ERenderCommandType::Custom, [i]() {
            UE_LOG_INFO(LogCategories::Core, "  ✓ Comando ejecutado #%d", i + 1);
        });
    }
    
    UE_LOG_INFO(LogCategories::Core, "Comandos en cola: %zu", renderQueue.Size());
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Ejecutando comandos...");
    renderQueue.ExecuteAll();
    
    // Verificar thread IDs
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🔍 DEMOSTRACIÓN: Thread IDs");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Main Thread ID: %zu", 
                std::hash<std::thread::id>{}(std::this_thread::get_id()));
    UE_LOG_INFO(LogCategories::Core, "Game Thread ID: %zu", 
                std::hash<std::thread::id>{}(threadMgr.GetGameThreadId()));
    UE_LOG_INFO(LogCategories::Core, "Render Thread ID: %zu", 
                std::hash<std::thread::id>{}(threadMgr.GetRenderThreadId()));
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Verificando thread actual:");
    UE_LOG_INFO(LogCategories::Core, "  IsInGameThread: %s", 
                threadMgr.IsInGameThread() ? "SÍ" : "NO");
    UE_LOG_INFO(LogCategories::Core, "  IsInRenderThread: %s", 
                threadMgr.IsInRenderThread() ? "SÍ" : "NO");
    
    // Ejecutar comandos desde otro thread
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "🔄 DEMOSTRACIÓN: Thread-Safe Commands");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "Encolando comandos desde main thread...");
    
    for (int i = 0; i < 3; i++) {
        ENQUEUE_RENDER_COMMAND(Custom, [i]() {
            UE_LOG_INFO(LogCategories::Core, "  ↻ Comando #%d ejecutado desde render queue", i + 1);
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    UE_LOG_INFO(LogCategories::Core, "Comandos pendientes: %zu", renderQueue.Size());
    UE_LOG_INFO(LogCategories::Core, "Ejecutando comandos pendientes...");
    renderQueue.ExecuteAll();
    
    // Esperar un poco para ver los ticks
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "⏱️  Esperando 2 segundos para observar ticks...");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "📊 RESUMEN");
    UE_LOG_INFO(LogCategories::Core, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    UE_LOG_INFO(LogCategories::Core, "");
    UE_LOG_INFO(LogCategories::Core, "  Game Thread Ticks:   %d", gameTickCount);
    UE_LOG_INFO(LogCategories::Core, "  Render Thread Ticks: %d", renderTickCount);
    UE_LOG_INFO(LogCategories::Core, "");
    
    UE_LOG_INFO(LogCategories::Core, "╔══════════════════════════════════════════════════════════╗");
    UE_LOG_INFO(LogCategories::Core, "║  ✅ Demostración completada exitosamente               ║");
    UE_LOG_INFO(LogCategories::Core, "╚══════════════════════════════════════════════════════════╝");
    UE_LOG_INFO(LogCategories::Core, "");
    
    UE_LOG_INFO(LogCategories::Core, "Shutting down ThreadManager...");
    threadMgr.Shutdown();
    
    return 0;
}

