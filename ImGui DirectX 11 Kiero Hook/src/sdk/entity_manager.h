#pragma once
#include "Entity.h"
#include "Offsets.h"
#include <array>
#include <unordered_map>
#include <string>
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
    uintptr_t localPlayerController;
    int localPlayerIndex;
    int entityHighestIndex;

    // Read pointer safely
    template<typename T>
    T ReadMemory(uintptr_t address) {
        if (!address) return T();
        __try {
            return *reinterpret_cast<T*>(address);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return T();
        }
    }

    // Read string safely
    void ReadString(uintptr_t address, char* buffer, size_t size) {
        if (!address || !buffer || size == 0) return;
        __try {
            const char* str = reinterpret_cast<const char*>(address);
            size_t i = 0;
            while (i < size - 1 && str[i] != '\0') {
                buffer[i] = str[i];
                i++;
            }
            buffer[i] = '\0';
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            buffer[0] = '\0';
        }
    }

    // Get entity from list using the C# logic
    uintptr_t GetEntityFromList(uint32_t index) {
        if (!entityListAddr || index == 0) return 0;

        // First level: (entityList + (0x8 * ((index & 0x7FFF) >> 9)) + 0x10)
        uintptr_t listEntry = ReadMemory<uintptr_t>(
            entityListAddr + (0x8 * ((index & 0x7FFF) >> 9)) + 0x10
        );
        if (!listEntry) return 0;

        // Second level: listEntry + 0x70 * (index & 0x1FF)
        uintptr_t entity = ReadMemory<uintptr_t>(
            listEntry + 0x70 * (index & 0x1FF)
        );

        return entity;
    }

    // Get designer name from entity
    bool GetDesignerName(uintptr_t entity, char* buffer, size_t size) {
        if (!entity || !buffer || size == 0) return false;

        // entity + 0x10 = CEntityIdentity*
        uintptr_t entityIdentity = ReadMemory<uintptr_t>(entity + 0x10);
        if (!entityIdentity) return false;

        // CEntityIdentity + 0x20 = m_designerName (CUtlSymbolLarge)
        uintptr_t designerNamePtr = ReadMemory<uintptr_t>(entityIdentity + 0x20);
        if (!designerNamePtr) return false;

        ReadString(designerNamePtr, buffer, size);
        return buffer[0] != '\0';
    }

    // Resolve CHandle to actual entity pointer (like C# GetPlayerByHandle)
    uintptr_t ResolveHandle(uint32_t handle) {
        if (handle == 0 || handle == 0xFFFFFFFF) return 0;
        return GetEntityFromList(handle);
    }

public:
    EntityManager() : clientBase(0), entityListAddr(0),
        localPlayerController(0), localPlayerIndex(-1), entityHighestIndex(1) {
    }

    void Initialize(uintptr_t clientDll) {
        clientBase = clientDll;
        if (!clientBase) return;

        entityListAddr = clientBase + Offsets::Client::dwEntityList;
        UpdateHighestEntityIndex();
    }

    void UpdateHighestEntityIndex() {
        if (!clientBase) return;
        entityHighestIndex = ReadMemory<int32_t>(
            clientBase + Offsets::Client::dwGameEntitySystem_highestEntityIndex
        );
    }

    void UpdateLocalPlayer() {
        if (!clientBase) return;
        localPlayerController = ReadMemory<uintptr_t>(
            clientBase + Offsets::Client::dwLocalPlayerController
        );
    }

    void UpdatePlayers() {
        if (!clientBase || !entityListAddr) return;

        UpdateLocalPlayer();

        // Update all player controllers (indices 1-64)
        for (int i = 1; i <= 64; i++) {
            auto& player = players[i - 1];
            player.isValid = false;

            uintptr_t controllerAddr = GetEntityFromList(i);
            if (!controllerAddr || controllerAddr == localPlayerController) continue;

            CCSPlayerController* controller = reinterpret_cast<CCSPlayerController*>(controllerAddr);
            if (!controller) continue;

            // Get pawn handle and resolve it (like C# code does)
            uint32_t pawnHandle = ReadMemory<uint32_t>(
                controllerAddr + 0x8FC // m_hPlayerPawn offset
            );

            if (pawnHandle == 0) continue;

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

            // Get sanitized name
            const char* nameStr = controller->GetPlayerName();
            if (nameStr) {
                strncpy_s(player.name, nameStr, sizeof(player.name) - 1);
                player.name[127] = '\0';
            }

            player.isValid = true;
        }
    }

    void FetchEntities() {
        UpdateHighestEntityIndex();
        entityCache.clear();

        int maxScan = min(entityHighestIndex, 1024);

        // Scan from 65 onwards (like C# code)
        for (uint32_t i = 65; i <= maxScan; i++) {
            uintptr_t entity = GetEntityFromList(i);
            if (!entity) continue;

            char designerName[64];
            if (!GetDesignerName(entity, designerName, sizeof(designerName))) continue;

            EntityData data;
            data.index = i;
            data.entityAddress = entity;
            strncpy_s(data.designerName, designerName, sizeof(data.designerName) - 1);
            data.isValid = true;

            entityCache[std::string(designerName)].push_back(data);
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
};

// Global instance
static EntityManager g_EntityManager;