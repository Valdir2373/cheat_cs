@echo off
echo === Compilacao Manual com g++ ===
echo.

REM Verificar se g++ existe
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [-] ERRO: g++ nao encontrado!
    echo.
    echo [+] Instale MinGW-w64 ou Visual Studio Build Tools
    echo [+] Veja instrucoes em COMPILACAO_G++.md
    echo.
    pause
    exit /b 1
)

echo [+] g++ encontrado! Vamos compilar...
echo.

set /p choice="Qual cheat compilar? (1-8): "
echo.

if "%choice%"=="1" (
    echo [+] Compilando Wallhack...
    g++ -std=c++17 wall.cpp -o wall_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="2" (
    echo [+] Compilando Offset Explorer...
    g++ -std=c++17 offset_explorer.cpp -o offset_explorer_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="3" (
    echo [+] Compilando Deep Scanner...
    g++ -std=c++17 deep_scanner.cpp -o deep_scanner_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="4" (
    echo [+] Compilando Flag Identifier...
    g++ -std=c++17 flag_identifier.cpp -o flag_identifier_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="5" (
    echo [+] Compilando Advanced Cheat...
    g++ -std=c++17 advanced_cheat.cpp -o advanced_cheat_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="6" (
    echo [+] Compilando Cheat Suite...
    g++ -std=c++17 cheat_suite.cpp -o cheat_suite_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="7" (
    echo [+] Compilando Triggerbot Advanced...
    g++ -std=c++17 triggerbot_advanced.cpp -o triggerbot_advanced_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

if "%choice%"=="8" (
    echo [+] Compilando No-Recoil...
    g++ -std=c++17 norecoil_cheat.cpp -o norecoil_cheat_g++.exe -luser32 -static -static-libgcc -static-libstdc++
    goto check_result
)

echo [-] Opcao invalida!
goto menu

:check_result
if %errorlevel% equ 0 (
    echo [+] Compilacao bem-sucedida!
    echo [+] Executavel criado no diretorio atual
) else (
    echo [-] Erro na compilacao!
    echo [-] Verifique os erros acima
)
echo.
pause
exit /b 0

:menu
echo === Menu de Compilacao ===
echo 1. Wallhack (wall.cpp)
echo 2. Offset Explorer (offset_explorer.cpp)
echo 3. Deep Scanner (deep_scanner.cpp)
echo 4. Flag Identifier (flag_identifier.cpp)
echo 5. Advanced Cheat (advanced_cheat.cpp)
echo 6. Cheat Suite (cheat_suite.cpp)
echo 7. Triggerbot Advanced (triggerbot_advanced.cpp)
echo 8. No-Recoil (norecoil_cheat.cpp)
echo.
goto menu