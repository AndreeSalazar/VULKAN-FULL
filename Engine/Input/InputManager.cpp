#include "InputManager.h"

InputManager& InputManager::Get() {
    static InputManager instance;
    return instance;
}

void InputManager::Initialize(GLFWwindow* window) {
    this->window = window;
    
    // Set callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}

void InputManager::Update() {
    // Copy current state to last frame (for just-pressed detection)
    for (int i = 0; i < 512; i++) {
        keysLastFrame[i] = keys[i];
    }
    for (int i = 0; i < 8; i++) {
        mouseButtonsLastFrame[i] = mouseButtons[i];
    }
    
    // Save previous mouse position BEFORE updating
    lastMouseX = mouseX;
    lastMouseY = mouseY;
    
    // Update keyboard state from GLFW (keys are updated in callbacks, but check current state)
    // This ensures we have the latest state
    
    // Update mouse position and calculate delta
    if (mouseLocked) {
        // Get mouse delta from GLFW
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        
        if (firstMouse) {
            mouseX = x;
            mouseY = y;
            lastMouseX = x;
            lastMouseY = y;
            firstMouse = false;
            mouseDeltaX = 0.0;
            mouseDeltaY = 0.0;
        } else {
            // Calculate delta before recentering
            mouseDeltaX = x - lastMouseX;
            mouseDeltaY = y - lastMouseY;
            
            // Center cursor if locked
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            
            // Update stored position to center
            mouseX = width / 2.0;
            mouseY = height / 2.0;
        }
    } else {
        // Not locked - just track position
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseDeltaX = mouseX - lastMouseX;
        mouseDeltaY = mouseY - lastMouseY;
    }
    
    // Don't reset scroll - let it accumulate until consumed
    // mouseScrollY is reset by Camera after use
}

void InputManager::Shutdown() {
    if (window) {
        glfwSetKeyCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
    }
}

bool InputManager::IsKeyPressed(int key) const {
    if (key < 0 || key >= 512) return false;
    return keys[key];
}

bool InputManager::IsKeyJustPressed(int key) const {
    if (key < 0 || key >= 512) return false;
    return keys[key] && !keysLastFrame[key];
}

bool InputManager::IsKeyReleased(int key) const {
    if (key < 0 || key >= 512) return false;
    return !keys[key] && keysLastFrame[key];
}

bool InputManager::IsMouseButtonPressed(int button) const {
    if (button < 0 || button >= 8) return false;
    return mouseButtons[button];
}

bool InputManager::IsMouseButtonJustPressed(int button) const {
    if (button < 0 || button >= 8) return false;
    return mouseButtons[button] && !mouseButtonsLastFrame[button];
}

void InputManager::GetMousePosition(double& x, double& y) const {
    x = mouseX;
    y = mouseY;
}

void InputManager::GetMouseDelta(double& dx, double& dy) const {
    dx = mouseDeltaX;
    dy = mouseDeltaY;
}

double InputManager::GetMouseScrollY() const {
    return mouseScrollY;
}

Camera::InputState InputManager::GetCameraInputState() const {
    Camera::InputState state;
    
    // Keyboard
    state.forward = IsKeyPressed(Keys::W);
    state.backward = IsKeyPressed(Keys::S);
    state.left = IsKeyPressed(Keys::D);  // D = izquierda
    state.right = IsKeyPressed(Keys::A); // A = derecha
    state.up = IsKeyPressed(Keys::Space) || IsKeyPressed(Keys::E);
    state.down = IsKeyPressed(Keys::LeftShift) || IsKeyPressed(Keys::Q);
    
    // Mouse
    state.mouseX = mouseX;
    state.mouseY = mouseY;
    state.mouseScroll = mouseScrollY;
    state.mouseLeft = IsMouseButtonPressed(MouseButton::Left);
    state.mouseRight = IsMouseButtonPressed(MouseButton::Right);
    state.mouseMiddle = IsMouseButtonPressed(MouseButton::Middle);
    
    // Mouse delta
    state.deltaMouseX = mouseDeltaX;
    state.deltaMouseY = mouseDeltaY;
    
    return state;
}

void InputManager::SetMouseLocked(bool locked) {
    mouseLocked = locked;
    firstMouse = true;
    
    if (window) {
        if (locked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// GLFW Callbacks
void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto& input = Get();
    if (key >= 0 && key < 512) {
        if (action == GLFW_PRESS) {
            input.keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            input.keys[key] = false;
        }
    }
}

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto& input = Get();
    if (button >= 0 && button < 8) {
        if (action == GLFW_PRESS) {
            input.mouseButtons[button] = true;
        } else if (action == GLFW_RELEASE) {
            input.mouseButtons[button] = false;
        }
    }
}

void InputManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto& input = Get();
    input.mouseX = xpos;
    input.mouseY = ypos;
}

void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto& input = Get();
    input.mouseScrollY = yoffset;
}

