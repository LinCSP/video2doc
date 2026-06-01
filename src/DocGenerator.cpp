#include "DocGenerator.h"
#include "ProcessRunner.h"
#include "PathValidator.h"
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/arrstr.h>
#include <algorithm>

DocGenerator::DocGenerator(wxTextCtrl* logCtrl, wxGauge* gaugeCtrl)
    : m_log(logCtrl)
    , m_gauge(gaugeCtrl)
    , m_running(false)
    , m_runner(nullptr)
{
}

DocGenerator::~DocGenerator() {
    Cancel();
}

bool DocGenerator::Start(const wxString& videoPath, const wxString& outDir,
                         const wxString& promptTemplate, const wxArrayString& projects,
                         DoneCallback onDone) {
    if (onDone) m_callback = onDone;
    if (m_running) {
        AppendLog(wxT("⚠️ Генерация документации уже запущена\n"));
        return false;
    }

    m_outDir = outDir;
    PathValidator::EnsureDirExists(m_outDir);
    PathValidator::EnsureDirExists(m_outDir + wxT("/doc"));
    PathValidator::EnsureDirExists(m_outDir + wxT("/doc/image"));

    // Generate final prompt
    wxString finalPrompt = SubstitutePrompt(promptTemplate, videoPath, outDir, projects);

    // Write prompt file
    wxString promptFile = m_outDir + wxT("/promt.md");
    wxFile file(promptFile, wxFile::write);
    if (!file.IsOpened()) {
        AppendLog(wxT("❌ Не удалось создать файл промпта: ") + promptFile + wxT("\n"));
        return false;
    }
    file.Write(finalPrompt, wxConvUTF8);
    file.Close();

    AppendLog(wxT("=== Этап 2: Генерация документации ===\n"));
    AppendLog(wxT("📝 Файл промпта сохранён: ") + promptFile + wxT("\n"));
    AppendLog(wxT("🤖 Запуск Kimi Code CLI...\n"));
    SetProgress(10);

    // Read prompt content
    wxString promptContent;
    {
        wxFile pf(promptFile, wxFile::read);
        if (pf.IsOpened()) {
            wxFileOffset len = pf.Length();
            if (len > 0) {
                wxCharBuffer buf(len);
                pf.Read(buf.data(), len);
                promptContent = wxString(buf.data(), wxConvUTF8, len);
            }
            pf.Close();
        }
    }

    // Build Kimi command as argv array to handle multi-line prompt correctly.
    wxArrayString argv;
    argv.Add(wxT("kimi"));
    argv.Add(wxT("-w"));
    argv.Add(m_outDir);
    argv.Add(wxT("--yolo"));
    argv.Add(wxT("--afk"));
    argv.Add(wxT("--prompt"));
    argv.Add(promptContent);

    m_running = true;
    m_runner = new ProcessRunner(m_log, [this](int code) {
        OnProcessDone(code);
    });

    if (!m_runner->Run(argv)) {
        m_running = false;
        m_runner = nullptr;
        return false;
    }

    return true;
}

void DocGenerator::Cancel() {
    if (m_runner) {
        m_runner->Kill();
        m_runner = nullptr;
    }
    m_running = false;
}

bool DocGenerator::IsRunning() const {
    return m_running;
}

wxString DocGenerator::SubstitutePrompt(const wxString& templateText,
                                         const wxString& videoPath,
                                         const wxString& outDir,
                                         const wxArrayString& projects) {
    wxString result = templateText;

    wxString projectName = PathValidator::GetVideoBaseName(videoPath);
    wxString screenshotsDir = outDir + wxT("/screenshots");

    wxString projectsList;
    for (size_t i = 0; i < projects.GetCount(); ++i) {
        if (i > 0) projectsList += wxT("\n");
        projectsList += wxT("- ") + projects[i];
    }
    if (projectsList.IsEmpty()) {
        projectsList = wxT("(нет дополнительных проектов)");
    }

    result.Replace(wxT("{VIDEO_PATH}"), videoPath);
    result.Replace(wxT("{OUT_DIR}"), outDir);
    result.Replace(wxT("{SCREENSHOTS_DIR}"), screenshotsDir);
    result.Replace(wxT("{PROJECT_NAME}"), projectName);
    result.Replace(wxT("{PROJECTS_LIST}"), projectsList);

    return result;
}

void DocGenerator::OnProcessDone(int exitCode) {
    m_runner = nullptr;
    m_running = false;

    if (exitCode != 0) {
        AppendLog(wxString::Format(wxT("❌ Kimi завершился с кодом %d\n"), exitCode));
        SetProgress(0);
        if (m_callback) m_callback(false, wxString::Format(wxT("Kimi завершился с кодом %d"), exitCode));
    } else {
        AppendLog(wxT("✅ Генерация документации завершена\n"));
        AppendLog(wxT("📂 Результат: ") + m_outDir + wxT("/doc/\n"));
        SetProgress(100);
        if (m_callback) m_callback(true, wxT("Генерация документации завершена"));
    }
}

void DocGenerator::AppendLog(const wxString& text) {
    if (m_log) m_log->AppendText(text);
}

void DocGenerator::SetProgress(int percent) {
    if (m_gauge) m_gauge->SetValue(std::clamp(percent, 0, 100));
}
