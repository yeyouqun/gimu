//
// check list's definition.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <wx/listctrl.h>

#ifndef __GM_CHECK_LIST_H__
#define __GM_CHECK_LIST_H__


class GmCheckListCtrl : public wxListCtrl
{
public:
	GmCheckListCtrl (wxWindow* parent
					, wxWindowID id
					, const wxPoint& pos = wxDefaultPosition
					, const wxSize& size = wxDefaultSize
					, long style = wxLC_ICON
					, const wxValidator& validator = wxDefaultValidator
					, const wxString& name = wxListCtrlNameStr);

	GmCheckListCtrl () : wxListCtrl () {}
private:
	DECLARE_DYNAMIC_CLASS (GmCheckListCtrl)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmCheckListCtrl)
};

#endif //__GM_CHECK_LIST_H__
