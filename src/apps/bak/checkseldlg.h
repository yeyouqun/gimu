//
// file select dialog's definition that with checkbox ctrl.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include <wx/dialog.h>
#include "checktreectrl.h"

#ifndef __GM_SELELCT_DLG_H__
#define __GM_SELELCT_DLG_H__

class GmSysFileTreeCtrl;

class GmFileSelectDialog : public wxDialog
{
public:
	GmFileSelectDialog (wxWindow * parent, const wxString & title, GmJobItem * pJobItem = 0);
	~GmFileSelectDialog ();
	const GmJobItem & GetJobItem () const;
	bool RunAtOnce () const;
private:
	void InitControlStatus (GmJobItem * pJobItem);
	void OnOptions (wxCommandEvent & event);
	void OnSelectDirectory (wxCommandEvent & event);
	void OnSelectFileFormat (wxCommandEvent & event);
	void OnOK (wxCommandEvent & event);
	enum CTRL_ID
	{
		IDC_CHECK_BOX_TREE = 10000,
		IDC_JOB_NAME,
		IDC_JOB_DEST,
		IDC_JOB_FORMAT,
		IDC_JOB_OPTIONS,
		IDC_JOB_PLAN,
		IDC_JOB_DEST_SEL_BUTT,
		IDC_RUN_AT_ONCE,
	};
private:
	GmSysFileTreeCtrl	*	m_tree;
	wxTextCtrl *			m_JobName;
	wxComboBox *			m_JobDest;
	wxComboBox *			m_OutFormat;
	GmJobItem *				m_pTempJobItem;

	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmFileSelectDialog)
};

class GmSysFileTreeCtrl : public GmCheckTreeCtrl
{
public:
	GmSysFileTreeCtrl ( wxWindow* parent
					, wxWindowID id
					, const wxPoint& pos = wxDefaultPosition
					, const wxSize& size = wxSize (400, 400)
					, long style = wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_ROW_LINES
					, const wxValidator& validator = wxDefaultValidator
					, const wxString& name = wxT ("treeCtrl"));
	GmSysFileTreeCtrl () : GmCheckTreeCtrl () {}
	void GetSelectedPathes (vector<wxString> & pathes);
	bool SetNodeSelected (const wxString & pathfile);
protected:
	//
	// Menu Item
	//
	void OnMouseRight (wxTreeEvent & event);
	void OnSelectItem (wxCommandEvent & event);
	void OnDeSelectItem (wxCommandEvent & event);
	void OnShowProperties (wxCommandEvent & event);
	////////
	bool SetNodeSelected (wxTreeItemId item, const wxString & pathfile);
	void AddRootNodes (wxTreeItemId parent);
	void AddDeskTopItems (wxTreeItemId parent);
	void AddDiskItems (wxTreeItemId parent);
	void AddNetworkItems (wxTreeItemId parent);
	void AddDirectoryItems (const wxString & dir, wxTreeItemId parent);
	void OnExpandingItem (wxTreeEvent & event);
	void OnExpandItem (wxTreeEvent & event);
	enum MENU_ID
	{
		IDC_SELECT = 1000,
		IDC_DESELECT,
		IDC_NODE_PROPERTIES,
	};
private:
	DECLARE_DYNAMIC_CLASS (GmSysFileTreeCtrl)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmSysFileTreeCtrl)
};

void SetNewJobItemWithDemandParams (GmJobItem * pNewJob
									, const wxString & szName
									, const wxString & szDest
									, const wxString & szTypeStr);

typedef GmFileSelectDialog GmJobOptionConfDlg;

#endif //__GM_SELELCT_DLG_H__
