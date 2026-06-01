#include "TranscriptionEngine.h"
#include "ProcessRunner.h"
#include "PathValidator.h"
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/utils.h>
#include <algorithm>

TranscriptionEngine::TranscriptionEngine(wxTextCtrl* logCtrl, wxGauge* gaugeCtrl)
    : m_log(logCtrl)
    , m_gauge(gaugeCtrl)
    , m_state(IDLE)
    , m_interval(1)
    , m_runner(nullptr)
    , m_callback(nullptr)
{
}

TranscriptionEngine::~TranscriptionEngine() {
    Cancel();
}

void TranscriptionEngine::SetCallback(StateCallback onDone) {
    m_callback = onDone;
}

bool TranscriptionEngine::Start(const wxString& videoPath, const wxString& outDir, int interval, StateCallback onDone) {
    if (onDone) m_callback = onDone;
    if (m_state != IDLE) {
        AppendLog(wxT("⚠️ Транскрибация уже запущена\n"));
        return false;
    }

    if (!PathValidator::IsValidVideoFile(videoPath)) {
        AppendLog(wxT("❌ Неверный видеофайл: ") + videoPath + wxT("\n"));
        return false;
    }

    m_videoPath = videoPath;
    m_outDir = outDir;
    m_interval = interval;

    PathValidator::EnsureDirExists(m_outDir);
    wxString screenshotsDir = m_outDir + wxT("/screenshots");
    PathValidator::EnsureDirExists(screenshotsDir);

    m_state = FFMPEG_RUNNING;
    AppendLog(wxT("=== Этап 1: Транскрибация ===\n"));
    AppendLog(wxT("📁 Выходная папка: ") + m_outDir + wxT("\n"));
    RunFfmpeg();
    return true;
}

void TranscriptionEngine::Cancel() {
    if (m_runner) {
        m_runner->Kill();
        m_runner = nullptr;
    }
    m_state = IDLE;
}

bool TranscriptionEngine::IsRunning() const {
    return m_state != IDLE && m_state != DONE && m_state != ERROR_STATE;
}

void TranscriptionEngine::RunFfmpeg() {
    SetProgress(5);
    AppendLog(wxT("📸 Извлечение скриншотов (ffmpeg)...\n"));

    wxString screenshotsDir = m_outDir + wxT("/screenshots");
    wxString cmd = wxString::Format(
        wxT("ffmpeg -y -i \"%s\" -vf \"fps=1/%d\" \"%s/frame_%%04d.jpg\""),
        m_videoPath, m_interval, screenshotsDir);

    m_runner = new ProcessRunner(m_log, [this](int code) {
        OnProcessDone(code);
    });

    if (!m_runner->Run(cmd)) {
        m_state = ERROR_STATE;
        m_runner = nullptr;
    }
}

void TranscriptionEngine::RunWhisper() {
    SetProgress(50);
    AppendLog(wxT("🎙️ Транскрибация речи (faster_whisper)...\n"));

    wxString scriptPath = m_outDir + wxT("/_run_whisper.py");

    // Build the Python script content
    wxString scriptContent = wxT(
        "import os, sys\n"
        "print('=== Whisper script started ===', flush=True)\n"
        "print(f'args: {sys.argv}', flush=True)\n"
        "\n"
        "video = sys.argv[1]\n"
        "out_dir = sys.argv[2]\n"
        "duration = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0\n"
        "print(f'video={video}, out_dir={out_dir}, duration={duration}', flush=True)\n"
        "\n"
        "print('Importing faster_whisper...', flush=True)\n"
        "from faster_whisper import WhisperModel\n"
        "print('Loading model medium...', flush=True)\n"
        "model = WhisperModel(\"medium\", device=\"cpu\", compute_type=\"int8\")\n"
        "print('Model loaded. Transcribing...', flush=True)\n"
        "segments, info = model.transcribe(video, language=\"ru\", beam_size=5)\n"
        "print(f'Language: {info.language} (confidence: {info.language_probability:.0%})', flush=True)\n"
        "\n"
        "txt_path = os.path.join(out_dir, \"transcript.txt\")\n"
        "srt_path = os.path.join(out_dir, \"transcript.srt\")\n"
        "\n"
        "def fmt(t):\n"
        "    h = int(t // 3600)\n"
        "    m = int((t % 3600) // 60)\n"
        "    s = int(t % 60)\n"
        "    ms = int((t % 1) * 1000)\n"
        "    return f\"{h:02d}:{m:02d}:{s:02d},{ms:03d}\"\n"
        "\n"
        "def progress_bar(current, total, width=20):\n"
        "    if total <= 0:\n"
        "        return ''\n"
        "    pct = min(100, int(current / total * 100))\n"
        "    filled = pct // (100 // width)\n"
        "    bar = '█' * filled + '░' * (width - filled)\n"
        "    cur_str = f\"{int(current//60):02d}:{int(current%60):02d}\"\n"
        "    tot_str = f\"{int(total//60):02d}:{int(total%60):02d}\"\n"
        "    return f\"\\r  [{bar}] {pct:3d}%  {cur_str} / {tot_str}\"\n"
        "\n"
        "print('Writing transcripts...', flush=True)\n"
        "with open(txt_path, 'w', encoding='utf-8') as txt, open(srt_path, 'w', encoding='utf-8') as srt:\n"
        "    for i, seg in enumerate(segments, 1):\n"
        "        pb = progress_bar(seg.end, duration)\n"
        "        if pb:\n"
        "            print(pb, end='', flush=True)\n"
        "        line = f\"[{seg.start:6.1f}s - {seg.end:6.1f}s]  {seg.text.strip()}\"\n"
        "        txt.write(line + '\\n')\n"
        "        srt.write(f\"{i}\\n{fmt(seg.start)} --> {fmt(seg.end)}\\n{seg.text.strip()}\\n\\n\")\n"
        "\n"
        "if duration > 0:\n"
        "    final = progress_bar(duration, duration)\n"
        "    if final:\n"
        "        print(final, flush=True)\n"
        "print('', flush=True)\n"
        "print(f'✅ Done: {txt_path}', flush=True)\n"
        "print(f'✅ Done: {srt_path}', flush=True)\n"
    );

    // Write script to file with explicit UTF-8 encoding
    {
        wxFile file(scriptPath, wxFile::write);
        if (!file.IsOpened()) {
            AppendLog(wxT("❌ Не удалось создать скрипт whisper: ") + scriptPath + wxT("\n"));
            m_state = ERROR_STATE;
            if (m_callback) m_callback(false, wxT("Ошибка создания скрипта"));
            return;
        }
        wxCharBuffer buf = scriptContent.utf8_str();
        if (!file.Write(buf.data(), buf.length())) {
            AppendLog(wxT("❌ Не удалось записать скрипт whisper\n"));
            m_state = ERROR_STATE;
            if (m_callback) m_callback(false, wxT("Ошибка записи скрипта"));
            return;
        }
        file.Close();
    }

    if (!wxFileExists(scriptPath)) {
        AppendLog(wxT("❌ Скрипт whisper не найден после записи\n"));
        m_state = ERROR_STATE;
        if (m_callback) m_callback(false, wxT("Скрипт не создан"));
        return;
    }

    // Get video duration using ffprobe for progress
    wxString duration = wxT("0");
    {
        wxArrayString output, errors;
        wxString ffprobeCmd = wxString::Format(
            wxT("ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"%s\""),
            m_videoPath);
        wxExecute(ffprobeCmd, output, errors);
        if (!output.IsEmpty()) {
            duration = output[0].BeforeFirst('.');
        }
    }

    // python3 (Linux) / python (Windows) — unbuffered stdout/stderr
#ifdef __WXMSW__
    wxString pythonCmd = wxT("python");
#else
    wxString pythonCmd = wxT("python3");
#endif
    wxString cmd = wxString::Format(
        wxT("%s -u \"%s\" \"%s\" \"%s\" %s"),
        pythonCmd, scriptPath, m_videoPath, m_outDir, duration);

    AppendLog(wxT("> ") + cmd + wxT("\n"));

    // Set PYTHONUNBUFFERED=1 in environment before launching
    wxSetEnv(wxT("PYTHONUNBUFFERED"), wxT("1"));

    m_runner = new ProcessRunner(m_log, [this](int code) {
        OnProcessDone(code);
    });

    if (!m_runner->Run(cmd)) {
        m_state = ERROR_STATE;
        m_runner = nullptr;
    }
}

void TranscriptionEngine::OnProcessDone(int exitCode) {
    m_runner = nullptr;

    if (exitCode != 0) {
        m_state = ERROR_STATE;
        AppendLog(wxString::Format(wxT("❌ Процесс завершился с ошибкой (код %d)\n"), exitCode));
        SetProgress(0);
        if (m_callback) m_callback(false, wxT("Ошибка транскрибации"));
        return;
    }

    switch (m_state) {
        case FFMPEG_RUNNING:
            AppendLog(wxT("✅ Скриншоты готовы\n"));
            m_state = WHISPER_RUNNING;
            RunWhisper();
            break;

        case WHISPER_RUNNING:
            AppendLog(wxT("✅ Транскрипция завершена\n"));
            m_state = DONE;
            SetProgress(100);
            if (m_callback) m_callback(true, wxT("Транскрипция завершена успешно"));
            break;

        default:
            break;
    }
}

void TranscriptionEngine::SetState(State state) {
    m_state = state;
}

void TranscriptionEngine::AppendLog(const wxString& text) {
    if (m_log) m_log->AppendText(text);
}

void TranscriptionEngine::SetProgress(int percent) {
    if (m_gauge) m_gauge->SetValue(std::clamp(percent, 0, 100));
}
