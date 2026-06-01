#pragma once

#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/statusbr.h>

class SettingsPanel;
class ProjectListPanel;
class PromptEditorPanel;
class ControlLogPanel;
class TranscriptionEngine;
class DocGenerator;

enum AppState {
    STATE_IDLE,
    STATE_STAGE1_RUNNING,
    STATE_STAGE2_RUNNING,
    STATE_FULL_CYCLE_STAGE1,
    STATE_FULL_CYCLE_STAGE2
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    ~MainFrame();

private:
    void CreateMenuBar();
    void CreateAppStatusBar();
    void LayoutPanels();
    void LoadSettings();
    void SaveSettings();

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSaveConfig(wxCommandEvent& event);
    void OnLoadConfig(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnRunStage1(wxCommandEvent& event);
    void OnRunStage2(wxCommandEvent& event);
    void OnRunFullCycle(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void OnStage1Done(bool success, const wxString& message);
    void OnStage2Done(bool success, const wxString& message);

    void OnPromptSave(const wxString& text);
    void OnPromptReset();

    void SetAppState(AppState state);
    void UpdateUIState();
    bool CheckStage1Artifacts(const wxString& outDir);

    AppState m_state;
    bool m_stage1Done;

    SettingsPanel* m_settingsPanel;
    ProjectListPanel* m_projectPanel;
    PromptEditorPanel* m_promptPanel;
    ControlLogPanel* m_controlPanel;

    TranscriptionEngine* m_transcriptionEngine;
    DocGenerator* m_docGenerator;
};
