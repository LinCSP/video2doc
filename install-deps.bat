@echo off
setlocal EnableDelayedExpansion

echo ========================================
echo   Video2Doc - Dependency Installer
echo ========================================
echo.

set "SCRIPT_DIR=%~dp0"
set "REQ_FILE=%SCRIPT_DIR%requirements.txt"
set "MISSING=0"

REM --- Check git ---
echo [1/5] Checking git...
git --version >nul 2>&1
if errorlevel 1 (
    echo   ! git not found (optional)
    echo     git is only needed for cloning repositories.
    echo     Download from https://git-scm.com/download/win if needed.
) else (
    for /f "tokens=*" %%a in ('git --version 2^>^&1') do echo   + %%a
)
echo.

REM --- Check Python ---
echo [2/5] Checking Python...
python --version >nul 2>&1
if errorlevel 1 (
    echo   - Python NOT FOUND
    echo.
    echo   Download and install Python 3.10+ from:
    echo   https://www.python.org/downloads/
    echo.
    echo   IMPORTANT: Check "Add Python to PATH" during installation.
    echo.
    set "MISSING=1"
) else (
    for /f "tokens=*" %%a in ('python --version 2^>^&1') do echo   + %%a
)

REM --- Check pip ---
echo [3/5] Checking pip...
pip --version >nul 2>&1
if errorlevel 1 (
    echo   - pip NOT FOUND
    echo     Make sure Python is installed correctly.
    set "MISSING=1"
) else (
    for /f "tokens=*" %%a in ('pip --version 2^>^&1') do echo   + %%a
)

REM --- Check ffmpeg ---
echo [4/5] Checking ffmpeg...
ffmpeg -version >nul 2>&1
if errorlevel 1 (
    echo   - ffmpeg NOT FOUND
    echo.
    echo   Download ffmpeg from:
    echo   https://ffmpeg.org/download.html#build-windows
    echo.
    echo   Extract the archive and add the bin\ folder to PATH,
    echo   or place ffmpeg.exe next to this program.
    echo.
    set "MISSING=1"
) else (
    for /f "tokens=*" %%a in ('ffmpeg -version 2^>^&1') do (
        echo   + %%a
        goto :ffmpeg_done
    )
)
:ffmpeg_done

if "%MISSING%"=="1" (
    echo.
    echo ========================================
    echo   Please install missing programs above
    echo   and run this script again.
    echo ========================================
    pause
    exit /b 1
)

REM --- Install Python packages ---
echo [5/5] Installing Python packages (faster-whisper, kimi-cli)...
if exist "%REQ_FILE%" (
    pip install -r "%REQ_FILE%"
) else (
    echo   requirements.txt not found, installing directly...
    pip install faster-whisper kimi-cli
)

if errorlevel 1 (
    echo   - Failed to install Python packages
    pause
    exit /b 1
)

echo   + Python packages installed
echo.

echo ========================================
echo   All dependencies are ready!
echo ========================================
echo.
echo To build from source, install MSYS2 and run:
echo   pacman -S mingw-w64-ucrt-x86_64-gcc cmake ninja mingw-w64-ucrt-x86_64-wxwidgets3.2-msw
echo.
echo Or use Docker:
echo   cd docker
echo   .\build-win.bat
echo.
pause
