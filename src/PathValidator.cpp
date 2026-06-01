#include "PathValidator.h"
#include <wx/filefn.h>
#include <wx/filename.h>

bool PathValidator::IsValidVideoFile(const wxString& path) {
    if (path.IsEmpty()) return false;
    if (!wxFileExists(path)) return false;
    wxFileName fn(path);
    wxString ext = fn.GetExt().Lower();
    return ext == wxT("mp4") || ext == wxT("mkv") || ext == wxT("avi") || ext == wxT("mov") ||
           ext == wxT("webm") || ext == wxT("flv") || ext == wxT("wmv");
}

bool PathValidator::EnsureDirExists(const wxString& path) {
    if (path.IsEmpty()) return false;
    if (wxDirExists(path)) return true;
    return wxMkdir(path, wxS_DIR_DEFAULT);
}

wxString PathValidator::GetDefaultOutputDir(const wxString& videoPath) {
    wxFileName fn(videoPath);
    wxString baseName = fn.GetName();
    wxFileName outFn(fn.GetPath(), baseName);
    return outFn.GetFullPath();
}

wxString PathValidator::GetVideoBaseName(const wxString& videoPath) {
    wxFileName fn(videoPath);
    return fn.GetName();
}
