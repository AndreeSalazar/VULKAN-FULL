#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "RHI/vulkan_cube.h"
#include "Core/Log.h"
#include "Core/Timer.h"
#include "Core/Threading/ThreadManager.h"
#include "Core/Threading/RenderCommandQueue.h"
#include "Rendering/Camera.h"
#include "Input/InputManager.h"

#include <iostream>
#include <stdexcept>
#include <atomic>
#include <mutex>

// Resolution settings
const uint32_t MIN_WIDTH = 800;
const uint32_t MIN_HEIGHT = 600;
const uint32_t DEFAULT_WIDTH = 1920;
const uint32_t DEFAULT_HEIGHT = 1080;
const uint32_t MAX_WIDTH = 1920;
const uint32_t MAX_HEIGHT = 1080;

class App {
public:
    void run() {
        initWindow();
        initVulkan();
        initThreading();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VulkanCube cube;
    FFrameTimer frameTimer;
    FFrameTimer renderTimer;
    Camera camera;
    bool bShowStats = true;
    bool bCameraLocked = false;
    bool bFramebufferResized = false;
    int currentWidth = DEFAULT_WIDTH;
    int currentHeight = DEFAULT_HEIGHT;
    bool bIsFullscreen = false;
    int windowedX = 100;
    int windowedY = 100;
    int windowedWidth = DEFAULT_WIDTH;
    int windowedHeight = DEFAULT_HEIGHT;
    
    // Thread-safe shared data
    std::mutex cameraDataMutex;
    Matrix4x4 sharedViewMatrix;
    Matrix4x4 sharedProjMatrix;
    std::atomic<bool> bMatricesDirty{false};

    void initWindow() {
        UE_LOG_INFO(LogCategories::Core, "Initializing GLFW...");
        glfwInit();
        
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidMode = glfwGetVideoMode(primaryMonitor);
        int monitorWidth = vidMode->width;
        int monitorHeight = vidMode->height;
        
        UE_LOG_INFO(LogCategories::Core, "Primary monitor resolution: %dx%d", monitorWidth, monitorHeight);
        
        int windowWidth = DEFAULT_WIDTH;
        int windowHeight = DEFAULT_HEIGHT;
        
        if (monitorWidth < static_cast<int>(DEFAULT_WIDTH)) windowWidth = monitorWidth;
        if (monitorHeight < static_cast<int>(DEFAULT_HEIGHT)) windowHeight = monitorHeight;
        
        if (windowWidth > static_cast<int>(MAX_WIDTH)) windowWidth = MAX_WIDTH;
        if (windowHeight > static_cast<int>(MAX_HEIGHT)) windowHeight = MAX_HEIGHT;
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        currentWidth = windowWidth;
        currentHeight = windowHeight;
        
        window = glfwCreateWindow(windowWidth, windowHeight, 
                                 "Vulkan Cube - Threaded | WASD: Move | Mouse: Look | ESC: Lock/Unlock | F11: Maximize", 
                                 nullptr, nullptr);
        if (!window) {
            UE_LOG_FATAL(LogCategories::Core, "Failed to create GLFW window!");
            throw std::runtime_error("failed to create GLFW window!");
        }
        
        windowedX = (monitorWidth - windowWidth) / 2;
        windowedY = (monitorHeight - windowHeight) / 2;
        windowedWidth = windowWidth;
        windowedHeight = windowHeight;
        glfwSetWindowPos(window, windowedX, windowedY);
        
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowSizeCallback(window, windowSizeCallback);
        
        InputManager::Get().Initialize(window);
        InputManager::Get().SetMouseLocked(false);
        
        glfwSetWindowSizeLimits(window, MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
        
        UE_LOG_INFO(LogCategories::Core, "Window created successfully at position (%d, %d)", windowedX, windowedY);
    }

    void initVulkan() {
        cube.initVulkan(window);
        
        float aspectRatio = (float)currentWidth / (float)currentHeight;
        camera.SetMode(ECameraMode::FPS);
        camera.SetPosition(Vector3(0.0f, 0.0f, -3.0f));
        camera.SetPerspective(45.0f, aspectRatio, 0.1f, 1000.0f);
        camera.SetMovementSpeed(8.0f);
        camera.SetMouseSensitivity(0.15f);
        
        UE_LOG_INFO(LogCategories::Core, "Camera initialized - Position: (0, 0, -3) | Aspect: %.2f", aspectRatio);
    }
    
    void initThreading() {
        UE_LOG_INFO(LogCategories::Core, "Initializing threading system...");
        
        // Configurar callbacks
        auto& threadMgr = ThreadManager::Get();
        
        // Game Thread Tick (lógica del juego, input, cámara)
        threadMgr.SetGameThreadTickFunction([this](float deltaTime) {
            gameThreadTick(deltaTime);
        });
        
        // Render Thread Tick (renderizado)
        threadMgr.SetRenderThreadTickFunction([this](float deltaTime) {
            renderThreadTick(deltaTime);
        });
        
        threadMgr.SetTargetGameFPS(60.0f);
        threadMgr.SetTargetRenderFPS(60.0f);
        
        threadMgr.Initialize();
        
        UE_LOG_INFO(LogCategories::Core, "Threading system initialized");
    }
    
    void gameThreadTick(float deltaTime) {
        // Actualizar input
        InputManager::Get().Update();
        
        // Manejar input y cámara
        handleInput();
        updateCamera(deltaTime);
        
        // Actualizar matrices compartidas (thread-safe)
        {
            std::lock_guard<std::mutex> lock(cameraDataMutex);
            sharedViewMatrix = camera.GetViewMatrix();
            sharedProjMatrix = camera.GetProjectionMatrix();
            bMatricesDirty = true;
        }
        
        // Encolar comando para actualizar matrices en render thread
        ENQUEUE_RENDER_COMMAND(UpdateCamera, [this]() {
            if (bMatricesDirty) {
                std::lock_guard<std::mutex> lock(cameraDataMutex);
                cube.UpdateMatrices(sharedViewMatrix.Data(), sharedProjMatrix.Data());
                bMatricesDirty = false;
            }
        });
    }
    
    void renderThreadTick(float deltaTime) {
        // Ejecutar comandos de renderizado
        RenderCommandQueue::Get().ExecuteAll();
        
        // Renderizar frame (esto debe estar en render thread)
        // Nota: GLFW requiere que glfwPollEvents esté en el main thread,
        // así que hacemos render desde main loop pero usando comandos encolados
    }
    
    void handleInput() {
        // ESC para lock/unlock mouse
        if (InputManager::Get().IsKeyJustPressed(Keys::Escape)) {
            bCameraLocked = !bCameraLocked;
            InputManager::Get().SetMouseLocked(bCameraLocked);
            UE_LOG_INFO(LogCategories::Core, "Mouse %s", bCameraLocked ? "LOCKED" : "UNLOCKED");
        }
        
        // F11 para fullscreen (debe estar en main thread por GLFW)
        static bool f11WasPressed = false;
        bool f11CurrentlyPressed = (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS);
        if (!f11CurrentlyPressed) {
            f11CurrentlyPressed = InputManager::Get().IsKeyJustPressed(Keys::F11);
        }
        
        bool f11JustPressed = f11CurrentlyPressed && !f11WasPressed;
        f11WasPressed = f11CurrentlyPressed;
        
        if (f11JustPressed) {
            toggleFullscreen();
        }
    }
    
    void updateCamera(float deltaTime) {
        // Actualizar cámara con input
        camera.SetInputState(InputManager::Get().GetCameraInputState());
        camera.Update(deltaTime);
        
        // Mouse look
        if (bCameraLocked) {
            double mouseDX, mouseDY;
            InputManager::Get().GetMouseDelta(mouseDX, mouseDY);
            if (std::abs(mouseDX) > 0.001 || std::abs(mouseDY) > 0.001) {
                camera.ProcessMouseMovement(static_cast<float>(mouseDX), -static_cast<float>(mouseDY));
            }
        }
        
        // Mouse scroll
        double scroll = InputManager::Get().GetMouseScrollY();
        if (std::abs(scroll) > 0.001) {
            camera.ProcessMouseScroll(static_cast<float>(scroll));
        }
    }
    
    void toggleFullscreen() {
        GLFWmonitor* currentMonitor = glfwGetWindowMonitor(window);
        bool isCurrentlyFullscreen = (currentMonitor != nullptr);
        
        if (isCurrentlyFullscreen || bIsFullscreen) {
            // Restaurar a windowed
            glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, 
                               windowedWidth, windowedHeight, GLFW_DONT_CARE);
            bIsFullscreen = false;
            UE_LOG_INFO(LogCategories::Core, "Window restored to windowed");
        } else {
            // Ir a fullscreen
            glfwGetWindowPos(window, &windowedX, &windowedY);
            glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
            
            GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
            if (primaryMonitor) {
                const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
                int fullWidth = mode->width;
                int fullHeight = mode->height;
                
                if (fullWidth > static_cast<int>(MAX_WIDTH)) fullWidth = MAX_WIDTH;
                if (fullHeight > static_cast<int>(MAX_HEIGHT)) fullHeight = MAX_HEIGHT;
                
                glfwSetWindowMonitor(window, primaryMonitor, 0, 0, fullWidth, fullHeight, mode->refreshRate);
                bIsFullscreen = true;
                UE_LOG_INFO(LogCategories::Core, "Window maximized to fullscreen %dx%d", fullWidth, fullHeight);
            }
        }
    }

    void mainLoop() {
        UE_LOG_INFO(LogCategories::Core, "Entering threaded main loop...");
        UE_LOG_INFO(LogCategories::Core, "Game Thread and Render Thread are running separately");
        UE_LOG_INFO(LogCategories::Core, "Controls: WASD - Move | Mouse - Look | ESC - Lock/Unlock Mouse | F11 - Maximize/Restore");
        
        GFrameTimer = &frameTimer;
        frameTimer.SetTargetFPS(60.0f);
        frameTimer.SetFrameLimiting(false);
        
        uint32_t frameCount = 0;
        FTimer statsTimer;
        
        // Main loop (maneja GLFW events y render)
        // Nota: GLFW requiere que glfwPollEvents y present estén en el main thread
        while (!glfwWindowShouldClose(window)) {
            frameTimer.Tick();
            
            // Poll events (debe estar en main thread)
            glfwPollEvents();
            
            // Render frame (usando comandos encolados desde game/render threads)
            // Ejecutar comandos pendientes
            RenderCommandQueue::Get().ExecuteAll();
            
            // Draw frame (Vulkan present debe estar en el mismo contexto que la creación)
            cube.drawFrame();
            
            frameTimer.LimitFrameRate();
            
            // Stats
            frameCount++;
            if (statsTimer.HasTimeElapsed(1.0)) {
                if (bShowStats) {
                    Vector3 pos = camera.GetPosition();
                    size_t queueSize = RenderCommandQueue::Get().Size();
                    UE_LOG_INFO(LogCategories::Core, "%s | Camera: (%.2f, %.2f, %.2f) | Render Queue: %zu", 
                                frameTimer.GetStatsString().c_str(), pos.x, pos.y, pos.z, queueSize);
                }
                statsTimer.Reset();
            }
        }
        
        UE_LOG_INFO(LogCategories::Core, "Main loop ended. Total frames: %llu", frameTimer.GetFrameCount());
        
        // Shutdown threads
        ThreadManager::Get().Shutdown();
        
        cube.waitDeviceIdle();
    }

    void cleanup() {
        InputManager::Get().Shutdown();
        cube.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        if (app && width > 0 && height > 0) {
            if (width < static_cast<int>(MIN_WIDTH)) width = MIN_WIDTH;
            if (height < static_cast<int>(MIN_HEIGHT)) height = MIN_HEIGHT;
            if (width > static_cast<int>(MAX_WIDTH)) width = MAX_WIDTH;
            if (height > static_cast<int>(MAX_HEIGHT)) height = MAX_HEIGHT;
            
            app->bFramebufferResized = true;
            app->currentWidth = width;
            app->currentHeight = height;
            app->cube.MarkFramebufferResized();
            
            float aspectRatio = (float)width / (float)height;
            app->camera.SetAspectRatio(aspectRatio);
            
            UE_LOG_INFO(LogCategories::Core, "Framebuffer resized to %dx%d (Aspect: %.2f)", width, height, aspectRatio);
        }
    }
    
    static void windowSizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        if (app && width > 0 && height > 0 && !app->bIsFullscreen) {
            app->windowedWidth = width;
            app->windowedHeight = height;
            app->currentWidth = width;
            app->currentHeight = height;
        }
    }
};

int main() {
    FLog::Initialize("Engine.log");
    FLog::SetVerbosity(ELogVerbosity::Display);
    FLog::SetConsoleOutput(true);
    FLog::SetFileOutput(true);
    
    {
        SCOPED_TIMER("EngineInitialization");
        
        UE_LOG_INFO(LogCategories::Core, "=== Vulkan Engine Starting (Threaded) ===");
        UE_LOG_INFO(LogCategories::Core, "Initializing engine systems...");
        
        App app;

        try {
            UE_LOG_INFO(LogCategories::Core, "Starting threaded application...");
            app.run();
            UE_LOG_INFO(LogCategories::Core, "Application ended normally");
        } catch (const std::exception& e) {
            UE_LOG_ERROR(LogCategories::Core, "Exception occurred: %s", e.what());
            FLog::Shutdown();
            return EXIT_FAILURE;
        } catch (...) {
            UE_LOG_FATAL(LogCategories::Core, "Unknown exception occurred!");
            FLog::Shutdown();
            return EXIT_FAILURE;
        }
    }
    
    UE_LOG_INFO(LogCategories::Core, "=== Vulkan Engine Shutting Down ===");
    FLog::Shutdown();

    return EXIT_SUCCESS;
}

