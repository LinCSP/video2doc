#!/bin/bash
# Установка всех зависимостей Video2Doc для Linux
# Запуск: ./install-deps.sh

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "  Video2Doc — установка зависимостей"
echo "========================================"
echo ""

# Определяем дистрибутив
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
else
    echo -e "${RED}Не удалось определить дистрибутив Linux${NC}"
    exit 1
fi

echo "Обнаружен дистрибутив: $DISTRO"
echo ""

# --- Системные зависимости ---
echo ">>> Установка системных пакетов..."

case "$DISTRO" in
    ubuntu|debian|linuxmint|pop)
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            libwxgtk3.2-dev \
            libgtk-3-dev \
            libglib2.0-dev \
            ffmpeg \
            python3 \
            python3-pip \
            python3-venv \
            git \
            wget \
            ca-certificates
        ;;

    arch|manjaro|endeavouros)
        sudo pacman -Sy --needed --noconfirm \
            base-devel \
            cmake \
            ninja \
            wxwidgets-gtk3 \
            gtk3 \
            glib2 \
            ffmpeg \
            python \
            python-pip \
            git \
            wget \
            ca-certificates-utils
        ;;

    fedora|rhel|centos|almalinux|rocky)
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            ninja-build \
            wxGTK3-devel \
            gtk3-devel \
            glib2-devel \
            ffmpeg \
            python3 \
            python3-pip \
            git \
            wget \
            ca-certificates
        ;;

    *)
        echo -e "${RED}Дистрибутив '$DISTRO' не поддерживается автоматической установкой.${NC}"
        echo "Установите зависимости вручную, см. README.md → Зависимости"
        exit 1
        ;;
esac

echo -e "${GREEN}✓ Системные пакеты установлены${NC}"
echo ""

# --- Python-зависимости ---
echo ">>> Установка Python-пакетов (faster-whisper, kimi-cli)..."

# Предпочитаем virtual environment, но если его нет — ставим в систему
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REQ_FILE="$SCRIPT_DIR/requirements.txt"

if [ -f "$REQ_FILE" ]; then
    pip3 install -r "$REQ_FILE" --user 2>/dev/null || pip3 install -r "$REQ_FILE"
else
    echo -e "${YELLOW}⚠ Файл requirements.txt не найден, ставим напрямую...${NC}"
    pip3 install faster-whisper kimi-cli --user 2>/dev/null || pip3 install faster-whisper kimi-cli
fi

echo -e "${GREEN}✓ Python-пакеты установлены${NC}"
echo ""

# --- Проверка ---
echo ">>> Проверка установленных программ..."

check_cmd() {
    if command -v "$1" &> /dev/null; then
        echo -e "  ${GREEN}✓${NC} $1"
        return 0
    else
        echo -e "  ${RED}✗${NC} $1 — НЕ НАЙДЕН"
        return 1
    fi
}

OK=0
check_cmd git || true  # git необязательный
check_cmd cmake || OK=1
check_cmd ninja || OK=1
check_cmd wx-config || check_cmd wx-config-gtk3 || OK=1
check_cmd ffmpeg || OK=1
check_cmd python3 || OK=1
check_cmd pip3 || OK=1

# Проверка Python-модулей
python3 -c "import faster_whisper" 2>/dev/null && echo -e "  ${GREEN}✓${NC} faster-whisper" || { echo -e "  ${RED}✗${NC} faster-whisper"; OK=1; }
python3 -c "import kimi" 2>/dev/null || command -v kimi &>/dev/null && echo -e "  ${GREEN}✓${NC} kimi-cli" || { echo -e "  ${RED}✗${NC} kimi-cli"; OK=1; }

echo ""
if [ $OK -eq 0 ]; then
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  Все зависимости установлены!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo "Теперь можно собирать проект:"
    echo "  mkdir build && cd build"
    echo "  cmake .. -G Ninja && ninja"
    echo "  ./bin/Video2Doc"
else
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}  Некоторые зависимости не найдены${NC}"
    echo -e "${YELLOW}========================================${NC}"
    echo ""
    echo "Перезапустите терминал (чтобы обновился PATH) и запустите скрипт снова."
    echo "Если проблема остаётся — см. README.md → Зависимости"
    exit 1
fi
