# Справочник по компонентам wxWidgets

> Данный справочник составлен с использованием официальной документации wxWidgets (версия 3.2+) и адаптирован под задачи проекта Video2Doc.

---

## 1. Основные классы приложения

### 1.1. `wxApp` — Точка входа

Каждое wxWidgets-приложение начинается с наследника `wxApp`:

```cpp
#include <wx/wx.h>

class Video2DocApp : public wxApp {
public:
    virtual bool OnInit() override {
        MainFrame* frame = new MainFrame("Video2Doc");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(Video2DocApp);
```

**Ключевые методы:**
- `OnInit()` — инициализация приложения, создание главного окна.
- `OnExit()` — очистка ресурсов перед завершением.

---

### 1.2. `wxFrame` — Главное окно

`wxFrame` — основной тип окна в wxWidgets. Поддерживает изменение размера пользователем, имеет заголовок, границы, и может содержать меню, панель инструментов и статусную строку.

```cpp
class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title)
        : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800)) {
        
        // Создание меню
        wxMenuBar* menuBar = new wxMenuBar();
        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(wxID_EXIT, _("Выход\tAlt+F4"));
        menuBar->Append(fileMenu, _("Файл"));
        SetMenuBar(menuBar);
        
        // Статусная строка
        CreateStatusBar(2);
        SetStatusText(_("Готово к работе"), 0);
        SetStatusText(_("v1.0.0"), 1);
    }
};
```

**Важные особенности:**
- `CreateStatusBar()` — создаёт строку состояния в нижней части окна.
- `CreateToolBar()` — создаёт панель инструментов.
- `GetClientSize()` возвращает размер клиентской области с учётом меню, статус-бара и тулбара.

---

## 2. Контейнеры и компоновка

### 2.1. `wxPanel` — Панель

`wxPanel` используется для группировки связанных элементов управления. В Video2Doc каждая логическая секция (настройки, проекты, промпт, лог) размещается на отдельной панели.

```cpp
wxPanel* settingsPanel = new wxPanel(this, wxID_ANY);
wxPanel* logPanel = new wxPanel(this, wxID_ANY);
```

### 2.2. `wxBoxSizer` — Компоновщик

`wxBoxSizer` управляет расположением дочерних окон в одном направлении (горизонтально `wxHORIZONTAL` или вертикально `wxVERTICAL`).

```cpp
wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

rowSizer->Add(new wxStaticText(panel, wxID_ANY, _("Видео:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
rowSizer->Add(videoPathText, 1, wxEXPAND | wxALL, 5);
rowSizer->Add(browseButton, 0, wxALL, 5);

mainSizer->Add(rowSizer, 0, wxEXPAND);
mainSizer->Add(logTextCtrl, 1, wxEXPAND | wxALL, 5);

panel->SetSizer(mainSizer);
```

**Флаги компоновки:**
- `wxEXPAND` — элемент растягивается в доступном пространстве.
- `wxALL`, `wxLEFT`, `wxRIGHT`, `wxTOP`, `wxBOTTOM` — отступы.
- Второй параметр `Add()` — пропорция (0 = фиксированный размер, 1+ = растягивается пропорционально).

---

## 3. Элементы управления (Controls)

### 3.1. `wxTextCtrl` — Текстовое поле

Универсальный текстовый редактор. Поддерживает однострочный и многострочный режимы.

```cpp
// Однострочное поле (путь к файлу)
wxTextCtrl* pathText = new wxTextCtrl(panel, wxID_ANY, "",
    wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

// Многострочное поле (редактор промпта / лог)
wxTextCtrl* logText = new wxTextCtrl(panel, wxID_ANY, "",
    wxDefaultPosition, wxSize(-1, 200),
    wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);

// Добавление текста в лог
logText->AppendText(wxString::Format("> %s\n", output));
```

**Стиль для редактора промпта:**
```cpp
wxTE_MULTILINE | wxTE_WORDWRAP
```

**Стиль для лога:**
```cpp
wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_DONTWRAP
```

---

### 3.2. `wxButton` — Кнопка

```cpp
wxButton* runButton = new wxButton(panel, wxID_ANY, _("Запустить"));
wxButton* cancelButton = new wxButton(panel, wxID_CANCEL, _("Отмена"));

// Привязка события
runButton->Bind(wxEVT_BUTTON, &MainFrame::OnRunStage1, this);
```

**Стандартные идентификаторы:**
- `wxID_OK`, `wxID_CANCEL`, `wxID_ADD`, `wxID_REMOVE`, `wxID_EXIT`

---

### 3.3. `wxSpinCtrl` — Числовое поле со стрелками

Идеально подходит для ввода интервала скриншотов.

```cpp
wxSpinCtrl* intervalSpin = new wxSpinCtrl(panel, wxID_ANY, "1",
    wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 3600, 1);

// Получение значения
int interval = intervalSpin->GetValue();
```

---

### 3.4. `wxListBox` — Список элементов

Для списка дополнительных проектов.

```cpp
wxListBox* projectList = new wxListBox(panel, wxID_ANY,
    wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);

// Добавление
projectList->Append(_("/home/user/projectA"));

// Удаление выбранного
int sel = projectList->GetSelection();
if (sel != wxNOT_FOUND) {
    projectList->Delete(sel);
}

// Получение всех элементов
wxArrayString projects;
for (unsigned int i = 0; i < projectList->GetCount(); ++i) {
    projects.Add(projectList->GetString(i));
}
```

---

### 3.5. `wxGauge` — Индикатор прогресса

```cpp
wxGauge* progressGauge = new wxGauge(panel, wxID_ANY, 100,
    wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);

progressGauge->SetValue(0);   // начало
progressGauge->SetValue(45);  // 45%
progressGauge->SetValue(100); // завершено
```

---

### 3.6. `wxStaticText` — Текстовая метка

```cpp
wxStaticText* label = new wxStaticText(panel, wxID_ANY, _("Интервал (сек):"));
```

---

## 4. Диалоговые окна

### 4.1. `wxFileDialog` — Выбор файла

```cpp
wxFileDialog openFileDialog(this, _("Выберите видеофайл"), "", "",
    _("Видео файls (*.mp4;*.mkv;*.avi;*.mov)|*.mp4;*.mkv;*.avi;*.mov|Все файлы (*.*)|*.*"),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST);

if (openFileDialog.ShowModal() == wxID_OK) {
    wxString videoPath = openFileDialog.GetPath();
    videoPathText->SetValue(videoPath);
}
```

---

### 4.2. `wxDirDialog` — Выбор папки

```cpp
wxDirDialog dirDialog(this, _("Выберите выходную папку"), defaultPath,
    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

if (dirDialog.ShowModal() == wxID_OK) {
    wxString outDir = dirDialog.GetPath();
    outDirText->SetValue(outDir);
}
```

---

### 4.3. `wxMessageBox` — Сообщение

```cpp
// Информация
wxMessageBox(_("Этап 1 завершён успешно!"), _("Готово"), wxOK | wxICON_INFORMATION);

// Ошибка
wxMessageBox(_("Файл не найден: ") + path, _("Ошибка"), wxOK | wxICON_ERROR);

// Подтверждение
int answer = wxMessageBox(_("Перезаписать существующие файлы?"),
    _("Подтверждение"), wxYES_NO | wxICON_QUESTION);
if (answer == wxYES) { /* ... */ }
```

---

## 5. Работа с процессами (`wxProcess`)

Запуск внешних инструментов (ffmpeg, whisper, kimi) выполняется через `wxProcess` с перехватом вывода.

```cpp
class ProcessRunner : public wxProcess {
public:
    ProcessRunner(wxTextCtrl* logCtrl, wxGauge* gauge)
        : wxProcess(wxPROCESS_REDIRECT), m_log(logCtrl), m_gauge(gauge) {}
    
    // Вызывается при завершении процесса
    virtual void OnTerminate(int pid, int status) override {
        wxMessageQueue().Post(wxString::Format(_("Процесс завершён с кодом %d"), status));
    }
    
    // Чтение вывода
    void PollOutput() {
        if (IsInputAvailable()) {
            wxTextInputStream tis(*GetInputStream());
            wxString line = tis.ReadLine();
            m_log->AppendText(line + "\n");
        }
    }
    
private:
    wxTextCtrl* m_log;
    wxGauge* m_gauge;
};

// Запуск процесса
wxProcess* process = new ProcessRunner(logText, gauge);
wxExecute("ffmpeg -i video.mp4 -vf fps=1/1 out/frame_%04d.jpg",
    wxEXEC_ASYNC | wxEXEC_MAKE_GROUP_LEADER, process);
```

**Важные флаги `wxExecute`:**
- `wxEXEC_ASYNC` — асинхронный запуск (не блокирует GUI).
- `wxEXEC_MAKE_GROUP_LEADER` — позволяет убить дочерние процессы.
- `wxPROCESS_REDIRECT` — перехват stdout/stderr.

---

## 6. Сохранение настроек (`wxConfig`)

```cpp
// Сохранение
wxConfig config("Video2Doc", "MyCompany");
config.Write("VideoPath", videoPath);
config.Write("Interval", interval);
config.Write("OutDir", outDir);

// Чтение
wxString videoPath = config.Read("VideoPath", "");
int interval = config.Read("Interval", 1);
```

---

## 7. Полезные макросы и константы

| Константа | Описание |
|-----------|----------|
| `wxID_ANY` | Автоматическое назначение идентификатора |
| `wxDefaultPosition` | Позиция по умолчанию |
| `wxDefaultSize` | Размер по умолчанию |
| `wxEXPAND` | Растягивание в sizer |
| `wxALL`, `wxLEFT`, `wxRIGHT`, `wxTOP`, `wxBOTTOM` | Флаги границ |
| `wxALIGN_CENTER`, `wxALIGN_LEFT`, `wxALIGN_RIGHT` | Выравнивание |

---

## 8. Сборка проекта (CMake)

```cmake
cmake_minimum_required(VERSION 3.16)
project(Video2Doc)

set(CMAKE_CXX_STANDARD 17)

# Поиск wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(Video2Doc
    src/main.cpp
    src/MainFrame.cpp
    src/MainFrame.h
    src/SettingsPanel.cpp
    src/SettingsPanel.h
    src/ProcessRunner.cpp
    src/ProcessRunner.h
    src/ConfigManager.cpp
    src/ConfigManager.h
)

target_link_libraries(Video2Doc ${wxWidgets_LIBRARIES})
```

---

## 9. Ссылки

- [wxWidgets Official Site](https://www.wxwidgets.org/)
- [wxWidgets Online Manual](https://docs.wxwidgets.org/3.2/)
- [wxWidgets GitHub Repository](https://github.com/wxwidgets/wxwidgets)
- Context7: `/wxwidgets/wxwidgets` — актуальная документация и примеры кода.
