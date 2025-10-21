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

// Entity identity structure (from offsets: 0x10 = m_pEntity, 0x20 = m_designerName)
struct CEntityIdentity {
    char pad_0000[0x14]; // 0x00
    int32_t m_nameStringableIndex; // 0x14
    uint64_t m_name; // 0x18 - CUtlSymbolLarge
    uint64_t m_designerName; // 0x20 - CUtlSymbolLarge
    char pad_0028[0x8]; // 0x28
    uint32_t m_flags; // 0x30
    char pad_0034[0x4]; // 0x34
    uint32_t m_worldGroupId; // 0x38
    uint32_t m_fDataObjectTypes; // 0x3C
    uint16_t m_PathIndex; // 0x40
    char pad_0042[0xE]; // 0x42
    CEntityIdentity* m_pPrev; // 0x50
    CEntityIdentity* m_pNext; // 0x58
    CEntityIdentity* m_pPrevByClass; // 0x60
    CEntityIdentity* m_pNextByClass; // 0x68
};

// Game scene node (m_vecOrigin at 0xD0 based on C_SkyCamera offsets)
struct CGameSceneNode {
    char pad_0000[0x88]; // 0x00
    uint32_t m_hParent; // 0x78 - CGameSceneNodeHandle
    char pad_008C[0xC]; // 0x7C
    Vector3 m_vecOrigin; // 0x88 - CNetworkOriginCellCoordQuantizedVector
    char pad_00B4[0xC]; // 0x94
    float m_angRotation[3]; // 0xC0 - QAngle
    float m_flScale; // 0xCC
    Vector3 m_vecAbsOrigin; // 0xD0 - VectorWS
    float m_angAbsRotation[3]; // 0xDC - QAngle
    float m_flAbsScale; // 0xE8
};

// Base entity (using C_BaseEntity offsets from the dump)
struct C_BaseEntity {
    char pad_0000[0x330]; // 0x00
    uintptr_t m_pGameSceneNode; // 0x330
    uintptr_t m_pRenderComponent; // 0x338
    uintptr_t m_pCollision; // 0x340
    int32_t m_iMaxHealth; // 0x348
    int32_t m_iHealth; // 0x34C
    char pad_0350[0x4]; // 0x350
    uint8_t m_lifeState; // 0x354
    bool m_bTakesDamage; // 0x355
    char pad_0356[0x2]; // 0x356
    uint32_t m_nTakeDamageFlags; // 0x358
    char pad_035C[0x4]; // 0x35C
    uint32_t m_nPlatformType; // 0x360
    uint8_t m_ubInterpolationFrame; // 0x361
    char pad_0362[0x2]; // 0x362
    uint32_t m_hSceneObjectController; // 0x364
    char pad_0368[0x7F]; // 0x368
    uint8_t m_iTeamNum; // 0x3EB
    char pad_03EC[0xC]; // 0x3EC
    uint32_t m_fFlags; // 0x3F8

    bool IsAlive() const { return m_lifeState == 0; }

    Vector3 GetOrigin() const {
        if (!m_pGameSceneNode) return Vector3();
        CGameSceneNode* sceneNode = reinterpret_cast<CGameSceneNode*>(m_pGameSceneNode);
        return sceneNode->m_vecAbsOrigin;
    }
};

// Player pawn structure (using C_CSPlayerPawn offsets)
struct C_CSPlayerPawn : public C_BaseEntity {
    char pad_03FC[0x127C]; // Padding from end of C_BaseEntity (0x3FC) to m_pBulletServices (0x1678)

    void* m_pBulletServices; // 0x1678
    void* m_pHostageServices; // 0x1680
    void* m_pBuyServices; // 0x1688
    void* m_pGlowServices; // 0x1690
    void* m_pActionTrackingServices; // 0x1698
    void* m_pDamageReactServices; // 0x16A0
    float m_flHealthShotBoostExpirationTime; // 0x16A8
    float m_flLastFiredWeaponTime; // 0x16AC
    bool m_bHasFemaleVoice; // 0x16B0
    char pad_16B1[0x3]; // 0x16B1
    float m_flLandingTimeSeconds; // 0x16B4
    float m_flOldFallVelocity; // 0x16B8
    char m_szLastPlaceName[18]; // 0x16BC
    // Add more members as needed from the offsets
};

// Controller structure (using CCSPlayerController offsets)
struct CCSPlayerController {
    char pad_0000[0x7F8]; // Padding up to m_pInGameMoneyServices

    void* m_pInGameMoneyServices; // 0x7F8
    void* m_pInventoryServices; // 0x800
    void* m_pActionTrackingServices; // 0x808
    void* m_pDamageServices; // 0x810
    uint32_t m_iPing; // 0x818
    bool m_bHasCommunicationAbuseMute; // 0x81C
    char pad_081D[3];
    uint32_t m_uiCommunicationMuteFlags; // 0x820
    char pad_0824[4];
    uint64_t m_szCrosshairCodes; // 0x828
    uint8_t m_iPendingTeamNum; // 0x830
    char pad_0831[3];
    float m_flForceTeamTime; // 0x834
    int32_t m_iCompTeammateColor; // 0x838
    bool m_bEverPlayedOnTeam; // 0x83C
    char pad_083D[3];
    float m_flPreviousForceJoinTeamTime; // 0x840
    char pad_0844[4];
    uint64_t m_szClan; // 0x848
    uint64_t m_sSanitizedPlayerName; // 0x850 - CUtlString
    int32_t m_iCoachingTeam; // 0x858
    char pad_085C[0xA0]; // Padding to m_hPlayerPawn
    uint32_t m_hPlayerPawn; // 0x8FC - CHandle<C_CSPlayerPawn>
    uint32_t m_hObserverPawn; // 0x900
    bool m_bPawnIsAlive; // 0x904
    char pad_0905[3];
    uint32_t m_iPawnHealth; // 0x908
    int32_t m_iPawnArmor; // 0x90C
    bool m_bPawnHasDefuser; // 0x910
    bool m_bPawnHasHelmet; // 0x911
    uint16_t m_nPawnCharacterDefIndex; // 0x912
    int32_t m_iPawnLifetimeStart; // 0x914
    int32_t m_iPawnLifetimeEnd; // 0x918
    int32_t m_iPawnBotDifficulty; // 0x91C
    uint32_t m_hOriginalControllerOfCurrentPawn; // 0x920
    int32_t m_iScore; // 0x924
    uint8_t m_recentKillQueue[8]; // 0x928
    uint8_t m_nFirstKill; // 0x930
    uint8_t m_nKillCount; // 0x931
    bool m_bMvpNoMusic; // 0x932
    char pad_0933[1];
    int32_t m_eMvpReason; // 0x934
    int32_t m_iMusicKitID; // 0x938
    int32_t m_iMusicKitMVPs; // 0x93C
    int32_t m_iMVPs; // 0x940

    // Helper to get sanitized name
    const char* GetPlayerName() {
        // m_sSanitizedPlayerName is a CUtlString, which is just a pointer to the actual string
        const char* namePtr = reinterpret_cast<const char*>(m_sSanitizedPlayerName);
        return namePtr ? namePtr : "";
    }
};