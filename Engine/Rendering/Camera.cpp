#include "Camera.h"
#include "Core/Math/MathUtils.h"
#include <algorithm>
#include <cmath>
#include <cstring>

Camera::Camera() {
    SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    SetPosition(Vector3(0.0f, 0.0f, -5.0f));
    SetRotation(Quaternion::Identity());
    UpdateVectors();
}

void Camera::SetPosition(const Vector3& position) {
    transform.SetPosition(position);
}

void Camera::SetRotation(const Quaternion& rotation) {
    transform.SetRotation(rotation);
    Vector3 euler = rotation.ToEuler();
    yaw = euler.y;
    pitch = euler.x;
    UpdateVectors();
}

void Camera::SetTarget(const Vector3& target) {
    orbitTarget = target;
    if (mode == ECameraMode::Orbit) {
        UpdateOrbitPosition();
    }
}

void Camera::SetMode(ECameraMode newMode) {
    mode = newMode;
    if (mode == ECameraMode::Orbit) {
        // Calculate initial orbit angles from current position
        Vector3 toCamera = transform.GetPosition() - orbitTarget;
        orbitDistance = toCamera.Size();
        if (orbitDistance < 0.001f) orbitDistance = 10.0f;
        
        Vector3 normalized = toCamera.Normalized();
        orbitAngleY = std::asin(normalized.y);
        orbitAngleX = std::atan2(normalized.x, normalized.z);
        
        UpdateOrbitPosition();
    }
}

void Camera::SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane) {
    this->fovDegrees = fovDegrees;
    this->aspectRatio = aspectRatio;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
}

void Camera::SetAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
}

Matrix4x4 Camera::GetViewMatrix() const {
    if (mode == ECameraMode::Orbit) {
        Vector3 position = transform.GetPosition();
        Vector3 target = orbitTarget;
        Vector3 up = transform.GetUp();
        return Matrix4x4::LookAt(position, target, up);
    } else {
        Vector3 position = transform.GetPosition();
        Vector3 forward = transform.GetForward();
        Vector3 up = transform.GetUp();
        return Matrix4x4::LookAt(position, position + forward, up);
    }
}

Matrix4x4 Camera::GetProjectionMatrix() const {
    return Matrix4x4::Perspective(fovDegrees, aspectRatio, nearPlane, farPlane);
}

Matrix4x4 Camera::GetViewProjectionMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

void Camera::ProcessKeyboard(int key, float deltaTime) {
    if (mode != ECameraMode::FPS && mode != ECameraMode::Free) {
        return;
    }
    
    float velocity = movementSpeed * deltaTime;
    Vector3 forward = transform.GetForward();
    Vector3 right = transform.GetRight();
    Vector3 up = Vector3::Up;
    
    // Note: Key codes would need to be defined (using GLFW keys)
    // For now, we'll use input state
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    if (mode == ECameraMode::Orbit) {
        Orbit(xOffset, yOffset);
        return;
    }
    
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;
    
    yaw += xOffset;
    pitch += yOffset;
    
    if (constrainPitch) {
        pitch = std::clamp(pitch, minPitch, maxPitch);
    }
    
    UpdateVectors();
}

void Camera::ProcessMouseScroll(float yOffset) {
    if (mode == ECameraMode::Orbit) {
        Zoom(-yOffset);
    } else {
        fovDegrees -= yOffset;
        fovDegrees = std::clamp(fovDegrees, minFOV, maxFOV);
    }
}

void Camera::Orbit(float deltaX, float deltaY) {
    orbitAngleX += deltaX * mouseSensitivity;
    orbitAngleY += deltaY * mouseSensitivity;
    
    // Clamp pitch for orbit
    orbitAngleY = std::clamp(orbitAngleY, -89.0f * MathUtils::DEG_TO_RAD, 89.0f * MathUtils::DEG_TO_RAD);
    
    UpdateOrbitPosition();
}

void Camera::Zoom(float delta) {
    orbitDistance -= delta * zoomSpeed;
    orbitDistance = std::max(0.1f, orbitDistance);
    UpdateOrbitPosition();
}

void Camera::Pan(float deltaX, float deltaY) {
    Vector3 right = transform.GetRight();
    Vector3 up = transform.GetUp();
    
    float panSpeed = orbitDistance * 0.001f;
    orbitTarget += right * (-deltaX * panSpeed);
    orbitTarget += up * (deltaY * panSpeed);
    
    UpdateOrbitPosition();
}

void Camera::SetOrbitDistance(float distance) {
    orbitDistance = std::max(0.1f, distance);
    UpdateOrbitPosition();
}

void Camera::Update(float deltaTime) {
    if (mode == ECameraMode::FPS || mode == ECameraMode::Free) {
        // Calculate movement velocity based on deltaTime
        float velocity = movementSpeed * static_cast<float>(deltaTime);
        
        // Get current camera orientation vectors
        Vector3 forward = transform.GetForward();
        Vector3 right = transform.GetRight();
        Vector3 up = Vector3::Up; // World up, not camera up
        
        // Calculate movement direction
        Vector3 movement = Vector3::Zero;
        
        // Forward/Backward movement
        if (inputState.forward) {
            movement += forward * velocity;
        }
        if (inputState.backward) {
            movement -= forward * velocity;
        }
        
        // Left/Right strafe movement
        if (inputState.right) {
            movement += right * velocity;
        }
        if (inputState.left) {
            movement -= right * velocity;
        }
        
        // Up/Down movement (world space)
        if (inputState.up) {
            movement += up * velocity;
        }
        if (inputState.down) {
            movement -= up * velocity;
        }
        
        // Apply movement if any input
        if (!movement.IsNearlyZero()) {
            Vector3 currentPos = transform.GetPosition();
            Vector3 newPos = currentPos + movement;
            SetPosition(newPos);
        }
        
        // Mouse look is handled externally in main loop when locked
    } else if (mode == ECameraMode::Orbit) {
        // Orbit mouse drag
        if (inputState.mouseRight) {
            if (std::abs(inputState.deltaMouseX) > 0.001f || std::abs(inputState.deltaMouseY) > 0.001f) {
                Orbit(inputState.deltaMouseX, inputState.deltaMouseY);
                inputState.deltaMouseX = 0.0f;
                inputState.deltaMouseY = 0.0f;
            }
        }
        
        // Pan with middle mouse
        if (inputState.mouseMiddle) {
            if (std::abs(inputState.deltaMouseX) > 0.001f || std::abs(inputState.deltaMouseY) > 0.001f) {
                Pan(inputState.deltaMouseX, inputState.deltaMouseY);
                inputState.deltaMouseX = 0.0f;
                inputState.deltaMouseY = 0.0f;
            }
        }
        
        // Scroll zoom
        if (std::abs(inputState.mouseScroll) > 0.001f) {
            Zoom(inputState.mouseScroll);
            inputState.mouseScroll = 0.0f;
        }
    }
}

void Camera::UpdateVectors() {
    // Convert yaw and pitch to quaternion
    Vector3 euler(pitch, yaw, 0.0f);
    transform.SetRotation(Quaternion::FromEuler(euler));
}

void Camera::UpdateOrbitPosition() {
    // Convert angles to position
    float cosY = std::cos(orbitAngleY);
    float sinY = std::sin(orbitAngleY);
    float cosX = std::cos(orbitAngleX);
    float sinX = std::sin(orbitAngleX);
    
    Vector3 offset(
        orbitDistance * cosY * sinX,
        orbitDistance * sinY,
        orbitDistance * cosY * cosX
    );
    
    transform.SetPosition(orbitTarget + offset);
    
    // Look at target
    Vector3 forward = (orbitTarget - transform.GetPosition()).Normalized();
    Vector3 right = Vector3::Up.Cross(forward).Normalized();
    Vector3 up = forward.Cross(right);
    
    transform.SetRotation(Quaternion::LookRotation(forward, up));
}

