@echo off
setlocal enabledelayedexpansion

echo ===================================================================
echo   DSA MICRO-PROJECT: UNDO AND REDO USING STACK IMPLEMENTATION
echo   Language: C (Winsock2 HTTP Server + Pure Stack Algorithm)
echo ===================================================================
echo.

:: 1. Search for GCC Compiler
set GCC_CMD=gcc

where gcc >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\Program Files\IDK\compilers\MinGW64\bin\gcc.exe" (
        set "GCC_CMD=C:\Program Files\IDK\compilers\MinGW64\bin\gcc.exe"
    ) else if exist "C:\MinGW\bin\gcc.exe" (
        set "GCC_CMD=C:\MinGW\bin\gcc.exe"
    ) else if exist "C:\msys64\mingw64\bin\gcc.exe" (
        set "GCC_CMD=C:\msys64\mingw64\bin\gcc.exe"
    ) else if exist "C:\TDM-GCC-64\bin\gcc.exe" (
        set "GCC_CMD=C:\TDM-GCC-64\bin\gcc.exe"
    ) else (
        echo [ERROR] GCC compiler was not found in PATH or standard directories.
        echo Please ensure MinGW or GCC is installed to compile server.c.
        echo.
        pause
        exit /b 1
    )
)

echo [1/3] Found C Compiler: "!GCC_CMD!"
echo [2/3] Compiling server.c with Winsock library (-lws2_32)...

"!GCC_CMD!" server.c -o server.exe -lws2_32
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %errorlevel%
)

echo [3/3] Compilation successful: server.exe is ready!
echo.
echo Launching default web browser to http://localhost:8080 ...
start http://localhost:8080
echo.
echo Starting C Backend Server...
echo (Press Ctrl+C in this window at any time to stop the server)
echo ===================================================================
echo.

server.exe
