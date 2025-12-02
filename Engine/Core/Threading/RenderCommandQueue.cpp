#include "RenderCommandQueue.h"
#include "../Log.h"
#include <thread>

RenderCommandQueue& RenderCommandQueue::Get() {
    static RenderCommandQueue instance;
    return instance;
}

void RenderCommandQueue::Enqueue(ERenderCommandType type, std::function<void()> command) {
    if (bShutdown) {
        UE_LOG_WARNING(LogCategories::Core, "Attempting to enqueue render command after shutdown");
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        commandQueue.push(std::make_unique<FRenderCommand>(type, command));
    }
    
    conditionVariable.notify_one();
}

void RenderCommandQueue::EnqueueBatch(const std::vector<FRenderCommand>& commands) {
    if (bShutdown) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (const auto& cmd : commands) {
            commandQueue.push(std::make_unique<FRenderCommand>(cmd.type, cmd.executeFunction));
        }
    }
    
    conditionVariable.notify_one();
}

void RenderCommandQueue::ExecuteAll() {
    std::queue<std::unique_ptr<FRenderCommand>> commandsToExecute;
    
    // Mover comandos a una cola local (minimizar tiempo con mutex)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!commandQueue.empty()) {
            commandsToExecute.push(std::move(commandQueue.front()));
            commandQueue.pop();
        }
    }
    
    // Ejecutar comandos sin mutex
    size_t commandCount = 0;
    while (!commandsToExecute.empty()) {
        auto& command = commandsToExecute.front();
        if (command && command->executeFunction) {
            try {
                command->executeFunction();
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in render command: %s", e.what());
            }
        }
        commandsToExecute.pop();
        commandCount++;
    }
    
    if (commandCount > 0) {
        UE_LOG_VERBOSE(LogCategories::Core, "Executed %zu render commands", commandCount);
    }
}

void RenderCommandQueue::ExecuteUntilEmpty() {
    while (!IsEmpty()) {
        ExecuteAll();
    }
}

void RenderCommandQueue::Clear() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::queue<std::unique_ptr<FRenderCommand>> empty;
    commandQueue.swap(empty);
}

size_t RenderCommandQueue::Size() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return commandQueue.size();
}

bool RenderCommandQueue::IsEmpty() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return commandQueue.empty();
}

void RenderCommandQueue::WaitForCommands() {
    std::unique_lock<std::mutex> lock(queueMutex);
    conditionVariable.wait(lock, [this] {
        return !commandQueue.empty() || bShutdown;
    });
}

void RenderCommandQueue::NotifyCommandsAvailable() {
    conditionVariable.notify_one();
}

void RenderCommandQueue::Shutdown() {
    bShutdown = true;
    conditionVariable.notify_all();
    Clear();
}

