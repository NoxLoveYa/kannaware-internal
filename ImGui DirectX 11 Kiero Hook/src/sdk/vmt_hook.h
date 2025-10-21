#pragma once
#include <Windows.h>
#include <cstdint>

class VMTHook {
private:
    void** newVTable;
    void** originalVTable;
    void* instance;
    size_t functionCount;

public:
    VMTHook(void* inst) : instance(inst), newVTable(nullptr), originalVTable(nullptr), functionCount(0) {
        if (!instance) return;

        // Get the CURRENT vtable (might already be hooked)
        void** currentVTable = *(void***)instance;
        if (!currentVTable) return;

        // Count virtual functions
        MEMORY_BASIC_INFORMATION mbi;
        while (VirtualQuery(currentVTable + functionCount, &mbi, sizeof(mbi)) &&
            mbi.Protect != PAGE_NOACCESS && functionCount < 200) {
            if (!currentVTable[functionCount]) break;
            functionCount++;
        }

        if (functionCount == 0) return;

        // Store CURRENT vtable as "original" (even if already hooked by someone else)
        originalVTable = currentVTable;

        // Allocate new vtable
        newVTable = new void* [functionCount];
        memcpy(newVTable, originalVTable, sizeof(void*) * functionCount);

        // Change memory protection
        DWORD oldProtect;
        VirtualProtect(instance, sizeof(void*), PAGE_READWRITE, &oldProtect);

        // Replace vtable pointer
        *(void***)instance = newVTable;

        VirtualProtect(instance, sizeof(void*), oldProtect, &oldProtect);
    }

    ~VMTHook() {
        if (instance && originalVTable && newVTable) {
            // Check if the instance still has our vtable before restoring
            void** currentVTable = *(void***)instance;
            if (currentVTable == newVTable) {
                DWORD oldProtect;
                VirtualProtect(instance, sizeof(void*), PAGE_READWRITE, &oldProtect);
                *(void***)instance = originalVTable;
                VirtualProtect(instance, sizeof(void*), oldProtect, &oldProtect);
            }
        }

        if (newVTable) {
            delete[] newVTable;
            newVTable = nullptr;
        }
    }

    bool HookFunction(size_t index, void* hook, void** original) {
        if (!newVTable || index >= functionCount) return false;

        if (original) {
            *original = newVTable[index];
        }

        DWORD oldProtect;
        VirtualProtect(&newVTable[index], sizeof(void*), PAGE_READWRITE, &oldProtect);
        newVTable[index] = hook;
        VirtualProtect(&newVTable[index], sizeof(void*), oldProtect, &oldProtect);

        return true;
    }

    void* GetOriginalFunction(size_t index) {
        if (!originalVTable || index >= functionCount) return nullptr;
        return originalVTable[index];
    }

    // Get the REAL original function, bypassing any existing hooks
    void* GetRealOriginalFunction(size_t index) {
        if (!originalVTable || index >= functionCount) return nullptr;

        void* func = originalVTable[index];

        // Check if this might be a hook/trampoline (starts with JMP)
        unsigned char* bytes = (unsigned char*)func;

        // Check for common hook patterns:
        // E9 = JMP rel32
        // FF 25 = JMP [rip+offset]
        // 48 B8 = MOV RAX, imm64 (then possibly JMP RAX)
        if (bytes[0] == 0xE9) {
            // Relative JMP - might be a hook, but we can't easily follow it
            // Just return what we have
            return func;
        }

        return func;
    }

    bool IsValid() const {
        return newVTable != nullptr && originalVTable != nullptr;
    }
};