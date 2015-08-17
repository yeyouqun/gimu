//
// job tree's implementation.
// Author:yeyouqun@163.com
// 2010-2-8
//
#ifndef __GM_JOB_TREE_H__
#define __GM_JOB_TREE_H__

class GmSysFileTreeCtrl;
class GmTreeWindow : public wxPanel
{
public:
	GmTreeWindow (GmAppFrame * m_pMainFrame, wxSplitterWindow * pParent);
	void SetDefaultType (const wxString &);
private:
	enum {
		IDC_FAST_LAUNCH_JOB = 1000,
		IDC_CLEAR_SEL,
		IDC_JOB_NAME,
		IDC_JOB_DEST,
		IDC_JOB_FORMAT,
		IDC_JOB_DEST_SEL_BUTT,
	};

	void OnSelectDirectory (wxCommandEvent & event);
	void OnFastLaunchJob (wxCommandEvent & event);
	void OnClearSelect (wxCommandEvent & event);
	void OnSelectFileFormat (wxCommandEvent & event);
	void OnSetJobName (wxCommandEvent & event);
	void OnSetDirectory (wxCommandEvent & event);
	void OnTreeChanged (wxCommandEvent &event);
	void EnableFastLaunchButtons (bool);
private:
	GmAppFrame *			m_pMainFrame;
	wxTextCtrl *			m_JobName;
	wxComboBox *			m_JobDest;
	wxComboBox *			m_OutFormat;
	GmSysFileTreeCtrl *		m_pSystemFileTree;
private:
	DECLARE_EVENT_TABLE ()
};

#endif //__GM_JOB_TREE_H__

