//
// App definition: checkbox tree ctrl.
// Author:yeyouqun@163.com
// 2010-1-24
//

#include <gmpre.h>
#include <wx/treectrl.h>

#ifndef __GM_CHECK_TREE_H__
#define __GM_CHECK_TREE_H__

	enum ImageIndex
	{
		II_DESKTOP,
		II_DESKTOP_SELECTED,
		II_MY_COMPUTER,
		II_DISK,
		II_DISK_SELECTED,
		II_FOLDER,
		II_FOLDER_SELECTED,
		II_FILE,
		II_FILE_SELECTED,
		II_NETWORK,
		II_DESKTOP_HF_SELECTED,
		II_DISK_HF_SELECTED,
		II_FOLDER_HF_SELECTED,
		II_TIMEPNT,
		II_TIMEPNT_SELECTED,
		II_TIMEPNT_HF_SELECTED,
		II_CD,
		II_CD_SELECTED,
		II_CD_HF_SELECTED,
	};

bool IsSelectedImage (int image);
int GetSelectedImage (int image);
int GetUnSelectedImage (int image);
int GetHfSellectedImage (int image);

class GmCheckTreeCtrl : public wxTreeCtrl
{
public:
	GmCheckTreeCtrl (wxWindow* parent
					, wxWindowID id
					, const wxPoint& pos = wxDefaultPosition
					, const wxSize& size = wxSize (400, 400)
					, long style = wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_ROW_LINES
					, const wxValidator& validator = wxDefaultValidator
					, const wxString& name = wxT ("treeCtrl"));
	GmCheckTreeCtrl () : wxTreeCtrl () {}
	void GetFullSelectedItems (vector<wxTreeItemData*> & vitems);
	void ClearAllSelected ();
	void AddForgedNode (wxTreeItemId & parent);
	void SetSubSelected (wxTreeItemId item);
	void SetSubSuperSelected (wxTreeItemId item, int image);
	void SetSubSuperUnSelected (wxTreeItemId item, int image);
protected:
	//
	// Other
	//
	int GetSelectedImage (wxTreeItemId item);
	int GetUnSelectedImage (wxTreeItemId item);
	void OnMouseLeftDown (wxMouseEvent & event);
	void GetSubSelectedPathes (wxTreeItemId item, vector<wxTreeItemData*> & vitems);
	void SetSubUnSelected (wxTreeItemId item);
	int GetSubFullSelectedCount (wxTreeItemId parent, int & hfselected);
	void SetSuperItem (wxTreeItemId item);
private:
	wxImageList		m_ImageList;
private:
	DECLARE_DYNAMIC_CLASS (GmCheckTreeCtrl)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmCheckTreeCtrl)
};

DECLARE_GM_EVENT_TYPE (GmEVT_NODE_CHANGED)

#endif //__GM_CHECK_TREE_H__
