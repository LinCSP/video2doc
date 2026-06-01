#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <functional>

class PromptEditorPanel : public wxPanel {
public:
    using SaveCallback = std::function<void(const wxString& text)>;
    using ResetCallback = std::function<void()>;

    PromptEditorPanel(wxWindow* parent, SaveCallback onSave = nullptr, ResetCallback onReset = nullptr);

    wxString GetTemplate() const;
    void SetTemplate(const wxString& tpl);

private:
    void OnSave(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);

    wxTextCtrl* m_editor;
    SaveCallback m_onSave;
    ResetCallback m_onReset;
};
