@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ========================================
echo   Video2Doc — установка зависимостей
echo ========================================
echo.

set "SCRIPT_DIR=%~dp0"
set "REQ_FILE=%SCRIPT_DIR%requirements.txt"
set "MISSING=0"

REM --- Проверка git ---
echo [1/5] Проверка git...
git --version >nul 2>&1
if errorlevel 1 (
    echo   ⚠ git НЕ НАЙДЕН (необязательно)
    echo   git нужен только для клонирования репозиториев.
    echo   Скачайте с https://git-scm.com/download/win если нужен.
) else (
    for /f "tokens=*" %%a in ('git --version 2^>^&1') do echo   ✓ %%a
)
echo.

REM --- Проверка Python ---
echo [2/5] Проверка Python...
python --version >nul 2>&1
if errorlevel 1 (
    echo   ✗ Python НЕ НАЙДЕН
    echo.
    echo   Скачайте и установите Python 3.10+ с сайта:
    echo   https://www.python.org/downloads/
    echo.
    echo   ☑ Важно: при установке поставьте галочку
    echo      "Add Python to PATH"
    echo.
    set "MISSING=1"
) else (
    for /f "tokens=*" %%a in ('python --version 2^>^&1') do echo   ✓ %%a
)

REM --- Проверка pip ---
echo [3/5] Проверка pip...
pip --version >nul 2>&1
if errorlevel 1 (
    echo   ✗ pip НЕ НАЙДЕН
    echo   Убедитесь, что Python установлен корректно.
    set "MISSING=1"
) else (
    for /f "tokens=*" %%a in ('pip --version 2^>^&1') do echo   ✓ %%a
)

REM --- Проверка ffmpeg ---
echo [4/5] Проверка ffmpeg...
ffmpeg -version >nul 2>&1
if errorlevel 1 (
    echo   ✗ ffmpeg НЕ НАЙДЕН
    echo.
    echo   Скачайте ffmpeg с сайта:
    echo   https://ffmpeg.org/download.html#build-windows
    echo.
    echo   Распакуйте архив и добавьте папку bin\ в переменную
    echo   окружения PATH, либо положите ffmpeg.exe рядом
    echo   с программой.
    echo.
    set "MISSING=1"
) else (
    for /f "tokens=*" %%a in ('ffmpeg -version 2^>^&1') do (
        echo   ✓ %%a
        goto :ffmpeg_done
    )
)
:ffmpeg_done

if "%MISSING%"=="1" (
    echo.
    echo ========================================
    echo   Установите недостающие программы
    echo   и запустите этот скрипт снова.
    echo ========================================
    pause
    exit /b 1
)

REM --- Установка Python-пакетов ---
echo [5/5] Установка Python-пакетов (faster-whisper, kimi-cli)...
if exist "%REQ_FILE%" (
    pip install -r "%REQ_FILE%"
) else (
    echo   Файл requirements.txt не найден, ставим напрямую...
    pip install faster-whisper kimi-cli
)

if errorlevel 1 (
    echo   ✗ Ошибка установки Python-пакетов
    pause
    exit /b 1
)

echo   ✓ Python-пакеты установлены
echo.

REM --- Итоговая проверка ---
echo ========================================
echo   Все зависимости установлены!
echo ========================================
echo.
echo Для сборки проекта установите MSYS2 и выполните:
echo   pacman -S mingw-w64-ucrt-x86_64-gcc cmake ninja mingw-w64-ucrt-x86_64-wxwidgets3.2-msw
echo.
echo Либо воспользуйтесь Docker:
echo   cd docker
echo   .\build-win.bat
echo.
pause
