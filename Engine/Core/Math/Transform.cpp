#include "Transform.h"

const Transform Transform::IdentityTransform(Vector3::Zero, Quaternion::Identity(), Vector3::One);

Transform::Transform()
    : position(Vector3::Zero)
    , rotation(Quaternion::Identity())
    , scale(Vector3::One)
{
}

Transform::Transform(const Vector3& inPosition, const Quaternion& inRotation, const Vector3& inScale)
    : position(inPosition)
    , rotation(inRotation)
    , scale(inScale)
{
}

Transform::Transform(const Vector3& inPosition, const Quaternion& inRotation)
    : position(inPosition)
    , rotation(inRotation)
    , scale(Vector3::One)
{
}

Transform::Transform(const Vector3& inPosition)
    : position(inPosition)
    , rotation(Quaternion::Identity())
    , scale(Vector3::One)
{
}

Transform Transform::Identity() {
    return Transform(Vector3::Zero, Quaternion::Identity(), Vector3::One);
}

Transform Transform::FromMatrix(const Matrix4x4& matrix) {
    Vector3 pos = matrix.GetTranslation();
    Vector3 scl = matrix.GetScale();
    Quaternion rot = matrix.GetRotation();
    return Transform(pos, rot, scl);
}

void Transform::SetEulerRotation(const Vector3& eulerDegrees) {
    rotation = Quaternion::FromEuler(eulerDegrees);
}

Matrix4x4 Transform::ToMatrix() const {
    return Matrix4x4::TRS(position, rotation, scale);
}

Vector3 Transform::TransformPoint(const Vector3& point) const {
    // Apply scale, then rotation, then translation
    Vector3 scaled = Vector3(point.x * scale.x, point.y * scale.y, point.z * scale.z);
    Vector3 rotated = rotation.RotateVector(scaled);
    return rotated + position;
}

Vector3 Transform::TransformVector(const Vector3& vec) const {
    // Apply scale and rotation, but not translation
    Vector3 scaled = Vector3(vec.x * scale.x, vec.y * scale.y, vec.z * scale.z);
    return rotation.RotateVector(scaled);
}

Vector3 Transform::TransformDirection(const Vector3& dir) const {
    // Only rotation, normalized
    return rotation.RotateVector(dir).Normalized();
}

Vector3 Transform::InverseTransformPoint(const Vector3& point) const {
    // Inverse: translate back, rotate back, scale back
    Vector3 translated = point - position;
    Vector3 rotated = rotation.Conjugate().RotateVector(translated);
    return Vector3(rotated.x / scale.x, rotated.y / scale.y, rotated.z / scale.z);
}

Vector3 Transform::InverseTransformVector(const Vector3& vec) const {
    Vector3 rotated = rotation.Conjugate().RotateVector(vec);
    return Vector3(rotated.x / scale.x, rotated.y / scale.y, rotated.z / scale.z);
}

Vector3 Transform::InverseTransformDirection(const Vector3& dir) const {
    return rotation.Conjugate().RotateVector(dir).Normalized();
}

Transform Transform::operator*(const Transform& other) const {
    // Combine transforms: this * other means apply other first, then this
    Vector3 combinedPos = TransformPoint(other.position);
    Quaternion combinedRot = rotation * other.rotation;
    Vector3 combinedScale = Vector3(scale.x * other.scale.x, scale.y * other.scale.y, scale.z * other.scale.z);
    
    return Transform(combinedPos, combinedRot, combinedScale);
}

Transform& Transform::operator*=(const Transform& other) {
    *this = *this * other;
    return *this;
}

bool Transform::operator==(const Transform& other) const {
    return position == other.position && 
           rotation == other.rotation && 
           scale == other.scale;
}

bool Transform::operator!=(const Transform& other) const {
    return !(*this == other);
}

Transform Transform::Inversed() const {
    Transform result;
    result.position = InverseTransformPoint(Vector3::Zero);
    result.rotation = rotation.Conjugate();
    result.scale = Vector3(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);
    return result;
}

void Transform::Inverse() {
    *this = Inversed();
}

bool Transform::IsNearlyEqual(const Transform& other, float positionTolerance, float rotationTolerance) const {
    return position.Distance(other.position) < positionTolerance &&
           rotation.Dot(other.rotation) > (1.0f - rotationTolerance) &&
           (scale - other.scale).IsNearlyZero(positionTolerance);
}

bool Transform::IsIdentity(float positionTolerance, float rotationTolerance) const {
    return position.IsNearlyZero(positionTolerance) &&
           rotation.IsIdentity(rotationTolerance) &&
           (scale - Vector3::One).IsNearlyZero(positionTolerance);
}

void Transform::Reset() {
    position = Vector3::Zero;
    rotation = Quaternion::Identity();
    scale = Vector3::One;
}

