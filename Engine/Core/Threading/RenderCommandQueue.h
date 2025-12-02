#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>

// ============================================================================
// RenderCommandQueue - Cola thread-safe para comandos de renderizado
// Similar a Unreal Engine's Render Command Queue
// ============================================================================

// Tipos de comandos de renderizado
enum class ERenderCommandType {
    Draw,
    UpdateUniforms,
    UpdateViewport,
    UpdateCamera,
    CreateResource,
    DestroyResource,
    Custom
};

// Estructura base para comandos de renderizado
struct FRenderCommand {
    ERenderCommandType type;
    std::function<void()> executeFunction;
    
    FRenderCommand(ERenderCommandType cmdType, std::function<void()> func)
        : type(cmdType), executeFunction(func) {}
};

class RenderCommandQueue {
public:
    static RenderCommandQueue& Get();
    
    // Agregar comando a la cola (thread-safe, puede ser llamado desde cualquier thread)
    void Enqueue(ERenderCommandType type, std::function<void()> command);
    
    // Encolar múltiples comandos
    void EnqueueBatch(const std::vector<FRenderCommand>& commands);
    
    // Ejecutar todos los comandos en la cola (debe ser llamado desde render thread)
    void ExecuteAll();
    
    // Ejecutar comandos hasta que la cola esté vacía
    void ExecuteUntilEmpty();
    
    // Limpiar cola (thread-safe)
    void Clear();
    
    // Obtener tamaño de la cola
    size_t Size() const;
    
    // Verificar si la cola está vacía
    bool IsEmpty() const;
    
    // Esperar hasta que haya comandos (para render thread)
    void WaitForCommands();
    
    // Notificar que hay comandos (para despertar render thread)
    void NotifyCommandsAvailable();
    
    // Shutdown (limpiar y detener notificaciones)
    void Shutdown();

private:
    RenderCommandQueue() = default;
    ~RenderCommandQueue() = default;
    RenderCommandQueue(const RenderCommandQueue&) = delete;
    RenderCommandQueue& operator=(const RenderCommandQueue&) = delete;
    
    mutable std::mutex queueMutex;
    std::queue<std::unique_ptr<FRenderCommand>> commandQueue;
    std::condition_variable conditionVariable;
    std::atomic<bool> bShutdown{false};
};

// Macros útiles para encolar comandos
#define ENQUEUE_RENDER_COMMAND(Type, Lambda) \
    RenderCommandQueue::Get().Enqueue(ERenderCommandType::Type, Lambda)

#define ENQUEUE_RENDER_COMMAND_UNIQUE(UniqueName, Type, Lambda) \
    RenderCommandQueue::Get().Enqueue(ERenderCommandType::Type, Lambda)

