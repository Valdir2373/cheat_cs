#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>

uintptr_t OFFSET_ESP = 0xBD7DAC;
uintptr_t OFFSET_LOCAL_PLAYER = 0x22F5028;
uintptr_t OFFSET_ENTITY_LIST = 0x24B0258;
uintptr_t OFFSET_VIEW_ANGLES = 0x23003A8;

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

struct Player {
    int health;
    int team;
    Vec3 position;
    Vec3 velocity;
    bool spotted;
};

bool GetPlayerInfo(HANDLE hProcess, uintptr_t playerPtr, Player& player) {
    if (!playerPtr) return false;

    ReadMemory(hProcess, playerPtr + 0x344, &player.health, sizeof(int));
    ReadMemory(hProcess, playerPtr + 0x3E3, &player.team, sizeof(int));
    ReadMemory(hProcess, playerPtr + 0xCC8, &player.position, sizeof(Vec3));
    ReadMemory(hProcess, playerPtr + 0xBC8, &player.velocity, sizeof(Vec3));
    ReadMemory(hProcess, playerPtr + 0x3288, &player.spotted, sizeof(bool));

    return true;
}

void ESP_Loop(HANDLE hProcess, uintptr_t clientBase) {
    uintptr_t espAddr = clientBase + OFFSET_ESP;
    std::cout << "[+] ESP Loop started - Press Ctrl+C to stop" << std::endl;

    try {
        while (true) {
            // Mantém ESP sempre ativo
            uint8_t espOn = 1;
            WriteMemory(hProcess, espAddr, &espOn, sizeof(uint8_t));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } catch (...) {
        std::cout << "[+] ESP Loop stopped" << std::endl;
    }
}

void Aimbot_Loop(HANDLE hProcess, uintptr_t clientBase) {
    std::cout << "[+] Aimbot Loop started - Press Ctrl+C to stop" << std::endl;

    try {
        while (true) {
            // Obter local player
            uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
            uint64_t localPlayerPtr;
            if (!ReadMemory(hProcess, lpAddr, &localPlayerPtr, sizeof(uint64_t))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            if (!localPlayerPtr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            Player localPlayer;
            if (!GetPlayerInfo(hProcess, localPlayerPtr, localPlayer)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Obter entity list
            uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
            uint64_t entityListPtr;
            if (!ReadMemory(hProcess, entityListAddr, &entityListPtr, sizeof(uint64_t))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Procurar inimigos próximos
            for (int i = 1; i < 64; ++i) {
                // Calcular endereço da entidade
                uint64_t entityAddr = entityListPtr + (i * 0x78); // Ajuste baseado na estrutura

                uint64_t entityPtr;
                if (!ReadMemory(hProcess, entityAddr, &entityPtr, sizeof(uint64_t))) continue;

                if (!entityPtr) continue;

                Player enemy;
                if (!GetPlayerInfo(hProcess, entityPtr, enemy)) continue;

                // Verificar se é inimigo vivo
                if (enemy.health <= 0 || enemy.health > 100) continue;
                if (enemy.team == localPlayer.team) continue;

                // Calcular distância
                float dx = enemy.position.x - localPlayer.position.x;
                float dy = enemy.position.y - localPlayer.position.y;
                float dz = enemy.position.z - localPlayer.position.z;
                float distance = sqrt(dx*dx + dy*dy + dz*dz);

                if (distance < 2000.0f) { // 20 metros
                    // Calcular ângulos para o inimigo
                    Vec3 angles;
                    angles.x = -atan2(dz, sqrt(dx*dx + dy*dy)) * 180.0f / 3.14159f;
                    angles.y = atan2(dy, dx) * 180.0f / 3.14159f;
                    angles.z = 0.0f;

                    // Aplicar aimbot (suavizado)
                    uintptr_t viewAnglesAddr = clientBase + OFFSET_VIEW_ANGLES;
                    Vec3 currentAngles;
                    ReadMemory(hProcess, viewAnglesAddr, &currentAngles, sizeof(Vec3));

                    // Suavização (smooth aim)
                    float smooth = 0.1f;
                    Vec3 newAngles;
                    newAngles.x = currentAngles.x + (angles.x - currentAngles.x) * smooth;
                    newAngles.y = currentAngles.y + (angles.y - currentAngles.y) * smooth;
                    newAngles.z = 0.0f;

                    WriteMemory(hProcess, viewAnglesAddr, &newAngles, sizeof(Vec3));
                    break; // Só mira no primeiro inimigo encontrado
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (...) {
        std::cout << "[+] Aimbot Loop stopped" << std::endl;
    }
}

void Triggerbot_Loop(HANDLE hProcess, uintptr_t clientBase) {
    std::cout << "[+] Triggerbot Loop started - Press Ctrl+C to stop" << std::endl;

    try {
        while (true) {
            // Obter local player
            uintptr_t lpAddr = clientBase + OFFSET_LOCAL_PLAYER;
            uint64_t localPlayerPtr;
            if (!ReadMemory(hProcess, lpAddr, &localPlayerPtr, sizeof(uint64_t))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            if (!localPlayerPtr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            Player localPlayer;
            if (!GetPlayerInfo(hProcess, localPlayerPtr, localPlayer)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Verificar se tem inimigo na mira (spotted)
            // Esta é uma implementação básica - em um triggerbot real
            // você verificaria a crosshair position vs entity positions

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (...) {
        std::cout << "[+] Triggerbot Loop stopped" << std::endl;
    }
}

int main() {
    std::cout << "=== CS2 Advanced Cheat Suite ===" << std::endl;
    std::cout << "[+] Finding CS2 process..." << std::endl;

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

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!hProcess) {
        std::cout << "[!] Failed to open process!" << std::endl;
        return 1;
    }

    std::cout << "[+] Connected to CS2 (PID: " << procId << ")" << std::endl;
    std::cout << "[+] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl;

    // Menu de seleção
    std::cout << "\n=== Cheat Menu ===" << std::endl;
    std::cout << "1. ESP Loop (Wallhack Always On)" << std::endl;
    std::cout << "2. Aimbot Loop (Auto Aim)" << std::endl;
    std::cout << "3. Triggerbot Loop (Auto Fire)" << std::endl;
    std::cout << "4. All Cheats Combined" << std::endl;
    std::cout << "Choose (1-4): ";

    int choice;
    std::cin >> choice;

    std::vector<std::thread> threads;

    switch (choice) {
        case 1:
            threads.emplace_back(ESP_Loop, hProcess, clientBase);
            break;
        case 2:
            threads.emplace_back(Aimbot_Loop, hProcess, clientBase);
            break;
        case 3:
            threads.emplace_back(Triggerbot_Loop, hProcess, clientBase);
            break;
        case 4:
            threads.emplace_back(ESP_Loop, hProcess, clientBase);
            threads.emplace_back(Aimbot_Loop, hProcess, clientBase);
            threads.emplace_back(Triggerbot_Loop, hProcess, clientBase);
            break;
        default:
            std::cout << "[!] Invalid choice!" << std::endl;
            CloseHandle(hProcess);
            return 1;
    }

    std::cout << "[+] Cheats activated! Press Ctrl+C to stop all." << std::endl;

    // Aguardar threads
    for (auto& t : threads) {
        t.join();
    }

    CloseHandle(hProcess);
    std::cout << "[+] All cheats stopped. Goodbye!" << std::endl;
    return 0;
}