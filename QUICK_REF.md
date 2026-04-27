# 🗂️ QUICK REFERENCE - CS2 Offsets

## 🎯 ESP (WORKING NOW!)
```
client.dll + 0xBD7DAC = bool (0x01 = ON, 0x00 = OFF)
```
**Status**: ✅ ATIVO E VERIFICADO

---

## 🔍 Secondary Discoveries
```
client.dll + 0xBD7D58 = bool (also 0x01 - purpose unknown)
Cluster: 0xBD7CB0 to 0xBD7D80 (23 flags total)
```

---

## 🎮 Player/Entity Access

### Get Local Player
```cpp
uintptr_t localPlayer = clientBase + 0x22F5028;
uint64_t lpPtr;
ReadMemory(hProcess, localPlayer, &lpPtr, 8);
// Now lpPtr = pointer to player struct
```

### Get All Entities
```cpp
uintptr_t entityList = clientBase + 0x24B0258;
uint64_t listPtr;
ReadMemory(hProcess, entityList, &listPtr, 8);
// Iterate through list
```

### Player Struct Layout (relative to player pointer)
```
+0x344   → Health (int)
+0x3E3   → Team (int)
+0x3288  → Spotted (bool)
+0xBC8   → Velocity (vec3 = 3 floats)
+0xCC8   → Position (vec3 = 3 floats)
+0x3A8   → Active Weapon (ptr)
+0x6C4   → Pawn Handle (int)
```

---

## 📐 Aimbot Ready
```
0x23003A8  → View Angles (vec3)  [can be modified for aimbot]
0x22F9F50  → View Matrix (4x4)   [for 3D math]
```

---

## 🛠️ Tools Ready To Use

### Quick Test
```bash
cd c:\Users\ildin\cs\newCheat
.\offset_explorer.exe        # See all offsets
.\deep_scanner.exe           # Deep analysis
.\flag_identifier.exe        # Test flags
```

### Original Wallhack
```bash
.\wall.exe  # One-shot toggle
```

---

## 🚀 Copy-Paste Code Template

```cpp
#include <windows.h>
#include <tlhelp32.h>

DWORD GetProcessId(const char* name) {
    DWORD procId = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe = {sizeof(pe)};
    if (Process32First(snap, &pe)) {
        do {
            if (!_stricmp(pe.szExeFile, name)) {
                procId = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    return procId;
}

uintptr_t GetModuleBase(DWORD procId, const char* mod) {
    uintptr_t base = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32, procId);
    MODULEENTRY32 me = {sizeof(me)};
    if (Module32First(snap, &me)) {
        do {
            if (!_stricmp(me.szModule, mod)) {
                base = (uintptr_t)me.modBaseAddr;
                break;
            }
        } while (Module32Next(snap, &me));
    }
    CloseHandle(snap);
    return base;
}

bool ReadMem(HANDLE h, uintptr_t a, void* b, size_t s) {
    SIZE_T read;
    return ReadProcessMemory(h, (void*)a, b, s, &read) && read == s;
}

bool WriteMem(HANDLE h, uintptr_t a, void* b, size_t s) {
    SIZE_T written;
    return WriteProcessMemory(h, (void*)a, b, s, &written) && written == s;
}

int main() {
    DWORD pid = GetProcessId("cs2.exe");
    uintptr_t client = GetModuleBase(pid, "client.dll");
    HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
    
    // Use offsets here!
    // client + 0xBD7DAC for ESP
    
    CloseHandle(proc);
    return 0;
}
```

---

## 📊 Compilation

```bash
g++ wallhack.cpp -o wallhack.exe
# Ready to go!
```

---

## ⚠️ Important Notes

1. **ASLR**: client.dll changes every boot - always get fresh base
2. **Pointer Dereference**: Some offsets point to pointers - need to read them first
3. **Bool Size**: Always 1 byte in CS2
4. **Admin Required**: Need PROCESS_ALL_ACCESS privileges
5. **VAC**: Using in online games = BAN RISK

---

## 📋 Offset Checklist

- [x] ESP Primary (0xBD7DAC) ✅
- [x] ESP Secondary (0xBD7D58) 🔍
- [ ] Aimbot angles (0x23003A8) ← TODO
- [ ] View Matrix (0x22F9F50) ← Ready
- [ ] Local Player (0x22F5028) ← Ready
- [ ] Entity List (0x24B0258) ← Ready
- [ ] Health offset (+0x344) ← Ready
- [ ] Team offset (+0x3E3) ← Ready
- [ ] Position (+0xCC8) ← Ready

---

Last Updated: 2026-04-27  
Status: Exploration Phase Complete  
Next: Feature Implementation