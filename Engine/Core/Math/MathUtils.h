#pragma once

#include "Vector.h"
#include <algorithm>
#include <cmath>

// ============================================================================
// Math Utilities
// ============================================================================

namespace MathUtils {
    // Constants
    constexpr float PI = 3.14159265359f;
    constexpr float PI_2 = 1.57079632679f;
    constexpr float PI_4 = 0.78539816339f;
    constexpr float TAU = 6.28318530718f;
    constexpr float E = 2.71828182846f;
    constexpr float EPSILON = 0.00001f;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    // Angle conversions
    inline float DegreesToRadians(float degrees) { return degrees * DEG_TO_RAD; }
    inline float RadiansToDegrees(float radians) { return radians * RAD_TO_DEG; }

    // Clamping
    template<typename T>
    inline T Clamp(T value, T min, T max) {
        return std::clamp(value, min, max);
    }

    inline float Clamp01(float value) {
        return Clamp(value, 0.0f, 1.0f);
    }

    // Lerp
    template<typename T>
    inline T Lerp(const T& a, const T& b, float t) {
        return a + (b - a) * Clamp01(t);
    }

    // Smooth step
    inline float SmoothStep(float edge0, float edge1, float x) {
        float t = Clamp01((x - edge0) / (edge1 - edge0));
        return t * t * (3.0f - 2.0f * t);
    }

    // Min/Max
    template<typename T>
    inline T Min(T a, T b) {
        return std::min(a, b);
    }

    template<typename T>
    inline T Max(T a, T b) {
        return std::max(a, b);
    }

    template<typename T>
    inline T Min3(T a, T b, T c) {
        return std::min(a, std::min(b, c));
    }

    template<typename T>
    inline T Max3(T a, T b, T c) {
        return std::max(a, std::max(b, c));
    }

    // Comparisons
    inline bool IsNearlyEqual(float a, float b, float tolerance = EPSILON) {
        return std::abs(a - b) < tolerance;
    }

    inline bool IsNearlyZero(float value, float tolerance = EPSILON) {
        return std::abs(value) < tolerance;
    }

    // Rounding
    inline float Floor(float value) { return std::floor(value); }
    inline float Ceil(float value) { return std::ceil(value); }
    inline float Round(float value) { return std::round(value); }
    inline float Trunc(float value) { return std::trunc(value); }

    // Abs
    inline float Abs(float value) { return std::abs(value); }
    inline int Abs(int value) { return std::abs(value); }

    // Square
    inline float Square(float value) { return value * value; }

    // Power
    inline float Pow(float base, float exponent) { return std::pow(base, exponent); }
    inline float Sqrt(float value) { return std::sqrt(value); }

    // Trigonometric
    inline float Sin(float radians) { return std::sin(radians); }
    inline float Cos(float radians) { return std::cos(radians); }
    inline float Tan(float radians) { return std::tan(radians); }
    inline float Asin(float value) { return std::asin(value); }
    inline float Acos(float value) { return std::acos(value); }
    inline float Atan(float value) { return std::atan(value); }
    inline float Atan2(float y, float x) { return std::atan2(y, x); }

    // Sign
    inline float Sign(float value) {
        if (value > 0.0f) return 1.0f;
        if (value < 0.0f) return -1.0f;
        return 0.0f;
    }

    // Modulo
    inline float Mod(float value, float mod) {
        return std::fmod(value, mod);
    }

    // Wrap angle to [-PI, PI]
    inline float WrapAngle(float angleRadians) {
        angleRadians = Mod(angleRadians + PI, TAU) - PI;
        return angleRadians;
    }

    // Distance
    inline float Distance(const Vector2& a, const Vector2& b) {
        return (a - b).Size();
    }

    inline float Distance(const Vector3& a, const Vector3& b) {
        return (a - b).Size();
    }

    inline float DistanceSquared(const Vector2& a, const Vector2& b) {
        return (a - b).SizeSquared();
    }

    inline float DistanceSquared(const Vector3& a, const Vector3& b) {
        return (a - b).SizeSquared();
    }

    // Dot product
    inline float Dot(const Vector2& a, const Vector2& b) {
        return a.Dot(b);
    }

    inline float Dot(const Vector3& a, const Vector3& b) {
        return a.Dot(b);
    }

    // Cross product
    inline float Cross(const Vector2& a, const Vector2& b) {
        return a.Cross(b);
    }

    inline Vector3 Cross(const Vector3& a, const Vector3& b) {
        return a.Cross(b);
    }
}

