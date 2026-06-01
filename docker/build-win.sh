#!/bin/bash
# Запуск сборки Windows-бинарника в Docker
# Использование: ./docker/build-win.sh [путь_к_исходникам]
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="${1:-$(dirname "$SCRIPT_DIR")}"
OUT_DIR="$SCRIPT_DIR/output"
IMAGE="video2doc-win-builder"

mkdir -p "$OUT_DIR"

echo ">>> Источники: $SRC_DIR"
echo ">>> Результат: $OUT_DIR"

# Сборка Docker-образа (только если изменился Dockerfile.win)
docker build \
    -f "$SCRIPT_DIR/Dockerfile.win" \
    -t "$IMAGE" \
    "$SCRIPT_DIR"

# Запуск сборки
docker run --rm \
    -v "$SRC_DIR:/src:ro" \
    -v "$OUT_DIR:/out" \
    "$IMAGE"

echo ""
echo "=== Готово ==="
ls -lh "$OUT_DIR"/*.zip 2>/dev/null || echo "Архив не найден в $OUT_DIR"
