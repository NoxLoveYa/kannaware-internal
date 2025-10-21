#pragma once
#include <cstddef>

// Update these offsets from https://github.com/a2x/cs2-dumper/tree/main/output
namespace Offsets {
    // Module offsets - client.dll
    namespace Client {
        constexpr std::ptrdiff_t dwEntityList = 0x1D00690;
        constexpr std::ptrdiff_t dwLocalPlayerController = 0x1E0A348;
        constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1BDBB10;
        constexpr std::ptrdiff_t dwViewMatrix = 0x1E1E920;
        constexpr std::ptrdiff_t dwGameEntitySystem = 0x1FA4FE0;
		constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x20F0;
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
    }

    // CGameSceneNode
    namespace SceneNode {
        constexpr std::ptrdiff_t m_vecAbsOrigin = 0xD0;
        constexpr std::ptrdiff_t m_angRotation = 0xC0;
        constexpr std::ptrdiff_t m_bDormant = 0x10B;
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
}