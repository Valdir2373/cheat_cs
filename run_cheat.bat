@echo off
REM CS2 Cheat Loader - Script de Execução
REM Uso: run_cheat.bat [numero_do_cheat]

if "%1"=="" (
    echo === CS2 Cheat Loader v3.0 ===
    echo.
    echo Modo: Menu Interativo
    echo.
    cd /d "%~dp0"
    cheat_loader_g++.exe
    goto :eof
)

echo === CS2 Cheat Loader v3.0 ===
echo.
echo Modo: Cheat Direto (Indice %1)
echo.
cd /d "%~dp0"
cheat_loader_g++.exe %1