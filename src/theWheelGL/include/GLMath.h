/// GLMath.h
/// Cross-platform math types for OpenGL ES renderer.
/// Replaces D3DXImpl.h with API-neutral Vec3f/Mat4f.
///
/// Copyright (C) 1996-2007 Derek G Lane

#pragma once

#include <cmath>
#include <cstring>

namespace theWheelGL {

struct Vec3f {
    float x, y, z;

    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vec3f& operator*=(float f) { x *= f; y *= f; z *= f; return *this; }
    Vec3f& operator+=(const Vec3f& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3f operator+(const Vec3f& v) const { return Vec3f(x + v.x, y + v.y, z + v.z); }
    Vec3f operator-(const Vec3f& v) const { return Vec3f(x - v.x, y - v.y, z - v.z); }
    Vec3f operator*(float f) const { return Vec3f(x * f, y * f, z * f); }

    float dot(const Vec3f& v) const { return x * v.x + y * v.y + z * v.z; }

    Vec3f cross(const Vec3f& v) const {
        return Vec3f(y * v.z - z * v.y,
                     z * v.x - x * v.z,
                     x * v.y - y * v.x);
    }

    float length() const { return std::sqrt(x * x + y * y + z * z); }

    Vec3f normalized() const {
        float len = length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            return Vec3f(x * inv, y * inv, z * inv);
        }
        return Vec3f();
    }
};

/// 4x4 column-major matrix (OpenGL convention).
/// Stored as float[16] in column-major order:
///   m[0..3]  = column 0
///   m[4..7]  = column 1
///   m[8..11] = column 2
///   m[12..15]= column 3
struct Mat4f {
    float m[16];

    Mat4f() { identity(); }

    void identity() {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    /// Access element at (row, col) — column-major
    float& at(int row, int col) { return m[col * 4 + row]; }
    float at(int row, int col) const { return m[col * 4 + row]; }

    const float* data() const { return m; }

    Mat4f operator*(const Mat4f& rhs) const {
        Mat4f result;
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                float sum = 0.0f;
                for (int k = 0; k < 4; k++) {
                    sum += at(row, k) * rhs.at(k, col);
                }
                result.at(row, col) = sum;
            }
        }
        return result;
    }

    /// Transform a position (w=1, with perspective divide)
    Vec3f transformCoord(const Vec3f& v) const {
        float rx = at(0,0)*v.x + at(0,1)*v.y + at(0,2)*v.z + at(0,3);
        float ry = at(1,0)*v.x + at(1,1)*v.y + at(1,2)*v.z + at(1,3);
        float rz = at(2,0)*v.x + at(2,1)*v.y + at(2,2)*v.z + at(2,3);
        float rw = at(3,0)*v.x + at(3,1)*v.y + at(3,2)*v.z + at(3,3);
        if (rw != 1.0f && rw != 0.0f) {
            float inv = 1.0f / rw;
            return Vec3f(rx * inv, ry * inv, rz * inv);
        }
        return Vec3f(rx, ry, rz);
    }

    /// Transform a normal (w=0, no translation)
    Vec3f transformNormal(const Vec3f& v) const {
        return Vec3f(
            at(0,0)*v.x + at(0,1)*v.y + at(0,2)*v.z,
            at(1,0)*v.x + at(1,1)*v.y + at(1,2)*v.z,
            at(2,0)*v.x + at(2,1)*v.y + at(2,2)*v.z
        );
    }

    static Mat4f Translation(float x, float y, float z) {
        Mat4f r;
        r.at(0,3) = x;
        r.at(1,3) = y;
        r.at(2,3) = z;
        return r;
    }

    static Mat4f Scaling(float sx, float sy, float sz) {
        Mat4f r;
        r.m[0] = sx; r.m[5] = sy; r.m[10] = sz;
        return r;
    }

    static Mat4f RotationZ(float angle) {
        float s = std::sin(angle), c = std::cos(angle);
        Mat4f r;
        r.at(0,0) = c;  r.at(0,1) = -s;
        r.at(1,0) = s;  r.at(1,1) = c;
        return r;
    }

    static Mat4f OrthoLH(float width, float height, float zNear, float zFar) {
        Mat4f r;
        std::memset(r.m, 0, sizeof(r.m));
        r.at(0,0) = 2.0f / width;
        r.at(1,1) = 2.0f / height;
        r.at(2,2) = 1.0f / (zFar - zNear);
        r.at(2,3) = zNear / (zNear - zFar);
        r.at(3,3) = 1.0f;
        return r;
    }

    static Mat4f LookAtLH(const Vec3f& eye, const Vec3f& target, const Vec3f& up) {
        Vec3f zaxis = (target - eye).normalized();
        Vec3f xaxis = up.cross(zaxis).normalized();
        Vec3f yaxis = zaxis.cross(xaxis);

        Mat4f r;
        r.at(0,0) = xaxis.x; r.at(0,1) = xaxis.y; r.at(0,2) = xaxis.z;
        r.at(1,0) = yaxis.x; r.at(1,1) = yaxis.y; r.at(1,2) = yaxis.z;
        r.at(2,0) = zaxis.x; r.at(2,1) = zaxis.y; r.at(2,2) = zaxis.z;
        r.at(0,3) = -xaxis.dot(eye);
        r.at(1,3) = -yaxis.dot(eye);
        r.at(2,3) = -zaxis.dot(eye);
        return r;
    }
};

} // namespace theWheelGL
