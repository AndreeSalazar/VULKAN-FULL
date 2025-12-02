#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "Rendering/Camera.h"

// GLFW key constants (if not already defined)
#ifndef GLFW_KEY_F11
#define GLFW_KEY_F11 300
#endif

// ============================================================================
// InputManager - Gestión de entrada (teclado y mouse)
// ============================================================================

class InputManager {
public:
    static InputManager& Get();
    
    void Initialize(GLFWwindow* window);
    void Update();
    void Shutdown();
    
    // Keyboard
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyReleased(int key) const;
    
    // Mouse
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonJustPressed(int button) const;
    void GetMousePosition(double& x, double& y) const;
    void GetMouseDelta(double& dx, double& dy) const;
    double GetMouseScrollY() const;
    
    // Camera input state (for Camera class)
    Camera::InputState GetCameraInputState() const;
    
    // Callbacks (set from GLFW)
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Settings
    void SetMouseLocked(bool locked);
    bool IsMouseLocked() const { return mouseLocked; }
    
private:
    InputManager() = default;
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    GLFWwindow* window = nullptr;
    
    // Keyboard state
    bool keys[512] = {false};
    bool keysLastFrame[512] = {false};
    
    // Mouse state
    bool mouseButtons[8] = {false};
    bool mouseButtonsLastFrame[8] = {false};
    double mouseX = 0.0;
    double mouseY = 0.0;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    double mouseDeltaX = 0.0;
    double mouseDeltaY = 0.0;
    double mouseScrollY = 0.0;
    
    bool mouseLocked = false;
    bool firstMouse = true;
};

// Key codes (GLFW key codes)
namespace Keys {
    constexpr int W = 87;
    constexpr int A = 65;
    constexpr int S = 83;
    constexpr int D = 68;
    constexpr int Q = 81;
    constexpr int E = 69;
    constexpr int Space = 32;
    constexpr int LeftShift = 340;
    constexpr int Escape = 256;
    constexpr int Tab = 258;
    constexpr int F11 = 300;
}

// Mouse buttons (GLFW button codes)
namespace MouseButton {
    constexpr int Left = 0;
    constexpr int Right = 1;
    constexpr int Middle = 2;
}

