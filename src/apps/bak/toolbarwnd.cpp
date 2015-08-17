//
// tool bar's windwos control
// Author:yeyouqun@163.com
// 2010-4-1
//

#include <gmpre.h>
#include "toolbarwnd.h"
#include "about.h"

IMPLEMENT_DYNAMIC_CLASS (GmToolBarWnd, wxControl)
BEGIN_EVENT_TABLE(GmToolBarWnd, wxControl)
	EVT_RIGHT_UP (GmToolBarWnd::OnRightClickWnd)
	EVT_MENU (CMD_PROG_CONFIGURE, GmToolBarWnd::OnConfigureProg)
	EVT_MENU (CMD_ABOUT, GmToolBarWnd::OnAbout)
END_EVENT_TABLE()

GmToolBarWnd::GmToolBarWnd (wxToolBar * parent, int id)
					: wxControl (parent, id, wxDefaultPosition, wxDefaultSize)
					, m_ToolBar (parent)
{
}

GmToolBarWnd::~GmToolBarWnd ()
{
}

void GmToolBarWnd::OnRightClickWnd (wxMouseEvent &event)
{
	wxMenu popup;
	popup.Append (CMD_ABOUT, _("IDS_ABOUT_GMSOFT"));
	popup.Append (CMD_PROG_CONFIGURE, _("IDS_PROG_OPTIONS_ITEM"));
	PopupMenu (&popup);
}


void GmToolBarWnd::OnAbout (wxCommandEvent &event)
{
	GmAboutDialog dlg (this, _("IDS_ABOUT_GMSOFT"));
	dlg.ShowModal ();
}

void GmToolBarWnd::OnConfigureProg (wxCommandEvent &event)
{
	wxCommandEvent cmd (GmEVT_TOOLWND_RPOG_CONF, wxID_ANY);
	event.SetEventObject (this);
	AddPendingEvent (cmd);
}

DEFINE_EVENT_TYPE (GmEVT_TOOLWND_RPOG_CONF)
