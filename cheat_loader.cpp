#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <functional>
#include <cfloat>

// Estruturas básicas
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

// Classe base para cheats
class CheatBase {
protected:
    DWORD procId;
    uintptr_t clientBase;
    HANDLE hProcess;
    bool active = false;

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

    virtual ~CheatBase() {
        if (hProcess) CloseHandle(hProcess);
    }

    virtual void Run() = 0;
    virtual std::string GetName() = 0;
    virtual std::string GetDescription() = 0;

    void SetActive(bool state) { active = state; }
    bool IsActive() { return active; }
};

// ===== CHEATS IMPLEMENTADOS =====

// 0. Wallhack Básico
class WallhackCheat : public CheatBase {
public:
    void Run() override {
        std::cout << "=== Wallhack (F5 Toggle) ===" << std::endl;
        std::cout << "[+] Press F5 to toggle ESP" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_F5) & 1) {
                active = !active;
                uintptr_t espAddr = clientBase + 0xBD7DAC;
                uint8_t value = active ? 1 : 0;
                WriteMemory(espAddr, &value, sizeof(uint8_t));
                std::cout << "[+] ESP " << (active ? "ON" : "OFF") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (GetAsyncKeyState(VK_END) & 1) {
                std::cout << "[+] Exiting..." << std::endl;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    std::string GetName() override { return "Wallhack"; }
    std::string GetDescription() override { return "ESP Wallhack with F5 toggle"; }
};

// 1. Aimbot Básico
class AimbotCheat : public CheatBase {
private:
    float aimSmooth = 0.1f;

public:
    void Run() override {
        std::cout << "=== Aimbot (Right Mouse) ===" << std::endl;
        std::cout << "[+] Hold RIGHT MOUSE BUTTON for aimbot" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            uintptr_t lpAddr = clientBase + 0x22F5028;
            uint64_t localPlayerPtr;
            if (!ReadMemory(lpAddr, &localPlayerPtr, sizeof(uint64_t))) continue;

            Player localPlayer;
            if (!GetPlayerInfo(localPlayerPtr, localPlayer)) continue;

            uintptr_t entityListAddr = clientBase + 0x24B0258;
            uint64_t entityListPtr;
            if (!ReadMemory(entityListAddr, &entityListPtr, sizeof(uint64_t))) continue;

            for (int i = 1; i < 64; ++i) {
                uint64_t entityAddr = entityListPtr + (i * 0x78);
                uint64_t entityPtr;
                if (!ReadMemory(entityAddr, &entityPtr, sizeof(uint64_t))) continue;

                Player enemy;
                if (!GetPlayerInfo(entityPtr, enemy)) continue;

                if (enemy.health <= 0 || enemy.health > 100) continue;
                if (enemy.team == localPlayer.team) continue;

                float dx = enemy.position.x - localPlayer.position.x;
                float dy = enemy.position.y - localPlayer.position.y;
                float dz = enemy.position.z - localPlayer.position.z;
                float distance = sqrt(dx*dx + dy*dy + dz*dz);

                if (distance < 2000.0f) {
                    Vec3 angles;
                    angles.x = -atan2(dz, sqrt(dx*dx + dy*dy)) * 180.0f / 3.14159f;
                    angles.y = atan2(dy, dx) * 180.0f / 3.14159f;
                    angles.z = 0.0f;

                    uintptr_t viewAnglesAddr = clientBase + 0x23003A8;
                    Vec3 currentAngles;
                    ReadMemory(viewAnglesAddr, &currentAngles, sizeof(Vec3));

                    Vec3 newAngles;
                    newAngles.x = currentAngles.x + (angles.x - currentAngles.x) * aimSmooth;
                    newAngles.y = currentAngles.y + (angles.y - currentAngles.y) * aimSmooth;
                    newAngles.z = 0.0f;

                    WriteMemory(viewAnglesAddr, &newAngles, sizeof(Vec3));
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] Aimbot stopped" << std::endl;
    }

    std::string GetName() override { return "Aimbot"; }
    std::string GetDescription() override { return "Basic aimbot with smoothing"; }
};

// 2. Silent Aimbot
class SilentAimbotCheat : public CheatBase {
private:
    bool IsShooting() { return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; }
    bool IsKeyPressed(int key) { return (GetAsyncKeyState(key) & 1) != 0; }

    float Distance(const Vec3& a, const Vec3& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    Vec3 CalcAngles(const Vec3& from, const Vec3& to) {
        Vec3 delta = {to.x - from.x, to.y - from.y, to.z - from.z};
        Vec3 angles;
        angles.x = -atan2f(delta.z, sqrtf(delta.x*delta.x + delta.y*delta.y)) * 180.0f / 3.14159265f;
        angles.y = atan2f(delta.y, delta.x) * 180.0f / 3.14159265f;
        angles.z = 0.0f;
        return angles;
    }

    bool FindBestTarget(const Player& localPlayer, Vec3& outAngles) {
        uintptr_t entityListAddr = clientBase + 0x24B0258;
        uint64_t entityListPtr;
        if (!ReadMemory(entityListAddr, &entityListPtr, sizeof(uint64_t))) return false;

        float bestScore = FLT_MAX;
        bool found = false;

        for (int i = 1; i < 64; ++i) {
            uint64_t entityAddr = entityListPtr + (i * 0x78);
            uint64_t entityPtr;
            if (!ReadMemory(entityAddr, &entityPtr, sizeof(uint64_t))) continue;
            if (!entityPtr) continue;

            Player enemy;
            if (!GetPlayerInfo(entityPtr, enemy)) continue;
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

public:
    void Run() override {
        std::cout << "=== Silent Aimbot ===" << std::endl;
        std::cout << "[+] Press F6 to toggle silent aimbot" << std::endl;
        std::cout << "[+] Hold LEFT MOUSE BUTTON to fire" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        bool wasShooting = false;
        uintptr_t viewAnglesAddr = clientBase + 0x23003A8;

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
                uintptr_t lpAddr = clientBase + 0x22F5028;
                uint64_t localPlayerPtr;
                if (!ReadMemory(lpAddr, &localPlayerPtr, sizeof(uint64_t))) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                Player localPlayer;
                if (!GetPlayerInfo(localPlayerPtr, localPlayer)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                Vec3 aimAngles;
                if (FindBestTarget(localPlayer, aimAngles)) {
                    Vec3 originalAngles;
                    if (!ReadMemory(viewAnglesAddr, &originalAngles, sizeof(Vec3))) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }

                    WriteMemory(viewAnglesAddr, &aimAngles, sizeof(Vec3));
                    std::this_thread::sleep_for(std::chrono::milliseconds(8));
                    WriteMemory(viewAnglesAddr, &originalAngles, sizeof(Vec3));
                }
            }

            wasShooting = shooting;
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }

        std::cout << "[+] Silent Aimbot stopped" << std::endl;
    }

    std::string GetName() override { return "Silent Aimbot"; }
    std::string GetDescription() override { return "Silent aimbot without visible aiming"; }
};

// 3. Radar Console
class RadarCheat : public CheatBase {
private:
    float GetDistance(Vec3 a, Vec3 b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

public:
    void Run() override {
        std::cout << "=== Console Radar ===" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            system("cls");

            uintptr_t lpAddr = clientBase + 0x22F5028;
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

            uintptr_t entityListAddr = clientBase + 0x24B0258;
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
                if (distance > 3000.0f) continue;

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

    std::string GetName() override { return "Console Radar"; }
    std::string GetDescription() override { return "Shows enemy positions in console"; }
};

// 4. Bunny Hop
class BhopCheat : public CheatBase {
public:
    void Run() override {
        std::cout << "=== Bunny Hop ===" << std::endl;
        std::cout << "[+] Hold SPACE to bunny hop" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                std::cout << "[+] Jump!" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] Bhop stopped" << std::endl;
    }

    std::string GetName() override { return "Bunny Hop"; }
    std::string GetDescription() override { return "Automatic jumping"; }
};

// 5. Triggerbot
class TriggerbotCheat : public CheatBase {
public:
    void Run() override {
        std::cout << "=== Triggerbot ===" << std::endl;
        std::cout << "[+] Hold LEFT ALT to activate triggerbot" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
                // Simular clique do mouse
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));

                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));

                std::cout << "[+] TRIGGER!" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] Triggerbot stopped" << std::endl;
    }

    std::string GetName() override { return "Triggerbot"; }
    std::string GetDescription() override { return "Auto-fire when enemy in crosshair"; }
};

// ===== SISTEMA PRINCIPAL =====
class CheatLoader {
private:
    std::map<int, std::function<CheatBase*()>> cheatRegistry;

    void RegisterCheats() {
        cheatRegistry[0] = []() { return new WallhackCheat(); };
        cheatRegistry[1] = []() { return new AimbotCheat(); };
        cheatRegistry[2] = []() { return new SilentAimbotCheat(); };
        cheatRegistry[3] = []() { return new RadarCheat(); };
        cheatRegistry[4] = []() { return new BhopCheat(); };
        cheatRegistry[5] = []() { return new TriggerbotCheat(); };
        // Espaço para mais cheats: 6, 7, 8, 9, 10, 11...
    }

public:
    CheatLoader() {
        RegisterCheats();
    }

    void ShowMenu() {
        std::cout << "=== CS2 Cheat Loader v3.0 ===" << std::endl;
        std::cout << "[+] Using discovered offsets from client.dll" << std::endl;
        std::cout << std::endl;

        std::cout << "Available Cheats:" << std::endl;
        for (const auto& pair : cheatRegistry) {
            // Criar instância temporária para obter nome e descrição
            auto cheat = pair.second();
            std::cout << "  " << pair.first << ". " << cheat->GetName()
                      << " - " << cheat->GetDescription() << std::endl;
            delete cheat;
        }
        std::cout << std::endl;
    }

    void RunCheat(int index) {
        auto it = cheatRegistry.find(index);
        if (it == cheatRegistry.end()) {
            std::cout << "[!] Cheat index " << index << " not found!" << std::endl;
            return;
        }

        std::cout << "[+] Loading cheat index " << index << "..." << std::endl;
        CheatBase* cheat = it->second();
        cheat->Run();
        delete cheat;
    }

    bool HasCheat(int index) {
        return cheatRegistry.find(index) != cheatRegistry.end();
    }
};

int main(int argc, char* argv[]) {
    CheatLoader loader;

    if (argc > 1) {
        // Modo linha de comando: ./cheat_loader.exe 2
        int cheatIndex = atoi(argv[1]);
        if (loader.HasCheat(cheatIndex)) {
            loader.RunCheat(cheatIndex);
        } else {
            std::cout << "[!] Invalid cheat index: " << cheatIndex << std::endl;
            return 1;
        }
    } else {
        // Modo interativo
        loader.ShowMenu();
        std::cout << "Choose cheat (0-5): ";

        int choice;
        std::cin >> choice;

        if (loader.HasCheat(choice)) {
            loader.RunCheat(choice);
        } else {
            std::cout << "[!] Invalid choice!" << std::endl;
            return 1;
        }
    }

    return 0;
}