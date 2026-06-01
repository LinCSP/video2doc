#include "DependencyChecker.h"
#include "ControlLogPanel.h"
#include <wx/process.h>
#include <wx/txtstrm.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <wx/app.h>
#include <algorithm>

static void Log(wxTextCtrl* log, const wxString& text) {
    if (log) {
        log->AppendText(text + wxT("\n"));
        log->ShowPosition(log->GetLastPosition());
        wxYieldIfNeeded();
    }
}

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
            int pos = text.Find(wxT('\n'));
            if (pos != wxNOT_FOUND) {
                text = text.Left(pos);
            }
            *version = text;
        }
    }
    
    return true;
}

static wxString GetPythonCmd() {
    wxProcess p(wxPROCESS_REDIRECT);
    if (wxExecute(wxT("python3 --version"), wxEXEC_SYNC, &p) != -1) {
        return wxT("python3");
    }
    return wxT("python");
}

bool DependencyChecker::CheckPythonModule(const wxString& module) {
    wxString py = GetPythonCmd();
    wxString cmd = py + wxT(" -c \"import ") + module + wxT("; print('OK')\"");
    
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

static bool TryInstallPythonModule(const wxString& module, wxString* output, wxTextCtrl* log) {
    wxString py = GetPythonCmd();
    wxString cmd = py + wxT(" -m pip install ") + module + wxT(" --user");
    
    Log(log, wxT("⏳ Установка ") + module + wxT(" (") + py + wxT(" -m pip install --user)..."));
    
    wxProcess process(wxPROCESS_REDIRECT);
    long exitCode = wxExecute(cmd, wxEXEC_SYNC, &process);
    
    if (output) {
        wxInputStream* stream = process.GetInputStream();
        if (stream && stream->CanRead()) {
            char buf[4096];
            stream->Read(buf, sizeof(buf));
            *output = wxString(buf, wxConvUTF8, stream->LastRead());
        }
    }
    
    if (exitCode == -1) {
        Log(log, wxT("  ✗ Не удалось запустить pip"));
        return false;
    }
    
    // Check if import works now
    bool ok = DependencyChecker::CheckPythonModule(module);
    if (ok) {
        Log(log, wxT("  ✓ ") + module + wxT(" установлен"));
    } else {
        Log(log, wxT("  ✗ ") + module + wxT(" не установлен"));
    }
    return ok;
}

static bool TryInstallPythonModuleFallback(const wxString& module, wxString* output, wxTextCtrl* log) {
    wxString cmd = wxT("pip install ") + module;
    
    Log(log, wxT("⏳ Повторная попытка: pip install ") + module + wxT("..."));
    
    wxProcess process(wxPROCESS_REDIRECT);
    long exitCode = wxExecute(cmd, wxEXEC_SYNC, &process);
    
    if (output) {
        wxInputStream* stream = process.GetInputStream();
        if (stream && stream->CanRead()) {
            char buf[4096];
            stream->Read(buf, sizeof(buf));
            *output = wxString(buf, wxConvUTF8, stream->LastRead());
        }
    }
    
    if (exitCode == -1) {
        Log(log, wxT("  ✗ Не удалось запустить pip"));
        return false;
    }
    
    bool ok = DependencyChecker::CheckPythonModule(module);
    if (ok) {
        Log(log, wxT("  ✓ ") + module + wxT(" установлен"));
    } else {
        Log(log, wxT("  ✗ ") + module + wxT(" не установлен"));
    }
    return ok;
}

std::vector<DependencyInfo> DependencyChecker::CheckAll(wxTextCtrl* log) {
    std::vector<DependencyInfo> deps;
    
    Log(log, wxT("=== Проверка зависимостей Video2Doc ==="));
    
    // System dependencies
    deps.push_back({wxT("git"), wxT("git"), wxT("Клонирование репозиториев (необязательно)"), false, false, wxT(""), wxT("sudo apt install git")});
    deps.push_back({wxT("ffmpeg"), wxT("ffmpeg"), wxT("Извлечение кадров и аудио (Этап 1)"), true, false, wxT(""), wxT("sudo apt install ffmpeg")});
    // Python: на Windows python3 не существует, проверяем python как fallback
    bool pythonFound = CheckCommand(wxT("python3")) || CheckCommand(wxT("python"));
    deps.push_back({wxT("Python 3"), GetPythonCmd(), wxT("Запуск скриптов транскрибации (Этап 1)"), true, pythonFound, wxT(""), wxT("sudo apt install python3 python3-pip  (Windows: скачать с python.org)")});
    // pip: на Arch pip3 может не существовать, проверяем pip как fallback
    bool pipFound = CheckCommand(wxT("pip3")) || CheckCommand(wxT("pip"));
    deps.push_back({wxT("pip"), wxT("pip"), wxT("Установка Python-пакетов"), true, pipFound, wxT(""), wxT("sudo apt install python3-pip  (Arch: sudo pacman -S python-pip)")});
    
    // Python modules
    deps.push_back({wxT("faster-whisper"), wxT("faster_whisper"), wxT("Распознавание речи (Этап 1)"), true, false, wxT(""), wxT("pip3 install faster-whisper")});
    deps.push_back({wxT("kimi-cli"), wxT("kimi"), wxT("Генерация документации AI (Этап 2)"), true, false, wxT(""), wxT("pip3 install kimi-cli")});
    
    // Check system commands
    for (auto& dep : deps) {
        if (dep.command != wxT("faster_whisper") && dep.command != wxT("kimi")) {
            wxString version;
            dep.found = CheckCommand(dep.command, &version);
            dep.version = version;
            if (dep.found) {
                Log(log, wxT("✓ ") + dep.name + wxT(" — ") + version);
            } else {
                Log(log, wxT("✗ ") + dep.name + wxT(" — ") + dep.purpose);
            }
        }
    }
    
    // Check Python modules and try to auto-install
    for (auto& dep : deps) {
        if (dep.command == wxT("faster_whisper")) {
            dep.found = CheckPythonModule(wxT("faster_whisper"));
            if (dep.found) {
                Log(log, wxT("✓ faster-whisper"));
            } else {
                Log(log, wxT("✗ faster-whisper — попытка автоматической установки..."));
                wxString output;
                dep.found = TryInstallPythonModule(wxT("faster-whisper"), &output, log);
                if (!dep.found) {
                    dep.found = TryInstallPythonModuleFallback(wxT("faster-whisper"), &output, log);
                }
                if (dep.found) {
                    dep.version = wxT("(установлен автоматически)");
                }
            }
        }
        else if (dep.command == wxT("kimi")) {
            wxString version;
            dep.found = CheckCommand(wxT("kimi"), &version);
            if (!dep.found) {
                dep.found = CheckPythonModule(wxT("kimi"));
            }
            if (dep.found) {
                dep.version = version.IsEmpty() ? wxT("(OK)") : version;
                Log(log, wxT("✓ kimi-cli"));
            } else {
                Log(log, wxT("✗ kimi-cli — попытка автоматической установки..."));
                wxString output;
                dep.found = TryInstallPythonModule(wxT("kimi-cli"), &output, log);
                if (!dep.found) {
                    dep.found = TryInstallPythonModuleFallback(wxT("kimi-cli"), &output, log);
                }
                if (dep.found) {
                    dep.version = wxT("(установлен автоматически)");
                }
            }
        }
    }
    
    Log(log, wxT("=== Проверка зависимостей завершена ==="));
    return deps;
}

void ShowDependencyCheckDialog(wxWindow* parent, wxTextCtrl* log) {
    auto deps = DependencyChecker::CheckAll(log);
    
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
