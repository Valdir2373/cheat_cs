#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

uintptr_t OFFSET_LOCAL_PLAYER = 0x22F5028;
uintptr_t OFFSET_ENTITY_LIST = 0x24B0258;
uintptr_t OFFSET_VIEW_ANGLES = 0x23003A8;

struct Vec3 {
    float x, y, z;
};

struct Player {
    int health;
    int team;
    Vec3 position;
    Vec3 velocity;
    bool spotted;
};

DWORD GetProcessId(const char* processName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32First(hSnap, &procEntry)) {
            do {
                if (!_stricmp(procEntry.szExeFile, processName)) {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do {
                if (!_stricmp(modEntry.szModule, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

bool ReadMemory(HANDLE hProcess, uintptr_t address, void* buffer, size_t size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
}

bool WriteMemory(HANDLE hProcess, uintptr_t address, const void* buffer, size_t size) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) && bytesWritten == size;
}

bool GetPlayerInfo(HANDLE hProcess, uintptr_t playerPtr, Player& player) {
    if (!playerPtr) return false;

    if (!ReadMemory(hProcess, playerPtr + 0x344, &player.health, sizeof(int))) return false;
    if (!ReadMemory(hProcess, playerPtr + 0x3E3, &player.team, sizeof(int))) return false;
    if (!ReadMemory(hProcess, playerPtr + 0xCC8, &player.position, sizeof(Vec3))) return false;
    if (!ReadMemory(hProcess, playerPtr + 0xBC8, &player.velocity, sizeof(Vec3))) return false;
    if (!ReadMemory(hProcess, playerPtr + 0x3288, &player.spotted, sizeof(bool))) return false;

    return true;
}

float Distance(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

Vec3 CalcAngles(const Vec3& from, const Vec3& to) {
    Vec3 delta = {to.x - from.x, to.y - from.y, to.z - from.z};
    Vec3 angles;
    angles.x = -atan2f(delta.z, sqrtf(delta.x * delta.x + delta.y * delta.y)) * 180.0f / 3.14159265f;
    angles.y = atan2f(delta.y, delta.x) * 180.0f / 3.14159265f;
    angles.z = 0.0f;
    return angles;
}

bool IsShooting() {
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

bool IsKeyPressed(int key) {
    return (GetAsyncKeyState(key) & 1) != 0;
}

bool FindBestTarget(HANDLE hProcess, uintptr_t clientBase, const Player& localPlayer, Vec3& outAngles) {
    uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
    uint64_t entityListPtr;
    if (!ReadMemory(hProcess, entityListAddr, &entityListPtr, sizeof(uint64_t))) return false;

    float bestScore = FLT_MAX;
    bool found = false;

    for (int i = 1; i < 64; ++i) {
        uint64_t entityAddr = entityListPtr + (i * 0x78);
        uint64_t entityPtr;
        if (!ReadMemory(hProcess, entityAddr, &entityPtr, sizeof(uint64_t))) continue;
        if (!entityPtr) continue;

        Player enemy;
        if (!GetPlayerInfo(hProcess, entityPtr, enemy)) continue;
        if (enemy.health <= 0 || enemy.health > 100) continue;
        if (enemy.team == localPlayer.team) continue;

        Vec3 angles = CalcAngles(localPlayer.position, enemy.position);
        float deltaYaw = fabsf(angles.y);
        float deltaPitch = fabsf(angles.x);
        float score = deltaYaw + deltaPitch + Distance(localPlayer.position, enemy.position) * 0.001f;

        if (score < bestScore) {
            bestScore = score;
            outAngles = angles;
            found = true;
        }
    }

    return found;
}

int main() {
    std::cout << "=== CS2 Silent Aimbot ===" << std::endl;
    std::cout << "[+] Press F6 to toggle silent aimbot" << std::endl;
    std::cout << "[+] Hold LEFT MOUSE BUTTON to fire" << std::endl;
    std::cout << "[+] Press END to exit" << std::endl;

    DWORD procId = GetProcessId("cs2.exe");
    if (!procId) {
        std::cout << "[!] CS2 not found!" << std::endl;
        return 1;
    }

    uintptr_t clientBase = GetModuleBaseAddress(procId, "client.dll");
    if (!clientBase) {
        std::cout << "[!] client.dll not found!" << std::endl;
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, procId);
    if (!hProcess) {
        std::cout << "[!] Failed to open process!" << std::endl;
        return 1;
    }

    bool active = false;
    bool wasShooting = false;
    uintptr_t viewAnglesAddr = clientBase + OFFSET_VIEW_ANGLES;

    while (true) {
        if (IsKeyPressed(VK_END)) break;
        if (IsKeyPressed(VK_F6)) {
            active = !active;
            std::cout << "[+] Silent Aimbot " << (active ? "ON" : "OFF") << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (!active) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        bool shooting = IsShooting();
        if (!shooting) {
            wasShooting = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (!wasShooting) {
            // Novo disparo detectado: mira silenciosamente durante o clique
            uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
            uint64_t localPlayerPtr;
            if (!ReadMemory(hProcess, lpAddr, &localPlayerPtr, sizeof(uint64_t))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            Player localPlayer;
            if (!GetPlayerInfo(hProcess, localPlayerPtr, localPlayer)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            Vec3 aimAngles;
            if (FindBestTarget(hProcess, clientBase, localPlayer, aimAngles)) {
                Vec3 originalAngles;
                if (!ReadMemory(hProcess, viewAnglesAddr, &originalAngles, sizeof(Vec3))) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                WriteMemory(hProcess, viewAnglesAddr, &aimAngles, sizeof(Vec3));
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
                WriteMemory(hProcess, viewAnglesAddr, &originalAngles, sizeof(Vec3));
            }
        }

        wasShooting = shooting;
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }

    CloseHandle(hProcess);
    std::cout << "[+] Silent Aimbot stopped" << std::endl;
    return 0;
}
