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
};

struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}
};

struct ViewMatrix_t {
    float matrix[4][4];
};

// Entity identity structure
struct CEntityIdentity {
    char pad_0000[0x10];
    uintptr_t pEntity;
};

// Game scene node
struct CGameSceneNode {
    char pad_0000[0xD0];
    Vector3 m_vecAbsOrigin;
};

// Base entity replica - only include members we need
struct C_BaseEntity {
    char pad_0000[0x318];
    uintptr_t m_pGameSceneNode; // 0x318
    char pad_0320[0x10];
    int32_t m_iMaxHealth; // 0x330
    int32_t m_iHealth; // 0x334
    int32_t m_lifeState; // 0x338
    char pad_033C[0x8F];
    int32_t m_iTeamNum; // 0x3CB
    int32_t m_fFlags; // 0x3CC

    bool IsAlive() const { return m_lifeState == 256; }

    Vector3 GetOrigin() const {
        if (!m_pGameSceneNode) return Vector3();
        CGameSceneNode* sceneNode = reinterpret_cast<CGameSceneNode*>(m_pGameSceneNode);
        return sceneNode->m_vecAbsOrigin;
    }
};

// Player pawn structure
struct C_CSPlayerPawn : public C_BaseEntity {
    char pad_player[0xF50]; // Extends from C_BaseEntity
    // Add more members if needed
};

// Controller structure
struct CCSPlayerController {
    char pad_0000[0x750];
    char m_sSanitizedPlayerName[128]; // 0x750
    char pad_07D0[0x14];
    uintptr_t m_hPlayerPawn; // 0x7E4 - Handle to pawn
    char pad_07EC[0x14];
    bool m_bPawnIsAlive; // 0x800

    // Resolve handle to actual pawn pointer
    uintptr_t GetPlayerPawn(uintptr_t entityListAddr) const;
};