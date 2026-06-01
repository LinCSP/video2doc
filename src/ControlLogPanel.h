#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/stattext.h>

class ControlLogPanel : public wxPanel {
public:
    ControlLogPanel(wxWindow* parent);

    void AppendLog(const wxString& text);
    void ClearLog();
    void SetProgress(int percent);
    void SetStatus(const wxString& status);

    wxButton* GetStage1Button() const { return m_stage1Btn; }
    wxButton* GetStage2Button() const { return m_stage2Btn; }
    wxButton* GetFullCycleButton() const { return m_fullCycleBtn; }
    wxButton* GetCancelButton() const { return m_cancelBtn; }
    wxGauge* GetGauge() const { return m_gauge; }
    wxTextCtrl* GetLogCtrl() const { return m_log; }

private:
    wxTextCtrl* m_log;
    wxGauge* m_gauge;
    wxStaticText* m_statusLabel;
    wxButton* m_stage1Btn;
    wxButton* m_stage2Btn;
    wxButton* m_fullCycleBtn;
    wxButton* m_cancelBtn;
};
