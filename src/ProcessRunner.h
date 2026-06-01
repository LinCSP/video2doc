#pragma once

#include <wx/process.h>
#include <wx/timer.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <functional>

class ProcessRunner : public wxProcess {
public:
    using Callback = std::function<void(int exitCode)>;

    ProcessRunner(wxTextCtrl* logCtrl, Callback onDone = nullptr);
    virtual ~ProcessRunner();

    bool Run(const wxString& command);
    bool Run(const wxArrayString& argv);
    void Kill();
    bool IsRunning() const;
    void PollOutput();

    virtual void OnTerminate(int pid, int status) override;

private:
    void OnTimer(wxTimerEvent& event);

    wxTextCtrl* m_log;
    Callback m_onDone;
    long m_pid;
    wxTimer m_timer;
};
