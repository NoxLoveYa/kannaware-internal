#pragma once
#include "Entity.h"
#include "Offsets.h"
#include <array>
#include <unordered_map>
#include <string>
#include <vector>
#include <Windows.h>

struct PlayerInfo {
    uintptr_t controllerAddress;
    uintptr_t pawnAddress;
    uintptr_t pawn;
    Vector3 position;
    int health;
    int maxHealth;
    int teamNum;
    char name[128];
    bool isAlive;
    bool isValid;

    PlayerInfo() : controllerAddress(0), pawnAddress(0), pawn(0),
        position(), health(0), maxHealth(0), teamNum(0),
        isAlive(false), isValid(false) {
        name[0] = '\0';
    }
};

struct EntityData {
    uint32_t index;
    uintptr_t entityAddress;
    char designerName[64];
    bool isValid;

    EntityData() : index(0), entityAddress(0), isValid(false) {
        designerName[0] = '\0';
    }
};

class EntityManager {
private:
    std::array<PlayerInfo, 64> players;
    std::unordered_map<std::string, std::vector<EntityData>> entityCache;
    uintptr_t clientBase;
    uintptr_t entityListAddr;
    int localPlayerIndex;

    // Get entity from list using the C# logic
public:
    uintptr_t localPlayerController;
    uintptr_t localPlayerPawn;

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
            auto& player = players[i - 1];
            player.isValid = false;

            uintptr_t controller = GetEntityFromList(i);
            if (!controller || controller == localPlayerController) continue;

            // Get pawn handle and resolve it
            uint32_t pawnHandle = *reinterpret_cast<uint32_t*>(
                controller + 0x8FC // m_hPlayerPawn offset
                );

            if (pawnHandle == 0) continue;

            uintptr_t pawn = ResolveHandle(pawnHandle);
            if (!pawn) continue;

            bool isAlive = (*reinterpret_cast<bool*>(controller + 0x904));
            // Check if alive
            if (!isAlive) continue;

			int health = *reinterpret_cast<int*>(pawn + 0x34C);
			int maxHealth = *reinterpret_cast<int*>(pawn + 0x348);
			int teamNum = *reinterpret_cast<uint8_t*>(pawn + 0x3EB);

			uintptr_t sceneNodePtr = *reinterpret_cast<uintptr_t*>(pawn + Offsets::Entity::m_pGameSceneNode);
			if (!sceneNodePtr) continue;
			Vector3 position = *reinterpret_cast<Vector3*>(sceneNodePtr + Offsets::SceneNode::m_vecAbsOrigin);

            // Cache player data
            player.controllerAddress = controller;
            player.pawn = pawn;
            player.health = health;
            player.maxHealth = maxHealth;
            player.teamNum = teamNum;
            player.position = position;
            player.isAlive = isAlive;

            // Get sanitized 
            strncpy_s(player.name, "test", sizeof(player.name) - 1);
			player.name[127] = '\0';

            player.isValid = true;
        }
    }

    void FetchEntities() {
        entityCache.clear();

        // Scan from 65 onwards
        for (uint32_t i = 65; i <= 1024; i++) {
            uintptr_t listEntry = *reinterpret_cast<uintptr_t*>((entityListAddr + (0x8 * ((i & 0x7FFF) >> 9)) + 0x10));
            if (!listEntry)
                continue;

            uintptr_t entity = *reinterpret_cast<uintptr_t*>((listEntry + 0x70 * (i & 0x1FF)));
            if (!entity)
                continue;

            uintptr_t entityIdentity = *reinterpret_cast<uintptr_t*>(entity + 0x10);
            if (!entityIdentity)
                continue;

            uintptr_t designerNamePtr = *reinterpret_cast<uintptr_t*>(entityIdentity + 0x20);
            if (!designerNamePtr)
                continue;

            char* designerName = reinterpret_cast<char*>(designerNamePtr);
            std::string designerNameStr(designerName, strnlen(designerName, 32));

            if (designerNameStr.empty())
                continue;

            EntityData data;
            data.index = i;
            data.entityAddress = entity;
            strncpy_s(data.designerName, designerNameStr.c_str(), sizeof(data.designerName) - 1);
            data.isValid = true;

            entityCache[designerNameStr].push_back(data);
        }
    }

    std::vector<EntityData> GetEntitiesByDesignerName(const char* name) {
        auto it = entityCache.find(std::string(name));
        if (it != entityCache.end()) {
            return it->second;
        }
        return std::vector<EntityData>();
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