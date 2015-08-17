//
// job opt's implementation.
// Author:yeyouqun@163.com
// 2010-2-28
//
#include <wx/notebook.h>

#ifndef __GM_OPTIONS_DLG_H__
#define __GM_OPTIONS_DLG_H__
class GmJobItem;

class GmOptionsDlg : public wxDialog
{
public:
	GmOptionsDlg (wxWindow * parent, const wxString & title);
	void InitControlStatus (GmJobItem * pJobItem);
	void SetJobData (GmJobItem * pJobItem);
private:
	void OnOK (wxCommandEvent &event);
private:
	wxBookCtrl *	m_pOptions;
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmOptionsDlg)
};

class GmRestorePropertyDlg : public wxDialog
{
public:
	GmRestorePropertyDlg (wxWindow * parent,  const GmStoreEntry & entry, const wxString & title);
};

bool CheckJobNameAndDest (const wxString & jobname, wxString & dest);

class GmProgOptionsDlg : public wxDialog
{
public:
	GmProgOptionsDlg (wxWindow * parent, const wxString & title);
private:
	void InitControls ();
	void OnOK (wxCommandEvent &event);
	enum {
		IDC_PASSWORD = 10000,
		IDC_MINI_TO_TRAY,
		IDC_START_WITH_OS,
		IDC_LANG_SIMPLIFIED_CHINESE,
		IDC_LANG_TRADITIONAL_CHINESE,
		IDC_LANG_ENGLISH,
		IDC_FORMAT_COPY,
		IDC_FORMAT_SYNC,
		IDC_FORMAT_ZIP,
		IDC_FORMAT_7Z,
		IDC_GET_DATA_SNAPMODE_FALSE,
		IDC_GET_DATA_SNAPMODE,
		IDC_LOG_REPORT_LIMIT,
	};
private:
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmProgOptionsDlg)
};

class GmPasswordDlg : public wxDialog
{
public:
	GmPasswordDlg (wxWindow * parent, const wxString & title);
	wxString GetPassword () const;
private:
	enum {
		IDC_PASSWORD = 10000,
	};
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmPasswordDlg)
};
#endif //