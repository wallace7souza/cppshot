#ifndef CPPSHOT_APP_H
#define CPPSHOT_APP_H

#include <wx/wx.h>

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
    void CaptureAndLaunchEditor(int displayIndex);
    void RestartCapture();
};

wxDECLARE_APP(MyApp);

#endif // CPPSHOT_APP_H
