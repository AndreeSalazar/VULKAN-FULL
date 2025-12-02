#pragma once

#include <cmath>
#include <iostream>
#include <algorithm>

// Forward declarations
struct Vector2;
struct Vector3;
struct Vector4;

// ============================================================================
// Vector2 - 2D Vector
// ============================================================================

struct Vector2 {
    float x, y;

    // Constructors
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float inX, float inY) : x(inX), y(inY) {}
    explicit Vector2(float value) : x(value), y(value) {}

    // Static constants
    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 UnitX;
    static const Vector2 UnitY;

    // Component access
    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    // Arithmetic operators
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }
    Vector2 operator-() const { return Vector2(-x, -y); }

    // Assignment operators
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

    // Comparison operators
    bool operator==(const Vector2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2& other) const { return !(*this == other); }

    // Functions
    float Size() const { return std::sqrt(x * x + y * y); }
    float SizeSquared() const { return x * x + y * y; }
    float Dot(const Vector2& other) const { return x * other.x + y * other.y; }
    float Cross(const Vector2& other) const { return x * other.y - y * other.x; }
    
    Vector2 Normalized() const {
        float len = Size();
        if (len > 0.00001f) {
            return Vector2(x / len, y / len);
        }
        return Vector2::Zero;
    }
    
    void Normalize() {
        float len = Size();
        if (len > 0.00001f) {
            x /= len;
            y /= len;
        } else {
            x = y = 0.0f;
        }
    }

    bool IsNearlyZero(float tolerance = 0.0001f) const {
        return std::abs(x) < tolerance && std::abs(y) < tolerance;
    }

    bool IsNormalized(float tolerance = 0.01f) const {
        return std::abs(SizeSquared() - 1.0f) < tolerance;
    }

    // Utility
    Vector2 GetAbs() const { return Vector2(std::abs(x), std::abs(y)); }
    Vector2 GetClamped(const Vector2& min, const Vector2& max) const {
        return Vector2(
            std::clamp(x, min.x, max.x),
            std::clamp(y, min.y, max.y)
        );
    }

    // Friend functions
    friend Vector2 operator*(float scalar, const Vector2& vec) { return vec * scalar; }
    friend std::ostream& operator<<(std::ostream& os, const Vector2& vec) {
        return os << "Vector2(" << vec.x << ", " << vec.y << ")";
    }
};

// ============================================================================
// Vector3 - 3D Vector
// ============================================================================

struct Vector3 {
    float x, y, z;

    // Constructors
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ) {}
    explicit Vector3(float value) : x(value), y(value), z(value) {}
    Vector3(const Vector2& vec2, float inZ = 0.0f) : x(vec2.x), y(vec2.y), z(inZ) {}

    // Static constants
    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 UnitX;
    static const Vector3 UnitY;
    static const Vector3 UnitZ;
    static const Vector3 Up;
    static const Vector3 Down;
    static const Vector3 Forward;
    static const Vector3 Backward;
    static const Vector3 Right;
    static const Vector3 Left;

    // Component access
    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    Vector2 XY() const { return Vector2(x, y); }
    Vector2 XZ() const { return Vector2(x, z); }
    Vector2 YZ() const { return Vector2(y, z); }

    // Arithmetic operators
    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 operator/(float scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    // Assignment operators
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vector3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector3& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    // Comparison operators
    bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vector3& other) const { return !(*this == other); }

    // Functions
    float Size() const { return std::sqrt(x * x + y * y + z * z); }
    float SizeSquared() const { return x * x + y * y + z * z; }
    float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vector3 Cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    Vector3 Normalized() const {
        float len = Size();
        if (len > 0.00001f) {
            return Vector3(x / len, y / len, z / len);
        }
        return Vector3::Zero;
    }
    
    void Normalize() {
        float len = Size();
        if (len > 0.00001f) {
            x /= len;
            y /= len;
            z /= len;
        } else {
            x = y = z = 0.0f;
        }
    }

    bool IsNearlyZero(float tolerance = 0.0001f) const {
        return std::abs(x) < tolerance && std::abs(y) < tolerance && std::abs(z) < tolerance;
    }

    bool IsNormalized(float tolerance = 0.01f) const {
        return std::abs(SizeSquared() - 1.0f) < tolerance;
    }

    // Utility
    Vector3 GetAbs() const { return Vector3(std::abs(x), std::abs(y), std::abs(z)); }
    Vector3 GetClamped(const Vector3& min, const Vector3& max) const {
        return Vector3(
            std::clamp(x, min.x, max.x),
            std::clamp(y, min.y, max.y),
            std::clamp(z, min.z, max.z)
        );
    }

    float Distance(const Vector3& other) const {
        return (*this - other).Size();
    }

    float DistanceSquared(const Vector3& other) const {
        return (*this - other).SizeSquared();
    }

    // Friend functions
    friend Vector3 operator*(float scalar, const Vector3& vec) { return vec * scalar; }
    friend std::ostream& operator<<(std::ostream& os, const Vector3& vec) {
        return os << "Vector3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    }
};

// ============================================================================
// Vector4 - 4D Vector / Homogeneous coordinates
// ============================================================================

struct Vector4 {
    float x, y, z, w;

    // Constructors
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float inX, float inY, float inZ, float inW) : x(inX), y(inY), z(inZ), w(inW) {}
    explicit Vector4(float value) : x(value), y(value), z(value), w(value) {}
    Vector4(const Vector3& vec3, float inW = 0.0f) : x(vec3.x), y(vec3.y), z(vec3.z), w(inW) {}

    // Static constants
    static const Vector4 Zero;
    static const Vector4 One;

    // Component access
    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    Vector3 XYZ() const { return Vector3(x, y, z); }
    Vector2 XY() const { return Vector2(x, y); }

    // Arithmetic operators
    Vector4 operator+(const Vector4& other) const { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vector4 operator-(const Vector4& other) const { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vector4 operator*(float scalar) const { return Vector4(x * scalar, y * scalar, z * scalar, w * scalar); }
    Vector4 operator/(float scalar) const { return Vector4(x / scalar, y / scalar, z / scalar, w / scalar); }
    Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

    // Assignment operators
    Vector4& operator+=(const Vector4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    Vector4& operator-=(const Vector4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    Vector4& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    Vector4& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

    // Comparison operators
    bool operator==(const Vector4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const Vector4& other) const { return !(*this == other); }

    // Functions
    float Size() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    float SizeSquared() const { return x * x + y * y + z * z + w * w; }
    float Dot(const Vector4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
    
    Vector4 Normalized() const {
        float len = Size();
        if (len > 0.00001f) {
            return Vector4(x / len, y / len, z / len, w / len);
        }
        return Vector4::Zero;
    }
    
    void Normalize() {
        float len = Size();
        if (len > 0.00001f) {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        } else {
            x = y = z = w = 0.0f;
        }
    }

    bool IsNearlyZero(float tolerance = 0.0001f) const {
        return std::abs(x) < tolerance && std::abs(y) < tolerance && 
               std::abs(z) < tolerance && std::abs(w) < tolerance;
    }

    // Friend functions
    friend Vector4 operator*(float scalar, const Vector4& vec) { return vec * scalar; }
    friend std::ostream& operator<<(std::ostream& os, const Vector4& vec) {
        return os << "Vector4(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
    }
};

