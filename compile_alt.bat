@echo off
echo === CS2 Cheat Compiler (Alternative) ===
echo.

REM Tentar diferentes compiladores
set "COMPILER_FOUND=0"

REM Verificar se cl.exe está disponível (Visual Studio)
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo [+] Using Microsoft Visual C++ Compiler (cl.exe)
    set "COMPILER=cl /EHsc"
    set "LINKER=/link user32.lib"
    set "COMPILER_FOUND=1"
)

REM Verificar se g++.exe está disponível (MinGW)
if %COMPILER_FOUND% equ 0 (
    where g++ >nul 2>nul
    if %errorlevel% equ 0 (
        echo [+] Using MinGW g++ Compiler
        set "COMPILER=g++ -std=c++17"
        set "LINKER=-luser32"
        set "COMPILER_FOUND=1"
    )
)

if %COMPILER_FOUND% equ 0 (
    echo [-] No C++ compiler found!
    echo.
    echo [+] Please install one of the following:
    echo     1. Visual Studio Build Tools (recommended)
    echo        - Download from: https://visualstudio.microsoft.com/downloads/
    echo        - Install "Desktop development with C++" workload
    echo.
    echo     2. MinGW-w64
    echo        - Download from: https://www.mingw-w64.org/
    echo        - Add bin folder to PATH
    echo.
    echo     3. LLVM/Clang
    echo        - Download from: https://releases.llvm.org/
    echo.
    pause
    exit /b 1
)

set "SOURCE_DIR=%~dp0"
set "OUTPUT_DIR=%SOURCE_DIR%compiled"

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo [+] Compiling cheats...
echo.

REM Função para compilar
call :compile_file "wall.cpp" "wall.exe"
call :compile_file "offset_explorer.cpp" "offset_explorer.exe"
call :compile_file "deep_scanner.cpp" "deep_scanner.exe"
call :compile_file "flag_identifier.cpp" "flag_identifier.exe"
call :compile_file "advanced_cheat.cpp" "advanced_cheat.exe"
call :compile_file "cheat_suite.cpp" "cheat_suite.exe"
call :compile_file "triggerbot_advanced.cpp" "triggerbot_advanced.exe"
call :compile_file "norecoil_cheat.cpp" "norecoil_cheat.exe"

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
exit /b 0

:compile_file
echo [+] Compiling %~1...
if "%COMPILER%"=="cl /EHsc" (
    %COMPILER% /Fe"%OUTPUT_DIR%\%~2" "%SOURCE_DIR%%~1" %LINKER%
) else (
    %COMPILER% "%SOURCE_DIR%%~1" -o "%OUTPUT_DIR%\%~2" %LINKER%
)
if %errorlevel% neq 0 (
    echo [-] Failed to compile %~1
) else (
    echo [+] %~2 compiled successfully
)
echo.
goto :eof