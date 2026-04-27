@echo off
echo Compiling wall.cpp with g++...
g++ -std=c++17 wall.cpp -o wall_g++.exe -luser32 -static -static-libgcc -static-libstdc++
if %errorlevel% equ 0 (
    echo [+] wall_g++.exe compiled successfully!
) else (
    echo [-] Compilation failed!
)
pause