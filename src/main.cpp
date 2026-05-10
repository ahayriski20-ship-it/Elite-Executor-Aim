#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dobby.h"

#define LOG_TAG "EliteExecutor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct CVector { float x, y, z; };

uintptr_t g_libBase = 0;
void* CurrentTargetPed = nullptr;

typedef void (*SetWeaponLockOnTarget_t)(void*, void*);
SetWeaponLockOnTarget_t SetWeaponLockOnTarget = nullptr;
typedef void (*UpdateAimingCoors_t)(void*, CVector*);
UpdateAimingCoors_t Original_UpdateAimingCoors = nullptr;

uintptr_t GetLibraryBase(const char* libraryName) {
    FILE* fp = fopen("/proc/self/maps", "rt");
    if (!fp) return 0;
    char line[512]; uintptr_t baseAddress = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libraryName)) {
            sscanf(line, "%x-%*x", &baseAddress);
            break;
        }
    }
    fclose(fp);
    return baseAddress;
}

void UnprotectMemory(uintptr_t addr, size_t len) {
    uintptr_t pageStart = addr & ~(PAGE_SIZE - 1);
    uintptr_t pageEnd = (addr + len + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    mprotect((void*)pageStart, pageEnd - pageStart, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void PatchMemory(uintptr_t address, const char* hexData, size_t size) {
    UnprotectMemory(address, size);
    memcpy((void*)address, hexData, size);
}

void Hooked_UpdateAimingCoors(void* camera, CVector* targetCoords) {
    if (camera && targetCoords && CurrentTargetPed) {
        // Logika koordinat kepala
    }
    Original_UpdateAimingCoors(camera, targetCoords);
}

void* MainThread(void* arg) {
    LOGI("Thread Elite Executor dimulai (Author: Riski Boren)...");
    while (g_libBase == 0) {
        g_libBase = GetLibraryBase("libGTASA.so");
        sleep(1);
    }
    
    SetWeaponLockOnTarget = (SetWeaponLockOnTarget_t)(g_libBase + 0x004a82d4 + 1);
    
    uintptr_t clearWeaponTargetAddr = g_libBase + 0x004c5874;
    const char bx_lr[] = "\x70\x47";
    PatchMemory(clearWeaponTargetAddr, bx_lr, 2);
    
    uintptr_t updateAimingCoorsAddr = g_libBase + 0x003e19ba;
    DobbyHook((void*)updateAimingCoorsAddr, (void*)Hooked_UpdateAimingCoors, (void**)&Original_UpdateAimingCoors);
    
    while (true) {
        void* localPlayer = nullptr; 
        if (localPlayer && CurrentTargetPed) {
            SetWeaponLockOnTarget(localPlayer, CurrentTargetPed);
        }
        usleep(15000);
    }
    return nullptr;
}

__attribute__((constructor))
void InitLibrary() {
    pthread_t ptid;
    pthread_create(&ptid, nullptr, MainThread, nullptr);
}
