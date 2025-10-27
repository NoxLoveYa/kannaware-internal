#pragma once
#include "Entity.h"
#include "Offsets.h"
#include <array>
#include <unordered_map>
#include <string>
#include <vector>
#include <Windows.h>

constexpr size_t MAX_NAME_LEN = 128;

struct PlayerInfo {
    uintptr_t controllerAddress;
    uintptr_t pawnAddress;
    uintptr_t pawn;
    Vector3 position;
    std::unordered_map<std::string, Vector3> bonePositions;
    int health;
    int maxHealth;
    int teamNum;
    char name[128];
    bool isAlive;
    bool isValid;

    PlayerInfo() : controllerAddress(0), pawnAddress(0), pawn(0),
        position(), bonePositions(), health(0), maxHealth(0), teamNum(0),
        isAlive(false), isValid(false) {
        name[0] = '\0';
    }
};

static std::unordered_map<std::string, int> Bones = {
    { "head", 6 },
    { "neck_0", 5 },
    { "spine_1", 4 },
    { "spine_2", 2 },
    { "pelvis", 0 },
    { "arm_upper_L", 8 },
    { "arm_lower_L", 9 },
    { "hand_L", 10 },
    { "arm_upper_R", 13 },
    { "arm_lower_R", 14 },
    { "hand_R", 15 },
    { "leg_upper_L", 22 },
    { "leg_lower_L", 23 },
    { "ankle_L", 24 },
    { "leg_upper_R", 25 },
    { "leg_lower_R", 26 },
    { "ankle_R", 27 }
};

class EntityManager {
private:
    std::array<PlayerInfo, 64> players;
    uintptr_t clientBase;
    uintptr_t entityListAddr;
    int localPlayerIndex;

    uintptr_t localPlayerController;
    uintptr_t localPlayerPawn;
public:

    uintptr_t GetEntityFromList(uint32_t index) {
        if (!entityListAddr) return 0;

        uintptr_t listEntry = *reinterpret_cast<uintptr_t*>((entityListAddr + (0x8 * ((index & 0x7FFF) >> 9)) + 0x10));
        if (!listEntry)
            return 0;

        uintptr_t entity = *reinterpret_cast<uintptr_t*>((listEntry + 0x70 * (index & 0x1FF)));
        return entity;
    }

    uintptr_t GetEntityIdentityFromEntity(uintptr_t entity) {
        if (!entity) return NULL;
        return *reinterpret_cast<uintptr_t*>(entity + 0x10);
	}

    // Get designer name from entity
    char* GetDesignerName(uintptr_t entityIdentity) {
        if (!entityIdentity) return NULL;

        uintptr_t designerNamePtr = *reinterpret_cast<uintptr_t*>(entityIdentity + 0x20);
        if (!designerNamePtr) return NULL;

        char* designerName = reinterpret_cast<char*>(designerNamePtr);
        return designerName;
    }

    std::vector<uintptr_t> GetEntityFromDesignerName(const char* name) {
        std::vector<uintptr_t> entities;
        for (uint32_t i = 1; i <= 64; i++) {
            uintptr_t entity = GetEntityFromList(i);
            if (!entity)
                continue;
            uintptr_t entityIdentity = GetEntityIdentityFromEntity(entity);
            if (!entityIdentity)
                continue;
            char* designerName = GetDesignerName(entityIdentity);
            if (!designerName)
                continue;
            if (strcmp(designerName, name) == 0) {
                entities.push_back(entity);
            }
        }
        return entities;
	}

    // Resolve CHandle to actual entity pointer
    uintptr_t ResolveHandle(uint32_t handle) {
        if (handle == 0 || handle == 0xFFFFFFFF) return 0;
        return GetEntityFromList(handle);
    }

    EntityManager() : clientBase(0), entityListAddr(0),
        localPlayerController(0), localPlayerPawn(0), localPlayerIndex(-1) {
    }

    void Initialize(uintptr_t clientDll) {
        clientBase = clientDll;
        if (!clientBase) return;

        entityListAddr = *reinterpret_cast<uintptr_t*>(clientBase + Offsets::Client::dwEntityList);
    }

    void UpdateLocalPlayer() {
        if (!clientBase) return;
        localPlayerController = *reinterpret_cast<uintptr_t*>(clientBase + Offsets::Client::dwLocalPlayerController);
		localPlayerPawn = *reinterpret_cast<uintptr_t*>(clientBase + Offsets::Client::dwLocalPlayerPawn);
    }

    void UpdatePlayers() {
        if (!clientBase || !entityListAddr) return;

        UpdateLocalPlayer();

        // Update all player controllers (indices 1-64)
        for (int i = 1; i <= 64; i++) {
            if (i > players.max_size()) continue;

            PlayerInfo& player = players[i - 1];
            player.isValid = false;

            uintptr_t controller = GetEntityFromList(i);
            if (!controller || controller == localPlayerController) continue;

            // Get pawn handle and resolve it
            uint32_t pawnHandle = *reinterpret_cast<uint32_t*>(
                controller + Offsets::Controller::m_hPlayerPawn // m_hPlayerPawn offset
                );

            if (pawnHandle == 0) continue;

            uintptr_t pawn = ResolveHandle(pawnHandle);
            if (!pawn) continue;

            bool isAlive = (*reinterpret_cast<bool*>(controller + Offsets::Controller::m_bPawnIsAlive));
            // Check if alive
            if (!isAlive) continue;

			uintptr_t sanitizedNamePtr = *reinterpret_cast<uintptr_t*>(controller + Offsets::Controller::m_sSanitizedPlayerName);
			if (!sanitizedNamePtr) continue;

			char* sanitizedName = reinterpret_cast<char*>(sanitizedNamePtr);
			int health = *reinterpret_cast<int*>(pawn + Offsets::Entity::m_iHealth);
			int maxHealth = *reinterpret_cast<int*>(pawn + Offsets::Entity::m_iMaxHealth);
			int teamNum = *reinterpret_cast<uint8_t*>(pawn + Offsets::Entity::m_iTeamNum);

			uintptr_t sceneNodePtr = *reinterpret_cast<uintptr_t*>(pawn + Offsets::Entity::m_pGameSceneNode);
			if (!sceneNodePtr) continue;
			uintptr_t boneArrayPtr = *reinterpret_cast<uintptr_t*>(sceneNodePtr + Offsets::SceneNode::m_modelState + 128);
			if (!boneArrayPtr) continue;

            for (const auto& [boneName, boneIndex] : Bones) {
                Vector3 bonePos = *reinterpret_cast<Vector3*>(boneArrayPtr + (boneIndex * 32));
				player.bonePositions[boneName] = bonePos;
			}

            Vector3 position = *reinterpret_cast<Vector3*>(sceneNodePtr + Offsets::SceneNode::m_vecAbsOrigin);

            if (sanitizedName) {
                // Copy string safely (ensure null termination)
                strncpy_s(player.name, sizeof(player.name), sanitizedName, _TRUNCATE);
                player.name[MAX_NAME_LEN - 1] = '\0';
            }
            else {
                player.name[0] = '\0';
            }

            // Cache player data
            player.controllerAddress = controller;
            player.pawn = pawn;
            player.health = health;
            player.maxHealth = maxHealth;
            player.teamNum = teamNum;
            player.position = position;
            player.isAlive = isAlive;
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

    uintptr_t GetLocalPlayerPawn() {
        return localPlayerPawn;
	}
};

// Global instance
static EntityManager g_EntityManager;