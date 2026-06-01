#!/bin/bash
set -e

IMAGE_NAME=video2doc
CONTAINER_NAME=video2doc-build
OUTPUT_DIR="$(cd "$(dirname "$0")" && pwd)/output"
SRC_DIR="$(cd "$(dirname "$0")/.." && pwd)"

mkdir -p "$OUTPUT_DIR"

# Удаляем старый контейнер если остался
docker rm -f ${CONTAINER_NAME} 2>/dev/null || true

# Сборка образа (из публичного репо, базовый слой с зависимостями)
docker build -t ${IMAGE_NAME} .

# Запускаем контейнер
docker run --name ${CONTAINER_NAME} -d ${IMAGE_NAME} sleep 3600

# Копируем локальные исходники в контейнер (вместо volume mount —
# решает проблему "mounts denied" в Docker Desktop)
echo ">>> Копирование исходников в контейнер..."
docker cp "$SRC_DIR/." "${CONTAINER_NAME}:/devel/video2doc/"

# Сборка внутри контейнера
echo ">>> Сборка внутри контейнера..."
docker exec ${CONTAINER_NAME} bash -c "
    set -e
    rm -rf /devel/video2doc-build
    mkdir -p /devel/video2doc-build
    cd /devel/video2doc-build
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release /devel/video2doc
    ninja
"

# Копируем бинарники обратно
echo ">>> Копирование результата..."
docker cp "${CONTAINER_NAME}:/devel/video2doc-build/bin/." "$OUTPUT_DIR/"

echo "=== Binaries copied to ${OUTPUT_DIR} ==="
ls -lh "$OUTPUT_DIR/"

# Очистка
docker rm -f ${CONTAINER_NAME}
