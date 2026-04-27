#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

const uintptr_t OFFSET_LOCAL_PLAYER = 0x22F5028;
const uintptr_t OFFSET_ENTITY_LIST = 0x24B0258;
const uintptr_t OFFSET_VIEW_ANGLES = 0x23003A8;

// Player struct offsets
const uintptr_t PLAYER_HEALTH = 0x344;
const uintptr_t PLAYER_TEAM = 0x3E3;
const uintptr_t PLAYER_POSITION = 0xCC8;
const uintptr_t PLAYER_SPOITED = 0x3288;

struct Vec3 {
    float x, y, z;
};

class Triggerbot {
private:
    DWORD pid;
    HANDLE hProcess;
    uintptr_t clientBase;
    bool active = false;

public:
    Triggerbot() {
        pid = GetProcessId("cs2.exe");
        if (!pid) {
            std::cout << "[!] CS2 not found!" << std::endl;
            return;
        }

        clientBase = GetModuleBaseAddress(pid, "client.dll");
        if (!clientBase) {
            std::cout << "[!] client.dll not found!" << std::endl;
            return;
        }

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!hProcess) {
            std::cout << "[!] Failed to open process!" << std::endl;
            return;
        }

        std::cout << "[+] Triggerbot initialized!" << std::endl;
    }

    ~Triggerbot() {
        if (hProcess) CloseHandle(hProcess);
    }

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

    uintptr_t GetLocalPlayer() {
        uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
        uint64_t lpPtr;
        if (ReadMemory(hProcess, lpAddr, &lpPtr, 8)) {
            return lpPtr;
        }
        return 0;
    }

    int GetPlayerTeam(uintptr_t player) {
        if (!player) return 0;
        int team;
        if (ReadMemory(hProcess, player + PLAYER_TEAM, &team, 4)) {
            return team;
        }
        return 0;
    }

    int GetPlayerHealth(uintptr_t player) {
        if (!player) return 0;
        int health;
        if (ReadMemory(hProcess, player + PLAYER_HEALTH, &health, 4)) {
            return health;
        }
        return 0;
    }

    Vec3 GetPlayerPosition(uintptr_t player) {
        Vec3 pos = {0, 0, 0};
        if (player) {
            ReadMemory(hProcess, player + PLAYER_POSITION, &pos, sizeof(Vec3));
        }
        return pos;
    }

    Vec3 GetViewAngles() {
        Vec3 angles = {0, 0, 0};
        uintptr_t anglesAddr = clientBase + OFFSET_VIEW_ANGLES;
        ReadMemory(hProcess, anglesAddr, &angles, sizeof(Vec3));
        return angles;
    }

    // Simulate mouse click (simple trigger)
    void TriggerFire() {
        // Send left mouse button down and up
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));

        std::cout << "[+] TRIGGER!" << std::endl;
    }

    // Check if enemy is in crosshair (simplified)
    bool IsEnemyInCrosshair() {
        uintptr_t localPlayer = GetLocalPlayer();
        if (!localPlayer) return false;

        int localTeam = GetPlayerTeam(localPlayer);
        Vec3 localPos = GetPlayerPosition(localPlayer);
        Vec3 viewAngles = GetViewAngles();

        // Get entity list
        uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
        uint64_t entityList;
        if (!ReadMemory(hProcess, entityListAddr, &entityList, 8)) return false;

        // Scan entities
        for (int i = 1; i < 64; ++i) {
            uintptr_t entityAddr = entityList + (i * 0x78);  // Approximate entity size
            uint64_t entityPtr;
            if (!ReadMemory(hProcess, entityAddr, &entityPtr, 8)) continue;
            if (!entityPtr) continue;

            int entityTeam = GetPlayerTeam(entityPtr);
            int entityHealth = GetPlayerHealth(entityPtr);

            // Skip if same team or dead
            if (entityTeam == localTeam || entityTeam == 0 || entityHealth <= 0) continue;

            Vec3 enemyPos = GetPlayerPosition(entityPtr);

            // Calculate angle to enemy
            Vec3 angleToEnemy = CalculateAngles(localPos, enemyPos);

            // Check if enemy is within 2 degrees of crosshair
            float angleDiff = fabs(angleToEnemy.y - viewAngles.y);
            if (angleDiff > 180.0f) angleDiff = 360.0f - angleDiff;

            if (angleDiff < 2.0f) {  // Within 2 degrees
                return true;
            }
        }

        return false;
    }

    Vec3 CalculateAngles(Vec3 from, Vec3 to) {
        Vec3 delta = {to.x - from.x, to.y - from.y, to.z - from.z};
        float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

        Vec3 angles;
        angles.x = atan2(delta.z, hyp) * 180.0f / 3.14159f;  // Pitch
        angles.y = atan2(delta.y, delta.x) * 180.0f / 3.14159f;  // Yaw
        angles.z = 0.0f;  // Roll

        return angles;
    }

    void TriggerLoop() {
        while (true) {
            if (active && IsEnemyInCrosshair()) {
                TriggerFire();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Delay between shots
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Check frequently
        }
    }

    void Toggle() {
        active = !active;
        std::cout << "[+] Triggerbot " << (active ? "ENABLED" : "DISABLED") << std::endl;
    }

    void Run() {
        if (!hProcess) return;

        std::cout << "[+] Triggerbot ready!" << std::endl;
        std::cout << "[+] Press F6 to toggle, F12 to exit" << std::endl;

        // Start trigger loop
        std::thread triggerThread(&Triggerbot::TriggerLoop, this);
        triggerThread.detach();

        while (true) {
            if (GetAsyncKeyState(VK_F6) & 1) {  // F6 to toggle
                Toggle();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (GetAsyncKeyState(VK_F12) & 1) {  // F12 to exit
                std::cout << "[+] Exiting..." << std::endl;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

int main() {
    Triggerbot triggerbot;
    triggerbot.Run();
    return 0;
}