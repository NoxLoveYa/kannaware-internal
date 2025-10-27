#pragma once
#include <Windows.h>
#include <cstdint>
#include <math.h>

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float f) const { return Vector3(x * f, y * f, z * f); }

    float Length() const {
        return sqrtf(x * x + y * y + z * z);
    }

    float Normalize() {
        float len = Length();
        if (len != 0) {
            x /= len;
            y /= len;
            z /= len;
        }
        return len;
	}

    float Dot(const Vector3& v) const {
        return x * v.x + y * v.y + z * v.z;
	}
};

struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
};

struct ViewMatrix_t {
    float matrix[4][4];
};