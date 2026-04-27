#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

uintptr_t OFFSET_VIEW_ANGLES = 0x23003A8;
uintptr_t OFFSET_LOCAL_PLAYER = 0x22F5028;

struct Vec3 {
    float x, y, z;

    Vec3 operator+(const Vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vec3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }
};

class NoRecoilCheat {
private:
    DWORD procId;
    uintptr_t clientBase;
    HANDLE hProcess;
    bool noRecoilActive = false;

    Vec3 lastAngles = {0, 0, 0};
    Vec3 recoilCompensation = {0, 0, 0};
    std::vector<Vec3> angleHistory;
    const int historySize = 10;

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

    // Detectar se está atirando (implementação básica)
    bool IsShooting() {
        // Verificar se botão esquerdo do mouse está pressionado
        return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    }

    // Calcular compensação de recoil baseada no histórico
    Vec3 CalculateRecoilCompensation() {
        if (angleHistory.size() < 2) return {0, 0, 0};

        // Calcular diferença média entre ângulos consecutivos
        Vec3 totalDiff = {0, 0, 0};
        int count = 0;

        for (size_t i = 1; i < angleHistory.size(); ++i) {
            Vec3 diff = angleHistory[i] - angleHistory[i-1];
            totalDiff = totalDiff + diff;
            count++;
        }

        if (count == 0) return {0, 0, 0};

        // Média das diferenças (padrão de recoil)
        Vec3 avgRecoil = totalDiff * (1.0f / count);

        // Aplicar compensação oposta
        return avgRecoil * -1.0f;
    }

public:
    NoRecoilCheat() {
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

        std::cout << "[+] No-Recoil connected to CS2" << std::endl;
    }

    ~NoRecoilCheat() {
        if (hProcess) CloseHandle(hProcess);
    }

    void Run() {
        std::cout << "=== No-Recoil Cheat ===" << std::endl;
        std::cout << "[+] Press F7 to toggle no-recoil" << std::endl;
        std::cout << "[+] Hold LEFT MOUSE BUTTON while shooting" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        uintptr_t viewAnglesAddr = clientBase + OFFSET_VIEW_ANGLES;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            // Toggle no-recoil
            if (GetAsyncKeyState(VK_F7) & 1) {
                noRecoilActive = !noRecoilActive;
                std::cout << "[+] No-Recoil " << (noRecoilActive ? "ON" : "OFF") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (!noRecoilActive) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Ler ângulos atuais
            Vec3 currentAngles;
            if (!ReadMemory(viewAnglesAddr, &currentAngles, sizeof(Vec3))) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Adicionar ao histórico
            angleHistory.push_back(currentAngles);
            if (angleHistory.size() > historySize) {
                angleHistory.erase(angleHistory.begin());
            }

            // Se estiver atirando, aplicar compensação
            if (IsShooting()) {
                Vec3 compensation = CalculateRecoilCompensation();

                if (compensation.x != 0 || compensation.y != 0) {
                    Vec3 correctedAngles = currentAngles + compensation;

                    // Aplicar correção gradual
                    Vec3 finalAngles;
                    finalAngles.x = currentAngles.x + (correctedAngles.x - currentAngles.x) * 0.3f;
                    finalAngles.y = currentAngles.y + (correctedAngles.y - currentAngles.y) * 0.3f;
                    finalAngles.z = 0.0f;

                    WriteMemory(viewAnglesAddr, &finalAngles, sizeof(Vec3));
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "[+] No-Recoil stopped" << std::endl;
    }
};

// ===== NO-RECOIL MELHORADO =====
class AdvancedNoRecoil : public NoRecoilCheat {
private:
    // Padrões de recoil por arma (aproximados)
    struct WeaponRecoil {
        std::string name;
        Vec3 pattern[30]; // Padrão de recoil por tiro
        int patternLength;
    };

    WeaponRecoil ak47 = {
        "AK-47",
        {
            {0.5f, 0.2f, 0}, {1.0f, 0.3f, 0}, {1.5f, 0.4f, 0}, {2.0f, 0.5f, 0},
            {2.5f, 0.6f, 0}, {3.0f, 0.7f, 0}, {3.5f, 0.8f, 0}, {4.0f, 0.9f, 0},
            {4.5f, 1.0f, 0}, {5.0f, 1.1f, 0}, {5.5f, 1.2f, 0}, {6.0f, 1.3f, 0},
            {6.5f, 1.4f, 0}, {7.0f, 1.5f, 0}, {7.5f, 1.6f, 0}, {8.0f, 1.7f, 0},
            {8.5f, 1.8f, 0}, {9.0f, 1.9f, 0}, {9.5f, 2.0f, 0}, {10.0f, 2.1f, 0}
        },
        20
    };

    WeaponRecoil m4a4 = {
        "M4A4",
        {
            {0.3f, 0.1f, 0}, {0.6f, 0.2f, 0}, {0.9f, 0.3f, 0}, {1.2f, 0.4f, 0},
            {1.5f, 0.5f, 0}, {1.8f, 0.6f, 0}, {2.1f, 0.7f, 0}, {2.4f, 0.8f, 0},
            {2.7f, 0.9f, 0}, {3.0f, 1.0f, 0}, {3.3f, 1.1f, 0}, {3.6f, 1.2f, 0},
            {3.9f, 1.3f, 0}, {4.2f, 1.4f, 0}, {4.5f, 1.5f, 0}, {4.8f, 1.6f, 0}
        },
        16
    };

    int shotCount = 0;
    WeaponRecoil* currentWeapon = &ak47;

    Vec3 GetRecoilForShot(int shot) {
        if (shot >= currentWeapon->patternLength) {
            return currentWeapon->pattern[currentWeapon->patternLength - 1];
        }
        return currentWeapon->pattern[shot];
    }

public:
    void RunAdvanced() {
        std::cout << "=== Advanced No-Recoil ===" << std::endl;
        std::cout << "[+] Press F7 to toggle" << std::endl;
        std::cout << "[+] Press F8 to switch weapon (AK47/M4A4)" << std::endl;
        std::cout << "[+] Press F9 to reset shot counter" << std::endl;
        std::cout << "[+] Press END to exit" << std::endl;

        uintptr_t viewAnglesAddr = clientBase + OFFSET_VIEW_ANGLES;

        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;

            // Toggle
            if (GetAsyncKeyState(VK_F7) & 1) {
                noRecoilActive = !noRecoilActive;
                std::cout << "[+] No-Recoil " << (noRecoilActive ? "ON" : "OFF") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            // Switch weapon
            if (GetAsyncKeyState(VK_F8) & 1) {
                currentWeapon = (currentWeapon == &ak47) ? &m4a4 : &ak47;
                shotCount = 0;
                std::cout << "[+] Switched to " << currentWeapon->name << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            // Reset counter
            if (GetAsyncKeyState(VK_F9) & 1) {
                shotCount = 0;
                std::cout << "[+] Shot counter reset" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (!noRecoilActive) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Detectar início de tiro
            static bool wasShooting = false;
            bool isShooting = IsShooting();

            if (isShooting && !wasShooting) {
                // Novo tiro começou
                Vec3 recoil = GetRecoilForShot(shotCount);
                shotCount++;

                // Aplicar compensação
                Vec3 currentAngles;
                if (ReadMemory(viewAnglesAddr, &currentAngles, sizeof(Vec3))) {
                    Vec3 correctedAngles = currentAngles - recoil;
                    WriteMemory(viewAnglesAddr, &correctedAngles, sizeof(Vec3));
                }

                std::cout << "[+] Shot " << shotCount << " - Recoil: (" << recoil.x << ", " << recoil.y << ")" << std::endl;
            }

            wasShooting = isShooting;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "[+] Advanced No-Recoil stopped" << std::endl;
    }
};

int main() {
    std::cout << "=== CS2 No-Recoil Suite ===" << std::endl;
    std::cout << "[+] Choose version:" << std::endl;
    std::cout << "1. Basic No-Recoil (Adaptive)" << std::endl;
    std::cout << "2. Advanced No-Recoil (Weapon-specific)" << std::endl;
    std::cout << "Choose (1-2): ";

    int choice;
    std::cin >> choice;

    if (choice == 1) {
        NoRecoilCheat norecoil;
        norecoil.Run();
    } else if (choice == 2) {
        AdvancedNoRecoil advNorecoil;
        advNorecoil.RunAdvanced();
    } else {
        std::cout << "[!] Invalid choice!" << std::endl;
    }

    return 0;
}