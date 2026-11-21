@echo off
setlocal
cd /d "%~dp0"

set CONFIGURATION=Release
set PLATFORM=x64

if "%1" neq "" set CONFIGURATION=%1
if "%2" neq "" set PLATFORM=%2

set MSBUILD_EXE=
if exist "%~dp0.tools\build_tools\VC\Auxiliary\Build\vcvars64.bat" (
    call "%~dp0.tools\build_tools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)

if exist "%~dp0.tools\build_tools\MSBuild\Current\Bin\amd64\MSBuild.exe" (
    set "MSBUILD_EXE=%~dp0.tools\build_tools\MSBuild\Current\Bin\amd64\MSBuild.exe"
) else if exist "%~dp0.tools\build_tools\MSBuild\Current\Bin\MSBuild.exe" (
    set "MSBUILD_EXE=%~dp0.tools\build_tools\MSBuild\Current\Bin\MSBuild.exe"
) else (
    where msbuild.exe >nul 2>&1
    if %errorlevel% equ 0 (
        set "MSBUILD_EXE=msbuild.exe"
    )
)

if "%MSBUILD_EXE%" == "" (
    echo [ERROR] MSBuild not found!
    echo Please run install_build_tools.cmd or install Visual Studio / Build Tools first.
    pause
    exit /b 1
)

echo ========================================================
echo Building OGSR Engine
echo Configuration : %CONFIGURATION%
echo Platform      : %PLATFORM%
echo Using MSBuild : %MSBUILD_EXE%
echo ========================================================

"%MSBUILD_EXE%" Engine.sln /t:Build /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM% -m
if %errorlevel% equ 0 (
    echo.
    echo ========================================================
    echo Build Succeeded! Binaries are in bin_%PLATFORM%\
    echo ========================================================
) else (
    echo.
    echo [ERROR] Build Failed! See errors above.
)
pause
