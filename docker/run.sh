#!/bin/bash
set -e

IMAGE_NAME=video2doc
CONTAINER_NAME=video2doc-build
OUTPUT_DIR=./output

# Удаляем старый контейнер если остался
docker rm -f ${CONTAINER_NAME} 2>/dev/null || true

# Сборка образа
docker build -t ${IMAGE_NAME} .

# Запуск контейнера с примонтированным локальным исходником
# (чтобы не делать git pull и не терять локальные изменения)
docker run --name ${CONTAINER_NAME} \
    -v "$(pwd)/..:/devel/video2doc" \
    -w /devel/video2doc-build \
    ${IMAGE_NAME} \
    bash -c "
        set -e
        cmake -G Ninja -DCMAKE_BUILD_TYPE=Release /devel/video2doc
        ninja
    "

# Копируем бинарники
mkdir -p ${OUTPUT_DIR}
docker cp ${CONTAINER_NAME}:/devel/video2doc-build/bin/. ${OUTPUT_DIR}/

echo "=== Binaries copied to ${OUTPUT_DIR} ==="

# Очистка
docker rm ${CONTAINER_NAME}
