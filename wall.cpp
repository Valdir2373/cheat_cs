#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

uintptr_t OFFSET_XRAY = 0xBD7DAC;

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
    DWORD procId = GetProcessId("cs2.exe");
    if (!procId) {
        std::cout << "Processo nao encontrado!" << std::endl;
        return 1;
    }

    uintptr_t clientBase = GetModuleBaseAddress(procId, "client.dll");
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    uintptr_t alvo = clientBase + OFFSET_XRAY;
    bool estado = true; // Liga o X-Ray

    WriteProcessMemory(hProcess, (LPVOID)alvo, &estado, sizeof(estado), NULL);

    std::cout << "Byte alterado com sucesso!" << std::endl;
    CloseHandle(hProcess);
    return 0;
}