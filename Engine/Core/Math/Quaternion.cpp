#include "Quaternion.h"
#include <algorithm>

const Quaternion Quaternion::IdentityQuaternion(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion(const Vector3& axis, float angleRadians) {
    float halfAngle = angleRadians * 0.5f;
    float s = std::sin(halfAngle);
    Vector3 normalizedAxis = axis.Normalized();
    
    x = normalizedAxis.x * s;
    y = normalizedAxis.y * s;
    z = normalizedAxis.z * s;
    w = std::cos(halfAngle);
}

Quaternion Quaternion::Identity() {
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::FromEuler(const Vector3& eulerDegrees) {
    float pitch = eulerDegrees.x * 3.14159265359f / 180.0f;
    float yaw = eulerDegrees.y * 3.14159265359f / 180.0f;
    float roll = eulerDegrees.z * 3.14159265359f / 180.0f;

    float cy = std::cos(yaw * 0.5f);
    float sy = std::sin(yaw * 0.5f);
    float cp = std::cos(pitch * 0.5f);
    float sp = std::sin(pitch * 0.5f);
    float cr = std::cos(roll * 0.5f);
    float sr = std::sin(roll * 0.5f);

    return Quaternion(
        cy * cp * sr - sy * sp * cr,
        sy * cp * sr + cy * sp * cr,
        sy * cp * cr - cy * sp * sr,
        cy * cp * cr + sy * sp * sr
    );
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angleRadians) {
    return Quaternion(axis, angleRadians);
}

Quaternion Quaternion::FromMatrix(const Matrix4x4& matrix) {
    float trace = matrix(0, 0) + matrix(1, 1) + matrix(2, 2);
    
    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            (matrix(2, 1) - matrix(1, 2)) * invS,
            (matrix(0, 2) - matrix(2, 0)) * invS,
            (matrix(1, 0) - matrix(0, 1)) * invS,
            s * 0.25f
        ).Normalized();
    } else if (matrix(0, 0) > matrix(1, 1) && matrix(0, 0) > matrix(2, 2)) {
        float s = std::sqrt(1.0f + matrix(0, 0) - matrix(1, 1) - matrix(2, 2)) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            s * 0.25f,
            (matrix(0, 1) + matrix(1, 0)) * invS,
            (matrix(0, 2) + matrix(2, 0)) * invS,
            (matrix(2, 1) - matrix(1, 2)) * invS
        ).Normalized();
    } else if (matrix(1, 1) > matrix(2, 2)) {
        float s = std::sqrt(1.0f + matrix(1, 1) - matrix(0, 0) - matrix(2, 2)) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            (matrix(0, 1) + matrix(1, 0)) * invS,
            s * 0.25f,
            (matrix(1, 2) + matrix(2, 1)) * invS,
            (matrix(0, 2) - matrix(2, 0)) * invS
        ).Normalized();
    } else {
        float s = std::sqrt(1.0f + matrix(2, 2) - matrix(0, 0) - matrix(1, 1)) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            (matrix(0, 2) + matrix(2, 0)) * invS,
            (matrix(1, 2) + matrix(2, 1)) * invS,
            s * 0.25f,
            (matrix(1, 0) - matrix(0, 1)) * invS
        ).Normalized();
    }
}

Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up) {
    Vector3 f = forward.Normalized();
    Vector3 r = up.Cross(f).Normalized();
    Vector3 u = f.Cross(r);

    float trace = r.x + u.y + f.z;
    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            (u.z - f.y) * invS,
            (f.x - r.z) * invS,
            (r.y - u.x) * invS,
            s * 0.25f
        ).Normalized();
    } else if (r.x > u.y && r.x > f.z) {
        float s = std::sqrt(1.0f + r.x - u.y - f.z) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            s * 0.25f,
            (r.y + u.x) * invS,
            (r.z + f.x) * invS,
            (u.z - f.y) * invS
        ).Normalized();
    } else if (u.y > f.z) {
        float s = std::sqrt(1.0f + u.y - r.x - f.z) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            (r.y + u.x) * invS,
            s * 0.25f,
            (u.z + f.y) * invS,
            (f.x - r.z) * invS
        ).Normalized();
    } else {
        float s = std::sqrt(1.0f + f.z - r.x - u.y) * 2.0f;
        float invS = 1.0f / s;
        return Quaternion(
            (r.z + f.x) * invS,
            (u.z + f.y) * invS,
            s * 0.25f,
            (r.y - u.x) * invS
        ).Normalized();
    }
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t) {
    float dot = a.Dot(b);
    Quaternion b2 = b;
    
    if (dot < 0.0f) {
        b2 = -b2;
        dot = -dot;
    }
    
    if (std::abs(dot) > 0.9995f) {
        return (a + (b2 - a) * t).Normalized();
    }
    
    float theta = std::acos(std::clamp(dot, -1.0f, 1.0f));
    float sinTheta = std::sin(theta);
    float w1 = std::sin((1.0f - t) * theta) / sinTheta;
    float w2 = std::sin(t * theta) / sinTheta;
    
    return (a * w1 + b2 * w2).Normalized();
}

Quaternion Quaternion::operator+(const Quaternion& other) const {
    return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}

Quaternion Quaternion::operator-(const Quaternion& other) const {
    return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w,
        w * other.w - x * other.x - y * other.y - z * other.z
    );
}

Quaternion Quaternion::operator*(float scalar) const {
    return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}

Quaternion Quaternion::operator/(float scalar) const {
    return Quaternion(x / scalar, y / scalar, z / scalar, w / scalar);
}

Quaternion Quaternion::operator-() const {
    return Quaternion(-x, -y, -z, -w);
}

Quaternion& Quaternion::operator+=(const Quaternion& other) {
    x += other.x; y += other.y; z += other.z; w += other.w;
    return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& other) {
    x -= other.x; y -= other.y; z -= other.z; w -= other.w;
    return *this;
}

Quaternion& Quaternion::operator*=(const Quaternion& other) {
    *this = *this * other;
    return *this;
}

Quaternion& Quaternion::operator*=(float scalar) {
    x *= scalar; y *= scalar; z *= scalar; w *= scalar;
    return *this;
}

Quaternion& Quaternion::operator/=(float scalar) {
    x /= scalar; y /= scalar; z /= scalar; w /= scalar;
    return *this;
}

bool Quaternion::operator==(const Quaternion& other) const {
    return std::abs(x - other.x) < 0.0001f &&
           std::abs(y - other.y) < 0.0001f &&
           std::abs(z - other.z) < 0.0001f &&
           std::abs(w - other.w) < 0.0001f;
}

bool Quaternion::operator!=(const Quaternion& other) const {
    return !(*this == other);
}

float Quaternion::Size() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Quaternion::SizeSquared() const {
    return x * x + y * y + z * z + w * w;
}

Quaternion Quaternion::Normalized() const {
    float len = Size();
    if (len > 0.00001f) {
        return Quaternion(x / len, y / len, z / len, w / len);
    }
    return Identity();
}

void Quaternion::Normalize() {
    float len = Size();
    if (len > 0.00001f) {
        x /= len; y /= len; z /= len; w /= len;
    } else {
        *this = Identity();
    }
}

Quaternion Quaternion::Conjugate() const {
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inversed() const {
    float lenSq = SizeSquared();
    if (lenSq > 0.00001f) {
        return Conjugate() / lenSq;
    }
    return Identity();
}

void Quaternion::Inverse() {
    *this = Inversed();
}

Vector3 Quaternion::RotateVector(const Vector3& vec) const {
    Quaternion vecQuat(vec.x, vec.y, vec.z, 0.0f);
    Quaternion result = (*this) * vecQuat * Conjugate();
    return Vector3(result.x, result.y, result.z);
}

Vector3 Quaternion::GetForwardVector() const {
    return RotateVector(Vector3::Forward);
}

Vector3 Quaternion::GetRightVector() const {
    return RotateVector(Vector3::Right);
}

Vector3 Quaternion::GetUpVector() const {
    return RotateVector(Vector3::Up);
}

Vector3 Quaternion::ToEuler() const {
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    float roll = std::atan2(sinr_cosp, cosr_cosp);

    float sinp = 2.0f * (w * y - z * x);
    float pitch;
    if (std::abs(sinp) >= 1.0f) {
        pitch = std::copysign(3.14159265359f / 2.0f, sinp);
    } else {
        pitch = std::asin(sinp);
    }

    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    float yaw = std::atan2(siny_cosp, cosy_cosp);

    return Vector3(
        pitch * 180.0f / 3.14159265359f,
        yaw * 180.0f / 3.14159265359f,
        roll * 180.0f / 3.14159265359f
    );
}

Matrix4x4 Quaternion::ToMatrix() const {
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    return Matrix4x4(
        1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
        2.0f * (xy + wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx), 0.0f,
        2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (xx + yy), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

float Quaternion::Dot(const Quaternion& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

bool Quaternion::IsNearlyZero(float tolerance) const {
    return std::abs(x) < tolerance && std::abs(y) < tolerance &&
           std::abs(z) < tolerance && std::abs(w) < tolerance;
}

bool Quaternion::IsNormalized(float tolerance) const {
    return std::abs(SizeSquared() - 1.0f) < tolerance;
}

bool Quaternion::IsIdentity(float tolerance) const {
    return std::abs(x) < tolerance && std::abs(y) < tolerance &&
           std::abs(z) < tolerance && std::abs(w - 1.0f) < tolerance;
}

float Quaternion::GetAngle() const {
    Quaternion normalized = Normalized();
    return 2.0f * std::acos(std::clamp(std::abs(normalized.w), 0.0f, 1.0f));
}

Vector3 Quaternion::GetAxis() const {
    Quaternion normalized = Normalized();
    float s = std::sqrt(1.0f - normalized.w * normalized.w);
    if (s < 0.0001f) {
        return Vector3::Forward;
    }
    return Vector3(normalized.x / s, normalized.y / s, normalized.z / s);
}

Quaternion operator*(float scalar, const Quaternion& q) {
    return q * scalar;
}

