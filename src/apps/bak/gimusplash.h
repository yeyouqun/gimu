//
// gimu app splash window's implementation.
// Author:yeyouqun@163.com
// 2010-8-7
//

#ifndef __GM_SPLASH_H__
#define __GM_SPLASH_H__

class GmAppFrame;
class GmSplashWindow: public wxTopLevelWindow
{
public:
    // for RTTI macros only
    GmSplashWindow() {}
    GmSplashWindow(const wxBitmap& bitmap, int milliseconds,
                   GmAppFrame * parent, wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxSIMPLE_BORDER | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP);
    virtual ~GmSplashWindow();
private:
    void OnCloseWindow(wxCloseEvent& event);
    void OnNotify(wxTimerEvent& event);
    void OnPaint(wxPaintEvent& event);
protected:
	enum { GM_SPLASH_TIMER_ID = 10000, };
    wxTimer                 m_timer;
	wxBitmap				m_bitmap;

    DECLARE_DYNAMIC_CLASS(GmSplashWindow)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(GmSplashWindow)
	GmAppFrame *			m_parent;
};

#endif //__GM_SPLASH_H__
