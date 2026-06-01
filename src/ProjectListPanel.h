#pragma once

#include <wx/panel.h>
#include <wx/listbox.h>
#include <wx/arrstr.h>

class ProjectListPanel : public wxPanel {
public:
    ProjectListPanel(wxWindow* parent);

    wxArrayString GetProjects() const;
    void SetProjects(const wxArrayString& projects);
    void AddProject(const wxString& path);

private:
    void OnAdd(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);

    wxListBox* m_listBox;
};
