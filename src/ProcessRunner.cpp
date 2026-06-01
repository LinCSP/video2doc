#include "ProcessRunner.h"
#include <wx/txtstrm.h>
#include <wx/utils.h>
#include <wx/tokenzr.h>

ProcessRunner::ProcessRunner(wxTextCtrl* logCtrl, Callback onDone)
    : wxProcess(wxPROCESS_REDIRECT)
    , m_log(logCtrl)
    , m_onDone(onDone)
    , m_pid(0)
    , m_timer(this, wxID_ANY)
{
    Bind(wxEVT_TIMER, &ProcessRunner::OnTimer, this, m_timer.GetId());
}

ProcessRunner::~ProcessRunner() {
    m_timer.Stop();
}

bool ProcessRunner::Run(const wxString& command) {
    // Simple quote-aware parser to split command into argv
    wxArrayString argv;
    wxStringTokenizer tok(command, wxT(" "), wxTOKEN_RET_EMPTY_ALL);
    bool inQuotes = false;
    wxString currentArg;
    
    while (tok.HasMoreTokens()) {
        wxString token = tok.GetNextToken();
        if (!inQuotes) {
            if (token.StartsWith(wxT("\""))) {
                if (token.EndsWith(wxT("\"")) && token.length() > 1) {
                    argv.Add(token.Mid(1, token.length() - 2));
                } else {
                    inQuotes = true;
                    currentArg = token.Mid(1);
                }
            } else {
                argv.Add(token);
            }
        } else {
            currentArg += wxT(" ") + token;
            if (token.EndsWith(wxT("\""))) {
                inQuotes = false;
                argv.Add(currentArg.Mid(0, currentArg.length() - 1));
                currentArg.Clear();
            }
        }
    }
    if (inQuotes && !currentArg.IsEmpty()) {
        argv.Add(currentArg);
    }
    
    return Run(argv);
}

bool ProcessRunner::Run(const wxArrayString& argv) {
    // Build wchar_t argv array for wxExecute
    std::vector<const wchar_t*> wargv;
    wxString displayCmd;
    for (const auto& s : argv) {
        wargv.push_back(s.wc_str());
        if (!displayCmd.IsEmpty()) displayCmd += wxT(" ");
        if (s.Contains(wxT(" ")) || s.Contains(wxT("\t"))) {
            displayCmd += wxT("\"") + s + wxT("\"");
        } else {
            displayCmd += s;
        }
    }
    wargv.push_back(nullptr);
    
    m_pid = wxExecute(wargv.data(), wxEXEC_ASYNC, this);
    
    if (m_pid == 0) {
        if (m_log) m_log->AppendText(wxT("❌ Не удалось запустить: ") + displayCmd + wxT("\n"));
        return false;
    }
    
    // Close stdin immediately so the child sees EOF and can terminate
    // when it finishes processing (needed for tools like kimi that wait
    // for interactive input).
    CloseOutput();
    
    if (m_log) m_log->AppendText(wxT("> ") + displayCmd + wxT("\n"));
    m_timer.Start(100);
    return true;
}

void ProcessRunner::Kill() {
    if (m_pid != 0) {
        wxKill(m_pid, wxSIGKILL, nullptr, wxKILL_CHILDREN);
        m_pid = 0;
    }
    m_timer.Stop();
}

bool ProcessRunner::IsRunning() const {
    return m_pid != 0;
}

void ProcessRunner::PollOutput() {
    wxInputStream* stream = GetInputStream();
    if (!stream || !stream->CanRead()) return;
    
    char buf[1024];
    if (stream->Read(buf, sizeof(buf)).LastRead() > 0) {
        wxString text(buf, wxConvUTF8, stream->LastRead());
        if (m_log && !text.IsEmpty()) {
            m_log->AppendText(text);
        }
    }
}

void ProcessRunner::OnTimer(wxTimerEvent& /*event*/) {
    if (m_pid != 0) {
        wxInputStream* errStream = GetErrorStream();
        if (errStream && errStream->CanRead()) {
            char buf[1024];
            if (errStream->Read(buf, sizeof(buf)).LastRead() > 0) {
                wxString text(buf, wxConvUTF8, errStream->LastRead());
                if (m_log && !text.IsEmpty()) {
                    m_log->AppendText(wxT("[stderr] ") + text);
                }
            }
        }
        
        wxInputStream* inStream = GetInputStream();
        if (inStream && inStream->CanRead()) {
            char buf[1024];
            if (inStream->Read(buf, sizeof(buf)).LastRead() > 0) {
                wxString text(buf, wxConvUTF8, inStream->LastRead());
                if (m_log && !text.IsEmpty()) {
                    m_log->AppendText(text);
                }
            }
        }
    }
}

void ProcessRunner::OnTerminate(int pid, int status) {
    m_timer.Stop();
    
    // Flush remaining output
    wxInputStream* errStream = GetErrorStream();
    if (errStream) {
        while (errStream->CanRead()) {
            char buf[1024];
            if (errStream->Read(buf, sizeof(buf)).LastRead() > 0) {
                wxString text(buf, wxConvUTF8, errStream->LastRead());
                if (m_log && !text.IsEmpty()) {
                    m_log->AppendText(wxT("[stderr] ") + text);
                }
            } else {
                break;
            }
        }
    }
    
    wxInputStream* inStream = GetInputStream();
    if (inStream) {
        while (inStream->CanRead()) {
            char buf[1024];
            if (inStream->Read(buf, sizeof(buf)).LastRead() > 0) {
                wxString text(buf, wxConvUTF8, inStream->LastRead());
                if (m_log && !text.IsEmpty()) {
                    m_log->AppendText(text);
                }
            } else {
                break;
            }
        }
    }
    
    if (m_log) {
        m_log->AppendText(wxString::Format(wxT("Процесс завершён (pid=%d, код=%d)\n"), pid, status));
    }
    m_pid = 0;
    if (m_onDone) {
        // Defer callback to next event loop iteration to avoid
        // reentrancy issues when calling wxExecute from OnTerminate.
        if (m_log) {
            m_log->GetEventHandler()->CallAfter([this, status]() {
                m_onDone(status);
            });
        } else {
            m_onDone(status);
        }
    }
    delete this;
}
