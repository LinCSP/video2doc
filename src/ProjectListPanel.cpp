#include "ProjectListPanel.h"
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/button.h>

ProjectListPanel::ProjectListPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT("Дополнительные проекты"));
    wxStaticBoxSizer* mainSizer = new wxStaticBoxSizer(box, wxVERTICAL);

    wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);

    m_listBox = new wxListBox(box, wxID_ANY, wxDefaultPosition, wxSize(-1, 100), 0, nullptr, wxLB_SINGLE);
    contentSizer->Add(m_listBox, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxVERTICAL);
    wxButton* addBtn = new wxButton(box, wxID_ADD, wxT("Добавить..."));
    addBtn->Bind(wxEVT_BUTTON, &ProjectListPanel::OnAdd, this);
    btnSizer->Add(addBtn, 0, wxEXPAND | wxALL, 5);

    wxButton* removeBtn = new wxButton(box, wxID_REMOVE, wxT("Удалить"));
    removeBtn->Bind(wxEVT_BUTTON, &ProjectListPanel::OnRemove, this);
    btnSizer->Add(removeBtn, 0, wxEXPAND | wxALL, 5);

    contentSizer->Add(btnSizer, 0, wxEXPAND);
    mainSizer->Add(contentSizer, 1, wxEXPAND);
    SetSizer(mainSizer);
}

wxArrayString ProjectListPanel::GetProjects() const {
    wxArrayString projects;
    for (unsigned int i = 0; i < m_listBox->GetCount(); ++i) {
        projects.Add(m_listBox->GetString(i));
    }
    return projects;
}

void ProjectListPanel::SetProjects(const wxArrayString& projects) {
    m_listBox->Clear();
    for (const auto& p : projects) {
        m_listBox->Append(p);
    }
}

void ProjectListPanel::AddProject(const wxString& path) {
    // Check for duplicates
    for (unsigned int i = 0; i < m_listBox->GetCount(); ++i) {
        if (m_listBox->GetString(i) == path) {
            return;
        }
    }
    m_listBox->Append(path);
}

void ProjectListPanel::OnAdd(wxCommandEvent& /*event*/) {
    wxDirDialog dlg(this, wxT("Выберите папку проекта"), wxGetCwd(),
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        AddProject(dlg.GetPath());
    }
}

void ProjectListPanel::OnRemove(wxCommandEvent& /*event*/) {
    int sel = m_listBox->GetSelection();
    if (sel != wxNOT_FOUND) {
        m_listBox->Delete(sel);
    } else {
        wxMessageBox(wxT("Выберите проект для удаления"), wxT("Внимание"), wxOK | wxICON_INFORMATION);
    }
}
