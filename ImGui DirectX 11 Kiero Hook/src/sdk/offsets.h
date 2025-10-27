#pragma once
#include <cstddef>

// Update these offsets from https://github.com/a2x/cs2-dumper/tree/main/output
namespace Offsets {
    // Module offsets - client.dll
    namespace Client {
        constexpr std::ptrdiff_t dwEntityList = 0x1D07A80;
        constexpr std::ptrdiff_t dwLocalPlayerController = 0x1E11978;
        constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1BE2D10;
        constexpr std::ptrdiff_t dwViewMatrix = 0x1E25F30;
        constexpr std::ptrdiff_t dwGameEntitySystem = 0x1FAC650;
		constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x20F0;
		constexpr std::ptrdiff_t dwViewAngles = 0x1E304B0;
    }

    // CEntityIdentity
    namespace Identity {
        constexpr std::ptrdiff_t pEntity = 0x10;
    }

    // C_BaseEntity
    namespace Entity {
        constexpr std::ptrdiff_t m_pGameSceneNode = 0x330;
        constexpr std::ptrdiff_t m_iHealth = 0x34C;
        constexpr std::ptrdiff_t m_iTeamNum = 0x3EB;
        constexpr std::ptrdiff_t m_vecViewOffset = 0xD80;
        constexpr std::ptrdiff_t m_lifeState = 0x354;
        constexpr std::ptrdiff_t m_iMaxHealth = 0x348;
        constexpr std::ptrdiff_t m_fFlags = 0x3F8;
        constexpr std::ptrdiff_t m_vecVelocity = 0x430; // CNetworkVelocityVector
    }

    // CGameSceneNode
    namespace SceneNode {
        constexpr std::ptrdiff_t m_vecAbsOrigin = 0xD0;
        constexpr std::ptrdiff_t m_angRotation = 0xC0;
        constexpr std::ptrdiff_t m_bDormant = 0x10B;
		constexpr std::ptrdiff_t m_modelState = 0x190;
		constexpr std::ptrdiff_t m_boneArray = 0x80;
		constexpr std::ptrdiff_t m_vecOrigin = 0x88;
    }

    // C_BasePlayerPawn
    namespace Pawn {
        constexpr std::ptrdiff_t m_vOldOrigin = 0x15A0;
        constexpr std::ptrdiff_t m_hController = 0x15B8;
        constexpr std::ptrdiff_t m_ArmorValue = 0x274C;
    }

    // CCSPlayerController
    namespace Controller {
        constexpr std::ptrdiff_t m_hPlayerPawn = 0x8FC;
        constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x850;
        constexpr std::ptrdiff_t m_bPawnIsAlive = 0x904;
    }

    namespace C_BaseModelEntity {
        constexpr std::ptrdiff_t m_CRenderComponent = 0xAE0; // CRenderComponent*
        constexpr std::ptrdiff_t m_CHitboxComponent = 0xAE8; // CHitboxComponent
        constexpr std::ptrdiff_t m_pDestructiblePartsSystemComponent = 0xB00; // CDestructiblePartsComponent*
        constexpr std::ptrdiff_t m_LastHitGroup = 0xB08; // HitGroup_t
        constexpr std::ptrdiff_t m_sLastDamageSourceName = 0xB10; // CGlobalSymbol
        constexpr std::ptrdiff_t m_vLastDamagePosition = 0xB18; // VectorWS
        constexpr std::ptrdiff_t m_bInitModelEffects = 0xB40; // bool
        constexpr std::ptrdiff_t m_bIsStaticProp = 0xB41; // bool
        constexpr std::ptrdiff_t m_nLastAddDecal = 0xB44; // int32
        constexpr std::ptrdiff_t m_nDecalsAdded = 0xB48; // int32
        constexpr std::ptrdiff_t m_iOldHealth = 0xB4C; // int32
        constexpr std::ptrdiff_t m_nRenderMode = 0xB50; // RenderMode_t
        constexpr std::ptrdiff_t m_nRenderFX = 0xB51; // RenderFx_t
        constexpr std::ptrdiff_t m_bAllowFadeInView = 0xB52; // bool
        constexpr std::ptrdiff_t m_clrRender = 0xB70; // Color
        constexpr std::ptrdiff_t m_vecRenderAttributes = 0xB78; // C_UtlVectorEmbeddedNetworkVar<EntityRenderAttribute_t>
        constexpr std::ptrdiff_t m_bRenderToCubemaps = 0xBF8; // bool
        constexpr std::ptrdiff_t m_bNoInterpolate = 0xBF9; // bool
        constexpr std::ptrdiff_t m_Collision = 0xC00; // CCollisionProperty
        constexpr std::ptrdiff_t m_Glow = 0xCB0; // CGlowProperty
        constexpr std::ptrdiff_t m_flGlowBackfaceMult = 0xD08; // float32
        constexpr std::ptrdiff_t m_fadeMinDist = 0xD0C; // float32
        constexpr std::ptrdiff_t m_fadeMaxDist = 0xD10; // float32
        constexpr std::ptrdiff_t m_flFadeScale = 0xD14; // float32
        constexpr std::ptrdiff_t m_flShadowStrength = 0xD18; // float32
        constexpr std::ptrdiff_t m_nObjectCulling = 0xD1C; // uint8
        constexpr std::ptrdiff_t m_nAddDecal = 0xD20; // int32
        constexpr std::ptrdiff_t m_vDecalPosition = 0xD24; // Vector
        constexpr std::ptrdiff_t m_vDecalForwardAxis = 0xD30; // Vector
        constexpr std::ptrdiff_t m_nDecalMode = 0xD3C; // DecalMode_t
        constexpr std::ptrdiff_t m_nRequiredDecalMode = 0xD3D; // DecalMode_t
        constexpr std::ptrdiff_t m_ConfigEntitiesToPropagateMaterialDecalsTo = 0xD40; // C_NetworkUtlVectorBase<CHandle<C_BaseModelEntity>>
        constexpr std::ptrdiff_t m_vecViewOffset = 0xD80; // CNetworkViewOffsetVector
        constexpr std::ptrdiff_t m_pClientAlphaProperty = 0xE60; // CClientAlphaProperty*
        constexpr std::ptrdiff_t m_ClientOverrideTint = 0xE68; // Color
        constexpr std::ptrdiff_t m_bUseClientOverrideTint = 0xE6C; // bool
        constexpr std::ptrdiff_t m_bvDisabledHitGroups = 0xEA8; // uint32[1]
    }
}