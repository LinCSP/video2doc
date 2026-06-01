#include "PromptEditorPanel.h"
#include <wx/sizer.h>
#include <wx/statbox.h>

PromptEditorPanel::PromptEditorPanel(wxWindow* parent, SaveCallback onSave, ResetCallback onReset)
    : wxPanel(parent, wxID_ANY)
    , m_onSave(onSave)
    , m_onReset(onReset)
{
    wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT("Шаблон промпта (promt.md)"));
    wxStaticBoxSizer* mainSizer = new wxStaticBoxSizer(box, wxVERTICAL);

    m_editor = new wxTextCtrl(box, wxID_ANY, wxT(""),
        wxDefaultPosition, wxSize(-1, 150),
        wxTE_MULTILINE | wxTE_WORDWRAP);
    wxFont font(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
    m_editor->SetFont(font);
    mainSizer->Add(m_editor, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* saveBtn = new wxButton(box, wxID_ANY, wxT("Сохранить шаблон"));
    saveBtn->Bind(wxEVT_BUTTON, &PromptEditorPanel::OnSave, this);
    btnSizer->Add(saveBtn, 0, wxALL, 5);

    wxButton* resetBtn = new wxButton(box, wxID_ANY, wxT("Сбросить по умолчанию"));
    resetBtn->Bind(wxEVT_BUTTON, &PromptEditorPanel::OnReset, this);
    btnSizer->Add(resetBtn, 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxALIGN_LEFT);
    SetSizer(mainSizer);
}

wxString PromptEditorPanel::GetTemplate() const {
    return m_editor->GetValue();
}

void PromptEditorPanel::SetTemplate(const wxString& tpl) {
    m_editor->SetValue(tpl);
}

void PromptEditorPanel::OnSave(wxCommandEvent& /*event*/) {
    if (m_onSave) {
        m_onSave(GetTemplate());
    }
}

void PromptEditorPanel::OnReset(wxCommandEvent& /*event*/) {
    if (m_onReset) {
        m_onReset();
    }
}
