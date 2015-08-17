//
// check list's definition.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include "checklistctrl.h"

IMPLEMENT_DYNAMIC_CLASS (GmCheckListCtrl, wxListCtrl)
	BEGIN_EVENT_TABLE(GmCheckListCtrl, wxListCtrl)
END_EVENT_TABLE()

GmCheckListCtrl::GmCheckListCtrl (wxWindow* parent
								, wxWindowID id
								, const wxPoint& pos
								, const wxSize& size
								, long style
								, const wxValidator& validator
								, const wxString& name)
								: wxListCtrl (parent, id, pos, size, style, validator, name)
{
}
