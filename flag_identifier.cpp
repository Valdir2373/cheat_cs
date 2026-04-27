#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

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

bool ReadMemory(HANDLE hProcess, uintptr_t addr, void* buffer, size_t size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, (LPCVOID)addr, buffer, size, &bytesRead) && bytesRead == size;
}

bool WriteMemory(HANDLE hProcess, uintptr_t addr, const void* buffer, size_t size) {
    SIZE_T bytesWritten;
    return WriteProcessMemory(hProcess, (LPVOID)addr, buffer, size, &bytesWritten) && bytesWritten == size;
}

struct OffsetFeature {
    uintptr_t offset;
    std::string name;
    std::string description;
};

int main() {
    std::cout << "=== CS2 Flag Identifier ===" << std::endl;
    std::cout << "Testing ESP flags to identify features..." << std::endl << std::endl;

    DWORD procId = GetProcessId("cs2.exe");
    if (!procId) {
        std::cout << "[!] CS2 not found!" << std::endl;
        return 1;
    }

    uintptr_t clientBase = GetModuleBaseAddress(procId, "client.dll");
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    std::cout << "[+] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl << std::endl;

    // Flags encontradas que estão como 0x01 (ativas)
    std::vector<OffsetFeature> flagsToTest = {
        {0xBD7DAC, "ESP/Wallhack", "Primary wallhack flag (VERIFIED WORKING)"},
        {0xBD7D58, "Flag2", "Unknown - second active flag"},
        {0xBD7D4C, "Flag3", "Old ESP offset (possibly outdated)"},
        {0xBD7D60, "Flag4", "Nearby flag - possibly related"},
        {0xBD7D50, "Flag5", "Nearby flag variant"},
    };

    std::cout << "[*] Offset Mapping Interface" << std::endl;
    std::cout << "[*] Each flag will be toggled for 2 seconds" << std::endl;
    std::cout << "[*] Watch the game and describe what changes!" << std::endl << std::endl;

    for (size_t i = 0; i < flagsToTest.size(); ++i) {
        const auto& flag = flagsToTest[i];
        uintptr_t addr = clientBase + flag.offset;

        std::cout << "[" << (i+1) << "/" << flagsToTest.size() << "] Testing: " << flag.name << std::endl;
        std::cout << "     Offset: 0x" << std::hex << flag.offset << std::dec << std::endl;
        std::cout << "     Desc: " << flag.description << std::endl;

        uint8_t original;
        ReadMemory(hProcess, addr, &original, 1);
        std::cout << "     Original value: 0x" << std::hex << (int)original << std::dec << std::endl;

        // Ativa a flag
        uint8_t testVal = 0x01;
        WriteMemory(hProcess, addr, &testVal, 1);
        std::cout << "     [ACTIVE] Watching for 2 seconds..." << std::endl;
        std::cout << "     What changed in the game? (describe after timer)" << std::endl;
        std::cout << "     ...";

        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Restaura
        WriteMemory(hProcess, addr, &original, 1);
        std::cout << std::endl << "     [RESTORED]" << std::endl;
        std::cout << "     Enter what you observed (or press Enter): ";

        std::string observation;
        std::getline(std::cin, observation);

        if (!observation.empty()) {
            std::cout << "     [NOTED] " << observation << std::endl;
        }

        std::cout << std::endl;
    }

    std::cout << "[+] Flag mapping complete!" << std::endl;
    std::cout << "[*] Now you know which offsets control which features" << std::endl;

    CloseHandle(hProcess);
    return 0;
}