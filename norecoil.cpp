#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>

const uintptr_t OFFSET_VIEW_ANGLES = 0x23003A8;
const uintptr_t OFFSET_VIEW_MATRIX = 0x22F9F50;

struct Vec3 {
    float x, y, z;
};

class NoRecoil {
private:
    DWORD pid;
    HANDLE hProcess;
    uintptr_t clientBase;
    bool active = false;
    Vec3 lastAngles = {0, 0, 0};

public:
    NoRecoil() {
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

        std::cout << "[+] No-Recoil initialized!" << std::endl;
    }

    ~NoRecoil() {
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

    bool WriteMemory(HANDLE hProcess, uintptr_t address, const void* buffer, size_t size) {
        SIZE_T bytesWritten;
        return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) && bytesWritten == size;
    }

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

    // Check if player is shooting (mouse button down)
    bool IsShooting() {
        return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    }

    // Simple no-recoil: reduce vertical recoil
    void ApplyNoRecoil() {
        if (!active) return;

        Vec3 currentAngles = GetViewAngles();

        // If shooting, reduce the vertical angle change (pitch)
        if (IsShooting()) {
            // Calculate recoil reduction (this is simplified)
            // In real CS2, you'd need to track weapon-specific recoil patterns
            float recoilReduction = 0.3f;  // Reduce recoil by 30%

            // Store the angle before shooting started
            static Vec3 baseAngles = currentAngles;
            static bool wasShooting = false;

            if (!wasShooting) {
                // Just started shooting, store base angles
                baseAngles = currentAngles;
                wasShooting = true;
            } else {
                // Apply recoil reduction
                Vec3 correctedAngles = currentAngles;
                correctedAngles.x = baseAngles.x + (currentAngles.x - baseAngles.x) * recoilReduction;

                SetViewAngles(correctedAngles);
            }
        } else {
            wasShooting = false;
        }
    }

    void NoRecoilLoop() {
        while (true) {
            ApplyNoRecoil();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Very fast for smooth correction
        }
    }

    void Toggle() {
        active = !active;
        std::cout << "[+] No-Recoil " << (active ? "ENABLED" : "DISABLED") << std::endl;
    }

    void Run() {
        if (!hProcess) return;

        std::cout << "[+] No-Recoil ready!" << std::endl;
        std::cout << "[+] Press F8 to toggle, F12 to exit" << std::endl;
        std::cout << "[+] Hold left mouse button to shoot and see effect" << std::endl;

        // Start no-recoil loop
        std::thread noRecoilThread(&NoRecoil::NoRecoilLoop, this);
        noRecoilThread.detach();

        while (true) {
            if (GetAsyncKeyState(VK_F8) & 1) {  // F8 to toggle
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
    NoRecoil noRecoil;
    noRecoil.Run();
    return 0;
}