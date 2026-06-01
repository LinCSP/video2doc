# Video2Doc

Графическое приложение на wxWidgets для автоматической генерации документации из видеофайлов с помощью AI.

## Что делает

Video2Doc выполняет двухэтапный pipeline:

1. **Этап 1 — Транскрибация** (`TranscriptionEngine`)
   - Извлекает скриншоты из видео через `ffmpeg` с заданным интервалом
   - Распознаёт речь из аудиодорожки через `faster-whisper` (CTranslate2)
   - Сохраняет результат: текст транскрипции + скриншоты

2. **Этап 2 — Генерация документации** (`DocGenerator`)
   - Подготавливает промпт на основе шаблона с подстановкой переменных
   - Запускает [Kimi Code CLI](https://github.com/MoonshotAI/kimi-cli) в неинтерактивном режиме (`--afk --yolo`)
   - AI анализирует текст и изображения, генерирует структурированную документацию

## Зависимости

### Системные зависимости сборки

| Компонент | Минимальная версия | Назначение |
|-----------|-------------------|------------|
| GCC / Clang / MSVC | C++20 support | Компилятор |
| CMake | 3.16 | Система сборки |
| Ninja | любая | Генератор сборки (рекомендуется) |
| wxWidgets | 3.2 | GUI-фреймворк |

### Внешние программы (не Python)

| Программа | Минимальная версия | Назначение | Установка |
|-----------|-------------------|------------|-----------|
| **ffmpeg** | 5.0 | Извлечение кадров и аудио из видео | Системный пакет (`apt`, `pacman`, или скачать с ffmpeg.org для Windows) |

### Python-зависимости

Устанавливаются через `pip` из файла `requirements.txt`:

```bash
pip install -r requirements.txt
```

| Пакет | Назначение |
|-------|-----------|
| **faster-whisper** | Распознавание речи на базе CTranslate2 (Этап 1 — Транскрибация) |
| **kimi-cli** | Kimi Code CLI — AI-агент для генерации документации (Этап 2) |

---

### Установка зависимостей

#### Ubuntu / Debian

```bash
# 1. Системные зависимости сборки
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libwxgtk3.2-dev \
    libwxgtk-webview3.2-dev \
    libgtk-3-dev \
    libglib2.0-dev

# 2. ffmpeg
sudo apt-get install -y ffmpeg

# 3. Python + pip (если ещё не установлены)
sudo apt-get install -y python3 python3-pip

# 4. Python-зависимости (faster-whisper + kimi-cli)
pip3 install -r requirements.txt
```

#### Arch Linux

```bash
# 1. Системные зависимости сборки
sudo pacman -S \
    base-devel \
    cmake \
    ninja \
    wxwidgets-gtk3 \
    gtk3 \
    glib2

# 2. ffmpeg
sudo pacman -S ffmpeg

# 3. Python + pip (если ещё не установлены)
sudo pacman -S python python-pip

# 4. Python-зависимости (faster-whisper + kimi-cli)
pip install -r requirements.txt --break-system-packages
```

#### Windows (MSYS2 / MinGW)

```bash
# 1. Установите MSYS2: https://www.msys2.org/

# 2. В MSYS2 UCRT64 терминале:
pacman -S \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-wxwidgets3.2-msw

# 3. ffmpeg — скачайте с https://ffmpeg.org/download.html#build-windows
#    и добавьте bin/ в PATH

# 4. Python — установите с https://python.org/downloads/windows/
#    (обязательно поставьте галочку "Add Python to PATH")

# 5. Python-зависимости (faster-whisper + kimi-cli)
pip install -r requirements.txt
```

> **Примечание:** Для Windows рекомендуется использовать готовый Docker-образ кросс-компиляции (см. раздел «Сборка в Docker»).

---

### Проверка зависимостей

```bash
# wxWidgets (Linux)
wx-config --version

# ffmpeg
ffmpeg -version | head -1

# faster-whisper
python3 -c "import faster_whisper; print('OK')"

# kimi
kimi --version
```

## Сборка

### Локальная сборка

```bash
# Из корня репозитория
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# Бинарник
./bin/Video2Doc
```

#### Debug-сборка
```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja
```

#### Установка
```bash
ninja install  # INSTALL_PREFIX по умолчанию /usr/local
```

### Сборка в Docker

В каталоге `docker/` подготовлены образы для кросс-платформенной сборки.

#### Linux (из публичного репозитория)
```bash
cd docker
docker build -t video2doc .
docker run --rm --name video2doc-build video2doc
# Бинарник внутри контейнера: /devel/video2doc-build/bin/Video2Doc
```

#### Linux (из локальных исходников)
```bash
cd docker
./run.sh
# Результат в docker/output/
```

#### Windows (кросс-компиляция MinGW)
```bash
cd docker
./build-win.sh
# Результат: docker/output/Video2Doc-win64-YYYYMMDD-HHMM.zip
```

Подробности — в `docker/README_DOCKER.md`.

## Использование

1. **Выберите видеофайл** — MP4, MKV, AVI и другие форматы, поддерживаемые ffmpeg
2. **Укажите выходную папку** — туда будут сохранены скриншоты, транскрипция и итоговый документ
3. **Настройте интервал скриншотов** — в секундах (по умолчанию 1 сек)
4. *(Опционально)* **Добавьте дополнительные проекты** — пути к codebase, которые Kimi проанализирует при генерации документации
5. **Отредактируйте шаблон промпта** — переменные `{VIDEO_PATH}`, `{OUT_DIR}`, `{SCREENSHOTS_DIR}`, `{PROJECT_NAME}`, `{PROJECTS_LIST}` подставляются автоматически
6. **Запустите**:
   - **Этап 1** — только транскрибация
   - **Этап 2** — только генерация документации (требуются артефакты Этапа 1)
   - **Полный цикл** — Этап 1 → Этап 2 подряд

### Файлы артефактов

После Этапа 1 в выходной папке создаётся:
```
<outDir>/
  screenshots/        # PNG-кадры из видео
  transcription.txt   # Распознанный текст
  promt.md            # Сгенерированный промпт для Kimi
  _run_whisper.py     # Вспомогательный скрипт (временный)
```

После Этапа 2:
```
<outDir>/doc/
  *.md               # Сгенерированная документация
  image/             # Скопированные изображения, встроенные в документ
```

## Архитектура

```
src/
  main.cpp              # Точка входа wxWidgets
  MainFrame.{h,cpp}     # Главное окно, меню, статус-бар, FSM состояний
  SettingsPanel.{h,cpp} # Ввод пути к видео, интервал, выходная папка
  ProjectListPanel.{h,cpp}   # Список дополнительных проектов для анализа
  PromptEditorPanel.{h,cpp}  # Редактор шаблона промпта с подсветкой переменных
  ControlLogPanel.{h,cpp}    # Кнопки запуска/отмены + журнал выполнения
  ProcessRunner.{h,cpp}      # Асинхронный запуск процессов через wxProcess
  TranscriptionEngine.{h,cpp}# Этап 1: ffmpeg + faster_whisper
  DocGenerator.{h,cpp}       # Этап 2: подготовка промпта + Kimi CLI
  ConfigManager.{h,cpp}      # Сохранение/загрузка настроек
  PathValidator.{h,cpp}      # Утилиты для работы с путями
```

### Особенности реализации

- **Асинхронные процессы** — `ProcessRunner` использует `wxProcess` + `wxTimer` для неблокирующего чтения stdout/stderr. Главный поток GTK никогда не блокируется.
- **Закрытие stdin** — после запуска дочернего процесса `CloseOutput()` закрывает pipe stdin. Это критично для Kimi CLI, который ожидает EOF для завершения в неинтерактивном режиме.
- **Реентерабельность** — колбэк `OnTerminate` откладывается через `CallAfter` на следующую итерацию event loop, чтобы избежать deadlock при запуске нового процесса из обработчика завершения предыдущего.

## Известные проблемы и решения

### Зависание GUI при запуске faster-whisper
**Причина:** CTranslate2 (бэкенд faster_whisper) порождает воркер-процессы, которые наследуют stdout/stderr пайпы `wxProcess`. Блокирующее чтение на главном потоке приводит к deadlock в GTK event loop.

**Решение:**
- `PollOutput()` работает по таймеру (`wxTimer`) с неблокирующим `CanRead()` + `Read(buf, 1024)`
- Колбэк завершения откладывается через `CallAfter`
- Убран флаг `wxEXEC_MAKE_GROUP_LEADER`
- Python-скрипт запускается с `-u` и `PYTHONUNBUFFERED=1`

### Ошибка `No such option '--prompt-file'`
**Причина:** В разных версиях Kimi CLI синтаксис различается.

**Решение:** Используется `--prompt "<content>"` с передачей содержимого файла через `wxArrayString`, что корректно обрабатывает многострочные строки.

## Лицензия

MIT License — см. файл `LICENSE` (если присутствует).
