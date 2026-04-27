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

    Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    float Length() const {
        return sqrt(x*x + y*y + z*z);
    }

    Vec3 Normalized() const {
        float len = Length();
        if (len == 0) return {0, 0, 0};
        return {x/len, y/len, z/len};
    }
};

struct Player {
    int health;
    int team;
    Vec3 position;
    Vec3 velocity;
    bool spotted;
};

class TriggerbotCheat {
private:
    DWORD procId;
    uintptr_t clientBase;
    HANDLE hProcess;
    bool triggerActive = false;
    float triggerDelay = 50.0f; // ms
    float lastShot = 0.0f;

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

    // Simular clique do mouse (esta é uma implementação básica)
    void SimulateMouseClick() {
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    // Verificar se há inimigo na mira (implementação básica)
    bool IsEnemyInCrosshair() {
        // Obter local player
        uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
        uint64_t localPlayerPtr;
        if (!ReadMemory(lpAddr, &localPlayerPtr, sizeof(uint64_t))) return false;

        Player localPlayer;
        if (!GetPlayerInfo(localPlayerPtr, localPlayer)) return false;

        // Obter view angles
        uintptr_t viewAnglesAddr = clientBase + OFFSET_VIEW_ANGLES;
        Vec3 viewAngles;
        if (!ReadMemory(viewAnglesAddr, &viewAngles, sizeof(Vec3))) return false;

        // Obter entity list
        uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
        uint64_t entityListPtr;
        if (!ReadMemory(entityListAddr, &entityListPtr, sizeof(uint64_t))) return false;

        // Procurar inimigos
        for (int i = 1; i < 64; ++i) {
            uint64_t entityAddr = entityListPtr + (i * 0x78);
            uint64_t entityPtr;
            if (!ReadMemory(entityAddr, &entityPtr, sizeof(uint64_t))) continue;

            Player enemy;
            if (!GetPlayerInfo(entityPtr, enemy)) continue;

            if (enemy.health <= 0 || enemy.health > 100) continue;
            if (enemy.team == localPlayer.team) continue;

            // Calcular ângulos para o inimigo
            Vec3 delta = enemy.position - localPlayer.position;
            Vec3 angles;
            angles.x = -atan2(delta.z, sqrt(delta.x*delta.x + delta.y*delta.y)) * 180.0f / 3.14159f;
            angles.y = atan2(delta.y, delta.x) * 180.0f / 3.14159f;
            angles.z = 0.0f;

            // Verificar se está na mira (FOV pequeno)
            float fovX = fabs(angles.x - viewAngles.x);
            float fovY = fabs(angles.y - viewAngles.y);

            if (fovX < 2.0f && fovY < 2.0f) { // 2 graus de tolerância
                return true;
            }
        }

        return false;
    }

public:
    TriggerbotCheat() {
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

        std::cout << "[+] Triggerbot connected to CS2" << std::endl;
    }

    ~TriggerbotCheat() {
        if (hProcess) CloseHandle(hProcess);
    }

    void Run() {
        std::cout << "=== Advanced Triggerbot ===" << std::endl;
        std::cout << "[+] Hold LEFT ALT to activate triggerbot" << std::endl;
        std::cout << "[+] Press F6 to toggle auto-trigger" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            // Toggle auto-trigger
            if (GetAsyncKeyState(VK_F6) & 1) {
                triggerActive = !triggerActive;
                std::cout << "[+] Auto-trigger " << (triggerActive ? "ON" : "OFF") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            // Verificar se deve atirar
            bool shouldShoot = false;

            if (triggerActive) {
                // Auto-trigger sempre ativo
                shouldShoot = IsEnemyInCrosshair();
            } else if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
                // Só quando ALT esquerdo pressionado
                shouldShoot = IsEnemyInCrosshair();
            }

            if (shouldShoot) {
                // Verificar delay entre tiros
                float currentTime = GetTickCount() / 1000.0f;
                if (currentTime - lastShot > triggerDelay / 1000.0f) {
                    std::cout << "[+] TRIGGER!" << std::endl;
                    SimulateMouseClick();
                    lastShot = currentTime;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] Triggerbot stopped" << std::endl;
    }
};

int main() {
    std::cout << "=== CS2 Triggerbot v2.0 ===" << std::endl;
    std::cout << "[+] Using offsets: LocalPlayer=0x22F5028, EntityList=0x24B0258, ViewAngles=0x23003A8" << std::endl;

    TriggerbotCheat triggerbot;
    triggerbot.Run();

    return 0;
}