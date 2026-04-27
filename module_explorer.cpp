#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

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

int main() {
    std::cout << "=== CS2 Module Explorer ===" << std::endl;
    std::cout << "[+] This tool lists all modules loaded by cs2.exe" << std::endl;
    std::cout << "[+] Use this to find engine.dll, panorama.dll, inputsystem.dll, etc." << std::endl;

    DWORD procId = GetProcessId("cs2.exe");
    if (!procId) {
        std::cout << "[!] CS2 not found!" << std::endl;
        return 1;
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap == INVALID_HANDLE_VALUE) {
        std::cout << "[!] Failed to create module snapshot" << std::endl;
        return 1;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);

    std::cout << "\nLoaded modules for cs2.exe:" << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;
    std::cout << "Name                          Base Address       Size" << std::endl;
    std::cout << "-------------------------------------------------------------" << std::endl;

    if (Module32First(hSnap, &moduleEntry)) {
        do {
            std::wcout << moduleEntry.szModule << L" "
                       << std::hex << std::showbase << (uintptr_t)moduleEntry.modBaseAddr << std::dec
                       << " " << moduleEntry.modBaseSize << std::endl;
        } while (Module32Next(hSnap, &moduleEntry));
    }

    CloseHandle(hSnap);
    std::cout << "-------------------------------------------------------------" << std::endl;
    std::cout << "[+] Module list complete" << std::endl;
    return 0;
}
