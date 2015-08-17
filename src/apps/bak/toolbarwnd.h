//
// tool bar's windwos control
// Author:yeyouqun@163.com
// 2010-4-1
//
#include <wx/control.h>
#ifndef __TOOL_BAR_WND_H__
#define __TOOL_BAR_WND_H__

class GmToolBarWnd : public wxControl
{
public:
	GmToolBarWnd (wxToolBar * parent, int id);
	GmToolBarWnd () : wxControl () {}
	~GmToolBarWnd ();
private:
	enum {
		CMD_ABOUT = 10000,
		CMD_PROG_CONFIGURE,
	};

	void OnRightClickWnd (wxMouseEvent &event);
	void OnAbout (wxCommandEvent &event);
	void OnConfigureProg (wxCommandEvent &event);

	wxToolBar *			m_ToolBar;
	DECLARE_DYNAMIC_CLASS (GmToolBarWnd)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmToolBarWnd)
};
DECLARE_GM_EVENT_TYPE (GmEVT_TOOLWND_RPOG_CONF)
#endif //__TOOL_BAR_WND_H__
