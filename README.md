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

### Системные
- **C++20 компилятор** (GCC 16+, Clang 18+, MSVC 2022+)
- **CMake** ≥ 3.16
- **wxWidgets** ≥ 3.2 (GTK3-бэкенд на Linux)
- **Ninja** или Make

### Внешние инструменты
- **ffmpeg** ≥ 5.0 — извлечение кадров и аудио
- **Python** ≥ 3.10 + `faster-whisper` — транскрибация речи
- **Kimi Code CLI** — генерация документации (`pip install kimi-cli`)

### Проверка зависимостей
```bash
# wxWidgets
cmake --find-package -DNAME=wxWidgets -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST

# ffmpeg
ffmpeg -version | head -1

# faster-whisper
python3 -c "import faster_whisper; print('OK')"

# kimi
kimi --version
```

## Сборка

```bash
# Из корня репозитория
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# Бинарник
./bin/Video2Doc
```

### Debug-сборка
```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja
```

### Установка
```bash
ninja install  # INSTALL_PREFIX по умолчанию /usr/local
```

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
