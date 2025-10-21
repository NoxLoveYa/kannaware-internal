#pragma once
#include "Entity.h"
#include <cmath>

class MathUtils {
public:
    // World to screen conversion
    static bool WorldToScreen(const Vector3& world, Vector2& screen, const ViewMatrix_t& matrix, int screenWidth, int screenHeight) {
        float w = matrix.matrix[3][0] * world.x +
            matrix.matrix[3][1] * world.y +
            matrix.matrix[3][2] * world.z +
            matrix.matrix[3][3];

        // Check if point is behind camera
        if (w < 0.001f)
            return false;

        float x = matrix.matrix[0][0] * world.x +
            matrix.matrix[0][1] * world.y +
            matrix.matrix[0][2] * world.z +
            matrix.matrix[0][3];

        float y = matrix.matrix[1][0] * world.x +
            matrix.matrix[1][1] * world.y +
            matrix.matrix[1][2] * world.z +
            matrix.matrix[1][3];

        float invW = 1.0f / w;
        x *= invW;
        y *= invW;

        float halfWidth = screenWidth * 0.5f;
        float halfHeight = screenHeight * 0.5f;

        screen.x = halfWidth + (halfWidth * x);
        screen.y = halfHeight - (halfHeight * y);

        return true;
    }

    // Calculate distance between two points
    static float Distance3D(const Vector3& a, const Vector3& b) {
        Vector3 delta = a - b;
        return delta.Length();
    }

    // Calculate 2D distance
    static float Distance2D(const Vector2& a, const Vector2& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return sqrtf(dx * dx + dy * dy);
    }

    // Get screen center
    static Vector2 GetScreenCenter(int width, int height) {
        return Vector2(width * 0.5f, height * 0.5f);
    }

    // Linear interpolation
    static float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    // Clamp value between min and max
    static float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    // Calculate box dimensions from head and feet positions
    static void CalculateBox(const Vector2& head, const Vector2& feet, float& width, float& height) {
        height = feet.y - head.y;
        width = height * 0.65f; // Approximate width as 65% of height
    }
};