#pragma once

#include "Vector.h"
#include "Matrix.h"
#include <cmath>

struct Vector3;
struct Matrix4x4;

// ============================================================================
// Quaternion - Rotation representation (w, x, y, z)
// ============================================================================

struct Quaternion {
    float x, y, z, w;

    // Constructors
    Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Quaternion(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
    Quaternion(const Vector3& axis, float angleRadians);

    // Static constructors
    static Quaternion Identity();
    static Quaternion FromEuler(const Vector3& eulerDegrees);
    static Quaternion FromAxisAngle(const Vector3& axis, float angleRadians);
    static Quaternion FromMatrix(const Matrix4x4& matrix);
    static Quaternion LookRotation(const Vector3& forward, const Vector3& up = Vector3::Up);
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);

    // Constants
    static const Quaternion IdentityQuaternion;

    // Component access
    Vector3 XYZ() const { return Vector3(x, y, z); }
    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    // Arithmetic operators
    Quaternion operator+(const Quaternion& other) const;
    Quaternion operator-(const Quaternion& other) const;
    Quaternion operator*(const Quaternion& other) const;
    Quaternion operator*(float scalar) const;
    Quaternion operator/(float scalar) const;
    Quaternion operator-() const;

    // Assignment operators
    Quaternion& operator+=(const Quaternion& other);
    Quaternion& operator-=(const Quaternion& other);
    Quaternion& operator*=(const Quaternion& other);
    Quaternion& operator*=(float scalar);
    Quaternion& operator/=(float scalar);

    // Comparison operators
    bool operator==(const Quaternion& other) const;
    bool operator!=(const Quaternion& other) const;

    // Functions
    float Size() const;
    float SizeSquared() const;
    Quaternion Normalized() const;
    void Normalize();
    Quaternion Conjugate() const;
    Quaternion Inversed() const;
    void Inverse();

    // Rotation operations
    Vector3 RotateVector(const Vector3& vec) const;
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpVector() const;
    Vector3 ToEuler() const; // Returns degrees
    Matrix4x4 ToMatrix() const;
    
    float Dot(const Quaternion& other) const;
    
    // Utility
    bool IsNearlyZero(float tolerance = 0.0001f) const;
    bool IsNormalized(float tolerance = 0.01f) const;
    bool IsIdentity(float tolerance = 0.0001f) const;
    
    float GetAngle() const; // Get rotation angle in radians
    Vector3 GetAxis() const; // Get rotation axis
};

// Friend operators
Quaternion operator*(float scalar, const Quaternion& q);

