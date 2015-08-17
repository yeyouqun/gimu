//
// file restoration dialog's definition that with checkbox ctrl.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include <wx/dialog.h>
#include <wx/image.h>
#include <wx/treectrl.h>
class GmRootCreator;
class wxTreeEvent;

#ifndef __GM_REST_DLG_H__
#define __GM_REST_DLG_H__
class GmCheckTreeCtrl;
struct GmStoreEntry;

class GmRestoreDialog : public wxDialog
{
public:
	GmRestoreDialog (wxWindow * parent, const wxString & title);
	void ShowTree (GmRootCreator * ptree);
	~GmRestoreDialog ();
	int GetRestoreOption () const;
	wxString GetDestPath () const;
	void GetSelectedItems (vector<GmStoreEntry*> & items) const;
private:
	void OnOK (wxCommandEvent &event);
	void OnToOriginal (wxCommandEvent &event);
	void OnToSpecifiedPath (wxCommandEvent &event);
	void OnSetOptions (wxCommandEvent &event);
	void OnSelectDirectory (wxCommandEvent &event);
	void OnExpandItem (wxTreeEvent & event);
	void OnExpandingItem (wxTreeEvent & event);
	///////
	//
	// Menu Item
	//
	void OnMouseRight (wxTreeEvent & event);
	void OnSelectItem (wxCommandEvent & event);
	void OnDeSelectItem (wxCommandEvent & event);
	void OnShowProperties (wxCommandEvent & event);
	void ExpandThisItem (wxTreeItemId item);
	enum {
		IDC_TREE_CTRL = 1000,
		IDC_REST_OPTIONS,
		IDC_TO_ORIGINAL,
		IDC_TO_SPECIFIED_PATH,
		IDC_JOB_DEST_SEL_BUTT,
		IDC_SELECT,
		IDC_DESELECT,
		IDC_NODE_PROPERTIES,
	};

	GmCheckTreeCtrl *	m_pTree;
	wxTextCtrl *		m_pPath;
	int					m_option;
private:
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmRestoreDialog)
};


#endif //__GM_REST_DLG_H__
