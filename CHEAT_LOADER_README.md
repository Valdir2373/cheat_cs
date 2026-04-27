# Cheat Loader System - CS2

## 🎯 Sistema Principal

O `cheat_loader.cpp` é o **app principal** que carrega todos os cheats dinamicamente usando índices (0,1,2,3,4,5,6,7,8,9,10,11...).

### Como Funciona

1. **Um único executável** (`cheat_loader_g++.exe`) contém todos os cheats
2. **Índices numéricos** referenciam diferentes cheats
3. **Fácil manutenção** - adicionar novo cheat = apenas registrar no `cheatRegistry`
4. **Modo linha de comando** ou **modo interativo**

## 🚀 Como Usar

### Compilação
```bash
g++ -std=c++17 cheat_loader.cpp -o cheat_loader_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Modo Interativo
```bash
./cheat_loader_g++.exe
# ou
run_cheat.bat
```
Mostra menu e você escolhe o número do cheat.

### Modo Linha de Comando
```bash
# Executar cheat específico diretamente
./cheat_loader_g++.exe 2    # Silent Aimbot
./cheat_loader_g++.exe 0    # Wallhack
./cheat_loader_g++.exe 3    # Radar

# ou usando o script batch
run_cheat.bat 2             # Silent Aimbot
run_cheat.bat 0             # Wallhack
run_cheat.bat 3             # Radar
```

## 📋 Cheats Registrados

| Índice | Cheat | Descrição |
|--------|-------|-----------|
| 0 | Wallhack | ESP com F5 toggle |
| 1 | Aimbot | Mira automática suavizada |
| 2 | Silent Aimbot | Mira silenciosa no disparo |
| 3 | Console Radar | Mostra posições no console |
| 4 | Bunny Hop | Pulo automático |
| 5 | Triggerbot | Tiro automático na mira |

## 🛠️ Como Adicionar Novos Cheats

### 1. Criar Classe do Cheat
```cpp
class MeuNovoCheat : public CheatBase {
public:
    void Run() override {
        // Implementação do cheat
    }

    std::string GetName() override {
        return "Meu Novo Cheat";
    }

    std::string GetDescription() override {
        return "Descrição do que faz";
    }
};
```

### 2. Registrar no Sistema
```cpp
void RegisterCheats() {
    cheatRegistry[0] = []() { return new WallhackCheat(); };
    cheatRegistry[1] = []() { return new AimbotCheat(); };
    cheatRegistry[2] = []() { return new SilentAimbotCheat(); };
    // ...
    cheatRegistry[6] = []() { return new MeuNovoCheat(); };  // ← NOVO
}
```

### 3. Pronto!
Agora o cheat está disponível como índice 6:
```bash
./cheat_loader_g++.exe 6
```

## 🎮 Vantagens do Sistema

### ✅ Manutenção Fácil
- Um arquivo principal para todos os cheats
- Adicionar novo cheat = apenas registrar no map
- Código organizado e reutilizável

### ✅ Flexibilidade
- Modo interativo com menu
- Modo linha de comando para scripts
- Sistema de índices extensível (0-∞)

### ✅ Performance
- Apenas um executável carregado
- Classes instanciadas sob demanda
- Memória limpa após uso

### ✅ Escalabilidade
- Fácil adicionar novos cheats
- Sistema de herança para funcionalidades comuns
- Separação clara entre loader e implementações

## 🔧 Arquitetura

```
cheat_loader.cpp
├── CheatBase (classe base)
│   ├── Conexão com CS2
│   ├── Leitura/escrita de memória
│   └── Utilitários comuns
│
├── Cheats Implementados
│   ├── WallhackCheat (0)
│   ├── AimbotCheat (1)
│   ├── SilentAimbotCheat (2)
│   ├── RadarCheat (3)
│   ├── BhopCheat (4)
│   └── TriggerbotCheat (5)
│
└── CheatLoader (sistema principal)
    ├── cheatRegistry (map de cheats)
    ├── ShowMenu() (modo interativo)
    └── RunCheat() (execução por índice)
```

## 🎯 Próximos Passos

Para adicionar mais cheats ao sistema:

1. **Criar classe** herdando de `CheatBase`
2. **Implementar métodos** `Run()`, `GetName()`, `GetDescription()`
3. **Registrar no `cheatRegistry`** com novo índice
4. **Recompilar** o `cheat_loader.cpp`

Exemplo de expansão:
```cpp
cheatRegistry[6] = []() { return new NoRecoilCheat(); };
cheatRegistry[7] = []() { return new GlowCheat(); };
cheatRegistry[8] = []() { return new SpeedHackCheat(); };
// ... e assim por diante
```

## 🚀 Uso Recomendado

```bash
# Compilar o loader principal
g++ -std=c++17 cheat_loader.cpp -o cheat_loader_g++.exe -luser32 -static -static-libgcc -static-libstdc++

# Usar modo interativo
./cheat_loader_g++.exe

# Ou executar cheats específicos
./cheat_loader_g++.exe 2  # Silent Aimbot
./cheat_loader_g++.exe 0  # Wallhack
```

## 📁 Arquivos do Sistema

- `cheat_loader.cpp` - Código fonte principal
- `cheat_loader_g++.exe` - Executável compilado
- `run_cheat.bat` - Script batch para facilitar uso
- `CHEAT_LOADER_README.md` - Esta documentação

Este sistema torna a manutenção muito mais simples e organizada! 🎉