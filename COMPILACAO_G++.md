# Compilação Manual com g++

Se o g++ estiver instalado, você pode compilar cada cheat manualmente usando estes comandos:

## 1. Verificar se g++ está instalado
```bash
g++ --version
```

## 2. Compilar cada cheat individualmente

### Wallhack Básico
```bash
g++ -std=c++17 wall.cpp -o wall_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Offset Explorer
```bash
g++ -std=c++17 offset_explorer.cpp -o offset_explorer_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Deep Scanner
```bash
g++ -std=c++17 deep_scanner.cpp -o deep_scanner_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Flag Identifier
```bash
g++ -std=c++17 flag_identifier.cpp -o flag_identifier_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Advanced Cheat (Multi-threaded)
```bash
g++ -std=c++17 advanced_cheat.cpp -o advanced_cheat_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Cheat Suite (Menu OOP)
```bash
g++ -std=c++17 cheat_suite.cpp -o cheat_suite_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### Triggerbot Advanced
```bash
g++ -std=c++17 triggerbot_advanced.cpp -o triggerbot_advanced_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

### No-Recoil Cheat
```bash
g++ -std=c++17 norecoil_cheat.cpp -o norecoil_cheat_g++.exe -luser32 -static -static-libgcc -static-libstdc++
```

## 3. Compilação em lote (Linux/Mac)
```bash
# Criar executáveis para todos os cheats
for file in *.cpp; do
    exe_name="${file%.cpp}_g++.exe"
    g++ -std=c++17 "$file" -o "$exe_name" -luser32 -static -static-libgcc -static-libstdc++
done
```

## 4. Flags de compilação explicadas
- `-std=c++17`: Usa C++17 (necessário para threads e filesystem)
- `-o nome.exe`: Define o nome do executável de saída
- `-luser32`: Linka com a biblioteca user32.dll (para Windows API)
- `-static -static-libgcc -static-libstdc++`: Cria executável standalone

## 5. Se g++ não estiver instalado

### Windows:
1. Instalar MinGW-w64:
   - Baixar de: https://www.mingw-w64.org/
   - Ou usar MSYS2: `pacman -S mingw-w64-x86_64-gcc`
   - Adicionar ao PATH

2. Ou instalar Visual Studio Build Tools (recomendado):
   - Baixar de: https://visualstudio.microsoft.com/downloads/
   - Instalar workload "Desktop development with C++"

### Linux:
```bash
sudo apt update
sudo apt install g++
```

### macOS:
```bash
# Usando Homebrew
brew install gcc

# Ou Xcode Command Line Tools
xcode-select --install
```

## 6. Testar compilação
Após compilar, teste executando:
```bash
./wall_g++.exe
```

Certifique-se que o CS2 está rodando antes de executar os cheats!