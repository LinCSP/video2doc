#include <wx/wx.h>
#include "MainFrame.h"

class Video2DocApp : public wxApp {
public:
    virtual bool OnInit() override {
        MainFrame* frame = new MainFrame(wxT("Video2Doc"));
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(Video2DocApp);
