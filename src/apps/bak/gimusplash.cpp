//
// gimu app splash window's implementation.
// Author:yeyouqun@163.com
// 2010-8-7
//
#include <gmpre.h>
#include <wx/event.h>
#include <wx/splash.h>
#include <wx/region.h>
#include "gimusplash.h"
#include "gimuframe.h"

IMPLEMENT_DYNAMIC_CLASS(GmSplashWindow, wxTopLevelWindow)
BEGIN_EVENT_TABLE(GmSplashWindow, wxTopLevelWindow)
    EVT_PAINT(GmSplashWindow::OnPaint)
    EVT_TIMER(GM_SPLASH_TIMER_ID, GmSplashWindow::OnNotify)
    EVT_CLOSE(GmSplashWindow::OnCloseWindow)
END_EVENT_TABLE()


static void GmDrawSplashBitmap (wxDC& dc, const wxBitmap& bitmap, int WXUNUSED(x), int WXUNUSED(y))
{
    wxMemoryDC dcMem;

#ifdef USE_PALETTE_IN_SPLASH
    bool hiColour = (wxDisplayDepth() >= 16) ;

    if (bitmap.GetPalette() && !hiColour)
    {
        dcMem.SetPalette(* bitmap.GetPalette());
    }
#endif // USE_PALETTE_IN_SPLASH

    dcMem.SelectObjectAsSource(bitmap);
    dc.Blit(0, 0, bitmap.GetWidth(), bitmap.GetHeight(), &dcMem, 0, 0, wxCOPY,
            true /* use mask */);
    dcMem.SelectObject(wxNullBitmap);

#ifdef USE_PALETTE_IN_SPLASH
    if (bitmap.GetPalette() && !hiColour)
    {
        dcMem.SetPalette(wxNullPalette);
    }
#endif // USE_PALETTE_IN_SPLASH
}

/* Note that unless we pass a non-default size to the frame, SetClientSize
 * won't work properly under Windows, and the splash screen frame is sized
 * slightly too small.
 */

GmSplashWindow::GmSplashWindow (const wxBitmap& bitmap
							, int milliseconds
							, GmAppFrame * parent
							, wxWindowID id
							, const wxPoint& pos
							, const wxSize& size
							, long style):
							wxTopLevelWindow (parent, id, wxEmptyString, wxPoint(0,0), wxSize(100, 100), style)
							, m_parent (parent)
{
	m_bitmap = bitmap;
	SetSize (wxSize(m_bitmap.GetWidth(), m_bitmap.GetHeight()));
	wxRegion Region (m_bitmap, *wxWHITE);
	SetShape(Region);
	CenterOnScreen ();
	m_timer.SetOwner (this, GM_SPLASH_TIMER_ID);
	m_timer.Start (milliseconds);
	Show ();
}

GmSplashWindow::~GmSplashWindow()
{
    m_timer.Stop();
}

void GmSplashWindow::OnNotify(wxTimerEvent& WXUNUSED(event))
{
    Close(true);
}

void GmSplashWindow::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
	m_parent->Show ();
	m_parent->Maximize ();
	m_parent->StartRegisterTimer ();
    m_timer.Stop();
    this->Destroy();
}

void GmSplashWindow::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    if (m_bitmap.Ok()) GmDrawSplashBitmap (dc, m_bitmap, 0, 0);
}
