#include "ControlLogPanel.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <algorithm>

ControlLogPanel::ControlLogPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Control buttons
    wxStaticBox* controlBox = new wxStaticBox(this, wxID_ANY, wxT("Управление процессом"));
    wxStaticBoxSizer* controlSizer = new wxStaticBoxSizer(controlBox, wxVERTICAL);
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

    m_stage1Btn = new wxButton(controlBox, wxID_ANY, wxT("Запустить Этап 1 — Транскрибацию"));
    btnSizer->Add(m_stage1Btn, 0, wxALL, 5);

    m_stage2Btn = new wxButton(controlBox, wxID_ANY, wxT("Запустить Этап 2 — Генерацию документации"));
    m_stage2Btn->Enable(false);
    btnSizer->Add(m_stage2Btn, 0, wxALL, 5);

    m_fullCycleBtn = new wxButton(controlBox, wxID_ANY, wxT("Запустить полный цикл (Этап 1 + Этап 2)"));
    btnSizer->Add(m_fullCycleBtn, 0, wxALL, 5);

    m_cancelBtn = new wxButton(controlBox, wxID_CANCEL, wxT("Отмена"));
    m_cancelBtn->Enable(false);
    btnSizer->Add(m_cancelBtn, 0, wxALL, 5);

    controlSizer->Add(btnSizer, 0, wxALIGN_CENTER_HORIZONTAL);
    mainSizer->Add(controlSizer, 0, wxEXPAND | wxALL, 5);

    // Progress
    wxBoxSizer* progressSizer = new wxBoxSizer(wxHORIZONTAL);
    m_gauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
    progressSizer->Add(m_gauge, 1, wxEXPAND | wxALL, 5);
    m_statusLabel = new wxStaticText(this, wxID_ANY, wxT("Готово к работе"));
    progressSizer->Add(m_statusLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    mainSizer->Add(progressSizer, 0, wxEXPAND);

    // Log
    wxStaticBox* logBox = new wxStaticBox(this, wxID_ANY, wxT("Журнал выполнения"));
    wxStaticBoxSizer* logSizer = new wxStaticBoxSizer(logBox, wxVERTICAL);
    m_log = new wxTextCtrl(logBox, wxID_ANY, wxT(""),
        wxDefaultPosition, wxSize(-1, 200),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_DONTWRAP);
    wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
    m_log->SetFont(font);
    logSizer->Add(m_log, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(logSizer, 1, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
}

void ControlLogPanel::AppendLog(const wxString& text) {
    m_log->AppendText(text);
    // Auto-scroll
    m_log->ShowPosition(m_log->GetLastPosition());
}

void ControlLogPanel::ClearLog() {
    m_log->Clear();
}

void ControlLogPanel::SetProgress(int percent) {
    m_gauge->SetValue(std::clamp(percent, 0, 100));
}

void ControlLogPanel::SetStatus(const wxString& status) {
    m_statusLabel->SetLabel(status);
}
