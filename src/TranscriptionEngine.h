#pragma once

#include <wx/string.h>
#include <wx/gauge.h>
#include <wx/textctrl.h>
#include <functional>

class ProcessRunner;

class TranscriptionEngine {
public:
    using StateCallback = std::function<void(bool success, const wxString& message)>;

    TranscriptionEngine(wxTextCtrl* logCtrl, wxGauge* gaugeCtrl);
    ~TranscriptionEngine();

    bool Start(const wxString& videoPath, const wxString& outDir, int interval, StateCallback onDone = nullptr);
    void Cancel();
    bool IsRunning() const;
    void SetCallback(StateCallback onDone);

private:
    enum State {
        IDLE,
        FFMPEG_RUNNING,
        WHISPER_RUNNING,
        DONE,
        ERROR_STATE
    };

    void RunFfmpeg();
    void RunWhisper();
    void OnProcessDone(int exitCode);
    void SetState(State state);
    void AppendLog(const wxString& text);
    void SetProgress(int percent);

    wxTextCtrl* m_log;
    wxGauge* m_gauge;
    State m_state;
    wxString m_videoPath;
    wxString m_outDir;
    int m_interval;
    ProcessRunner* m_runner;
    StateCallback m_callback;
};
