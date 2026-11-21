@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo Installing MSVC Build Tools 2022 to .tools\build_tools
echo ========================================================

set INSTALL_DIR=%~dp0.tools\build_tools
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

echo Downloading vs_BuildTools.exe from Microsoft...
curl.exe -Lo "%TEMP%\vs_BuildTools.exe" "https://aka.ms/vs/17/release/vs_BuildTools.exe"
if %errorlevel% neq 0 (
    echo [ERROR] Failed to download vs_BuildTools.exe
    pause
    exit /b %errorlevel%
)

echo.
echo Launching installer (this requires Administrator permissions)...
echo Target directory: %INSTALL_DIR%
echo Components: MSVC v143 (x64/x86) + Windows 11 SDK
echo Please wait, installation progress window will appear...
echo.

"%TEMP%\vs_BuildTools.exe" --installPath "%INSTALL_DIR%" ^
  --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
  --add Microsoft.VisualStudio.Component.VC.ATL ^
  --add Microsoft.VisualStudio.Component.Windows11SDK.22621 ^
  --passive --wait --norestart

if %errorlevel% equ 0 (
    echo.
    echo ========================================================
    echo Build Tools successfully installed!
    echo Location: %INSTALL_DIR%
    echo ========================================================
) else (
    echo.
    echo [WARNING] Installer exited with code %errorlevel%
)

if exist "%TEMP%\vs_BuildTools.exe" del "%TEMP%\vs_BuildTools.exe"
pause
