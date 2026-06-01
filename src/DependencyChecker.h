#pragma once

#include <wx/string.h>
#include <wx/window.h>
#include <wx/textctrl.h>
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
    static std::vector<DependencyInfo> CheckAll(wxTextCtrl* log = nullptr);
    static bool CheckCommand(const wxString& cmd, wxString* version = nullptr);
    static bool CheckPythonModule(const wxString& module);
};

// Показывает диалог проверки зависимостей; при отсутствии Python-модулей
// пытается установить их автоматически через pip.
// Вывод пишется в log (если передан) для отображения в журнале.
void ShowDependencyCheckDialog(wxWindow* parent = nullptr, wxTextCtrl* log = nullptr);
