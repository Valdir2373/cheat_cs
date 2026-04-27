#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>

const uintptr_t OFFSET_ESP = 0xBD7DAC;
const uintptr_t OFFSET_LOCAL_PLAYER = 0x22F5028;
const uintptr_t OFFSET_ENTITY_LIST = 0x24B0258;
const uintptr_t OFFSET_VIEW_ANGLES = 0x23003A8;

// Player struct offsets (relative to player pointer)
const uintptr_t PLAYER_HEALTH = 0x344;
const uintptr_t PLAYER_TEAM = 0x3E3;
const uintptr_t PLAYER_POSITION = 0xCC8;
const uintptr_t PLAYER_VELOCITY = 0xBC8;
const uintptr_t PLAYER_SPOITED = 0x3288;

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

struct Vec3 {
    float x, y, z;
};

class CS2Cheat {
private:
    DWORD pid;
    HANDLE hProcess;
    uintptr_t clientBase;
    bool espActive = false;

public:
    CS2Cheat() {
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

        std::cout << "[+] CS2 Cheat initialized!" << std::endl;
        std::cout << "[+] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl;
    }

    ~CS2Cheat() {
        if (hProcess) CloseHandle(hProcess);
    }

    // ===== WALLHACK =====
    void ToggleESP() {
        espActive = !espActive;
        uint8_t value = espActive ? 0x01 : 0x00;
        uintptr_t espAddr = clientBase + OFFSET_ESP;

        if (WriteMemory(hProcess, espAddr, &value, 1)) {
            std::cout << "[+] ESP " << (espActive ? "ENABLED" : "DISABLED") << std::endl;
        } else {
            std::cout << "[!] Failed to toggle ESP" << std::endl;
        }
    }

    void ESPLoop() {
        while (true) {
            if (espActive) {
                uint8_t value = 0x01;
                WriteMemory(hProcess, clientBase + OFFSET_ESP, &value, 1);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // ===== PLAYER INFO =====
    uintptr_t GetLocalPlayer() {
        uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
        uint64_t lpPtr;
        if (ReadMemory(hProcess, lpAddr, &lpPtr, 8)) {
            return lpPtr;
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

    int GetPlayerTeam(uintptr_t player) {
        if (!player) return 0;
        int team;
        if (ReadMemory(hProcess, player + PLAYER_TEAM, &team, 4)) {
            return team;
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

    void PrintPlayerInfo() {
        uintptr_t localPlayer = GetLocalPlayer();
        if (!localPlayer) {
            std::cout << "[!] Local player not found" << std::endl;
            return;
        }

        int health = GetPlayerHealth(localPlayer);
        int team = GetPlayerTeam(localPlayer);
        Vec3 pos = GetPlayerPosition(localPlayer);

        std::cout << "[+] Local Player Info:" << std::endl;
        std::cout << "    Health: " << health << std::endl;
        std::cout << "    Team: " << team << std::endl;
        std::cout << "    Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }

    // ===== AIMBOT BASIC =====
    Vec3 GetViewAngles() {
        Vec3 angles = {0, 0, 0};
        uintptr_t anglesAddr = clientBase + OFFSET_VIEW_ANGLES;
        ReadMemory(hProcess, anglesAddr, &angles, sizeof(Vec3));
        return angles;
    }

    void SetViewAngles(Vec3 angles) {
        uintptr_t anglesAddr = clientBase + OFFSET_VIEW_ANGLES;
        WriteMemory(hProcess, anglesAddr, &angles, sizeof(Vec3));
    }

    // Simple aimbot - aim at closest enemy
    void SimpleAimbot() {
        uintptr_t localPlayer = GetLocalPlayer();
        if (!localPlayer) return;

        int localTeam = GetPlayerTeam(localPlayer);
        Vec3 localPos = GetPlayerPosition(localPlayer);

        // Get entity list
        uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
        uint64_t entityList;
        if (!ReadMemory(hProcess, entityListAddr, &entityList, 8)) return;

        Vec3 closestEnemy = {0, 0, 0};
        float closestDist = 999999.0f;

        // Scan entities (simplified - should check for valid entities)
        for (int i = 1; i < 64; ++i) {  // Max players
            // This is simplified - real implementation needs proper entity validation
            uintptr_t entityAddr = entityList + (i * 0x78);  // Approximate entity size
            uint64_t entityPtr;
            if (!ReadMemory(hProcess, entityAddr, &entityPtr, 8)) continue;
            if (!entityPtr) continue;

            int entityTeam = GetPlayerTeam(entityPtr);
            if (entityTeam == localTeam || entityTeam == 0) continue;

            Vec3 enemyPos = GetPlayerPosition(entityPtr);
            float dist = sqrt(pow(enemyPos.x - localPos.x, 2) +
                            pow(enemyPos.y - localPos.y, 2) +
                            pow(enemyPos.z - localPos.z, 2));

            if (dist < closestDist && dist > 1.0f) {  // Not too close (self)
                closestDist = dist;
                closestEnemy = enemyPos;
            }
        }

        if (closestDist < 999999.0f) {
            // Calculate angles to enemy (simplified)
            Vec3 angles = CalculateAngles(localPos, closestEnemy);
            SetViewAngles(angles);
            std::cout << "[+] Aimbot: Aiming at enemy" << std::endl;
        }
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

    // ===== MENU SYSTEM =====
    void ShowMenu() {
        std::cout << "\n=== CS2 Cheat Menu ===" << std::endl;
        std::cout << "1. Toggle ESP" << std::endl;
        std::cout << "2. Show Player Info" << std::endl;
        std::cout << "3. Simple Aimbot (once)" << std::endl;
        std::cout << "4. ESP Loop (continuous)" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Choice: ";
    }

    void Run() {
        if (!hProcess) return;

        // Start ESP loop in background
        std::thread espThread(&CS2Cheat::ESPLoop, this);
        espThread.detach();

        while (true) {
            ShowMenu();
            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1:
                    ToggleESP();
                    break;
                case 2:
                    PrintPlayerInfo();
                    break;
                case 3:
                    SimpleAimbot();
                    break;
                case 4:
                    espActive = !espActive;
                    std::cout << "[+] ESP Loop " << (espActive ? "ENABLED" : "DISABLED") << std::endl;
                    break;
                case 0:
                    std::cout << "[+] Exiting..." << std::endl;
                    return;
                default:
                    std::cout << "[!] Invalid choice" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

int main() {
    CS2Cheat cheat;
    cheat.Run();
    return 0;
}