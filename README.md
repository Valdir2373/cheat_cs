# CS2 External Cheat Suite v2.0

Uma coleção completa de cheats externos para Counter-Strike 2, desenvolvidos usando engenharia reversa e exploração de offsets da client.dll.

## 🚀 Funcionalidades

### Cheats Disponíveis
- **Wallhack (ESP)** - Visualiza inimigos através das paredes
- **Aimbot** - Mira automaticamente nos inimigos
- **Triggerbot** - Atira automaticamente quando inimigo está na mira
- **Radar** - Mostra posições dos inimigos no console
- **Bunny Hop** - Pulo automático (básico)
- **No-Recoil** - Remove o recuo das armas

### Ferramentas de Exploração
- **Offset Explorer** - Mostra todos os offsets descobertos
- **Deep Scanner** - Exploração profunda da área ESP
- **Flag Identifier** - Teste interativo de flags booleanas

## 📋 Offsets Descobertos

### Principais Offsets (client.dll)
```
ESP Toggle:           0xBD7DAC  (uint8_t)
ESP Secondary:        0xBD7D58  (uint8_t)
Local Player:         0x22F5028 (uint64_t*)
Entity List:          0x24B0258 (uint64_t*)
View Angles:          0x23003A8 (Vec3)
```

### Offsets de Player (+base)
```
Health:               +0x344    (int32_t)
Team:                 +0x3E3    (int32_t)
Position:             +0xCC8    (Vec3)
Velocity:             +0xBC8    (Vec3)
Spotted:              +0x3288   (bool)
```

### Outros Offsets Encontrados
```
0xBD7D5C: Boolean flag (possivelmente glow)
0xBD7D60: Boolean flag (possivelmente chams)
0xBD7D64: Boolean flag (radar?)
0xBD7D68: Boolean flag (crosshair?)
0xBD7D6C: Boolean flag (thirdperson?)
0xBD7D70: Boolean flag (noflash?)
...
```

## 🛠️ Como Compilar

### Opção 1: Compilador Automático
```batch
# Execute o script de compilação
compile_all.bat
```

### Opção 2: Compilação Manual
```batch
# Para cada arquivo .cpp
cl /EHsc arquivo.cpp /link user32.lib
```

### Requisitos
- Visual Studio Build Tools (cl.exe)
- Windows SDK
- Windows 10/11

## 🎮 Como Usar

### 1. Preparação
- Certifique-se que CS2 está rodando
- Execute o cheat como administrador
- O cheat irá conectar automaticamente ao processo cs2.exe

### 2. Wallhack (ESP)
```cpp
// Executar wall.exe ou escolher opção 1 no cheat_suite.exe
// Pressione F5 para ligar/desligar ESP
```

### 3. Aimbot
```cpp
// Executar advanced_cheat.exe ou escolher opção 2 no cheat_suite.exe
// Segure botão direito do mouse para ativar aimbot
```

### 4. Triggerbot
```cpp
// Executar triggerbot_advanced.exe
// Pressione F6 para toggle auto-trigger
// OU segure ALT esquerdo para trigger manual
```

### 5. No-Recoil
```cpp
// Executar norecoil_cheat.exe
// Escolha versão básica (1) ou avançada (2)
// Pressione F7 para ligar/desligar
```

### 6. Radar
```cpp
// Executar advanced_cheat.exe ou escolher opção 3 no cheat_suite.exe
// Veja posições dos inimigos no console
```

## 🔧 Arquitetura dos Cheats

### Classe Base (CheatBase)
```cpp
class CheatBase {
protected:
    DWORD procId;
    uintptr_t clientBase;
    HANDLE hProcess;

    // Métodos de acesso à memória
    bool ReadMemory(uintptr_t address, void* buffer, size_t size);
    bool WriteMemory(uintptr_t address, const void* buffer, size_t size);

    // Métodos de processo
    DWORD GetProcessId(const char* processName);
    uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName);
};
```

### Estruturas de Dados
```cpp
struct Vec3 {
    float x, y, z;
};

struct Player {
    int health;
    int team;
    Vec3 position;
    Vec3 velocity;
    bool spotted;
};
```

## ⚠️ Avisos Importantes

### Segurança
- **Uso próprio**: Estes cheats são para fins educacionais apenas
- **Detecção**: Cheats externos são facilmente detectáveis
- **Ban**: Uso em servidores oficiais resultará em ban permanente
- **Responsabilidade**: O autor não se responsabiliza pelo uso indevido

### Limitações
- Offsets podem mudar com updates do jogo
- Funciona apenas em Windows
- Requer privilégios de administrador
- Não funciona em modo fullscreen exclusivo (use windowed)

## 🔍 Metodologia de Desenvolvimento

### 1. Exploração Inicial
- Encontrar base da client.dll
- Localizar ponteiros para Local Player e Entity List

### 2. Mapeamento de Offsets
- Usar pattern scanning para encontrar flags booleanas
- Explorar estruturas de player através de offsets relativos

### 3. Implementação de Features
- ESP: Toggle de flag booleana
- Aimbot: Cálculo de ângulos e manipulação de view angles
- Triggerbot: Detecção de inimigos na mira
- No-Recoil: Compensação de recoil patterns

### 4. Testes e Validação
- Teste individual de cada offset
- Validação em diferentes cenários
- Debugging de crashes e comportamentos inesperados

## 📊 Estatísticas do Projeto

- **Offsets descobertos**: 15+ flags booleanas
- **Cheats implementados**: 6 funcionalidades principais
- **Linhas de código**: ~2000+ linhas
- **Arquivos criados**: 9 arquivos .cpp + 1 .bat
- **Tempo de desenvolvimento**: ~2 horas de exploração + implementação

## 🎯 Próximos Passos

### Melhorias Planejadas
- [ ] Implementar glow/chams
- [ ] Adicionar no-flash
- [ ] Melhorar aimbot (previsão de movimento)
- [ ] Implementar ESP 2D/3D
- [ ] Adicionar configuração via arquivo
- [ ] Criar interface gráfica (ImGui)

### Pesquisa Adicional
- [ ] Explorar mais offsets na client.dll
- [ ] Encontrar offsets de armas e inventário
- [ ] Investigar sistema de recoil real
- [ ] Mapear offsets de granadas e utilitários

## 📝 Notas Técnicas

### ASLR Handling
O código encontra dinamicamente a base da client.dll para lidar com Address Space Layout Randomization.

### Memory Reading/Writing
Usa Windows API (ReadProcessMemory/WriteProcessMemory) para acesso seguro à memória do processo.

### Threading
Implementa loops contínuos em threads separadas para features que precisam rodar constantemente.

### Input Handling
Usa GetAsyncKeyState para detecção de hotkeys e estados de mouse/teclado.

---

**Desenvolvido para fins educacionais - Counter-Strike 2 Cheat Development Study**