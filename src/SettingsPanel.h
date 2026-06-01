#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>

class SettingsPanel : public wxPanel {
public:
    SettingsPanel(wxWindow* parent);

    wxString GetVideoPath() const;
    void SetVideoPath(const wxString& path);

    int GetInterval() const;
    void SetInterval(int interval);

    wxString GetOutputDir() const;
    void SetOutputDir(const wxString& dir);

private:
    void OnBrowseVideo(wxCommandEvent& event);
    void OnBrowseOutput(wxCommandEvent& event);
    void UpdateOutputDirFromVideo();

    wxTextCtrl* m_videoPath;
    wxSpinCtrl* m_interval;
    wxTextCtrl* m_outputDir;
};
