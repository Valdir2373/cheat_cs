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

struct OffsetInfo {
    std::string name;
    uintptr_t offset;
    std::string type;
    std::string category;
};

int main() {
    std::cout << "=== CS2 client.dll Offset Explorer ===" << std::endl;
    std::cout << "Scanning for working offsets..." << std::endl << std::endl;

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

    std::cout << "[+] client.dll base: 0x" << std::hex << clientBase << std::dec << std::endl << std::endl;

    // Offsets conhecidos para testar
    std::vector<OffsetInfo> offsets = {
        // ESP - Visibilidade
        {"ESP (X-Ray)", 0xBD7DAC, "bool", "ESP"},
        
        // Posição e Ângulos
        {"Local Player Controller", 0x22F5028, "ptr", "Core"},
        {"Entity List", 0x24B0258, "ptr", "Core"},
        {"Pawn Handle", 0x6C4, "int", "Entity"},
        
        // Mira e Aimbot
        {"View Angles", 0x23003A8, "vec3", "Aimbot"},
        {"View Matrix", 0x22F9F50, "matrix", "View"},
        
        // Status do Jogador
        {"Health Offset", 0x344, "int", "Player"},
        {"Team Num Offset", 0x3E3, "int", "Player"},
        {"Spotted Offset", 0x3288, "bool", "Player"},
        
        // Movimento
        {"Velocity", 0xBC8, "vec3", "Movement"},
        {"Position", 0xCC8, "vec3", "Movement"},
        
        // Mira
        {"Glow Manager", 0x22EC878, "ptr", "ESP"},
        
        // Armas
        {"Active Weapon", 0x3A8, "ptr", "Weapon"},
        {"Weapon List", 0x3A0, "ptr", "Weapon"},
    };

    std::cout << "[*] Testing offsets from client.dll..." << std::endl << std::endl;
    
    // Agrupa por categoria
    std::vector<std::string> categories = {"ESP", "Aimbot", "View", "Player", "Movement", "Weapon", "Core"};
    
    for (const auto& cat : categories) {
        std::cout << "=== " << cat << " ===" << std::endl;
        
        for (const auto& info : offsets) {
            if (info.category != cat) continue;
            
            uintptr_t addr = clientBase + info.offset;
            
            // Tenta ler como diferente tipos
            uint8_t byteVal;
            uint32_t intVal;
            uint64_t ptrVal;
            float floatVal;
            
            std::cout << std::left << std::setw(30) << info.name;
            std::cout << " | 0x" << std::hex << std::setw(8) << info.offset << std::dec;
            std::cout << " | Type: " << std::setw(6) << info.type;
            
            if (ReadMemory(hProcess, addr, &byteVal, 1)) {
                std::cout << " | Byte: 0x" << std::hex << (int)byteVal << std::dec;
            }
            
            if (ReadMemory(hProcess, addr, &intVal, 4)) {
                std::cout << " | Int: " << intVal;
            }
            
            if (ReadMemory(hProcess, addr, &ptrVal, 8)) {
                if (ptrVal > 0x1000000 && ptrVal < 0x10000000000000) {
                    std::cout << " | Ptr: 0x" << std::hex << ptrVal << std::dec;
                }
            }
            
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "[+] Offset exploration complete!" << std::endl;
    std::cout << "[*] Working ESP offset: 0xBD7DAC (bool)" << std::endl;
    std::cout << "[*] To find more offsets: use Cheat Engine pattern scan" << std::endl;

    CloseHandle(hProcess);
    return 0;
}