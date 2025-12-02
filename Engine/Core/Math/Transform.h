#pragma once

#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"

// ============================================================================
// Transform - Position, Rotation, Scale (PRS)
// ============================================================================

struct Transform {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;

    // Constructors
    Transform();
    Transform(const Vector3& inPosition, const Quaternion& inRotation, const Vector3& inScale);
    Transform(const Vector3& inPosition, const Quaternion& inRotation);
    explicit Transform(const Vector3& inPosition);

    // Static constructors
    static Transform Identity();
    static Transform FromMatrix(const Matrix4x4& matrix);

    // Constants
    static const Transform IdentityTransform;

    // Get/Set components
    Vector3 GetPosition() const { return position; }
    void SetPosition(const Vector3& newPosition) { position = newPosition; }
    
    Quaternion GetRotation() const { return rotation; }
    void SetRotation(const Quaternion& newRotation) { rotation = newRotation; }
    
    Vector3 GetScale() const { return scale; }
    void SetScale(const Vector3& newScale) { scale = newScale; }

    // Euler angles (convenience)
    Vector3 GetEulerRotation() const { return rotation.ToEuler(); }
    void SetEulerRotation(const Vector3& eulerDegrees);

    // Transform operations
    Matrix4x4 ToMatrix() const;
    
    // Transform vectors/points
    Vector3 TransformPoint(const Vector3& point) const;
    Vector3 TransformVector(const Vector3& vec) const;
    Vector3 TransformDirection(const Vector3& dir) const;
    
    // Inverse transform
    Vector3 InverseTransformPoint(const Vector3& point) const;
    Vector3 InverseTransformVector(const Vector3& vec) const;
    Vector3 InverseTransformDirection(const Vector3& dir) const;

    // Direction vectors (relative to transform)
    Vector3 GetForward() const { return rotation.GetForwardVector(); }
    Vector3 GetRight() const { return rotation.GetRightVector(); }
    Vector3 GetUp() const { return rotation.GetUpVector(); }

    // Combining transforms
    Transform operator*(const Transform& other) const;
    Transform& operator*=(const Transform& other);

    // Comparison
    bool operator==(const Transform& other) const;
    bool operator!=(const Transform& other) const;

    // Utility
    Transform Inversed() const;
    void Inverse();
    
    bool IsNearlyEqual(const Transform& other, float positionTolerance = 0.0001f, float rotationTolerance = 0.0001f) const;
    bool IsIdentity(float positionTolerance = 0.0001f, float rotationTolerance = 0.0001f) const;
    
    // Reset to identity
    void Reset();
};

