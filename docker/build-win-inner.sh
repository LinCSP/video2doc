#!/bin/bash
# Выполняется внутри контейнера Windows-сборки
set -e

BUILD_DIR=/tmp/build-win
DIST_DIR=/tmp/dist-win

echo "=== Configure ==="
cmake -S /src -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/mingw-toolchain.cmake \
    -DCMAKE_PREFIX_PATH="$WXWIN"

echo "=== Build ==="
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

echo "=== Collect ==="
mkdir -p "$DIST_DIR"
cp "$BUILD_DIR/bin/Video2Doc.exe" "$DIST_DIR/"

# Дополнительные файлы для пользователя
if [ -f "/src/install-deps.bat" ]; then
    cp "/src/install-deps.bat" "$DIST_DIR/" && echo "Copied install-deps.bat"
fi
if [ -f "/src/requirements.txt" ]; then
    cp "/src/requirements.txt" "$DIST_DIR/" && echo "Copied requirements.txt"
fi

# MinGW runtime DLL — спрашиваем у используемого компилятора, где лежат его DLL
for dll in libgcc_s_seh-1 libstdc++-6 libwinpthread-1; do
    src=$(x86_64-w64-mingw32-g++ -print-file-name="${dll}.dll")
    if [ -f "$src" ] && [ "$src" != "${dll}.dll" ]; then
        cp "$src" "$DIST_DIR/" && echo "Copied $dll.dll"
    else
        echo "WARNING: $dll.dll not found"
    fi
done

echo "=== Archive ==="
mkdir -p /out
ARCHIVE="/out/Video2Doc-win64-$(date +%Y%m%d-%H%M).zip"
cd "$DIST_DIR" && zip -r "$ARCHIVE" .
echo "Done: $ARCHIVE  ($(du -sh "$ARCHIVE" | cut -f1))"
