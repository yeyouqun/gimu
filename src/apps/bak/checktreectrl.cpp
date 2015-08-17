//
// App definition: checkbox tree ctrl.
// Author:yeyouqun@163.com
// 2010-1-24
//

#include "checktreectrl.h"
#include "res/dlg_res/desktop.xpm"
#include "res/dlg_res/desktop_selected.xpm"
#include "res/dlg_res/disk.xpm"
#include "res/dlg_res/disk_selected.xpm"
#include "res/dlg_res/file.xpm"
#include "res/dlg_res/file_selected.xpm"
#include "res/dlg_res/folder.xpm"
#include "res/dlg_res/folder_selected.xpm"
#include "res/dlg_res/network_noselected.xpm"
#include "res/dlg_res/my_computer_no_selected.xpm"
#include "res/dlg_res/desktop_hf_selected.xpm"
#include "res/dlg_res/disk_hf_selected.xpm"
#include "res/dlg_res/folder_hf_selected.xpm"
#include "res/dlg_res/time_pnt.xpm"
#include "res/dlg_res/time_pnt_hf_selected.xpm"
#include "res/dlg_res/time_pnt_selected.xpm"

#include "res/dlg_res/cd.xpm"
#include "res/dlg_res/cd_selected.xpm"
#include "res/dlg_res/cd_hf_selected.xpm"


IMPLEMENT_DYNAMIC_CLASS (GmCheckTreeCtrl, wxTreeCtrl)
BEGIN_EVENT_TABLE(GmCheckTreeCtrl, wxTreeCtrl)
	EVT_LEFT_DOWN (GmCheckTreeCtrl::OnMouseLeftDown)
END_EVENT_TABLE()

GmCheckTreeCtrl::GmCheckTreeCtrl (wxWindow* parent
								, wxWindowID id
								, const wxPoint& pos
								, const wxSize& size
								, long style
								, const wxValidator& validator
								, const wxString& name)
								: wxTreeCtrl (parent, id, pos, size, style, validator, name)
								, m_ImageList (32, 16)
{
	m_ImageList.Add (wxBitmap (desktop_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (desktop_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (my_computer_no_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (disk_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (disk_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (folder_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (folder_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (file_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (file_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (network_noselected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (desktop_hf_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (disk_hf_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (folder_hf_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (time_pnt_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (time_pnt_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (time_pnt_hf_selected_xpm), wxColor (0, 0, 0));	
	m_ImageList.Add (wxBitmap (cd_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (cd_selected_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (cd_hf_selected_xpm), wxColor (0, 0, 0));
	SetImageList (&m_ImageList);
	wxTreeItemId root = AddRoot (wxT (""));
}

void GmCheckTreeCtrl::GetSubSelectedPathes (wxTreeItemId item, vector<wxTreeItemData*> & vitems)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (item, cookie);
	while (child.IsOk ()) {
		int image = GetItemImage (child);
		wxTreeItemData * pData = 0;
		switch (image)
		{
			case II_DESKTOP_SELECTED:
			case II_DISK_SELECTED:
			case II_FOLDER_SELECTED:
			case II_FILE_SELECTED:
			case II_TIMEPNT_SELECTED:
			case II_CD_SELECTED:
				//
				// 如果是选择状态，则立即取得。
				//
				pData = (wxTreeItemData*)GetItemData (child);
				if (pData != 0) vitems.push_back (pData);
				break;
			case II_MY_COMPUTER:
			case II_NETWORK:
			case II_DESKTOP_HF_SELECTED:
			case II_DISK_HF_SELECTED:
			case II_FOLDER_HF_SELECTED:
			case II_TIMEPNT_HF_SELECTED:
			case II_CD_HF_SELECTED:
				//
				// 半选状态，则递归选取。
				//
				GetSubSelectedPathes (child, vitems);
				break;
			default:
				break;
		}

		child = GetNextChild (item, cookie);
	}
}

int GetSelectedImage (int image)
{
	switch (image)
	{
		case II_DESKTOP:
		case II_DESKTOP_SELECTED:
		case II_DESKTOP_HF_SELECTED:
			image = II_DESKTOP_SELECTED;
			break;
		case II_DISK:
		case II_DISK_SELECTED:
		case II_DISK_HF_SELECTED:
			image = II_DISK_SELECTED;
			break;
		case II_FOLDER:
		case II_FOLDER_SELECTED:
		case II_FOLDER_HF_SELECTED:
			image = II_FOLDER_SELECTED;
			break;
		case II_TIMEPNT:
		case II_TIMEPNT_SELECTED:
		case II_TIMEPNT_HF_SELECTED:
			image = II_TIMEPNT_SELECTED;
			break;
		case II_CD:
		case II_CD_SELECTED:
		case II_CD_HF_SELECTED:
			image = II_CD_SELECTED;
			break;
		case II_FILE:
		case II_FILE_SELECTED:
			image = II_FILE_SELECTED;
			break;
		default:
			break;
	}

	return image;
}

int GetUnSelectedImage (int image)
{
	switch (image)
	{
		case II_DESKTOP:
		case II_DESKTOP_SELECTED:
		case II_DESKTOP_HF_SELECTED:
			image = II_DESKTOP;
			break;
		case II_DISK:
		case II_DISK_SELECTED:
		case II_DISK_HF_SELECTED:
			image = II_DISK;
			break;
		case II_FOLDER:
		case II_FOLDER_SELECTED:
		case II_FOLDER_HF_SELECTED:
			image = II_FOLDER;
			break;
		case II_TIMEPNT:
		case II_TIMEPNT_SELECTED:
		case II_TIMEPNT_HF_SELECTED:
			image = II_TIMEPNT;
			break;
		case II_CD:
		case II_CD_SELECTED:
		case II_CD_HF_SELECTED:
			image = II_CD;
			break;
		case II_FILE:
		case II_FILE_SELECTED:
			image = II_FILE;
			break;
		default:
			break;
	}

	return image;
}

int GetHfSellectedImage (int image)
{
	switch (image)
	{
		case II_DESKTOP:
		case II_DESKTOP_SELECTED:
		case II_DESKTOP_HF_SELECTED:
			image = II_DESKTOP_HF_SELECTED;
			break;
		case II_DISK:
		case II_DISK_SELECTED:
		case II_DISK_HF_SELECTED:
			image = II_DISK_HF_SELECTED;
			break;
		case II_FOLDER:
		case II_FOLDER_SELECTED:
		case II_FOLDER_HF_SELECTED:
			image = II_FOLDER_HF_SELECTED;
			break;
		case II_TIMEPNT:
		case II_TIMEPNT_SELECTED:
		case II_TIMEPNT_HF_SELECTED:
			image = II_TIMEPNT_HF_SELECTED;
			break;
		case II_CD:
		case II_CD_SELECTED:
		case II_CD_HF_SELECTED:
			image = II_CD_HF_SELECTED;
			break;
		default:
			break;
	}

	return image;
}

int GmCheckTreeCtrl::GetSelectedImage (wxTreeItemId item)
{
	int image = GetItemImage (item);
	if (image == -1)
		return -1;
	return ::GetSelectedImage (image);
}

int GmCheckTreeCtrl::GetUnSelectedImage (wxTreeItemId item)
{
	int image = GetItemImage (item);
	if (image == -1)
		return -1;
	return ::GetUnSelectedImage (image);
}

void GmCheckTreeCtrl::GetFullSelectedItems (vector<wxTreeItemData*> & vitems)
{
	//
	// 从根往下，遇到第一个 Selected 就取得一个。
	//
	wxTreeItemId root = GetRootItem ();
	vitems.clear ();
	GetSubSelectedPathes (root, vitems);
}

void GmCheckTreeCtrl::SetSubSelected (wxTreeItemId item)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (item, cookie);
	while (child.IsOk ()) {
		int image = GetItemImage (child);
		int NewImage = image;
		switch (image)
		{
			case II_DESKTOP:
			case II_DESKTOP_HF_SELECTED:
				NewImage = II_DESKTOP_SELECTED;
				SetSubSelected (child);
				break;
			case II_DISK:
			case II_DISK_HF_SELECTED:
				NewImage = II_DISK_SELECTED;
				SetSubSelected (child);
				break;
			case II_FOLDER:
			case II_FOLDER_HF_SELECTED:
				NewImage = II_FOLDER_SELECTED;
				SetSubSelected (child);
				break;
			case II_TIMEPNT:
			case II_TIMEPNT_HF_SELECTED:
				NewImage = II_TIMEPNT_SELECTED;
				SetSubSelected (child);
				break;
			case II_CD:
			case II_CD_HF_SELECTED:
				NewImage = II_CD_SELECTED;
				SetSubSelected (child);
				break;
			case II_FILE:
				NewImage = II_FILE_SELECTED;
				break;
			default:
				break;
		}

		SetItemImage (child, NewImage);
		child = GetNextChild (item, cookie);
	}
}

void GmCheckTreeCtrl::SetSubUnSelected (wxTreeItemId item)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (item, cookie);
	while (child.IsOk ()) {
		int image = GetItemImage (child);
		int NewImage = image;
		switch (image)
		{
			case II_MY_COMPUTER:
			case II_NETWORK:
				SetSubUnSelected (child);
				break;
			case II_DESKTOP_SELECTED:
			case II_DESKTOP_HF_SELECTED:
				NewImage = II_DESKTOP;
				SetSubUnSelected (child);
				break;
			case II_DISK_SELECTED:
			case II_DISK_HF_SELECTED:
				NewImage = II_DISK;
				SetSubUnSelected (child);
				break;
			case II_FOLDER_SELECTED:
			case II_FOLDER_HF_SELECTED:
				NewImage = II_FOLDER;
				SetSubUnSelected (child);
				break;
			case II_TIMEPNT_SELECTED:
			case II_TIMEPNT_HF_SELECTED:
				NewImage = II_TIMEPNT;
				SetSubUnSelected (child);
				break;
			case II_CD_SELECTED:
			case II_CD_HF_SELECTED:
				NewImage = II_CD;
				SetSubUnSelected (child);
				break;
			case II_FILE_SELECTED:
				NewImage = II_FILE;
				break;
			default:
				break;
		}

		SetItemImage (child, NewImage);
		child = GetNextChild (item, cookie);
	}
}

int GmCheckTreeCtrl::GetSubFullSelectedCount (wxTreeItemId parent, int & hfselected)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (parent, cookie);
	int nFullSel = 0;
	hfselected = 0;
	while (child.IsOk ()) {
		int image = GetItemImage (child);
		switch (image)
		{
			case II_DESKTOP_SELECTED:
			case II_DISK_SELECTED:
			case II_FOLDER_SELECTED:
			case II_FILE_SELECTED:
			case II_TIMEPNT_SELECTED:
			case II_CD_SELECTED:
				++nFullSel;
				break;
			case II_DESKTOP_HF_SELECTED:
			case II_DISK_HF_SELECTED:
			case II_FOLDER_HF_SELECTED:
			case II_TIMEPNT_HF_SELECTED:
			case II_CD_HF_SELECTED:
				++hfselected;
				break;
			default:
				break;
		}
		child = GetNextChild (parent, cookie);
	}

	return nFullSel;
}

void GmCheckTreeCtrl::SetSuperItem (wxTreeItemId item)
{
	while (true) {
		item = GetItemParent (item);
		if (item == GetRootItem ())
			return;

		int image = GetItemImage (item);
		if (image == II_MY_COMPUTER || image == II_NETWORK)
			return;

		int NewImage = image;
		int hfselected = 0;
		int nFullCount = GetSubFullSelectedCount (item, hfselected);
		int nCount = GetChildrenCount (item, false);
		if (nFullCount == nCount) {
			//
			// 子元素全选择，那么你元素也全选。
			//
			switch (image)
			{
				case II_DESKTOP:
				case II_DESKTOP_SELECTED:
				case II_DESKTOP_HF_SELECTED:
					NewImage = II_DESKTOP_SELECTED;
					break;
				case II_DISK:
				case II_DISK_SELECTED:
				case II_DISK_HF_SELECTED:
					NewImage = II_DISK_SELECTED;
					break;
				case II_FOLDER:
				case II_FOLDER_SELECTED:
				case II_FOLDER_HF_SELECTED:
					NewImage = II_FOLDER_SELECTED;
					break;
				case II_TIMEPNT:
				case II_TIMEPNT_SELECTED:
				case II_TIMEPNT_HF_SELECTED:
					NewImage = II_TIMEPNT_SELECTED;
					break;
				case II_CD:
				case II_CD_SELECTED:
				case II_CD_HF_SELECTED:
					NewImage = II_CD_SELECTED;
					break;
				default:
					break;
			}
		}
		else if (hfselected > 0 || (nFullCount != 0 && nFullCount < nCount)) {
			//
			//  如果子目录中有半选的，或者没有全选，那么父则半先，如果上面两个都没有满足，
			// 那么子元素是一个都没有选择，那么父元素则也取消选择。
			//
			switch (image)
			{
				case II_DESKTOP:
				case II_DESKTOP_SELECTED:
				case II_DESKTOP_HF_SELECTED:
					NewImage = II_DESKTOP_HF_SELECTED;
					break;
				case II_DISK:
				case II_DISK_SELECTED:
				case II_DISK_HF_SELECTED:
					NewImage = II_DISK_HF_SELECTED;
					break;
				case II_FOLDER:
				case II_FOLDER_SELECTED:
				case II_FOLDER_HF_SELECTED:
					NewImage = II_FOLDER_HF_SELECTED;
					break;
				case II_TIMEPNT:
				case II_TIMEPNT_SELECTED:
				case II_TIMEPNT_HF_SELECTED:
					NewImage = II_TIMEPNT_HF_SELECTED;
					break;
				case II_CD:
				case II_CD_SELECTED:
				case II_CD_HF_SELECTED:
					NewImage = II_CD_HF_SELECTED;
					break;
				default:
					break;
			}
		}
		else {
			switch (image)
			{
				case II_DESKTOP:
				case II_DESKTOP_SELECTED:
				case II_DESKTOP_HF_SELECTED:
					NewImage = II_DESKTOP;
					break;
				case II_DISK:
				case II_DISK_SELECTED:
				case II_DISK_HF_SELECTED:
					NewImage = II_DISK;
					break;
				case II_FOLDER:
				case II_FOLDER_SELECTED:
				case II_FOLDER_HF_SELECTED:
					NewImage = II_FOLDER;
					break;
				case II_TIMEPNT:
				case II_TIMEPNT_SELECTED:
				case II_TIMEPNT_HF_SELECTED:
					NewImage = II_TIMEPNT;
					break;
				case II_CD:
				case II_CD_SELECTED:
				case II_CD_HF_SELECTED:
					NewImage = II_CD;
					break;
				default:
					break;
			}
		}

		SetItemImage (item, NewImage);
	}
}

void GmCheckTreeCtrl::SetSubSuperSelected (wxTreeItemId item, int image)
{
	SetItemImage (item, image);
	SetSubSelected (item);
	SetSuperItem (item);
}

void GmCheckTreeCtrl::SetSubSuperUnSelected (wxTreeItemId item, int image)
{
	SetItemImage (item, image);
	SetSubUnSelected (item);
	SetSuperItem (item);
}

void GmCheckTreeCtrl::OnMouseLeftDown (wxMouseEvent & event)
{
	wxPoint point = event.GetPosition ();
	int flag = -1;
	wxTreeItemId item = HitTest (point, flag);
	if (!item.IsOk ())
		return;

	SelectItem (item);
	if ((flag & wxTREE_HITTEST_ONITEMICON) != 0) {
		int image = GetItemImage (item);
		if (image == -1)
			return;

		//
		// 让只有在点击方框时才有效选择。
		//
		point.x += 16;
		item = HitTest (point, flag);
		if (!item.IsOk ())
			return;

		if ((flag & wxTREE_HITTEST_ONITEMICON) == 0) {
			if (event.LeftDClick ()) {
				if (IsExpanded (item)) Toggle (item);
				else Expand (item);
			}
		}
		else {
#define SelectItem(Item,Image,SelImage)								\
					case Image:										\
					SetSubSuperSelected (Item, SelImage);			\
					break;

#define UnSelectItem(Item,Image,UnSelImage)							\
					case Image:										\
					SetSubSuperUnSelected (Item, UnSelImage);		\
					break;

			switch (image)
			{
				case II_MY_COMPUTER:
				case II_NETWORK:
					break;
				SelectItem (item, II_DESKTOP, II_DESKTOP_SELECTED);
				SelectItem (item, II_DISK, II_DISK_SELECTED);
				SelectItem (item, II_TIMEPNT, II_TIMEPNT_SELECTED);
				SelectItem (item, II_FOLDER, II_FOLDER_SELECTED);
				SelectItem (item, II_CD, II_CD_SELECTED);

				UnSelectItem (item, II_DISK_SELECTED, II_DISK);
				UnSelectItem (item, II_DESKTOP_SELECTED, II_DESKTOP);
				UnSelectItem (item, II_FOLDER_SELECTED, II_FOLDER);
				UnSelectItem (item, II_TIMEPNT_SELECTED, II_TIMEPNT);
				UnSelectItem (item, II_CD_SELECTED, II_CD);

				UnSelectItem (item, II_DISK_HF_SELECTED, II_DISK);
				UnSelectItem (item, II_DESKTOP_HF_SELECTED, II_DESKTOP);
				UnSelectItem (item, II_FOLDER_HF_SELECTED, II_FOLDER);
				UnSelectItem (item, II_TIMEPNT_HF_SELECTED, II_TIMEPNT);
				UnSelectItem (item, II_CD_HF_SELECTED, II_CD);
				case II_FILE:
					SetItemImage (item, II_FILE_SELECTED);
					SetSuperItem (item);
					break;
				case II_FILE_SELECTED:
					SetItemImage (item, II_FILE);
					SetSuperItem (item);
					break;
				default:
					break;
			}
		}
	}
	else if ((flag & wxTREE_HITTEST_ONITEMBUTTON ) != 0) {
		if (IsExpanded (item)) Toggle (item);
		else Expand (item);
	}

	wxCommandEvent cmd (GmEVT_NODE_CHANGED, wxID_ANY);
	event.SetEventObject (this);
	AddPendingEvent (cmd);
	return;
}

void GmCheckTreeCtrl::ClearAllSelected ()
{
	SetSubUnSelected (GetRootItem ());
}

void GmCheckTreeCtrl::AddForgedNode (wxTreeItemId & parent)
{
	AppendItem (parent, wxT (""));
}

bool IsSelectedImage (int image)
{
	switch (image) {
		case II_DESKTOP_SELECTED:
		case II_DISK_SELECTED:
		case II_FOLDER_SELECTED:
		case II_FILE_SELECTED:
		case II_DESKTOP_HF_SELECTED:
		case II_DISK_HF_SELECTED:
		case II_FOLDER_HF_SELECTED:
		case II_TIMEPNT_SELECTED:
		case II_TIMEPNT_HF_SELECTED:
		case II_CD_SELECTED:
		case II_CD_HF_SELECTED:
			return true;
		default:
			return false;
	}

	return false;
}


DEFINE_EVENT_TYPE(GmEVT_NODE_CHANGED);