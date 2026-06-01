#pragma once

#include <wx/string.h>

class PathValidator {
public:
    static bool IsValidVideoFile(const wxString& path);
    static bool EnsureDirExists(const wxString& path);
    static wxString GetDefaultOutputDir(const wxString& videoPath);
    static wxString GetVideoBaseName(const wxString& videoPath);
};
