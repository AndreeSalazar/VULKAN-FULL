# Threading System - Game Thread / Render Thread Separation

## 📋 Descripción

Sistema de threads separados para el motor gráfico, similar a Unreal Engine 5:
- **Game Thread**: Lógica del juego, input, física, actualización de objetos
- **Render Thread**: Renderizado, comandos de GPU, presentación

## 🔧 Componentes

### 1. RenderCommandQueue
Cola thread-safe para comandos de renderizado que se ejecutan en el render thread.

**Características**:
- Thread-safe con mutex y condition variables
- Cola FIFO de comandos
- Ejecución batch de comandos
- Notificación cuando hay comandos disponibles

**Uso**:
```cpp
// Encolar comando desde cualquier thread
ENQUEUE_RENDER_COMMAND(UpdateCamera, [this]() {
    cube.UpdateMatrices(viewMatrix.Data(), projMatrix.Data());
});

// En render thread, ejecutar todos los comandos
RenderCommandQueue::Get().ExecuteAll();
```

### 2. ThreadManager
Gestión de threads del motor (Game Thread y Render Thread).

**Características**:
- Inicialización y shutdown de threads
- Verificación de thread actual (IsInGameThread, IsInRenderThread)
- Callbacks configurables para cada thread
- Frame limiting por thread
- Sincronización entre threads

**Uso**:
```cpp
auto& threadMgr = ThreadManager::Get();

// Configurar callbacks
threadMgr.SetGameThreadTickFunction([this](float deltaTime) {
    // Game logic aquí
});

threadMgr.SetRenderThreadTickFunction([this](float deltaTime) {
    // Render logic aquí
});

// Inicializar
threadMgr.Initialize();

// Verificar thread
if (threadMgr.IsInGameThread()) {
    // Estamos en game thread
}
```

## 🎯 Arquitectura

```
Main Thread (GLFW/Vulkan context)
├── Window events (glfwPollEvents)
├── Vulkan present (debe estar en main thread)
└── Render command execution

Game Thread
├── Input handling
├── Camera updates
├── Object updates
├── Physics simulation
└── Enqueue render commands

Render Thread
├── Execute render commands
├── GPU command recording
└── Resource management
```

## ⚠️ Notas Importantes

### Restricciones de Vulkan/GLFW:
1. **GLFW Events**: `glfwPollEvents()` debe estar en el thread principal
2. **Vulkan Present**: Debe estar en el mismo thread que la creación del contexto
3. **Vulkan Commands**: Pueden ser grabados en cualquier thread, pero submission debe ser sincronizado

### Patrón Recomendado:
- **Game Thread**: Actualiza lógica y encola comandos
- **Render Thread**: Ejecuta comandos y graba command buffers
- **Main Thread**: Poll events y presenta frames

## 📚 Ejemplo de Uso

Ver `Source/main_threaded.cpp` para un ejemplo completo de integración.

## 🚀 Próximas Mejoras

- [ ] Task Graph system
- [ ] Async asset loading thread
- [ ] Physics thread
- [ ] Audio thread
- [ ] Thread-safe resource management
- [ ] Profiling de threads

