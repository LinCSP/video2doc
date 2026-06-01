#pragma once

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/gauge.h>
#include <wx/textctrl.h>
#include <functional>

class ProcessRunner;

class DocGenerator {
public:
    using DoneCallback = std::function<void(bool success, const wxString& message)>;

    DocGenerator(wxTextCtrl* logCtrl, wxGauge* gaugeCtrl);
    ~DocGenerator();

    bool Start(const wxString& videoPath, const wxString& outDir,
               const wxString& promptTemplate, const wxArrayString& projects,
               DoneCallback onDone = nullptr);
    void Cancel();
    bool IsRunning() const;

    static wxString SubstitutePrompt(const wxString& templateText,
                                      const wxString& videoPath,
                                      const wxString& outDir,
                                      const wxArrayString& projects);

private:
    void OnProcessDone(int exitCode);
    void AppendLog(const wxString& text);
    void SetProgress(int percent);

    wxTextCtrl* m_log;
    wxGauge* m_gauge;
    bool m_running;
    ProcessRunner* m_runner;
    wxString m_outDir;
    DoneCallback m_callback;

};
