#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

const uintptr_t OFFSET_LOCAL_PLAYER = 0x22F5028;
const uintptr_t OFFSET_ENTITY_LIST = 0x24B0258;

// Player struct offsets
const uintptr_t PLAYER_HEALTH = 0x344;
const uintptr_t PLAYER_TEAM = 0x3E3;
const uintptr_t PLAYER_POSITION = 0xCC8;
const uintptr_t PLAYER_SPOITED = 0x3288;

struct Vec3 {
    float x, y, z;
};

struct PlayerInfo {
    int team;
    int health;
    Vec3 position;
    bool spotted;
};

class Radar {
private:
    DWORD pid;
    HANDLE hProcess;
    uintptr_t clientBase;
    bool active = false;

public:
    Radar() {
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

        std::cout << "[+] Radar initialized!" << std::endl;
    }

    ~Radar() {
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

    PlayerInfo GetPlayerInfo(uintptr_t player) {
        PlayerInfo info = {0, 0, {0, 0, 0}, false};

        if (!player) return info;

        ReadMemory(hProcess, player + PLAYER_TEAM, &info.team, 4);
        ReadMemory(hProcess, player + PLAYER_HEALTH, &info.health, 4);
        ReadMemory(hProcess, player + PLAYER_POSITION, &info.position, sizeof(Vec3));
        ReadMemory(hProcess, player + PLAYER_SPOITED, &info.spotted, 1);

        return info;
    }

    std::vector<PlayerInfo> GetAllPlayers() {
        std::vector<PlayerInfo> players;

        uintptr_t localPlayer = GetLocalPlayer();
        if (!localPlayer) return players;

        PlayerInfo localInfo = GetPlayerInfo(localPlayer);
        players.push_back(localInfo);  // Index 0 = local player

        // Get entity list
        uintptr_t entityListAddr = clientBase + OFFSET_ENTITY_LIST;
        uint64_t entityList;
        if (!ReadMemory(hProcess, entityListAddr, &entityList, 8)) return players;

        // Scan entities
        for (int i = 1; i < 64; ++i) {
            uintptr_t entityAddr = entityList + (i * 0x78);  // Approximate entity size
            uint64_t entityPtr;
            if (!ReadMemory(hProcess, entityAddr, &entityPtr, 8)) continue;
            if (!entityPtr) continue;

            PlayerInfo info = GetPlayerInfo(entityPtr);
            if (info.team != 0 && info.health > 0) {  // Valid player
                players.push_back(info);
            }
        }

        return players;
    }

    void DrawRadar(const std::vector<PlayerInfo>& players) {
        if (players.empty()) return;

        const int RADAR_SIZE = 20;
        const float SCALE = 50.0f;  // Units per radar unit

        Vec3 localPos = players[0].position;  // Local player position

        // Clear console
        system("cls");

        std::cout << "=== CS2 RADAR ===" << std::endl;
        std::cout << "Local Player: Team " << players[0].team << " | Health: " << players[0].health << std::endl;
        std::cout << "Enemies: " << (players.size() - 1) << std::endl << std::endl;

        // Draw radar grid
        for (int y = -RADAR_SIZE; y <= RADAR_SIZE; ++y) {
            for (int x = -RADAR_SIZE; x <= RADAR_SIZE; ++x) {
                bool hasPlayer = false;
                char symbol = '.';

                // Check if any player is at this position
                for (size_t i = 0; i < players.size(); ++i) {
                    const PlayerInfo& player = players[i];

                    int radarX = (int)((player.position.x - localPos.x) / SCALE);
                    int radarY = (int)((player.position.y - localPos.y) / SCALE);

                    if (radarX == x && radarY == y) {
                        hasPlayer = true;
                        if (i == 0) {
                            symbol = 'X';  // Local player
                        } else if (player.team == players[0].team) {
                            symbol = 'A';  // Ally
                        } else {
                            symbol = 'E';  // Enemy
                        }
                        break;
                    }
                }

                // Draw center cross
                if (x == 0 && y == 0) {
                    std::cout << '+';
                } else if (x == 0) {
                    std::cout << '|';
                } else if (y == 0) {
                    std::cout << '-';
                } else {
                    std::cout << symbol;
                }
            }
            std::cout << std::endl;
        }

        std::cout << std::endl << "Legend: X=You, A=Ally, E=Enemy, .=Empty" << std::endl;
        std::cout << "Scale: " << SCALE << " units per dot" << std::endl;
    }

    void RadarLoop() {
        while (true) {
            if (active) {
                std::vector<PlayerInfo> players = GetAllPlayers();
                DrawRadar(players);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void Toggle() {
        active = !active;
        if (active) {
            std::cout << "[+] Radar ENABLED" << std::endl;
        } else {
            std::cout << "[+] Radar DISABLED" << std::endl;
            system("cls");
        }
    }

    void Run() {
        if (!hProcess) return;

        std::cout << "[+] Radar ready!" << std::endl;
        std::cout << "[+] Press F7 to toggle, F12 to exit" << std::endl;

        // Start radar loop
        std::thread radarThread(&Radar::RadarLoop, this);
        radarThread.detach();

        while (true) {
            if (GetAsyncKeyState(VK_F7) & 1) {  // F7 to toggle
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
    Radar radar;
    radar.Run();
    return 0;
}