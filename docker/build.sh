#!/bin/bash
set -e

echo "=== Video2Doc Linux build.sh ==="

# Обновляем исходники
cd /devel/video2doc
git pull origin main
echo "Sources updated"

# Сборка
cd /devel/video2doc-build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release /devel/video2doc
ninja

echo "=== Build complete ==="
echo "Binary: /devel/video2doc-build/bin/Video2Doc"
