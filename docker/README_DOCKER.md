# Docker — шпаргалка Video2Doc

---

## Быстрый старт

### Сборка Linux-бинарника (из публичного репозитория)

```bash
cd docker
docker build -t video2doc .
docker run --rm --name video2doc-build video2doc
```

Бинарник останется внутри контейнера, скопируйте его:

```bash
mkdir -p ./output
docker cp video2doc-build:/devel/video2doc-build/bin/Video2Doc ./output/
```

---

### Сборка Linux-бинарника из локальных исходников

```bash
cd docker
./run.sh
```

Результат появится в `docker/output/`.

---

### Сборка Windows-бинарника (кросс-компиляция MinGW)

```bash
cd docker
./build-win.sh
```

Результат — zip-архив в `docker/output/`.

Если исходники не в корне репозитория:

```bash
./build-win.sh /путь/к/video2doc
```

---

## Обновить файл в образе без полной пересборки

Создать файл `Dockerfile.patch`:

```dockerfile
FROM video2doc
COPY ./some_fix.cpp /devel/video2doc/src/
```

Собрать новый образ поверх существующего:

```bash
docker build -f Dockerfile.patch -t video2doc:02 .
```

Так пересобирается только последний слой — быстро.

---

## Запуск контейнера

```bash
# Интерактивно с терминалом
docker run -it --rm --name video2doc-build video2doc /bin/bash

# Обычный запуск
docker run --rm --name video2doc-build video2doc
```

---

## Копирование файлов

```bash
# Локальный файл → контейнер
docker cp ./some_file video2doc-build:/devel/video2doc/src/

# Бинарники из контейнера → локальный каталог
docker cp video2doc-build:/devel/video2doc-build/bin/. ./output/
```

---

## Управление образами

```bash
# Список образов
docker images

# Удалить образ
docker rmi <image>

# Очистить все неиспользуемые ресурсы
docker system prune
```

---

## Прочее

```bash
unset DOCKER_HOST
```
