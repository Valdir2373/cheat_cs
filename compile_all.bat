@echo off
echo === CS2 Cheat Compiler ===
echo.

set "SOURCE_DIR=%~dp0"
set "OUTPUT_DIR=%SOURCE_DIR%compiled"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo [+] Compiling cheats...
echo.

REM Wallhack básico
echo [+] Compiling wall.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\wall.exe" "%SOURCE_DIR%wall.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile wall.cpp
) else (
    echo [+] wall.exe compiled successfully
)

REM Offset explorer
echo [+] Compiling offset_explorer.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\offset_explorer.exe" "%SOURCE_DIR%offset_explorer.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile offset_explorer.cpp
) else (
    echo [+] offset_explorer.exe compiled successfully
)

REM Deep scanner
echo [+] Compiling deep_scanner.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\deep_scanner.exe" "%SOURCE_DIR%deep_scanner.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile deep_scanner.cpp
) else (
    echo [+] deep_scanner.exe compiled successfully
)

REM Flag identifier
echo [+] Compiling flag_identifier.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\flag_identifier.exe" "%SOURCE_DIR%flag_identifier.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile flag_identifier.cpp
) else (
    echo [+] flag_identifier.exe compiled successfully
)

REM Advanced cheat
echo [+] Compiling advanced_cheat.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\advanced_cheat.exe" "%SOURCE_DIR%advanced_cheat.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile advanced_cheat.cpp
) else (
    echo [+] advanced_cheat.exe compiled successfully
)

REM Cheat suite
echo [+] Compiling cheat_suite.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\cheat_suite.exe" "%SOURCE_DIR%cheat_suite.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile cheat_suite.cpp
) else (
    echo [+] cheat_suite.exe compiled successfully
)

REM Triggerbot advanced
echo [+] Compiling triggerbot_advanced.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\triggerbot_advanced.exe" "%SOURCE_DIR%triggerbot_advanced.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile triggerbot_advanced.cpp
) else (
    echo [+] triggerbot_advanced.exe compiled successfully
)

REM No-recoil
echo [+] Compiling norecoil_cheat.cpp...
cl /EHsc /Fe"%OUTPUT_DIR%\norecoil_cheat.exe" "%SOURCE_DIR%norecoil_cheat.cpp" /link user32.lib
if %errorlevel% neq 0 (
    echo [-] Failed to compile norecoil_cheat.cpp
) else (
    echo [+] norecoil_cheat.exe compiled successfully
)

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
echo.
echo [+] Run any .exe file to start the corresponding cheat
echo [+] Make sure CS2 is running before starting cheats
echo.
pause