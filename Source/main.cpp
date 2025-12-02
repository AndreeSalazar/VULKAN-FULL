#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "RHI/vulkan_cube.h"
#include "Core/Log.h"
#include "Core/Timer.h"
#include "Core/Threading/RenderCommandQueue.h"
#include "UI/UIManager.h"
#include "UI/Panels/DebugOverlay.h"
#include "UI/Panels/StatsPanel.h"
#include "UI/Panels/ObjectHierarchyPanel.h"
#include "UI/Panels/ViewportPanel.h"
#include "UI/Panels/DetailsPanel.h"
#include "UI/Panels/ContentBrowserPanel.h"
#include "UI/Panels/ConsolePanel.h"
#include "UI/ImGuiWrapper.h"
#include "Rendering/Camera.h"
#include "Input/InputManager.h"

#include <iostream>
#include <stdexcept>

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
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VulkanCube cube;
    FFrameTimer frameTimer;
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

    void initWindow() {
        UE_LOG_INFO(LogCategories::Core, "Initializing GLFW...");
        glfwInit();
        
        // Get primary monitor info
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidMode = glfwGetVideoMode(primaryMonitor);
        int monitorWidth = vidMode->width;
        int monitorHeight = vidMode->height;
        
        UE_LOG_INFO(LogCategories::Core, "Primary monitor resolution: %dx%d", monitorWidth, monitorHeight);
        
        // Determine window size - use 1920x1080 if monitor supports it, otherwise use monitor size
        int windowWidth = DEFAULT_WIDTH;
        int windowHeight = DEFAULT_HEIGHT;
        
        // If monitor is smaller than default, use monitor size
        if (monitorWidth < static_cast<int>(DEFAULT_WIDTH)) {
            windowWidth = monitorWidth;
        }
        if (monitorHeight < static_cast<int>(DEFAULT_HEIGHT)) {
            windowHeight = monitorHeight;
        }
        
        // Clamp to maximum supported resolution
        if (windowWidth > static_cast<int>(MAX_WIDTH)) windowWidth = MAX_WIDTH;
        if (windowHeight > static_cast<int>(MAX_HEIGHT)) windowHeight = MAX_HEIGHT;
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        currentWidth = windowWidth;
        currentHeight = windowHeight;
        
        UE_LOG_INFO(LogCategories::Core, "Creating window: %dx%d (Min: %dx%d, Max: %dx%d)", 
                    windowWidth, windowHeight, MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
        
        window = glfwCreateWindow(windowWidth, windowHeight, 
                                 "Vulkan Cube - WASD: Move | Mouse: Look | ESC: Lock/Unlock | F11: Maximize", 
                                 nullptr, nullptr);
        if (!window) {
            UE_LOG_FATAL(LogCategories::Core, "Failed to create GLFW window!");
            throw std::runtime_error("failed to create GLFW window!");
        }
        
        // Center window on screen
        windowedX = (monitorWidth - windowWidth) / 2;
        windowedY = (monitorHeight - windowHeight) / 2;
        windowedWidth = windowWidth;
        windowedHeight = windowHeight;
        glfwSetWindowPos(window, windowedX, windowedY);
        
        UE_LOG_INFO(LogCategories::Core, "Window created successfully at position (%d, %d)", windowedX, windowedY);
        
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowSizeCallback(window, windowSizeCallback);
        
        // Initialize Input Manager
        InputManager::Get().Initialize(window);
        InputManager::Get().SetMouseLocked(false);
        
        // Set minimum window size
        glfwSetWindowSizeLimits(window, MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        if (app && width > 0 && height > 0) {
            // Clamp resolution to supported range
            if (width < static_cast<int>(MIN_WIDTH)) width = MIN_WIDTH;
            if (height < static_cast<int>(MIN_HEIGHT)) height = MIN_HEIGHT;
            if (width > static_cast<int>(MAX_WIDTH)) width = MAX_WIDTH;
            if (height > static_cast<int>(MAX_HEIGHT)) height = MAX_HEIGHT;
            
            app->bFramebufferResized = true;
            app->currentWidth = width;
            app->currentHeight = height;
            app->cube.MarkFramebufferResized();
            
            // Update camera aspect ratio immediately
            float aspectRatio = (float)width / (float)height;
            app->camera.SetAspectRatio(aspectRatio);
            
            if (app->bIsFullscreen) {
                UE_LOG_INFO(LogCategories::Core, "Framebuffer resized (fullscreen) to %dx%d (Aspect: %.2f)", 
                           width, height, aspectRatio);
            } else {
                UE_LOG_VERBOSE(LogCategories::Core, "Framebuffer resized to %dx%d (Aspect: %.2f)", 
                              width, height, aspectRatio);
            }
        }
    }
    
    static void windowSizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        if (app && width > 0 && height > 0) {
            // Only update if not in fullscreen (fullscreen uses framebuffer callback)
            if (!app->bIsFullscreen) {
                app->windowedWidth = width;
                app->windowedHeight = height;
                app->currentWidth = width;
                app->currentHeight = height;
            }
            
            // Get framebuffer size (more accurate)
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            if (fbWidth > 0 && fbHeight > 0) {
                float aspectRatio = (float)fbWidth / (float)fbHeight;
                app->camera.SetAspectRatio(aspectRatio);
                UE_LOG_VERBOSE(LogCategories::Core, "Window resized to %dx%d (Framebuffer: %dx%d, Aspect: %.2f)", 
                              width, height, fbWidth, fbHeight, aspectRatio);
            }
        }
    }

    void initVulkan() {
        UE_LOG_INFO(LogCategories::RHI, "Initializing Vulkan...");
        cube.initVulkan(window);
        UE_LOG_INFO(LogCategories::RHI, "Vulkan initialized successfully");
        
        // Get initial framebuffer size
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspectRatio = (float)fbWidth / (float)fbHeight;
        
        // Initialize Camera
        camera.SetMode(ECameraMode::FPS);
        camera.SetPosition(Vector3(0.0f, 0.0f, -3.0f));
        camera.SetPerspective(45.0f, aspectRatio, 0.1f, 1000.0f);
        camera.SetMovementSpeed(8.0f);
        camera.SetMouseSensitivity(0.15f);
        
        UE_LOG_INFO(LogCategories::Core, "Camera initialized - Position: (0, 0, -3) | Aspect: %.2f", aspectRatio);
        
        // Initialize ImGui
        UE_LOG_INFO(LogCategories::UI, "Initializing ImGui...");
        bool imguiInitialized = UI::ImGuiWrapper::Get().Initialize(
            window,
            cube.GetInstance(),
            cube.GetPhysicalDevice(),
            cube.GetDevice(),
            cube.GetGraphicsQueue(),
            cube.GetGraphicsQueueFamilyIndex(),
            cube.GetRenderPass(),
            cube.GetDescriptorPool(),
            2,  // minImageCount
            3   // imageCount (MAX_FRAMES_IN_FLIGHT, típicamente 2-3)
        );
        
        if (imguiInitialized) {
            // Aplicar estilo Unreal Engine 5
            UI::ImGuiWrapper::Get().SetStyleUE5();
            UE_LOG_INFO(LogCategories::UI, "ImGui initialized successfully with UE5 style");
        } else {
            UE_LOG_WARNING(LogCategories::UI, "Failed to initialize ImGui, continuing without UI");
        }
    }

    void mainLoop() {
        UE_LOG_INFO(LogCategories::Core, "Entering main loop...");
        UE_LOG_INFO(LogCategories::Core, "Controls: WASD - Move | Mouse - Look | ESC - Lock/Unlock Mouse | F11 - Maximize/Restore | TAB - Toggle UI");
        UE_LOG_INFO(LogCategories::Core, "Supported resolutions: %dx%d to %dx%d", MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
        
        // Initialize UI Manager
        UI::UIManager::Get().Initialize();
        
        // Registrar paneles por defecto
        UI::UIManager::Get().RegisterPanel("DebugOverlay", std::make_shared<UI::DebugOverlay>());
        UI::UIManager::Get().RegisterWindow("StatsPanel", std::make_shared<UI::StatsPanel>());
        UI::UIManager::Get().RegisterWindow("ObjectHierarchy", std::make_shared<UI::ObjectHierarchyPanel>());
        
        // Registrar paneles estilo UE5
        UI::UIManager::Get().RegisterWindow("Viewport", std::make_shared<UI::ViewportPanel>());
        UI::UIManager::Get().RegisterWindow("Details", std::make_shared<UI::DetailsPanel>());
        UI::UIManager::Get().RegisterWindow("ContentBrowser", std::make_shared<UI::ContentBrowserPanel>());
        UI::UIManager::Get().RegisterWindow("Console", std::make_shared<UI::ConsolePanel>());
        
        // Mostrar paneles principales por defecto
        UI::UIManager::Get().SetShowDebugOverlay(true);
        UI::UIManager::Get().ShowWindow("Viewport");
        UI::UIManager::Get().ShowWindow("Details");
        UI::UIManager::Get().ShowWindow("ContentBrowser");
        UI::UIManager::Get().ShowWindow("Console");
        
        // Initialize global frame timer
        GFrameTimer = &frameTimer;
        frameTimer.SetTargetFPS(60.0f);
        frameTimer.SetFrameLimiting(false);
        
        uint32_t frameCount = 0;
        FTimer statsTimer;
        
        UE_LOG_INFO(LogCategories::Core, "Starting main loop...");
        uint32_t loopIteration = 0;
        while (!glfwWindowShouldClose(window)) {
            loopIteration++;
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "First iteration of main loop starting...");
            }
            
            // Tick frame timer
            try {
                frameTimer.Tick();
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in frameTimer.Tick(): %s", e.what());
                break;
            }
            double deltaTime = frameTimer.GetDeltaTime();
            
            // Update Input
            try {
                InputManager::Get().Update();
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in InputManager::Update(): %s", e.what());
                break;
            }
            
            // Handle ESC key to lock/unlock mouse
            if (InputManager::Get().IsKeyJustPressed(Keys::Escape)) {
                bCameraLocked = !bCameraLocked;
                InputManager::Get().SetMouseLocked(bCameraLocked);
                UE_LOG_INFO(LogCategories::Core, "Mouse %s", bCameraLocked ? "LOCKED" : "UNLOCKED");
            }
            
            // Handle TAB key to toggle UI
            static bool uiVisible = true;
            if (InputManager::Get().IsKeyJustPressed(Keys::Tab)) {
                uiVisible = !uiVisible;
                UI::UIManager::Get().SetShowDebugOverlay(uiVisible);
                UI::UIManager::Get().ToggleWindow("StatsPanel");
                UI::UIManager::Get().ToggleWindow("ObjectHierarchy");
                UE_LOG_INFO(LogCategories::Core, "UI %s", uiVisible ? "SHOWN" : "HIDDEN");
            }
            
            // Handle F11 for maximize/restore
            // Use direct GLFW check since F11 might be intercepted by window manager
            static bool f11WasPressed = false;
            bool f11CurrentlyPressed = (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS);
            
            // Also check via InputManager as fallback
            if (!f11CurrentlyPressed) {
                f11CurrentlyPressed = InputManager::Get().IsKeyJustPressed(Keys::F11) || 
                                     InputManager::Get().IsKeyJustPressed(GLFW_KEY_F11);
            }
            
            bool f11JustPressed = f11CurrentlyPressed && !f11WasPressed;
            f11WasPressed = f11CurrentlyPressed;
            
            if (f11JustPressed) {
                UE_LOG_INFO(LogCategories::Core, "=== F11 PRESSED ===");
                UE_LOG_INFO(LogCategories::Core, "Current fullscreen state: %s", 
                           (bIsFullscreen ? "YES" : "NO"));
                GLFWmonitor* currentMonitor = glfwGetWindowMonitor(window);
                bool isCurrentlyFullscreen = (currentMonitor != nullptr);
                
                if (isCurrentlyFullscreen || bIsFullscreen) {
                    // Currently fullscreen - restore to windowed
                    UE_LOG_INFO(LogCategories::Core, "Restoring from fullscreen...");
                    
                    glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, 
                                       windowedWidth, windowedHeight, GLFW_DONT_CARE);
                    glfwSetWindowSizeLimits(window, MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
                    bIsFullscreen = false;
                    
                    // Force window to update
                    glfwPollEvents();
                    glfwWaitEventsTimeout(0.01); // Small wait to ensure window updates
                    
                    // Get actual framebuffer size after restore
                    int fbWidth, fbHeight;
                    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
                    
                    if (fbWidth > 0 && fbHeight > 0) {
                        currentWidth = fbWidth;
                        currentHeight = fbHeight;
                        camera.SetAspectRatio((float)fbWidth / (float)fbHeight);
                        bFramebufferResized = true;
                        cube.MarkFramebufferResized();
                        
                        UE_LOG_INFO(LogCategories::Core, "Window restored to windowed %dx%d (Framebuffer: %dx%d)", 
                                   windowedWidth, windowedHeight, fbWidth, fbHeight);
                    }
                } else {
                    // Currently windowed - save current state and go fullscreen
                    UE_LOG_INFO(LogCategories::Core, "Maximizing to fullscreen...");
                    
                    // Save current window state
                    glfwGetWindowPos(window, &windowedX, &windowedY);
                    glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
                    
                    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
                    if (!primaryMonitor) {
                        UE_LOG_ERROR(LogCategories::Core, "No primary monitor found, cannot maximize");
                    } else {
                        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
                        
                        // Use actual monitor resolution (up to MAX_WIDTH x MAX_HEIGHT)
                        int fullWidth = mode->width;
                        int fullHeight = mode->height;
                        
                        // Clamp to maximum supported resolution
                        if (fullWidth > static_cast<int>(MAX_WIDTH)) fullWidth = MAX_WIDTH;
                        if (fullHeight > static_cast<int>(MAX_HEIGHT)) fullHeight = MAX_HEIGHT;
                        
                        UE_LOG_INFO(LogCategories::Core, "Setting fullscreen to %dx%d", fullWidth, fullHeight);
                        
                        // Set fullscreen with monitor's refresh rate
                        glfwSetWindowMonitor(window, primaryMonitor, 0, 0, fullWidth, fullHeight, mode->refreshRate);
                        bIsFullscreen = true;
                        
                        // Force window to update
                        glfwPollEvents();
                        glfwWaitEventsTimeout(0.01); // Small wait to ensure window updates
                        
                        // Get actual framebuffer size after fullscreen
                        int fbWidth, fbHeight;
                        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
                        
                        if (fbWidth > 0 && fbHeight > 0) {
                            currentWidth = fbWidth;
                            currentHeight = fbHeight;
                            camera.SetAspectRatio((float)fbWidth / (float)fbHeight);
                            bFramebufferResized = true;
                            cube.MarkFramebufferResized();
                            
                            UE_LOG_INFO(LogCategories::Core, "Window maximized to fullscreen %dx%d (Framebuffer: %dx%d)", 
                                       fullWidth, fullHeight, fbWidth, fbHeight);
                        } else {
                            UE_LOG_WARNING(LogCategories::Core, "Failed to get framebuffer size after fullscreen");
                        }
                    }
                }
            }
            
            // Handle framebuffer resize
            if (bFramebufferResized) {
                int fbWidth, fbHeight;
                glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
                if (fbWidth > 0 && fbHeight > 0) {
                    // Update camera aspect ratio
                    camera.SetAspectRatio((float)fbWidth / (float)fbHeight);
                    bFramebufferResized = false;
                    UE_LOG_INFO(LogCategories::Core, "Framebuffer resized - New size: %dx%d", fbWidth, fbHeight);
                }
            }
            
            // Update camera with input
            camera.SetInputState(InputManager::Get().GetCameraInputState());
            camera.Update(deltaTime);
            
            // Process mouse look only when mouse is locked
            if (bCameraLocked) {
                double mouseDX, mouseDY;
                InputManager::Get().GetMouseDelta(mouseDX, mouseDY);
                if (std::abs(mouseDX) > 0.001 || std::abs(mouseDY) > 0.001) {
                    camera.ProcessMouseMovement(static_cast<float>(mouseDX), -static_cast<float>(mouseDY));
                }
            }
            
            // Process mouse scroll
            double scroll = InputManager::Get().GetMouseScrollY();
            if (std::abs(scroll) > 0.001) {
                camera.ProcessMouseScroll(static_cast<float>(scroll));
            }
            
            // Update matrices in renderer (always)
            Matrix4x4 view = camera.GetViewMatrix();
            Matrix4x4 proj = camera.GetProjectionMatrix();
            cube.UpdateMatrices(view.Data(), proj.Data());
            
            // Poll events
            try {
                glfwPollEvents();
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "Events polled, checking window close status...");
                }
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in glfwPollEvents(): %s", e.what());
                break;
            }
            
            // Check if window should close (after polling events)
            if (glfwWindowShouldClose(window)) {
                UE_LOG_INFO(LogCategories::Core, "Window close requested (iteration %u)", loopIteration);
                break;
            }
            
            // ImGui: New frame (must be called before any ImGui calls)
            try {
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "Calling ImGui::NewFrame()...");
                }
                UI::ImGuiWrapper::Get().NewFrame();
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "ImGui::NewFrame() completed");
                }
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in ImGuiWrapper::NewFrame(): %s", e.what());
                break;
            } catch (...) {
                UE_LOG_FATAL(LogCategories::Core, "Unknown exception in ImGuiWrapper::NewFrame()");
                break;
            }
            
            // Update UI data BEFORE creating UI widgets
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "Updating UI data...");
            }
            try {
                auto debugOverlay = std::dynamic_pointer_cast<UI::DebugOverlay>(UI::UIManager::Get().GetPanel("DebugOverlay"));
                if (debugOverlay) {
                    debugOverlay->SetFPS(frameTimer.GetFPS());
                    debugOverlay->SetDeltaTime(deltaTime);
                    debugOverlay->SetFrameCount(frameTimer.GetFrameCount());
                    debugOverlay->SetTotalTime(frameTimer.GetTotalTime());
                    
                    Vector3 pos = camera.GetPosition();
                    debugOverlay->SetCameraPosition(pos.x, pos.y, pos.z);
                    
                    debugOverlay->SetRenderQueueSize(RenderCommandQueue::Get().Size());
                }
                
                auto statsPanel = std::dynamic_pointer_cast<UI::StatsPanel>(UI::UIManager::Get().GetWindow("StatsPanel"));
                if (statsPanel) {
                    statsPanel->UpdateStats(
                        frameTimer.GetFPS(),
                        deltaTime,
                        frameTimer.GetFrameCount(),
                        frameTimer.GetTotalTime()
                    );
                }
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "UI data updated successfully");
                }
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception updating UI data: %s", e.what());
                break;
            }
            
            // Update UI (actualiza lógica de paneles, pero NO renderiza aún)
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "Calling UIManager::Update()...");
            }
            try {
                UI::UIManager::Get().Update(deltaTime);
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "UIManager::Update() completed");
                }
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in UIManager::Update(): %s", e.what());
                break;
            }
            
            // Render UI (crea los widgets de ImGui - DEBE estar entre NewFrame y ImGui::Render)
            // Los paneles llaman ImGui::Begin() aquí, dentro del frame scope
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "Calling UIManager::Render()...");
            }
            try {
                UI::UIManager::Get().Render();
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "UIManager::Render() completed");
                }
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::UI, "Exception in UIManager::Render(): %s", e.what());
                break;
            } catch (...) {
                UE_LOG_ERROR(LogCategories::UI, "Unknown exception in UIManager::Render()");
                break;
            }
            
            // ImGui::Render() DEBE llamarse DESPUÉS de todos los widgets pero ANTES del command buffer
            // Esto prepara los datos de renderizado para GPU
            if (UI::ImGuiWrapper::Get().IsInitialized()) {
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "Calling PrepareRender()...");
                }
                try {
                    UI::ImGuiWrapper::Get().PrepareRender(); // Llama ImGui::Render()
                    if (loopIteration == 1) {
                        UE_LOG_INFO(LogCategories::Core, "PrepareRender() completed");
                    }
                } catch (const std::exception& e) {
                    UE_LOG_ERROR(LogCategories::UI, "Exception in PrepareRender(): %s", e.what());
                    break;
                } catch (...) {
                    UE_LOG_ERROR(LogCategories::UI, "Unknown exception in PrepareRender()");
                    break;
                }
            }
            
            // Render frame (includes ImGui rendering in command buffer)
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "About to enter drawFrame scope (after PrepareRender)...");
                fflush(stdout);  // Force flush to see logs immediately
            }
            {
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "Inside drawFrame scope, creating SCOPED_TIMER_SILENT...");
                    fflush(stdout);
                }
                SCOPED_TIMER_SILENT("DrawFrame");
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "SCOPED_TIMER_SILENT created, calling drawFrame()...");
                    fflush(stdout);
                }
                try {
                    cube.drawFrame();
                    if (loopIteration == 1) {
                        UE_LOG_INFO(LogCategories::Core, "First drawFrame() completed successfully!");
                        fflush(stdout);
                    }
                } catch (const std::exception& e) {
                    UE_LOG_FATAL(LogCategories::Core, "Exception in drawFrame() (iteration %u): %s", loopIteration, e.what());
                    fflush(stdout);
                    break; // Salir del loop en caso de error fatal
                } catch (...) {
                    UE_LOG_FATAL(LogCategories::Core, "Unknown exception in drawFrame() (iteration %u)", loopIteration);
                    fflush(stdout);
                    break;
                }
                if (loopIteration == 1) {
                    UE_LOG_INFO(LogCategories::Core, "About to exit drawFrame scope...");
                    fflush(stdout);
                }
            }
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "Exited drawFrame scope successfully");
                fflush(stdout);
            }
            
            // Update textures after frame ends (safe to do transfer operations now)
            if (UI::ImGuiWrapper::Get().IsInitialized()) {
                UI::ImGuiWrapper::Get().PostRender();
            }
            
            // Limit frame rate if enabled
            try {
                frameTimer.LimitFrameRate();
            } catch (const std::exception& e) {
                UE_LOG_ERROR(LogCategories::Core, "Exception in frameTimer.LimitFrameRate(): %s", e.what());
                break;
            }
            
            // Print stats every second
            frameCount++;
            if (statsTimer.HasTimeElapsed(1.0)) {
                if (bShowStats) {
                    Vector3 pos = camera.GetPosition();
                    UE_LOG_INFO(LogCategories::Core, "%s | Camera: (%.2f, %.2f, %.2f)", 
                                frameTimer.GetStatsString().c_str(), pos.x, pos.y, pos.z);
                }
                statsTimer.Reset();
            }
            
            // Log first successful frame completion
            if (loopIteration == 1) {
                UE_LOG_INFO(LogCategories::Core, "First frame completed successfully! Loop continuing...");
            }
        }
        
        UE_LOG_INFO(LogCategories::Core, "Main loop ended. Total frames: %llu", frameTimer.GetFrameCount());
        try {
            cube.waitDeviceIdle();
            UE_LOG_INFO(LogCategories::Core, "Vulkan device idle, cleanup complete");
        } catch (const std::exception& e) {
            UE_LOG_ERROR(LogCategories::Core, "Exception during cleanup: %s", e.what());
        }
    }

    void cleanup() {
        UI::UIManager::Get().Shutdown();
        InputManager::Get().Shutdown();
        cube.cleanup();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    // Initialize logging system
    FLog::Initialize("Engine.log");
    FLog::SetVerbosity(ELogVerbosity::Display);
    FLog::SetConsoleOutput(true);
    FLog::SetFileOutput(true);
    
    {
        SCOPED_TIMER("EngineInitialization");
        
        UE_LOG_INFO(LogCategories::Core, "=== Vulkan Engine Starting ===");
        UE_LOG_INFO(LogCategories::Core, "Initializing engine systems...");
        
        App app;

        try {
            UE_LOG_INFO(LogCategories::Core, "Starting application main loop...");
            app.run();
            UE_LOG_INFO(LogCategories::Core, "Application main loop ended normally");
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

