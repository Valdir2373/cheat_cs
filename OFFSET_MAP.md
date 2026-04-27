# CS2 Offset Discovery Map

## 🎯 Primary Findings

### ✅ Confirmed Working
- **0xBD7DAC** - ESP/Wallhack Flag (PRIMARY) ← **CURRENTLY ACTIVE!**
- **0xBD7D58** - Secondary Flag (Status: Likely related to ESP)

### 🔍 Secondary Offsets (Nearby)
- 0xBD7D4C - Older ESP offset (may work)
- 0xBD7D60 - Variant
- 0xBD7D50 - Variant
- 0xBD7D40 - Variant
- 0xBD7CB0-0xBD7D7C - Boolean flags area (cluster)

## 📊 Offset Structure Map

```
client.dll + 0xBD7DAC ──→ ESP Flag (bool: 1 = on, 0 = off)
                 ↓
         boolean flags cluster (±0x100)
                 ↓
         View Matrix @ 0x22F9F50
         Local Player @ 0x22F5028
         Entity List @ 0x24B0258
```

## 🛠️ Main Access Points

### Core Structures (client.dll)
| Offset | Size | Type | Purpose |
|--------|------|------|---------|
| 0x22F5028 | 8 | ptr | Local Player Controller |
| 0x24B0258 | 8 | ptr | Entity List |
| 0x22F9F50 | 8 | ptr | View Matrix (for camera/aimbot) |
| 0x22EC878 | 8 | ptr | Glow Manager (ESP) |
| 0x23003A8 | 12 | vec3 | View Angles |

### Player Structure Offsets (relative to player entity)
| Offset | Type | Purpose |
|--------|------|---------|
| 0x344 | int | Health |
| 0x3E3 | int | Team Number |
| 0x3288 | bool | Spotted flag |
| 0x3A8 | ptr | Active Weapon |
| 0xBC8 | vec3 | Velocity |
| 0xCC8 | vec3 | Position |
| 0x6C4 | int | Pawn Handle |

## 🎮 Features Implemented

### wall.cpp (Currently Working)
- Locates cs2.exe process
- Gets client.dll base address (handles ASLR)
- Writes to 0xBD7DAC to toggle ESP
- Simple one-shot execution

### offset_explorer.exe
- Maps all known offsets
- Tests values at each location
- Displays in categorized format

### deep_scanner.exe
- Follows pointer chains
- Analyzes ESP area (±0x100)
- Pattern scanning for common structures
- Found: 0xBD7D58 (secondary flag)

### flag_identifier.exe (TODO: compile)
- Interactive flag testing
- Shows what each offset does
- Helps identify new features

## 🔄 Next Steps for Expansion

1. **Aimbot** → Use View Angles + View Matrix + Entity positions
2. **Triggerbot** → Detect enemies in crosshair + Fire
3. **No Recoil** → Reverse view angles when shooting
4. **Radar** → Draw entity positions on minimap
5. **Bhop** → Predict jump + timing

## 💡 How to Find New Offsets

1. **Pattern Scanning**: Search for specific byte sequences
   ```
   ESP area: look for 01 00 00 00 patterns
   Health: search for 100 then 99 when damaged
   ```

2. **Cheat Engine Method**:
   - Attach to cs2.exe
   - Search for known values
   - Compare with teammates
   - Narrow down offsets

3. **Reverse Engineering**:
   - Use IDA Pro / Ghidra
   - Analyze client.dll directly
   - Find function pointers to structures

## 📝 Offset Notes

- All offsets are in **decimal** when shown in struct dumps
- Must add **client.dll base address** to all offsets (ASLR)
- Pointer chains: follow each pointer (+0 at start = dereference)
- Boolean flags: 1 byte (0x00 or 0x01)

## 🧪 Testing Offsets

```cpp
// Basic offset test structure
uintptr_t addr = clientBase + OFFSET;
uint8_t value = 0;
ReadMemory(hProcess, addr, &value, 1);
// If value is reasonable → offset is likely correct
```

---

**Last Updated**: 2026-04-27  
**Status**: ESP confirmed working, exploring aimbot offsets  
**Next Focus**: Pointer chain following for player data