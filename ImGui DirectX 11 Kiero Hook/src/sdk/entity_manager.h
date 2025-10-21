#pragma once
#include "Entity.h"
#include "Offsets.h"
#include <array>
#include <Windows.h>

struct PlayerInfo {
    uintptr_t controllerAddress;
    uintptr_t pawnAddress;
    C_CSPlayerPawn* pawn;
    Vector3 position;
    int health;
    int maxHealth;
    int teamNum;
    char name[128];
    bool isAlive;
    bool isValid;

    PlayerInfo() : controllerAddress(0), pawnAddress(0), pawn(nullptr),
        position(), health(0), maxHealth(0), teamNum(0),
        isAlive(false), isValid(false) {
        name[0] = '\0';
    }
};

class EntityManager {
private:
    std::array<PlayerInfo, 64> players;
    uintptr_t clientBase;
    uintptr_t entityListAddr;
    uintptr_t localPlayerController;
    int localPlayerIndex;

    // Helper to read entity from list
    uintptr_t GetEntityFromIndex(int index) {
        if (!entityListAddr) return 0;

        uintptr_t listEntry = entityListAddr + (8 * (index & 0x7FFF) >> 9) + 16;
        if (!listEntry) return 0;

        uintptr_t controller = *reinterpret_cast<uintptr_t*>(listEntry);
        if (!controller) return 0;

        uintptr_t identityAddr = controller + 112 * (index & 0x1FF);
        if (!identityAddr) return 0;

        CEntityIdentity* identity = reinterpret_cast<CEntityIdentity*>(identityAddr);
        if (!identity) return 0;

        return identity->pEntity;
    }

    // Resolve handle to entity pointer
    uintptr_t ResolveHandle(uint32_t handle) {
        if (handle == 0 || handle == 0xFFFFFFFF) return 0;
        int index = handle & 0x7FFF;
        return GetEntityFromIndex(index);
    }

public:
    EntityManager() : clientBase(0), entityListAddr(0),
        localPlayerController(0), localPlayerIndex(-1) {
    }

    void Initialize(uintptr_t clientDll) {
        clientBase = clientDll;
        if (!clientBase) return;

        entityListAddr = clientBase + Offsets::Client::dwEntityList;

        uintptr_t localControllerPtr = clientBase + Offsets::Client::dwLocalPlayerController;
        localPlayerController = *reinterpret_cast<uintptr_t*>(localControllerPtr);
    }

    void UpdateEntities() {
        if (!clientBase || !entityListAddr) return;

        // Find local player index for team checking
        int localTeam = -1;
        if (localPlayerController) {
            C_BaseEntity* localEntity = reinterpret_cast<C_BaseEntity*>(localPlayerController);
            localTeam = localEntity->m_iTeamNum;
        }

        // Update all players
        for (int i = 0; i < 64; i++) {
            auto& player = players[i];
            player.isValid = false;

            uintptr_t controllerAddr = GetEntityFromIndex(i);
            if (!controllerAddr || controllerAddr == localPlayerController) continue;

            CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(controllerAddr);
            if (!controller) continue;

            // Get pawn handle and resolve it
            uint32_t pawnHandle = *reinterpret_cast<uint32_t*>(controllerAddr + Offsets::Controller::m_hPlayerPawn);
            uintptr_t pawnAddr = ResolveHandle(pawnHandle);
            if (!pawnAddr) continue;

            C_CSPlayerPawn* pawn = reinterpret_cast<C_CSPlayerPawn*>(pawnAddr);
            if (!pawn) continue;

            // Check if alive
            if (!pawn->IsAlive() || pawn->m_iHealth <= 0) continue;

            // Cache player data
            player.controllerAddress = controllerAddr;
            player.pawnAddress = pawnAddr;
            player.pawn = pawn;
            player.health = pawn->m_iHealth;
            player.maxHealth = pawn->m_iMaxHealth;
            player.teamNum = pawn->m_iTeamNum;
            player.position = pawn->GetOrigin();
            player.isAlive = pawn->IsAlive();

            // Copy name
            memcpy(player.name, controller->m_sSanitizedPlayerName, sizeof(player.name));
            player.name[127] = '\0';

            player.isValid = true;
        }
    }

    // Get all valid players
    template<typename Func>
    void ForEachPlayer(Func&& func) {
        for (auto& player : players) {
            if (player.isValid && player.pawn) {
                func(player);
            }
        }
    }

    PlayerInfo* GetPlayer(int index) {
        if (index < 0 || index >= 64) return nullptr;
        return players[index].isValid ? &players[index] : nullptr;
    }

    uintptr_t GetLocalPlayerController() const { return localPlayerController; }
};

// Global instance
static EntityManager g_EntityManager;