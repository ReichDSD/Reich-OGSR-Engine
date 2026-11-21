@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo Updating engine binaries in Resources_SoC_1.0006...
echo ========================================================

if not exist "%~dp0bin_x64" mkdir "%~dp0bin_x64"

if not exist "%~dp0..\..\bin_x64\xrEngine.exe" (
    echo [ERROR] bin_x64\xrEngine.exe not found!
    echo Please build the engine first using build.cmd.
    pause
    exit /b 1
)

copy /Y "%~dp0..\..\bin_x64\*.*" "%~dp0bin_x64\"

echo.
echo Launching xrEngine...
echo.

start "" "%~dp0bin_x64\xrEngine.exe" -nointro -noprefetch %*
