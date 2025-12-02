#pragma once

#include "Vector.h"
#include <array>
#include <cstring>
#include <cmath>

struct Vector3;
struct Vector4;
struct Quaternion;

// ============================================================================
// Matrix4x4 - 4x4 Matrix (Column-major, Vulkan style)
// ============================================================================

struct Matrix4x4 {
    // Matrix stored in column-major order (Vulkan style)
    // m[col][row] or access via m[row + col*4]
    float m[16];

    // Constructors
    Matrix4x4();
    explicit Matrix4x4(float value);
    Matrix4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    );

    // Static constructors
    static Matrix4x4 Identity();
    static Matrix4x4 Zero();
    
    // Transformation matrices
    static Matrix4x4 Translation(const Vector3& translation);
    static Matrix4x4 Rotation(const Quaternion& rotation);
    static Matrix4x4 Scale(const Vector3& scale);
    static Matrix4x4 TRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale);
    
    // View/Projection matrices
    static Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
    static Matrix4x4 Perspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
    static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    
    // Component access
    float& operator()(int row, int col) { return m[row + col * 4]; }
    const float& operator()(int row, int col) const { return m[row + col * 4]; }
    float& operator[](int index) { return m[index]; }
    const float& operator[](int index) const { return m[index]; }

    // Get row/column
    Vector4 GetRow(int row) const;
    Vector4 GetColumn(int col) const;
    void SetRow(int row, const Vector4& value);
    void SetColumn(int col, const Vector4& value);

    // Get translation/rotation/scale
    Vector3 GetTranslation() const;
    Vector3 GetScale() const;
    Quaternion GetRotation() const;

    // Arithmetic operators
    Matrix4x4 operator+(const Matrix4x4& other) const;
    Matrix4x4 operator-(const Matrix4x4& other) const;
    Matrix4x4 operator*(const Matrix4x4& other) const;
    Matrix4x4 operator*(float scalar) const;
    Vector4 operator*(const Vector4& vec) const;
    Vector3 operator*(const Vector3& vec) const;
    
    // Assignment operators
    Matrix4x4& operator+=(const Matrix4x4& other);
    Matrix4x4& operator-=(const Matrix4x4& other);
    Matrix4x4& operator*=(const Matrix4x4& other);
    Matrix4x4& operator*=(float scalar);

    // Comparison operators
    bool operator==(const Matrix4x4& other) const;
    bool operator!=(const Matrix4x4& other) const;

    // Functions
    Matrix4x4 Transposed() const;
    void Transpose();
    
    Matrix4x4 Inversed() const;
    bool Inverse(Matrix4x4& out) const;
    
    float Determinant() const;
    
    Vector3 TransformPoint(const Vector3& point) const;
    Vector3 TransformVector(const Vector3& vec) const;
    Vector3 TransformDirection(const Vector3& dir) const;

    // Utility
    bool IsNearlyZero(float tolerance = 0.0001f) const;
    bool IsIdentity(float tolerance = 0.0001f) const;

    // Conversion to float array (for Vulkan)
    const float* Data() const { return m; }
    float* Data() { return m; }
};

// Friend operators
Matrix4x4 operator*(float scalar, const Matrix4x4& mat);
Vector4 operator*(const Vector4& vec, const Matrix4x4& mat);

