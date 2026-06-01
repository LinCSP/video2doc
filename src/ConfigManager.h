#pragma once

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/gdicmn.h>

class ConfigManager {
public:
    static wxString GetLastVideoPath();
    static void SetLastVideoPath(const wxString& path);

    static wxString GetLastOutputDir();
    static void SetLastOutputDir(const wxString& dir);

    static int GetScreenshotInterval();
    static void SetScreenshotInterval(int interval);

    static wxArrayString GetProjects();
    static void SetProjects(const wxArrayString& projects);

    static wxString GetPromptTemplate();
    static void SetPromptTemplate(const wxString& tpl);

    static wxSize GetWindowSize();
    static void SetWindowSize(const wxSize& size);

    static bool GetWindowMaximized();
    static void SetWindowMaximized(bool maximized);

    static wxString GetDefaultPromptTemplate();

private:
    static wxString GetConfigGroup();
};
