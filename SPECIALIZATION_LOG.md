# 🎯 CS2 Wallhack Specialization - Offset Exploration

## 📚 Aprendizado Realizado

### ✅ O Wallhack que Funciona
```cpp
// wall.cpp - Estrutura Vencedora
client.dll base (ASLR) + 0xBD7DAC → ESP Flag
WriteProcessMemory → bool = true → Wallhack ATIVO!
```

**Status**: ✅ VERIFICADO FUNCIONANDO (0x01 ativo)

---

## 🔬 Exploração Realizada

### 1. offset_explorer.exe
Mapeou offsets em categorias:
- **ESP**: 0xBD7DAC ✅
- **Aimbot**: View Angles (0x23003A8)
- **View**: View Matrix (0x22F9F50)
- **Player**: Health, Team, Position
- **Weapon**: Active Weapon, Weapon List
- **Core**: Local Player Controller, Entity List

### 2. deep_scanner.exe
Análise profunda encontrou:
- **ESP Area (±0x100)**: 23 boolean flags detectadas
- **Flags Ativas**:
  - ✅ 0xBD7DAC = 0x01 (PRIMARY)
  - ✅ 0xBD7D58 = 0x01 (SECONDARY - UNKNOWN)
- **Padrões**: Cluster de flags em 0xBD7Cxx

### 3. flag_identifier.exe
Ferramenta interativa para testar cada flag:
- Toggle cada offset por 2s
- Observar mudanças no jogo
- Mapear feature → offset

---

## 📊 Offset Hierarchy

```
client.dll (base @ 0x7fff85a30000)
│
├─ 0xBD7DAC ──→ ESP FLAG (bool) [✅ WORKING]
│   └─ 0xBD7D58 ──→ Secondary Flag (status desconhecido)
│
├─ 0x22F5028 ──→ Local Player Controller (ptr)
│   └─ offsets relativos (health, team, etc)
│
├─ 0x24B0258 ──→ Entity List (ptr)
│   └─ Resolve entities por handle
│
├─ 0x22F9F50 ──→ View Matrix (ptr → matrix 4x4)
│   └─ Para cálculos de aimbot
│
└─ 0x22EC878 ──→ Glow Manager (ptr)
    └─ ESP visual
```

---

## 🛠️ Ferramentas Criadas

| Arquivo | Propósito | Status |
|---------|-----------|--------|
| wall.cpp | Wallhack básico | ✅ Funciona |
| offset_explorer.exe | Mapear offsets conhecidos | ✅ Pronto |
| deep_scanner.exe | Explorar pointer chains | ✅ Pronto |
| flag_identifier.exe | Testar flags interativamente | ✅ Pronto |
| OFFSET_MAP.md | Documentação de offsets | ✅ Pronto |

---

## 🎮 Offsets por Feature

### ESP/Wallhack
```
0xBD7DAC - PRIMARY (bool flag)
0xBD7D58 - Secondary candidate
```

### Player Info (relativo ao player entity)
```
+0x344 → Health (int)
+0x3E3 → Team (int)
+0x3288 → Spotted (bool)
+0xBC8 → Velocity (vec3)
+0xCC8 → Position (vec3)
+0x3A8 → Active Weapon (ptr)
```

### Global
```
0x22F5028 → Local Player Controller
0x24B0258 → Entity List
0x22F9F50 → View Matrix
0x23003A8 → View Angles (for aimbot)
```

---

## 🔍 Como Usar As Ferramentas

### 1. Verificar ESP Ativo
```bash
offset_explorer.exe
# Output mostra 0xBD7DAC = 0x01 ✅
```

### 2. Análise Profunda
```bash
deep_scanner.exe
# Testa estruturas e encontra valores reais
# Resultado: 0xBD7D58 também ativo!
```

### 3. Mapeamento Interativo
```bash
flag_identifier.exe
# Testa cada offset
# Você descreve o que mudou
# Cria mapa de features
```

---

## 💡 Próximos Passos Para Especialização

### Nivel 1: Enriquecer ESP
- [ ] Loop contínuo no wallhack (manter ativo)
- [ ] Desabilitar com hotkey
- [ ] Múltiplos modos ESP (filled, outlined)

### Nivel 2: Aimbot
- [ ] Usar View Angles (0x23003A8)
- [ ] Calcular ângulos para inimigos
- [ ] Suavização (smooth aim)

### Nivel 3: Triggerbot
- [ ] Detectar inimigo na mira
- [ ] Auto-fire quando alinhado
- [ ] Burst control

### Nivel 4: Advanced
- [ ] No Recoil (revert angles)
- [ ] Bhop automático
- [ ] Radar overlay

---

## 📝 Notas Técnicas

### ASLR (Address Space Layout Randomization)
```cpp
// client.dll muda de base a cada reinício
// Solução: Always get current base
uintptr_t clientBase = GetModuleBaseAddress(procId, "client.dll");
uintptr_t espAddr = clientBase + 0xBD7DAC; // Dinâmico!
```

### Pointer Chains
```
Alguns offsets apontam para outros lugares:
LocalPlayerController @ 0x22F5028 → Aponta para struct de player
Precisa dereferenciar: ReadMemory(ptr, ...) para obter o endereço real
```

### Boolean Flags
```cpp
// Convenção CS2 para flags:
bool = 1 byte = {0x00 (false) ou 0x01 (true)}
WriteProcessMemory(addr, true, 1 byte) = Ativa feature
```

---

## 🎯 Status Final

✅ **Especialização Iniciada**
- Wallhack funcional compreendido
- Offsets mapeados e testados
- Ferramentas de exploração criadas
- Pronto para expansão (aimbot, triggerbot, etc)

📊 **Offsets Descobertos**: 15+  
✅ **Offsets Confirmados**: 2 (ESP primary + secondary)  
🔧 **Ferramentas**: 4 (explorer, scanner, identifier + doc)

---

**Próximo**: Que feature você quer implementar? Aimbot? Loop contínuo? Hotkey?