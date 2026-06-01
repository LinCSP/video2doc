#include "ConfigManager.h"
#include <wx/config.h>
#include <wx/stdpaths.h>

wxString ConfigManager::GetConfigGroup() {
    return wxT("/Video2Doc");
}

wxString ConfigManager::GetLastVideoPath() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    return config.Read(wxT("LastVideoPath"), wxT(""));
}

void ConfigManager::SetLastVideoPath(const wxString& path) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.Write(wxT("LastVideoPath"), path);
}

wxString ConfigManager::GetLastOutputDir() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    return config.Read(wxT("LastOutputDir"), wxT(""));
}

void ConfigManager::SetLastOutputDir(const wxString& dir) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.Write(wxT("LastOutputDir"), dir);
}

int ConfigManager::GetScreenshotInterval() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    long val = 1;
    config.Read(wxT("ScreenshotInterval"), &val);
    return static_cast<int>(val);
}

void ConfigManager::SetScreenshotInterval(int interval) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.Write(wxT("ScreenshotInterval"), static_cast<long>(interval));
}

wxArrayString ConfigManager::GetProjects() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup() + wxT("/Projects"));
    wxArrayString projects;
    long index;
    wxString key;
    bool hasMore = config.GetFirstEntry(key, index);
    while (hasMore) {
        wxString value;
        if (config.Read(key, &value)) {
            projects.Add(value);
        }
        hasMore = config.GetNextEntry(key, index);
    }
    return projects;
}

void ConfigManager::SetProjects(const wxArrayString& projects) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.DeleteGroup(wxT("Projects"));
    config.SetPath(GetConfigGroup() + wxT("/Projects"));
    for (size_t i = 0; i < projects.GetCount(); ++i) {
        config.Write(wxString::Format(wxT("Project%zu"), i), projects[i]);
    }
}

wxString ConfigManager::GetPromptTemplate() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    wxString tpl;
    if (!config.Read(wxT("PromptTemplate"), &tpl) || tpl.IsEmpty()) {
        tpl = GetDefaultPromptTemplate();
    }
    return tpl;
}

void ConfigManager::SetPromptTemplate(const wxString& tpl) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.Write(wxT("PromptTemplate"), tpl);
}

wxSize ConfigManager::GetWindowSize() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    long w = 1200, h = 800;
    config.Read(wxT("WindowWidth"), &w);
    config.Read(wxT("WindowHeight"), &h);
    return wxSize(static_cast<int>(w), static_cast<int>(h));
}

void ConfigManager::SetWindowSize(const wxSize& size) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.Write(wxT("WindowWidth"), static_cast<long>(size.GetWidth()));
    config.Write(wxT("WindowHeight"), static_cast<long>(size.GetHeight()));
}

bool ConfigManager::GetWindowMaximized() {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    bool maximized = false;
    config.Read(wxT("WindowMaximized"), &maximized);
    return maximized;
}

void ConfigManager::SetWindowMaximized(bool maximized) {
    wxConfig config(wxT("Video2Doc"));
    config.SetPath(GetConfigGroup());
    config.Write(wxT("WindowMaximized"), maximized);
}

wxString ConfigManager::GetDefaultPromptTemplate() {
    return wxT(
        "Я перевёл видео в текст и картинки {PROJECT_NAME}/*.\n"
        "Нужно тщательно проанализировать текст и картинки.\n"
        "И написать документацию в {OUT_DIR}/*.\n"
        "Также необходимые картинки скопировать в {OUT_DIR}/image/*\n"
        "и встроить в документацию.\n"
        "Если нужно проанализировать сам проект, то {PROJECTS_LIST}.\n"
    );
}
