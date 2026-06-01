#!/bin/bash
# Запуск сборки Windows-бинарника в Docker
# Использование: ./docker/build-win.sh [путь_к_исходникам]
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="${1:-$(dirname "$SCRIPT_DIR")}"
OUT_DIR="$SCRIPT_DIR/output"
IMAGE="video2doc-win-builder"
CONTAINER_NAME="video2doc-win-build"

mkdir -p "$OUT_DIR"

echo ">>> Источники: $SRC_DIR"
echo ">>> Результат: $OUT_DIR"

# Удаляем старый контейнер если остался
docker rm -f "$CONTAINER_NAME" 2>/dev/null || true

# Сборка Docker-образа (только если изменился Dockerfile.win)
docker build \
    -f "$SCRIPT_DIR/Dockerfile.win" \
    -t "$IMAGE" \
    "$SCRIPT_DIR"

# Запускаем контейнер в фоне (без volume mounts — совместимо с Docker Desktop)
# Переопределяем ENTRYPOINT, чтобы контейнер просто ждал
docker run --name "$CONTAINER_NAME" -d --entrypoint sleep "$IMAGE" 3600

# Копируем исходники в контейнер
echo ">>> Копирование исходников в контейнер..."
docker cp "$SRC_DIR/." "${CONTAINER_NAME}:/src/"

# Запускаем сборку (ENTRYPOINT вручную, т.к. контейнер уже запущен)
echo ">>> Сборка Windows-бинарника..."
docker exec "$CONTAINER_NAME" /build-win-inner.sh

# Копируем результат обратно
echo ">>> Копирование результата..."
docker cp "${CONTAINER_NAME}:/out/." "$OUT_DIR/"

# Очистка
docker rm -f "$CONTAINER_NAME"

echo ""
echo "=== Готово ==="
ls -lh "$OUT_DIR"/*.zip 2>/dev/null || echo "Архив не найден в $OUT_DIR"
