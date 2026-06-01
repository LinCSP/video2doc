#pragma once

#include <wx/string.h>
#include <wx/window.h>
#include <vector>

struct DependencyInfo {
    wxString name;
    wxString command;
    wxString purpose;
    bool required;
    bool found;
    wxString version;
    wxString installHint;
};

class DependencyChecker {
public:
    static std::vector<DependencyInfo> CheckAll();
    static bool CheckCommand(const wxString& cmd, wxString* version = nullptr);
    static bool CheckPythonModule(const wxString& module);
};

// Показывает диалог проверки зависимостей; при отсутствии Python-модулей
// пытается установить их автоматически через pip.
void ShowDependencyCheckDialog(wxWindow* parent = nullptr);
