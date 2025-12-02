#pragma once

#include "Core/Math/Vector.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"

// ============================================================================
// Camera - Sistema de cámara con diferentes modos
// ============================================================================

enum class ECameraMode {
    FPS,        // First-person shooter style
    Orbit,      // Orbital around target
    Free        // Free look (editor style)
};

class Camera {
public:
    Camera();
    
    // Setup
    void SetPosition(const Vector3& position);
    void SetRotation(const Quaternion& rotation);
    void SetTarget(const Vector3& target); // For orbit mode
    void SetMode(ECameraMode mode);
    
    // Perspective settings
    void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
    void SetAspectRatio(float aspectRatio);
    
    // Getters
    Vector3 GetPosition() const { return transform.GetPosition(); }
    Quaternion GetRotation() const { return transform.GetRotation(); }
    Vector3 GetForward() const { return transform.GetForward(); }
    Vector3 GetRight() const { return transform.GetRight(); }
    Vector3 GetUp() const { return transform.GetUp(); }
    
    // Matrices
    Matrix4x4 GetViewMatrix() const;
    Matrix4x4 GetProjectionMatrix() const;
    Matrix4x4 GetViewProjectionMatrix() const;
    
    // Camera controls (FPS mode)
    void ProcessKeyboard(int key, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yOffset);
    
    // Orbit controls
    void Orbit(float deltaX, float deltaY);
    void Zoom(float delta);
    void Pan(float deltaX, float deltaY);
    
    // Update (call each frame)
    void Update(float deltaTime);
    
    // Settings
    void SetMovementSpeed(float speed) { movementSpeed = speed; }
    void SetMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
    void SetZoomSpeed(float speed) { zoomSpeed = speed; }
    void SetOrbitDistance(float distance);
    
    float GetFOV() const { return fovDegrees; }
    float GetNearPlane() const { return nearPlane; }
    float GetFarPlane() const { return farPlane; }
    float GetAspectRatio() const { return aspectRatio; }
    
    // Input state (set from outside)
    struct InputState {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        
        float mouseX = 0.0f;
        float mouseY = 0.0f;
        float mouseScroll = 0.0f;
        bool mouseLeft = false;
        bool mouseRight = false;
        bool mouseMiddle = false;
        
        float deltaMouseX = 0.0f;
        float deltaMouseY = 0.0f;
    };
    
    void SetInputState(const InputState& state) { inputState = state; }
    
private:
    // Camera transform
    Transform transform;
    
    // Camera mode
    ECameraMode mode = ECameraMode::FPS;
    
    // Orbit mode specific
    Vector3 orbitTarget = Vector3::Zero;
    float orbitDistance = 10.0f;
    float orbitAngleX = 0.0f; // Yaw
    float orbitAngleY = 0.0f; // Pitch
    
    // Perspective
    float fovDegrees = 45.0f;
    float aspectRatio = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    
    // Controls
    float movementSpeed = 5.0f;
    float mouseSensitivity = 0.1f;
    float zoomSpeed = 2.0f;
    
    // FPS mode
    float yaw = -90.0f;   // Starts looking along -Z
    float pitch = 0.0f;
    
    // Constraints
    float minPitch = -89.0f;
    float maxPitch = 89.0f;
    float minFOV = 10.0f;
    float maxFOV = 120.0f;
    
    // Input
    InputState inputState;
    
    // Internal
    void UpdateVectors();
    void UpdateOrbitPosition();
};

