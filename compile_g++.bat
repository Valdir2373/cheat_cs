@echo off
echo === CS2 Cheat Compiler (g++) ===
echo.

REM Verificar se g++ está disponível
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [-] g++ not found!
    echo.
    echo [+] Please install MinGW-w64:
    echo     1. Download from: https://www.mingw-w64.org/
    echo     2. Install and add to PATH
    echo     3. Or use MSYS2: pacman -S mingw-w64-x86_64-gcc
    echo.
    pause
    exit /b 1
)

echo [+] Using g++ compiler
set "SOURCE_DIR=%~dp0"
set "OUTPUT_DIR=%SOURCE_DIR%compiled"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo [+] Compiling cheats with g++...
echo.

REM Compilar cada arquivo
call :compile_file "wall.cpp" "wall.exe"
call :compile_file "offset_explorer.cpp" "offset_explorer.exe"
call :compile_file "deep_scanner.cpp" "deep_scanner.exe"
call :compile_file "flag_identifier.cpp" "flag_identifier.exe"
call :compile_file "advanced_cheat.cpp" "advanced_cheat.exe"
call :compile_file "cheat_suite.cpp" "cheat_suite.exe"
call :compile_file "triggerbot_advanced.cpp" "triggerbot_advanced.exe"
call :compile_file "norecoil_cheat.cpp" "norecoil_cheat.exe"
call :compile_file "silent_aimbot.cpp" "silent_aimbot.exe"
call :compile_file "module_explorer.cpp" "module_explorer.exe"

echo.
echo [+] Compilation complete!
echo [+] Executables saved to: %OUTPUT_DIR%
echo.
echo [+] Available cheats:
echo     wall.exe - Basic ESP wallhack
echo     offset_explorer.exe - Display all discovered offsets
echo     deep_scanner.exe - Deep ESP area exploration
echo     flag_identifier.exe - Interactive flag testing
echo     advanced_cheat.exe - Multi-threaded cheats (ESP, Aimbot, etc.)
echo     cheat_suite.exe - OOP cheat suite with menu
echo     triggerbot_advanced.exe - Advanced triggerbot
echo     norecoil_cheat.exe - No-recoil system
echo     silent_aimbot.exe - Silent aimbot without visible aiming
echo     module_explorer.exe - List CS2 loaded DLLs and module bases
echo.
echo [+] Run any .exe file to start the corresponding cheat
echo [+] Make sure CS2 is running before starting cheats
echo.
pause
exit /b 0

:compile_file
echo [+] Compiling %~1...
g++ -std=c++17 "%SOURCE_DIR%%~1" -o "%OUTPUT_DIR%\%~2" -luser32 -static -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo [-] Failed to compile %~1
    echo [-] Check for compilation errors above
) else (
    echo [+] %~2 compiled successfully
)
echo.
goto :eof