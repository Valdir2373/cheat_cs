#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

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

void PrintHex(uint8_t val) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)val << std::dec;
}

int main() {
    std::cout << "=== CS2 client.dll Deep Offset Scanner ===" << std::endl;
    std::cout << "Following pointer chains to find structures..." << std::endl << std::endl;

    DWORD procId = GetProcessId("cs2.exe");
    if (!procId) {
        std::cout << "[!] CS2 not found!" << std::endl;
        return 1;
    }

    uintptr_t clientBase = GetModuleBaseAddress(procId, "client.dll");
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    std::cout << "[+] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl << std::endl;

    // ===== ESP ANALYSIS =====
    std::cout << "=== ESP Analysis ===" << std::endl;
    uintptr_t espAddr = clientBase + 0xBD7DAC;
    uint8_t espVal;
    ReadMemory(hProcess, espAddr, &espVal, 1);
    std::cout << "[*] ESP Flag @ 0xBD7DAC: ";
    PrintHex(espVal);
    std::cout << " (Active: " << (espVal ? "YES" : "NO") << ")" << std::endl;

    // Scan nearby offsets para encontrar mais flags
    std::cout << "\n[*] Scanning ESP area (±0x100)..." << std::endl;
    std::vector<std::pair<uintptr_t, uint8_t>> boolFlags;
    for (int offset = -0x100; offset <= 0x100; offset += 4) {
        uint8_t val;
        if (ReadMemory(hProcess, espAddr + offset, &val, 1)) {
            if (val == 0x01 || val == 0x00) {
                boolFlags.push_back({espAddr + offset - clientBase, val});
            }
        }
    }

    std::cout << "[+] Found " << boolFlags.size() << " boolean flags nearby:" << std::endl;
    for (size_t i = 0; i < boolFlags.size() && i < 10; ++i) {
        std::cout << "    0x" << std::hex << std::setw(8) << std::setfill('0') << boolFlags[i].first << std::dec;
        std::cout << " -> ";
        PrintHex(boolFlags[i].second);
        std::cout << std::endl;
    }

    // ===== LOCAL PLAYER ANALYSIS =====
    std::cout << "\n=== Local Player Structure ===" << std::endl;
    uintptr_t lpControllerAddr = clientBase + 0x22F5028;
    uint64_t lpController;
    if (ReadMemory(hProcess, lpControllerAddr, &lpController, 8)) {
        std::cout << "[+] Local Player Controller @ 0x22F5028:" << std::endl;
        std::cout << "    Pointer value: 0x" << std::hex << lpController << std::dec << std::endl;

        // Testa offsets comuns em relação ao LocalPlayerController
        if (lpController && lpController > 0x1000) {
            std::cout << "[*] Testing offsets from LocalPlayerController..." << std::endl;
            
            std::vector<std::pair<std::string, uintptr_t>> playerOffsets = {
                {"Health", 0x344},
                {"Team", 0x3E3},
                {"Spotted", 0x3288},
                {"Active Weapon", 0x3A8},
                {"Velocity", 0xBC8},
                {"Position", 0xCC8},
            };

            for (const auto& off : playerOffsets) {
                uint8_t byte1;
                uint32_t int1;
                if (ReadMemory(hProcess, lpController + off.second, &byte1, 1)) {
                    ReadMemory(hProcess, lpController + off.second, &int1, 4);
                    std::cout << "    " << off.first << " @ +0x" << std::hex << off.second << ": ";
                    PrintHex(byte1);
                    std::cout << " (int: " << std::dec << int1 << ")" << std::endl;
                }
            }
        }
    }

    // ===== ENTITY LIST ANALYSIS =====
    std::cout << "\n=== Entity List ===" << std::endl;
    uintptr_t entityListAddr = clientBase + 0x24B0258;
    uint64_t entityList;
    if (ReadMemory(hProcess, entityListAddr, &entityList, 8)) {
        std::cout << "[+] Entity List @ 0x24B0258:" << std::endl;
        std::cout << "    Pointer value: 0x" << std::hex << entityList << std::dec << std::endl;
    }

    // ===== VIEW MATRIX ANALYSIS =====
    std::cout << "\n=== View Matrix (for Aimbot) ===" << std::endl;
    uintptr_t viewMatrixAddr = clientBase + 0x22F9F50;
    uint64_t viewMatrixPtr;
    if (ReadMemory(hProcess, viewMatrixAddr, &viewMatrixPtr, 8)) {
        std::cout << "[+] View Matrix @ 0x22F9F50:" << std::endl;
        std::cout << "    Pointer value: 0x" << std::hex << viewMatrixPtr << std::dec << std::endl;

        // View Matrix é 4x4 matrix de floats
        if (viewMatrixPtr && viewMatrixPtr > 0x1000) {
            std::cout << "[*] View Matrix values (first 4 entries):" << std::endl;
            for (int i = 0; i < 4; ++i) {
                float val;
                if (ReadMemory(hProcess, viewMatrixPtr + (i * 4), &val, 4)) {
                    std::cout << "    [" << i << "]: " << val << std::endl;
                }
            }
        }
    }

    // ===== PATTERN SCAN PARA MAIS OFFSETS =====
    std::cout << "\n=== Searching for Common Patterns ===" << std::endl;
    
    // Pattern: ESP flag nearby com mais bool flags
    uint8_t pattern[4] = {0x01, 0x00, 0x00, 0x00};
    std::cout << "[*] Scanning for pattern [01 00 00 00] (common bool pattern)..." << std::endl;
    
    int patternCount = 0;
    for (uintptr_t offset = 0; offset < 0x200000 && patternCount < 5; offset += 0x1000) {
        uint32_t val;
        if (ReadMemory(hProcess, clientBase + offset, &val, 4)) {
            if (val == 0x00000001) {
                std::cout << "    Found @ 0x" << std::hex << offset << std::dec << std::endl;
                patternCount++;
            }
        }
    }

    std::cout << "\n[+] Deep scan complete!" << std::endl;
    std::cout << "[*] Next: Use Cheat Engine for detailed structure analysis" << std::endl;

    CloseHandle(hProcess);
    return 0;
}