#include "MainFrame.h"
#include "SettingsPanel.h"
#include "ProjectListPanel.h"
#include "PromptEditorPanel.h"
#include "ControlLogPanel.h"
#include "TranscriptionEngine.h"
#include "DocGenerator.h"
#include "ConfigManager.h"
#include "PathValidator.h"
#include "DependencyChecker.h"

#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>
#include <wx/filefn.h>
#include <wx/file.h>

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800))
    , m_state(STATE_IDLE)
    , m_stage1Done(false)
    , m_settingsPanel(nullptr)
    , m_projectPanel(nullptr)
    , m_promptPanel(nullptr)
    , m_controlPanel(nullptr)
    , m_transcriptionEngine(nullptr)
    , m_docGenerator(nullptr)
{
    SetMinSize(wxSize(900, 600));

    CreateMenuBar();
    CreateAppStatusBar();
    LayoutPanels();

    m_transcriptionEngine = new TranscriptionEngine(
        m_controlPanel->GetLogCtrl(),
        m_controlPanel->GetGauge());

    m_docGenerator = new DocGenerator(
        m_controlPanel->GetLogCtrl(),
        m_controlPanel->GetGauge());

    LoadSettings();
    UpdateUIState();

    // Проверка зависимостей при старте (с небольшой задержкой, чтобы окно отрисовалось)
    CallAfter([this]() {
        ShowDependencyCheckDialog(this);
    });

    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
}

MainFrame::~MainFrame() {
    delete m_transcriptionEngine;
    delete m_docGenerator;
}

void MainFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_EXIT, wxT("Выход\tAlt+F4"));
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
    menuBar->Append(fileMenu, wxT("Файл"));

    wxMenu* settingsMenu = new wxMenu();
    settingsMenu->Append(wxID_SAVE, wxT("Сохранить конфигурацию"));
    settingsMenu->Append(wxID_OPEN, wxT("Загрузить конфигурацию"));
    Bind(wxEVT_MENU, &MainFrame::OnSaveConfig, this, wxID_SAVE);
    Bind(wxEVT_MENU, &MainFrame::OnLoadConfig, this, wxID_OPEN);
    menuBar->Append(settingsMenu, wxT("Настройки"));

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, wxT("О программе"));
    Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
    menuBar->Append(helpMenu, wxT("Справка"));

    SetMenuBar(menuBar);
}

void MainFrame::CreateAppStatusBar() {
    CreateStatusBar(2);
    SetStatusText(wxT("Готово к работе"), 0);
    SetStatusText(wxT("v1.0.0"), 1);
}

void MainFrame::LayoutPanels() {
    wxPanel* mainPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Settings
    m_settingsPanel = new SettingsPanel(mainPanel);
    mainSizer->Add(m_settingsPanel, 0, wxEXPAND | wxALL, 5);

    // Middle: Projects + Prompt
    wxBoxSizer* midSizer = new wxBoxSizer(wxHORIZONTAL);
    m_projectPanel = new ProjectListPanel(mainPanel);
    midSizer->Add(m_projectPanel, 1, wxEXPAND | wxALL, 5);

    m_promptPanel = new PromptEditorPanel(mainPanel,
        [this](const wxString& text) { OnPromptSave(text); },
        [this]() { OnPromptReset(); });
    midSizer->Add(m_promptPanel, 2, wxEXPAND | wxALL, 5);
    mainSizer->Add(midSizer, 1, wxEXPAND);

    // Control + Log
    m_controlPanel = new ControlLogPanel(mainPanel);
    m_controlPanel->GetStage1Button()->Bind(wxEVT_BUTTON, &MainFrame::OnRunStage1, this);
    m_controlPanel->GetStage2Button()->Bind(wxEVT_BUTTON, &MainFrame::OnRunStage2, this);
    m_controlPanel->GetFullCycleButton()->Bind(wxEVT_BUTTON, &MainFrame::OnRunFullCycle, this);
    m_controlPanel->GetCancelButton()->Bind(wxEVT_BUTTON, &MainFrame::OnCancel, this);
    mainSizer->Add(m_controlPanel, 1, wxEXPAND | wxALL, 5);

    mainPanel->SetSizer(mainSizer);
}

void MainFrame::LoadSettings() {
    wxString videoPath = ConfigManager::GetLastVideoPath();
    wxString outDir = ConfigManager::GetLastOutputDir();
    int interval = ConfigManager::GetScreenshotInterval();
    wxArrayString projects = ConfigManager::GetProjects();
    wxString promptTpl = ConfigManager::GetPromptTemplate();
    wxSize winSize = ConfigManager::GetWindowSize();
    bool maximized = ConfigManager::GetWindowMaximized();

    if (!videoPath.IsEmpty()) {
        m_settingsPanel->SetVideoPath(videoPath);
    }
    if (!outDir.IsEmpty()) {
        m_settingsPanel->SetOutputDir(outDir);
    }
    m_settingsPanel->SetInterval(interval);
    m_projectPanel->SetProjects(projects);
    m_promptPanel->SetTemplate(promptTpl);

    SetSize(winSize);
    if (maximized) {
        Maximize(true);
    }

    // Check if Stage 1 artifacts exist for current output dir
    if (!outDir.IsEmpty() && CheckStage1Artifacts(outDir)) {
        m_stage1Done = true;
    }
}

void MainFrame::SaveSettings() {
    ConfigManager::SetLastVideoPath(m_settingsPanel->GetVideoPath());
    ConfigManager::SetLastOutputDir(m_settingsPanel->GetOutputDir());
    ConfigManager::SetScreenshotInterval(m_settingsPanel->GetInterval());
    ConfigManager::SetProjects(m_projectPanel->GetProjects());
    ConfigManager::SetPromptTemplate(m_promptPanel->GetTemplate());

    wxSize size = GetSize();
    if (!IsMaximized()) {
        ConfigManager::SetWindowSize(size);
    }
    ConfigManager::SetWindowMaximized(IsMaximized());
}

void MainFrame::OnExit(wxCommandEvent& /*event*/) {
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& /*event*/) {
    wxAboutDialogInfo info;
    info.SetName(wxT("Video2Doc"));
    info.SetVersion(wxT("1.0.0"));
    info.SetDescription(wxT("Автоматизированная генерация документации из видео"));
    info.SetCopyright(wxT("(C) 2025"));
    wxAboutBox(info);
}

void MainFrame::OnSaveConfig(wxCommandEvent& /*event*/) {
    SaveSettings();
    wxMessageBox(wxT("Конфигурация сохранена"), wxT("Готово"), wxOK | wxICON_INFORMATION);
}

void MainFrame::OnLoadConfig(wxCommandEvent& /*event*/) {
    LoadSettings();
    wxMessageBox(wxT("Конфигурация загружена"), wxT("Готово"), wxOK | wxICON_INFORMATION);
}

void MainFrame::OnClose(wxCloseEvent& event) {
    SaveSettings();
    if (m_transcriptionEngine->IsRunning() || m_docGenerator->IsRunning()) {
        int answer = wxMessageBox(wxT("Процесс выполняется. Прервать и выйти?"),
            wxT("Подтверждение"), wxYES_NO | wxICON_QUESTION);
        if (answer == wxYES) {
            m_transcriptionEngine->Cancel();
            m_docGenerator->Cancel();
            event.Skip();
        } else {
            event.Veto();
        }
    } else {
        event.Skip();
    }
}

void MainFrame::OnRunStage1(wxCommandEvent& /*event*/) {
    wxString videoPath = m_settingsPanel->GetVideoPath();
    wxString outDir = m_settingsPanel->GetOutputDir();
    int interval = m_settingsPanel->GetInterval();

    if (!PathValidator::IsValidVideoFile(videoPath)) {
        wxMessageBox(wxT("Выберите корректный видеофайл"), wxT("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    if (outDir.IsEmpty()) {
        outDir = PathValidator::GetDefaultOutputDir(videoPath);
        m_settingsPanel->SetOutputDir(outDir);
    }

    m_controlPanel->ClearLog();
    SetAppState(STATE_STAGE1_RUNNING);

    m_transcriptionEngine->Start(videoPath, outDir, interval,
        [this](bool success, const wxString& msg) {
            OnStage1Done(success, msg);
        });
}

void MainFrame::OnRunStage2(wxCommandEvent& /*event*/) {
    wxString videoPath = m_settingsPanel->GetVideoPath();
    wxString outDir = m_settingsPanel->GetOutputDir();
    wxString tpl = m_promptPanel->GetTemplate();
    wxArrayString projects = m_projectPanel->GetProjects();

    if (!PathValidator::IsValidVideoFile(videoPath)) {
        wxMessageBox(wxT("Выберите корректный видеофайл"), wxT("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    if (outDir.IsEmpty()) {
        outDir = PathValidator::GetDefaultOutputDir(videoPath);
        m_settingsPanel->SetOutputDir(outDir);
    }

    m_controlPanel->ClearLog();
    SetAppState(STATE_STAGE2_RUNNING);

    m_docGenerator->Start(videoPath, outDir, tpl, projects,
        [this](bool success, const wxString& msg) {
            OnStage2Done(success, msg);
        });
}

void MainFrame::OnRunFullCycle(wxCommandEvent& /*event*/) {
    wxString videoPath = m_settingsPanel->GetVideoPath();
    wxString outDir = m_settingsPanel->GetOutputDir();
    int interval = m_settingsPanel->GetInterval();

    if (!PathValidator::IsValidVideoFile(videoPath)) {
        wxMessageBox(wxT("Выберите корректный видеофайл"), wxT("Ошибка"), wxOK | wxICON_ERROR);
        return;
    }

    if (outDir.IsEmpty()) {
        outDir = PathValidator::GetDefaultOutputDir(videoPath);
        m_settingsPanel->SetOutputDir(outDir);
    }

    m_controlPanel->ClearLog();
    SetAppState(STATE_FULL_CYCLE_STAGE1);

    m_transcriptionEngine->Start(videoPath, outDir, interval,
        [this](bool success, const wxString& msg) {
            OnStage1Done(success, msg);
        });
}

void MainFrame::OnCancel(wxCommandEvent& /*event*/) {
    if (m_transcriptionEngine->IsRunning()) {
        m_transcriptionEngine->Cancel();
        m_controlPanel->AppendLog(wxT("⛔ Транскрибация отменена\n"));
    }
    if (m_docGenerator->IsRunning()) {
        m_docGenerator->Cancel();
        m_controlPanel->AppendLog(wxT("⛔ Генерация документации отменена\n"));
    }
    SetAppState(STATE_IDLE);
}

void MainFrame::OnStage1Done(bool success, const wxString& /*message*/) {
    if (!success) {
        wxMessageBox(wxT("Этап 1 завершился с ошибкой"), wxT("Ошибка"), wxOK | wxICON_ERROR);
        SetAppState(STATE_IDLE);
        return;
    }

    m_stage1Done = true;
    SaveSettings();

    if (m_state == STATE_FULL_CYCLE_STAGE1) {
        wxString videoPath = m_settingsPanel->GetVideoPath();
        wxString outDir = m_settingsPanel->GetOutputDir();
        wxString tpl = m_promptPanel->GetTemplate();
        wxArrayString projects = m_projectPanel->GetProjects();

        SetAppState(STATE_FULL_CYCLE_STAGE2);
        m_docGenerator->Start(videoPath, outDir, tpl, projects,
            [this](bool success, const wxString& msg) {
                OnStage2Done(success, msg);
            });
    } else {
        wxMessageBox(wxT("Этап 1 завершён успешно!"), wxT("Готово"), wxOK | wxICON_INFORMATION);
        SetAppState(STATE_IDLE);
    }
}

void MainFrame::OnStage2Done(bool success, const wxString& /*message*/) {
    if (!success) {
        wxMessageBox(wxT("Этап 2 завершился с ошибкой"), wxT("Ошибка"), wxOK | wxICON_ERROR);
    } else {
        wxMessageBox(wxT("Документация сгенерирована!"), wxT("Готово"), wxOK | wxICON_INFORMATION);
    }
    SaveSettings();
    SetAppState(STATE_IDLE);
}

void MainFrame::OnPromptSave(const wxString& text) {
    ConfigManager::SetPromptTemplate(text);

    wxString outDir = m_settingsPanel->GetOutputDir();
    if (!outDir.IsEmpty()) {
        PathValidator::EnsureDirExists(outDir);
        wxString promptFile = outDir + wxT("/promt.md");
        wxFile file(promptFile, wxFile::write);
        if (file.IsOpened()) {
            file.Write(text, wxConvUTF8);
            file.Close();
            m_controlPanel->AppendLog(wxT("📝 Шаблон сохранён в ") + promptFile + wxT("\n"));
        }
    }
    wxMessageBox(wxT("Шаблон промпта сохранён"), wxT("Готово"), wxOK | wxICON_INFORMATION);
}

void MainFrame::OnPromptReset() {
    wxString tpl = ConfigManager::GetDefaultPromptTemplate();
    m_promptPanel->SetTemplate(tpl);
    ConfigManager::SetPromptTemplate(tpl);
    wxMessageBox(wxT("Шаблон сброшен к значению по умолчанию"), wxT("Готово"), wxOK | wxICON_INFORMATION);
}

void MainFrame::SetAppState(AppState state) {
    m_state = state;
    UpdateUIState();
}

void MainFrame::UpdateUIState() {
    bool running = (m_state != STATE_IDLE);
    bool canRunStage2 = m_stage1Done || CheckStage1Artifacts(m_settingsPanel->GetOutputDir());

    m_settingsPanel->Enable(!running);
    m_projectPanel->Enable(!running);
    m_promptPanel->Enable(!running);

    m_controlPanel->GetStage1Button()->Enable(!running);
    m_controlPanel->GetStage2Button()->Enable(!running && canRunStage2);
    m_controlPanel->GetFullCycleButton()->Enable(!running);
    m_controlPanel->GetCancelButton()->Enable(running);

    switch (m_state) {
        case STATE_STAGE1_RUNNING:
        case STATE_FULL_CYCLE_STAGE1:
            SetStatusText(wxT("Выполняется Этап 1..."), 0);
            break;
        case STATE_STAGE2_RUNNING:
        case STATE_FULL_CYCLE_STAGE2:
            SetStatusText(wxT("Выполняется Этап 2..."), 0);
            break;
        default:
            SetStatusText(wxT("Готово к работе"), 0);
            break;
    }
}

bool MainFrame::CheckStage1Artifacts(const wxString& outDir) {
    if (outDir.IsEmpty()) return false;
    wxString screenshotsDir = outDir + wxT("/screenshots");
    wxString transcriptFile = outDir + wxT("/transcript.txt");
    return wxDirExists(screenshotsDir) && wxFileExists(transcriptFile);
}
