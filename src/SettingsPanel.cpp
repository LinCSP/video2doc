#include "SettingsPanel.h"
#include "PathValidator.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/filename.h>

SettingsPanel::SettingsPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT("Исходное видео и параметры"));
    wxStaticBoxSizer* mainSizer = new wxStaticBoxSizer(box, wxVERTICAL);

    // Video path row
    wxBoxSizer* videoSizer = new wxBoxSizer(wxHORIZONTAL);
    videoSizer->Add(new wxStaticText(box, wxID_ANY, wxT("Видео:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_videoPath = new wxTextCtrl(box, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    videoSizer->Add(m_videoPath, 1, wxEXPAND | wxALL, 5);
    wxButton* browseVideoBtn = new wxButton(box, wxID_ANY, wxT("Обзор..."));
    browseVideoBtn->Bind(wxEVT_BUTTON, &SettingsPanel::OnBrowseVideo, this);
    videoSizer->Add(browseVideoBtn, 0, wxALL, 5);
    mainSizer->Add(videoSizer, 0, wxEXPAND);

    // Interval and output dir row
    wxBoxSizer* paramsSizer = new wxBoxSizer(wxHORIZONTAL);
    paramsSizer->Add(new wxStaticText(box, wxID_ANY, wxT("Интервал скриншотов (сек):")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_interval = new wxSpinCtrl(box, wxID_ANY, wxT("1"), wxDefaultPosition, wxSize(80, -1), wxSP_ARROW_KEYS, 1, 3600, 1);
    paramsSizer->Add(m_interval, 0, wxALL, 5);

    paramsSizer->Add(new wxStaticText(box, wxID_ANY, wxT("Выходная папка:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_outputDir = new wxTextCtrl(box, wxID_ANY, wxT(""));
    paramsSizer->Add(m_outputDir, 1, wxEXPAND | wxALL, 5);
    wxButton* browseOutBtn = new wxButton(box, wxID_ANY, wxT("Обзор..."));
    browseOutBtn->Bind(wxEVT_BUTTON, &SettingsPanel::OnBrowseOutput, this);
    paramsSizer->Add(browseOutBtn, 0, wxALL, 5);
    mainSizer->Add(paramsSizer, 0, wxEXPAND);

    SetSizer(mainSizer);
}

wxString SettingsPanel::GetVideoPath() const {
    return m_videoPath->GetValue();
}

void SettingsPanel::SetVideoPath(const wxString& path) {
    m_videoPath->SetValue(path);
    if (!path.IsEmpty() && m_outputDir->IsEmpty()) {
        UpdateOutputDirFromVideo();
    }
}

int SettingsPanel::GetInterval() const {
    return m_interval->GetValue();
}

void SettingsPanel::SetInterval(int interval) {
    m_interval->SetValue(interval);
}

wxString SettingsPanel::GetOutputDir() const {
    return m_outputDir->GetValue();
}

void SettingsPanel::SetOutputDir(const wxString& dir) {
    m_outputDir->SetValue(dir);
}

void SettingsPanel::OnBrowseVideo(wxCommandEvent& /*event*/) {
    wxFileDialog dlg(this, wxT("Выберите видеофайл"), wxT(""), wxT(""),
        wxT("Видео файлы (*.mp4;*.mkv;*.avi;*.mov)|*.mp4;*.mkv;*.avi;*.mov|Все файлы (*.*)|*.*"),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        SetVideoPath(dlg.GetPath());
    }
}

void SettingsPanel::OnBrowseOutput(wxCommandEvent& /*event*/) {
    wxString defaultPath = m_outputDir->GetValue();
    if (defaultPath.IsEmpty()) {
        defaultPath = wxGetCwd();
    }
    wxDirDialog dlg(this, wxT("Выберите выходную папку"), defaultPath,
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        SetOutputDir(dlg.GetPath());
    }
}

void SettingsPanel::UpdateOutputDirFromVideo() {
    wxString path = m_videoPath->GetValue();
    if (!path.IsEmpty()) {
        wxString defaultDir = PathValidator::GetDefaultOutputDir(path);
        m_outputDir->SetValue(defaultDir);
    }
}
