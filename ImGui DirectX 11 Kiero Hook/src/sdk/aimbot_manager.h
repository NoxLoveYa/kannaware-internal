#pragma once
#include "entity_manager.h"
#include <algorithm>
#include <cmath>

#define M_PI 3.14159265358979323846f
#define RAD2DEG 57.295779513f
#define DEG2RAD 0.01745329251f

class AimbotManager {
private:
    PlayerInfo* targetPawn = nullptr;

    // Normalize angle to [-180, 180] range
    float NormalizeAngle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    // Calculate angles from source to destination (like C# version)
    Vector3 CalculateAngles(const Vector3& source, const Vector3& destination) {
        Vector3 delta = destination - source;
        float length = delta.Length();

        if (length < 0.001f)
            return Vector3(0, 0, 0);

        float hyp = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        float pitch = std::atan2(-delta.z, hyp) * RAD2DEG;
        float yaw = std::atan2(delta.y, delta.x) * RAD2DEG;

        return Vector3(pitch, yaw, 0);
    }

    // Calculate angular distance between two angles
    float CalculateAngularDistance(const Vector3& current, const Vector3& target) {
        float pitchDiff = NormalizeAngle(target.x - current.x);
        float yawDiff = NormalizeAngle(target.y - current.y);

        return std::sqrt(pitchDiff * pitchDiff + yawDiff * yawDiff);
    }

    // Smooth angles (basic linear interpolation)
    Vector3 SmoothAngles(const Vector3& current, const Vector3& target, float smoothFactor) {
        if (smoothFactor <= 0.01f) return target;
        if (smoothFactor >= 1.0f) return current;

        float pitchDelta = NormalizeAngle(target.x - current.x);
        float yawDelta = NormalizeAngle(target.y - current.y);

        float smoothedPitch = current.x + (pitchDelta / smoothFactor);
        float smoothedYaw = current.y + (yawDelta / smoothFactor);

        return Vector3(smoothedPitch, smoothedYaw, 0);
    }

public:
    AimbotManager() {}

    void AngleVectors(const Vector3& angles, Vector3& forward) {
        float pitch = angles.x * DEG2RAD;
        float yaw = angles.y * DEG2RAD;

        float sp = std::sin(pitch);
        float cp = std::cos(pitch);
        float sy = std::sin(yaw);
        float cy = std::cos(yaw);

        forward.x = cp * cy;
        forward.y = cp * sy;
        forward.z = -sp;
    }

    float GetFOVToHitbox(const PlayerInfo* player, const std::string& boneName) {
        LocalPlayerInfo& localPlayerInfo = g_EntityManager.GetLocalPlayerInfo();

        if (!localPlayerInfo.isValid || !localPlayerInfo.isAlive || !localPlayerInfo.ViewAngles)
            return FLT_MAX;

        Vector3 viewAngles = *localPlayerInfo.ViewAngles;
        Vector3 eyePosition = localPlayerInfo.eyePosition;

        // Check if bone exists
        auto it = player->bonePositions.find(boneName);
        if (it == player->bonePositions.end())
            return FLT_MAX;

        Vector3 targetBonePos = it->second;
        Vector3 direction = targetBonePos - eyePosition;
        direction.Normalize();

        // Convert view angles to direction vector
        Vector3 forward;
        AngleVectors(viewAngles, forward);
        forward.Normalize();

        // Calculate angle between vectors
        float dotProduct = forward.Dot(direction);
        dotProduct = std::clamp(dotProduct, -1.0f, 1.0f);
        float angle = std::acos(dotProduct) * RAD2DEG;

        return angle;
    }

    // Find closest player and return target info
    PlayerInfo* GetClosestPlayerPawn(float fovLimit = 180.0f) {
        LocalPlayerInfo& localPlayerInfo = g_EntityManager.GetLocalPlayerInfo();

        if (!localPlayerInfo.isValid || !localPlayerInfo.isAlive)
            return nullptr;

        PlayerInfo* closestPlayer = nullptr;
        float closestFOV = FLT_MAX;
        Vector3 bestTargetAngles(0, 0, 0);

        g_EntityManager.ForEachPlayer([&](const PlayerInfo& player) {
            if (!player.isValid || !player.isAlive)
                return;

            // Skip teammates
            if (player.teamNum == localPlayerInfo.teamNum)
                return;

            // Check if head bone exists
            auto it = player.bonePositions.find("head");
            if (it == player.bonePositions.end())
                return;

            Vector3 headPos = it->second;
            Vector3 eyePos = localPlayerInfo.eyePosition;

            // Distance check
            float distance = (headPos - eyePos).Length();
            if (distance < 1.0f || distance > 10000.0f)
                return;

            // Calculate angles to target
            Vector3 targetAngles = CalculateAngles(eyePos, headPos);
            Vector3 currentAngles = *localPlayerInfo.ViewAngles;

            // Calculate angular distance (FOV)
            float angularDist = CalculateAngularDistance(currentAngles, targetAngles);

            // FOV check
            if (angularDist > fovLimit)
                return;

            // Combine angular distance with physical distance for scoring
            float score = angularDist + (distance * 0.001f);

            if (score < closestFOV) {
                closestFOV = score;
                closestPlayer = const_cast<PlayerInfo*>(&player);
                bestTargetAngles = targetAngles;
            }
            });

        if (closestPlayer) {
            targetPawn = closestPlayer;
        }

        return closestPlayer;
    }

    // Apply aim to target with smoothing
    void AimAtTarget(float smoothFactor = 5.0f, float fovLimit = 10.0f) {
        LocalPlayerInfo& localPlayerInfo = g_EntityManager.GetLocalPlayerInfo();

        if (!localPlayerInfo.isValid || !localPlayerInfo.isAlive || !localPlayerInfo.ViewAngles)
            return;

        // Find target
        PlayerInfo* target = GetClosestPlayerPawn(fovLimit);
        if (!target)
            return;

        // Check if head bone exists
        auto it = target->bonePositions.find("head");
        if (it == target->bonePositions.end())
            return;

        Vector3 targetPos = it->second;
        Vector3 eyePos = localPlayerInfo.eyePosition;

        targetPos = targetPos + ((target->velocity) * (1.f/50.f));
		eyePos = eyePos + ((localPlayerInfo.velocity) * (1.f / 55.f));

        // Calculate target angles
        Vector3 targetAngles = CalculateAngles(eyePos, targetPos);
        Vector3 currentAngles = *localPlayerInfo.ViewAngles;

        // Apply smoothing
        Vector3 finalAngles = targetAngles;
        if (smoothFactor > 1.0f) {
            finalAngles = SmoothAngles(currentAngles, targetAngles, smoothFactor);
        }

        // Normalize and clamp
        finalAngles.x = NormalizeAngle(finalAngles.x);
        finalAngles.y = NormalizeAngle(finalAngles.y);
        finalAngles.z = 0.0f;

        // Clamp pitch to valid range
        finalAngles.x = std::clamp(finalAngles.x, -89.0f, 89.0f);

        // Apply angles
        *localPlayerInfo.ViewAngles = finalAngles;
    }

    void RunAimbot(float smoothFactor = 0.0f, float fovLimit = 180.0f) {
        LocalPlayerInfo& localPlayerInfo = g_EntityManager.GetLocalPlayerInfo();

        if (!localPlayerInfo.isValid || !localPlayerInfo.isAlive || !localPlayerInfo.ViewAngles)
            return;

        AimAtTarget(smoothFactor, fovLimit);
    }

    PlayerInfo* GetCurrentTarget() const {
        return targetPawn;
    }

    void ClearTarget() {
        targetPawn = nullptr;
    }
};

// Global instance
static AimbotManager g_AimbotManager;