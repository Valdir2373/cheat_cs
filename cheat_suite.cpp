#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>

uintptr_t OFFSET_ESP = 0xBD7DAC;
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

class CheatBase {
protected:
    DWORD procId;
    uintptr_t clientBase;
    HANDLE hProcess;
    bool espActive = false;

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

    bool ReadMemory(uintptr_t address, void* buffer, size_t size) {
        SIZE_T bytesRead;
        return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
    }

    bool WriteMemory(uintptr_t address, const void* buffer, size_t size) {
        SIZE_T bytesWritten;
        return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) && bytesWritten == size;
    }

    bool GetPlayerInfo(uintptr_t playerPtr, Player& player) {
        if (!playerPtr) return false;
        ReadMemory(playerPtr + 0x344, &player.health, sizeof(int));
        ReadMemory(playerPtr + 0x3E3, &player.team, sizeof(int));
        ReadMemory(playerPtr + 0xCC8, &player.position, sizeof(Vec3));
        ReadMemory(playerPtr + 0xBC8, &player.velocity, sizeof(Vec3));
        ReadMemory(playerPtr + 0x3288, &player.spotted, sizeof(bool));
        return true;
    }

public:
    CheatBase() {
        procId = GetProcessId("cs2.exe");
        if (!procId) {
            std::cout << "[!] CS2 not found!" << std::endl;
            return;
        }

        clientBase = GetModuleBaseAddress(procId, "client.dll");
        if (!clientBase) {
            std::cout << "[!] client.dll not found!" << std::endl;
            return;
        }

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
        if (!hProcess) {
            std::cout << "[!] Failed to open process!" << std::endl;
            return;
        }

        std::cout << "[+] Connected to CS2 (PID: " << procId << ")" << std::endl;
        std::cout << "[+] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl;
    }

    ~CheatBase() {
        if (hProcess) CloseHandle(hProcess);
    }

    void ToggleESP() {
        espActive = !espActive;
        uintptr_t espAddr = clientBase + OFFSET_ESP;
        uint8_t value = espActive ? 1 : 0;
        WriteMemory(espAddr, &value, sizeof(uint8_t));
        std::cout << "[+] ESP " << (espActive ? "ON" : "OFF") << std::endl;
    }
};

// ===== WALLHACK COM HOTKEY =====
class WallhackCheat : public CheatBase {
public:
    void Run() {
        std::cout << "=== Wallhack with Hotkey ===" << std::endl;
        std::cout << "[+] Press F5 to toggle ESP" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_F5) & 1) {
                ToggleESP();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (GetAsyncKeyState(VK_END) & 1) {
                std::cout << "[+] Exiting..." << std::endl;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// ===== AIMBOT BÁSICO =====
class AimbotCheat : public CheatBase {
private:
    float aimSmooth = 0.1f;
    float aimFOV = 10.0f;

    float GetDistance(Vec3 a, Vec3 b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

    Vec3 CalcAngles(Vec3 from, Vec3 to) {
        Vec3 angles;
        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float dz = to.z - from.z;

        angles.x = -atan2(dz, sqrt(dx*dx + dy*dy)) * 180.0f / 3.14159f;
        angles.y = atan2(dy, dx) * 180.0f / 3.14159f;
        angles.z = 0.0f;

        return angles;
    }

public:
    void Run() {
        std::cout << "=== Aimbot Cheat ===" << std::endl;
        std::cout << "[+] Hold RIGHT MOUSE BUTTON for aimbot" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            // Só ativa quando botão direito do mouse pressionado
            if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Obter local player
            uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
            uint64_t localPlayerPtr;
            if (!ReadMemory(lpAddr, &localPlayerPtr, sizeof(uint64_t))) continue;

            Player localPlayer;
            if (!GetPlayerInfo(localPlayerPtr, localPlayer)) continue;

            // Obter entity list
            uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
            uint64_t entityListPtr;
            if (!ReadMemory(entityListAddr, &entityListPtr, sizeof(uint64_t))) continue;

            // Procurar melhor alvo
            Player bestTarget;
            float bestDistance = aimFOV;
            bool foundTarget = false;

            for (int i = 1; i < 64; ++i) {
                uint64_t entityAddr = entityListPtr + (i * 0x78);
                uint64_t entityPtr;
                if (!ReadMemory(entityAddr, &entityPtr, sizeof(uint64_t))) continue;

                Player enemy;
                if (!GetPlayerInfo(entityPtr, enemy)) continue;

                if (enemy.health <= 0 || enemy.health > 100) continue;
                if (enemy.team == localPlayer.team) continue;

                float distance = GetDistance(localPlayer.position, enemy.position);
                if (distance < bestDistance) {
                    bestTarget = enemy;
                    bestDistance = distance;
                    foundTarget = true;
                }
            }

            if (foundTarget) {
                // Calcular ângulos para o alvo
                Vec3 targetAngles = CalcAngles(localPlayer.position, bestTarget.position);

                // Obter ângulos atuais
                uintptr_t viewAnglesAddr = clientBase + OFFSET_VIEW_ANGLES;
                Vec3 currentAngles;
                ReadMemory(viewAnglesAddr, &currentAngles, sizeof(Vec3));

                // Aplicar suavização
                Vec3 newAngles;
                newAngles.x = currentAngles.x + (targetAngles.x - currentAngles.x) * aimSmooth;
                newAngles.y = currentAngles.y + (targetAngles.y - currentAngles.y) * aimSmooth;
                newAngles.z = 0.0f;

                // Aplicar aimbot
                WriteMemory(viewAnglesAddr, &newAngles, sizeof(Vec3));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] Aimbot stopped" << std::endl;
    }
};

// ===== RADAR CONSOLE =====
class RadarCheat : public CheatBase {
public:
    void Run() {
        std::cout << "=== Console Radar ===" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            system("cls"); // Limpar console

            // Obter local player
            uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
            uint64_t localPlayerPtr;
            if (!ReadMemory(lpAddr, &localPlayerPtr, sizeof(uint64_t))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            Player localPlayer;
            if (!GetPlayerInfo(localPlayerPtr, localPlayer)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            std::cout << "=== CS2 RADAR ===" << std::endl;
            std::cout << "Local Player: HP=" << localPlayer.health
                      << " Team=" << localPlayer.team
                      << " Pos=(" << localPlayer.position.x << ","
                      << localPlayer.position.y << ","
                      << localPlayer.position.z << ")" << std::endl;
            std::cout << std::endl;

            // Obter entity list
            uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
            uint64_t entityListPtr;
            if (!ReadMemory(entityListAddr, &entityListPtr, sizeof(uint64_t))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            std::cout << "ENEMIES:" << std::endl;
            int enemyCount = 0;

            for (int i = 1; i < 64; ++i) {
                uint64_t entityAddr = entityListPtr + (i * 0x78);
                uint64_t entityPtr;
                if (!ReadMemory(entityAddr, &entityPtr, sizeof(uint64_t))) continue;

                Player enemy;
                if (!GetPlayerInfo(entityPtr, enemy)) continue;

                if (enemy.health <= 0 || enemy.health > 100) continue;
                if (enemy.team == localPlayer.team) continue;

                float distance = GetDistance(localPlayer.position, enemy.position);
                if (distance > 3000.0f) continue; // Só mostrar próximos

                enemyCount++;
                std::cout << "  [" << enemyCount << "] HP=" << enemy.health
                          << " Dist=" << (int)distance
                          << " Pos=(" << (int)enemy.position.x << ","
                          << (int)enemy.position.y << ","
                          << (int)enemy.position.z << ")" << std::endl;
            }

            if (enemyCount == 0) {
                std::cout << "  No enemies nearby" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::cout << "[+] Radar stopped" << std::endl;
    }

private:
    float GetDistance(Vec3 a, Vec3 b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }
};

// ===== BHOP BÁSICO =====
class BhopCheat : public CheatBase {
public:
    void Run() {
        std::cout << "=== Bunny Hop Cheat ===" << std::endl;
        std::cout << "[+] Hold SPACE to bunny hop" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            // Verificar se SPACE está pressionado
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                // Simular pulo (esta é uma implementação básica)
                // Em um bhop real, você precisaria encontrar o offset de jump
                // e manipular o input system

                // Por enquanto, apenas um placeholder
                std::cout << "[+] Jump!" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] Bhop stopped" << std::endl;
    }
};

// ===== MENU PRINCIPAL =====
int main() {
    std::cout << "=== CS2 Cheat Suite v2.0 ===" << std::endl;
    std::cout << "[+] Using discovered offsets:" << std::endl;
    std::cout << "    ESP: 0xBD7DAC" << std::endl;
    std::cout << "    Local Player: 0x22F5028" << std::endl;
    std::cout << "    Entity List: 0x24B0258" << std::endl;
    std::cout << "    View Angles: 0x23003A8" << std::endl;
    std::cout << std::endl;

    std::cout << "=== Cheat Menu ===" << std::endl;
    std::cout << "1. Wallhack (F5 Toggle)" << std::endl;
    std::cout << "2. Aimbot (Right Mouse)" << std::endl;
    std::cout << "3. Console Radar" << std::endl;
    std::cout << "4. Bunny Hop (Space)" << std::endl;
    std::cout << "Choose (1-4): ";

    int choice;
    std::cin >> choice;

    switch (choice) {
        case 1: {
            WallhackCheat wallhack;
            wallhack.Run();
            break;
        }
        case 2: {
            AimbotCheat aimbot;
            aimbot.Run();
            break;
        }
        case 3: {
            RadarCheat radar;
            radar.Run();
            break;
        }
        case 4: {
            BhopCheat bhop;
            bhop.Run();
            break;
        }
        default:
            std::cout << "[!] Invalid choice!" << std::endl;
            return 1;
    }

    std::cout << "[+] Cheat session ended. Goodbye!" << std::endl;
    return 0;
}