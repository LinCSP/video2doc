#include "DependencyChecker.h"
#include <wx/process.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <wx/app.h>
#include <algorithm>

bool DependencyChecker::CheckCommand(const wxString& cmd, wxString* version) {
    wxString fullCmd = cmd + wxT(" --version");
    
    wxProcess process(wxPROCESS_REDIRECT);
    long exitCode = wxExecute(fullCmd, wxEXEC_SYNC, &process);
    
    if (exitCode == -1) {
        return false;
    }
    
    if (version) {
        wxInputStream* stream = process.GetInputStream();
        if (stream && stream->CanRead()) {
            char buf[256];
            stream->Read(buf, sizeof(buf));
            wxString text(buf, wxConvUTF8, stream->LastRead());
            text.Trim(true).Trim(false);
            // Take first line only
            int pos = text.Find(wxT('\n'));
            if (pos != wxNOT_FOUND) {
                text = text.Left(pos);
            }
            *version = text;
        }
    }
    
    return true;
}

bool DependencyChecker::CheckPythonModule(const wxString& module) {
    wxString cmd = wxT("python3 -c \"import ") + module + wxT("; print('OK')\"");
    
    wxProcess process(wxPROCESS_REDIRECT);
    long exitCode = wxExecute(cmd, wxEXEC_SYNC, &process);
    
    if (exitCode == -1) {
        return false;
    }
    
    wxInputStream* stream = process.GetInputStream();
    if (stream && stream->CanRead()) {
        char buf[64];
        stream->Read(buf, sizeof(buf));
        wxString text(buf, wxConvUTF8, stream->LastRead());
        return text.Contains(wxT("OK"));
    }
    
    return false;
}

static bool TryInstallPythonModule(const wxString& module, wxString* output) {
    wxString cmd = wxT("python3 -m pip install ") + module + wxT(" --user");
    
    wxProcess process(wxPROCESS_REDIRECT);
    long exitCode = wxExecute(cmd, wxEXEC_SYNC, &process);
    
    if (output) {
        wxInputStream* stream = process.GetInputStream();
        if (stream && stream->CanRead()) {
            char buf[1024];
            stream->Read(buf, sizeof(buf));
            *output = wxString(buf, wxConvUTF8, stream->LastRead());
        }
    }
    
    if (exitCode == -1) {
        return false;
    }
    
    // Check if import works now
    return DependencyChecker::CheckPythonModule(module);
}

static bool TryInstallPythonModuleFallback(const wxString& module, wxString* output) {
    wxString cmd = wxT("pip3 install ") + module;
    
    wxProcess process(wxPROCESS_REDIRECT);
    long exitCode = wxExecute(cmd, wxEXEC_SYNC, &process);
    
    if (output) {
        wxInputStream* stream = process.GetInputStream();
        if (stream && stream->CanRead()) {
            char buf[1024];
            stream->Read(buf, sizeof(buf));
            *output = wxString(buf, wxConvUTF8, stream->LastRead());
        }
    }
    
    if (exitCode == -1) {
        return false;
    }
    
    return DependencyChecker::CheckPythonModule(module);
}

std::vector<DependencyInfo> DependencyChecker::CheckAll() {
    std::vector<DependencyInfo> deps;
    
    // System dependencies
    deps.push_back({wxT("git"), wxT("git"), wxT("Клонирование репозиториев (необязательно)"), false, false, wxT(""), wxT("sudo apt install git")});
    deps.push_back({wxT("ffmpeg"), wxT("ffmpeg"), wxT("Извлечение кадров и аудио (Этап 1)"), true, false, wxT(""), wxT("sudo apt install ffmpeg")});
    deps.push_back({wxT("Python 3"), wxT("python3"), wxT("Запуск скриптов транскрибации (Этап 1)"), true, false, wxT(""), wxT("sudo apt install python3 python3-pip")});
    deps.push_back({wxT("pip3"), wxT("pip3"), wxT("Установка Python-пакетов"), true, false, wxT(""), wxT("sudo apt install python3-pip")});
    
    // Python modules
    deps.push_back({wxT("faster-whisper"), wxT("faster_whisper"), wxT("Распознавание речи (Этап 1)"), true, false, wxT(""), wxT("pip3 install faster-whisper")});
    deps.push_back({wxT("kimi-cli"), wxT("kimi"), wxT("Генерация документации AI (Этап 2)"), true, false, wxT(""), wxT("pip3 install kimi-cli")});
    
    // Check system commands
    for (auto& dep : deps) {
        if (dep.command != wxT("faster_whisper") && dep.command != wxT("kimi")) {
            wxString version;
            dep.found = CheckCommand(dep.command, &version);
            dep.version = version;
        }
    }
    
    // Check Python modules and try to auto-install
    for (auto& dep : deps) {
        if (dep.command == wxT("faster_whisper")) {
            dep.found = CheckPythonModule(wxT("faster_whisper"));
            if (!dep.found) {
                wxString output;
                dep.found = TryInstallPythonModule(wxT("faster-whisper"), &output);
                if (!dep.found) {
                    dep.found = TryInstallPythonModuleFallback(wxT("faster-whisper"), &output);
                }
                if (dep.found) {
                    dep.version = wxT("(установлен автоматически)");
                }
            }
        }
        else if (dep.command == wxT("kimi")) {
            // kimi-cli may install as 'kimi' command, not Python module
            wxString version;
            dep.found = CheckCommand(wxT("kimi"), &version);
            if (!dep.found) {
                dep.found = CheckPythonModule(wxT("kimi"));
            }
            if (!dep.found) {
                wxString output;
                dep.found = TryInstallPythonModule(wxT("kimi-cli"), &output);
                if (!dep.found) {
                    dep.found = TryInstallPythonModuleFallback(wxT("kimi-cli"), &output);
                }
                if (dep.found) {
                    dep.version = wxT("(установлен автоматически)");
                }
            } else {
                dep.version = version;
            }
        }
    }
    
    return deps;
}

void ShowDependencyCheckDialog(wxWindow* parent) {
    auto deps = DependencyChecker::CheckAll();
    
    wxString msg = wxT("Проверка зависимостей Video2Doc\n\n");
    wxString missingRequired;
    wxString missingOptional;
    wxString installHints = wxT("\n");
    
    for (const auto& dep : deps) {
        wxString status = dep.found ? wxT("✓") : wxT("✗");
        wxString line = status + wxT(" ") + dep.name;
        if (!dep.version.IsEmpty()) {
            line += wxT(" — ") + dep.version;
        }
        if (!dep.found) {
            line += wxT("  [") + dep.purpose + wxT("]");
            if (dep.required) {
                missingRequired += wxT("  • ") + dep.name + wxT("\n");
            } else {
                missingOptional += wxT("  • ") + dep.name + wxT("\n");
            }
            if (!dep.installHint.IsEmpty()) {
                installHints += wxT("  ") + dep.name + wxT(": ") + dep.installHint + wxT("\n");
            }
        }
        msg += line + wxT("\n");
    }
    
    bool allOk = missingRequired.IsEmpty();
    
    if (!allOk) {
        msg += wxT("\n❌ Отсутствуют обязательные зависимости:\n") + missingRequired;
        if (!missingOptional.IsEmpty()) {
            msg += wxT("\n⚠ Отсутствуют необязательные зависимости:\n") + missingOptional;
        }
        msg += wxT("\nУстановите вручную:\n") + installHints;
        msg += wxT("\nИли выполните: ./install-deps.sh");
        
        wxMessageDialog dlg(parent, msg, wxT("Зависимости — требуется установка"), wxOK | wxICON_WARNING);
        dlg.ShowModal();
    } else {
        msg += wxT("\n✅ Все обязательные зависимости найдены.");
        if (!missingOptional.IsEmpty()) {
            msg += wxT("\n\n⚠ Необязательные:\n") + missingOptional;
        }
        
        wxMessageDialog dlg(parent, msg, wxT("Зависимости — ОК"), wxOK | wxICON_INFORMATION);
        dlg.ShowModal();
    }
}
