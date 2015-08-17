//
// job option's implementation.
// Author:yeyouqun@163.com
// 2010-2-28
//
#include <gmpre.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/xml/xml.h>
#include "res/options/config.xpm"
#include "res/options/plan.xpm"
#include "res/options/filter.xpm"
#include <wx/listctrl.h>
#include "res/button/folder.xpm"
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include "engineinc.h"
#include "gmutil.h"
#include "optionsdlg.h"
#include <engine/uifblock.h>
#include "appconfig.h"
#include "password.h"

class GmDirFileFilterDialog : public wxDialog
{
public:
	GmDirFileFilterDialog (wxWindow * parent, const wxString & title);
	GmDirFileFilterDialog () : wxDialog () {}
	wxString GetFileName ()
	{
		return m_pFileName->GetLabel ();
	}
	wxString GetDirName ()
	{
		return m_pDirName->GetLabel ();
	}

private:
	enum {
		IDC_SELECT_DIRECTORY = 1000,
	};

	void OnSelectDirectory (wxCommandEvent &event);
	wxTextCtrl *	m_pDirName;
	wxTextCtrl *	m_pFileName;
	DECLARE_DYNAMIC_CLASS (GmDirFileFilterDialog)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmDirFileFilterDialog)
};

IMPLEMENT_DYNAMIC_CLASS (GmDirFileFilterDialog, wxDialog)
BEGIN_EVENT_TABLE(GmDirFileFilterDialog, wxDialog)
	EVT_BUTTON(IDC_SELECT_DIRECTORY, GmDirFileFilterDialog::OnSelectDirectory)
END_EVENT_TABLE()

void GmDirFileFilterDialog::OnSelectDirectory (wxCommandEvent &event)
{
	wxDirDialog dlg (this);
	if (dlg.ShowModal () == wxID_OK) {
		m_pDirName->SetLabel (dlg.GetPath ());
	}
}

GmDirFileFilterDialog::GmDirFileFilterDialog (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this);
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer * pBoxSizerLine = new wxBoxSizer (wxHORIZONTAL);
	pStaticSizer->Add (pBoxSizerLine, 0, wxLEFT | wxTOP, 15);
	pBoxSizerLine->Add (new wxStaticText (this, wxID_ANY, _("IDS_APPLY_NAME")), 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
	m_pFileName = new wxTextCtrl (this, wxID_ANY, _(""));
	pBoxSizerLine->Add (m_pFileName, 1, wxEXPAND | wxLEFT, 5);

	pBoxSizerLine->Add (new wxStaticText (this, wxID_ANY, _("IDS_AT")), 0, wxLEFT | wxALIGN_CENTER, 5);
	m_pDirName = new wxTextCtrl (this, wxID_ANY, _(""));
	pBoxSizerLine->Add (m_pDirName, 1, wxEXPAND | wxLEFT, 5);

	wxBitmap bitmap (folder_xpm);
	wxImage image = bitmap.ConvertToImage ();
	image.SetMaskColour (0, 0, 0);
	wxBitmapButton * pb = new wxBitmapButton (this, IDC_SELECT_DIRECTORY, wxBitmap (image));

	pBoxSizerLine->Add (pb, 0, wxLEFT, 5);
	pBoxSizerLine->Add (new wxStaticText (this, wxID_ANY, _("IDS_DIRECTORY")), 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);

	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND | wxRIGHT);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnParent ();
}

class GmPlanSomeTimeDialog : public wxDialog
{
public:
	GmPlanSomeTimeDialog (wxWindow * parent, const wxString & title);
	GmPlanSomeTimeDialog () : wxDialog () {}
	char GetPlan (ubyte8 & begin, ubyte8 & end, ubyte4 & WeekOrDays, ubyte2 & AtHour);
private:
	void OnChangeInterval (wxCommandEvent &);
	void OnOK (wxCommandEvent & event);
private:
	enum {
		IDC_MONDAY = 1000,
		IDC_TUESDAY,
		IDC_WEDNESDAY,
		IDC_THURSDAY,
		IDC_FRIDAY,
		IDC_SATURDAY,
		IDC_SUNDAY,
		IDC_INTERVAL,
	};

	wxDatePickerCtrl *	m_pFrom;
	wxDatePickerCtrl *	m_pTo;
	wxSpinCtrl		*	m_pDays;
	wxSpinCtrl		*	m_pHours;
	wxComboBox		*	m_pInterval;
	wxComboBox		*	m_pRunType;
	wxGridSizer *		m_pWeekDays;
	wxBoxSizer *		m_pDaysSizer;

	DECLARE_DYNAMIC_CLASS (GmPlanSomeTimeDialog)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmPlanSomeTimeDialog)
};
//////////////////////////

class GmOptionPage : public wxPanel
{
public:
	GmOptionPage (wxWindow * parent) : wxPanel (parent) {}
	GmOptionPage () : wxPanel () {}
	virtual void InitControlStatus (GmJobItem * pJobItem) = 0;
	virtual void SetJobData (GmJobItem * pJobItem) = 0;
	virtual bool CheckParams () = 0;
};

class GmGeneralOptionPanel : public GmOptionPage
{
public:
	GmGeneralOptionPanel (wxWindow * parent);
	GmGeneralOptionPanel () : GmOptionPage () {}
	void InitControlStatus (GmJobItem * pJobItem);
	void SetJobData (GmJobItem * pJobItem);
	bool CheckParams ();
private:
	void OnSetCompress (wxCommandEvent &);
	void OnSetEncrypt (wxCommandEvent &);
	void OnSelectEncryptStrength (wxCommandEvent &);
	void OnSetSplit (wxCommandEvent &event);
	void SetDupOrSyncTypeControls ();
private:
	GmJobItem *		m_pJobItem;
	wxComboBox *	m_pEncStrength;
	wxTextCtrl *	m_pPasswd;
	wxTextCtrl *	m_pPasswdConfirm;
	wxComboBox *	m_pSplitSize;
	wxTextCtrl *	m_pComment;
	wxSpinCtrl *	m_pTimesMerge;

	wxCheckBox *	m_pSyncRemovedFile;
	wxCheckBox *	m_pCompress;
	wxCheckBox *	m_pEncode;
	enum {
		IDC_SYNC_REMOVED_FILE = 10000,
		IDC_COMPRESS,
		IDC_ENCRYPT,
		IDC_SELECT_STRENGTH,
		IDC_SPLIT_SIZE,
		IDC_COMMENT_MESSAGE,
	};

	DECLARE_DYNAMIC_CLASS (GmGeneralOptionPanel)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmGeneralOptionPanel)
};


IMPLEMENT_DYNAMIC_CLASS (GmGeneralOptionPanel, GmOptionPage)
BEGIN_EVENT_TABLE(GmGeneralOptionPanel, GmOptionPage)
	EVT_CHECKBOX (IDC_COMPRESS, GmGeneralOptionPanel::OnSetCompress)
	EVT_CHECKBOX (IDC_ENCRYPT, GmGeneralOptionPanel::OnSetEncrypt)
	EVT_COMBOBOX (IDC_SELECT_STRENGTH, GmGeneralOptionPanel::OnSelectEncryptStrength)
	EVT_COMBOBOX (IDC_SPLIT_SIZE, GmGeneralOptionPanel::OnSetSplit)
END_EVENT_TABLE()

void GmGeneralOptionPanel::OnSetSplit (wxCommandEvent &event)
{
}


void GmGeneralOptionPanel::OnSetCompress (wxCommandEvent &event)
{
}

void GmGeneralOptionPanel::OnSetEncrypt (wxCommandEvent &event)
{
	bool value = false;
	if (m_pEncode->GetValue ())
		value = true;

	m_pEncStrength->Enable (value);
	m_pPasswd->Enable (value);;
	m_pPasswdConfirm->Enable (value);
}

void GmGeneralOptionPanel::OnSelectEncryptStrength (wxCommandEvent &event)
{
}

GmGeneralOptionPanel::GmGeneralOptionPanel (wxWindow * parent)
				: GmOptionPage (parent)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);

	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_CONFIG_GENERAL"));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 20);

	m_pSyncRemovedFile = new wxCheckBox (this, IDC_SYNC_REMOVED_FILE, _("IDS_SYNC_REMOVED_FILE"));
	m_pCompress = new wxCheckBox (this, IDC_COMPRESS, _("IDS_COMPRESS_DATA"));
	m_pEncode = new wxCheckBox (this, IDC_ENCRYPT, _("IDS_ENCRYPT_DATA"));
	pStaticSizer->Add (m_pSyncRemovedFile, 0, wxBOTTOM, 5);
	pStaticSizer->Add (m_pCompress, 0, wxBOTTOM, 5);
	pStaticSizer->Add (m_pEncode, 0, wxBOTTOM, 5);
	
	//
	// 密码强度
	//
	wxFlexGridSizer * pFlexGridSizer = new wxFlexGridSizer (3, 2, 5, 5);
	pStaticSizer->Add (pFlexGridSizer, 0, wxEXPAND | wxLEFT, 20);

	pFlexGridSizer->AddGrowableCol (1);
	wxArrayString encodes;
	encodes.Add (wxT ("AES-128"));
	encodes.Add (wxT ("AES-192"));
	encodes.Add (wxT ("AES-256"));
	pFlexGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_ENCRYPTION_STRENGTH")), 0, wxALIGN_RIGHT);
	m_pEncStrength = new wxComboBox (this, IDC_SELECT_STRENGTH, _(""), wxDefaultPosition
									, wxDefaultSize, encodes, wxCB_READONLY);
	m_pEncStrength->SetValue (wxT ("AES-128"));
	pFlexGridSizer->Add (m_pEncStrength, 1, wxEXPAND);
	//
	// 密码
	//
	m_pPasswd = new wxTextCtrl (this, wxID_ANY, wxT (""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	pFlexGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_ENCRYPTION_PASSWD")), 0, wxALIGN_RIGHT);
	pFlexGridSizer->Add (m_pPasswd, 1, wxEXPAND);
	//
	// 确证密码
	//
	m_pPasswdConfirm = new wxTextCtrl (this, wxID_ANY, wxT (""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	pFlexGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_ENCRYPTION_PASSWD_CONFIRM")), 0, wxALIGN_RIGHT);
	pFlexGridSizer->Add (m_pPasswdConfirm, 1, wxEXPAND);

	pStaticSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxBOTTOM | wxTOP, 15);

	pFlexGridSizer = new wxFlexGridSizer (3, 2, 5, 5);
	pFlexGridSizer->AddGrowableCol (1);
	pFlexGridSizer->AddGrowableRow (2);
	pFlexGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_MERGE")), 0, wxALIGN_RIGHT);
	m_pTimesMerge = new wxSpinCtrl (this, wxID_ANY, _T("2"));
	m_pTimesMerge->SetRange (2, 100);
	pFlexGridSizer->Add (m_pTimesMerge, 0, wxEXPAND | wxLEFT, 5);
	pFlexGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_SPLIT_SIZE")), 0, wxALIGN_RIGHT);

	m_pEncStrength->Enable (false);
	m_pPasswd->Enable (false);
	m_pPasswdConfirm->Enable (false);

	wxArrayString items;
	items.Add (wxT ("Don't split"));
	items.Add (wxT ("1MB"));
	items.Add (wxT ("5MB"));
	items.Add (wxT ("100MB"));
	items.Add (wxT ("600MB"));
	items.Add (wxT ("2G"));

	m_pSplitSize = new wxComboBox (this, IDC_SPLIT_SIZE, _(""), wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
	m_pSplitSize->SetValue (wxT ("Don't split"));
	pFlexGridSizer->Add (m_pSplitSize, 0, wxEXPAND | wxLEFT, 5);
	pFlexGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_COMMENT_MESSAGE")), 0, wxALIGN_RIGHT);
	m_pComment = new wxTextCtrl (this, IDC_COMMENT_MESSAGE, _(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	pFlexGridSizer->Add (m_pComment, 0, wxEXPAND | wxLEFT, 5);
	pStaticSizer->Add (pFlexGridSizer, 1, wxEXPAND | wxBOTTOM, 20);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

void GmGeneralOptionPanel::SetDupOrSyncTypeControls ()
{
	//
	// 同步或者复制任务，相应的控件必须可用。
	//
	m_pPasswd->Clear ();
	m_pPasswdConfirm->Clear ();

	m_pEncode->SetValue (false);
	m_pCompress->SetValue (false);

	m_pPasswd->Enable (false);
	m_pPasswdConfirm->Enable (false);

	m_pEncode->Enable (false);
	m_pCompress->Enable (false);

	m_pEncStrength->Enable (false);
	m_pSplitSize->Enable (false);
}

void GmGeneralOptionPanel::InitControlStatus (GmJobItem * pJobItem)
{
	assert (pJobItem);
	ubyte4 type = pJobItem->GetJobType ();
	m_pSyncRemovedFile->Enable (false);
	m_pSyncRemovedFile->SetValue (false);

	if (type == GO_TYPE_SYNC || type == GO_TYPE_DUP) {
		SetDupOrSyncTypeControls ();
		if (type == GO_TYPE_SYNC) {
			m_pTimesMerge->Enable (false);
			//
			// 只有同步任务有这个选项。
			//
			m_pSyncRemovedFile->Enable (true);
			bool bSyncRemoveFiles = pJobItem->GetOptions () & GO_SYNC_REMOVED_FILE ? true : false;
			m_pSyncRemovedFile->SetValue (bSyncRemoveFiles);
		}
	}
	else {
		m_pCompress->SetValue (true);
	}

	if (pJobItem->GetDeflateType () != DEF_NO_DEFLATE) {
		m_pCompress->SetValue (true);
	}
	else {
		m_pCompress->SetValue (false);
	}

	if (type != GO_TYPE_SYNC && type != GO_TYPE_DUP) {
		//
		// 可以更改强度，但不能更改密码。只有打打包的任务类型才能更改这些属性。
		//
		m_pPasswd->Enable (false);
		m_pPasswdConfirm->Enable (false);
		m_pPasswd->SetValue (pJobItem->GetPass ());
		m_pPasswdConfirm->SetValue (pJobItem->GetPass ());

		if (pJobItem->GetEncryptType () != ENC_NO_ENC) {
			m_pEncode->SetValue (true);
			m_pEncStrength->SetSelection (pJobItem->GetEncryptType () - 1);
			m_pEncStrength->Enable (true);
		}
		else {
			m_pEncode->SetValue (false);
			m_pEncStrength->Enable (false);
		}

		int select = 0;
		ubyte8 SplitSize = pJobItem->GetSplitSize ();
		if (SplitSize == 1 * 1024 * 1024) {
			select = 1;
		}
		else if (SplitSize == 5 * 1024 * 1024) {
			select = 2;
		}
		else if (SplitSize == 100 * 1024 * 1024) {
			select = 3;
		}
		else if (SplitSize == 600 * 1024 * 1024) {
			select = 4;
		}
		else if (SplitSize == 2 * 1024 * 1024 * 1024LL) {
			select = 5;
		}

		m_pSplitSize->SetSelection (select);
	}
	
	m_pComment->SetValue (pJobItem->DescStr ());
	m_pTimesMerge->SetValue (pJobItem->Times ());
}

bool GmGeneralOptionPanel::CheckParams ()
{
	if (m_pEncode->GetValue () && m_pPasswd->GetValue () != m_pPasswdConfirm->GetValue ()) {
		wxMessageBox (_("IDS_PASSWD_MISMATCH"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	return true;
}

void GmGeneralOptionPanel::SetJobData (GmJobItem * pJobItem)
{
	assert (pJobItem);
	ubyte4 type = pJobItem->GetJobType ();
	if (type == GO_TYPE_SYNC || type == GO_TYPE_DUP) {
		//
		// 这两种类型不支持加密及压缩，不支持分割。
		//
		pJobItem->SetEncryptType (ENC_NO_ENC);
		pJobItem->SetDeflateType (DEF_NO_DEFLATE);
		pJobItem->SetSplitSize (-1);
		if (type == GO_TYPE_SYNC && m_pSyncRemovedFile->GetValue ()) {
			pJobItem->SetOption (GO_SYNC_REMOVED_FILE);
		}
		else {
			pJobItem->ClearOption (GO_SYNC_REMOVED_FILE);
		}
	}
	else {
		if (m_pEncode->GetValue ()) {
			pJobItem->SetEncryptType (static_cast<GmEncryptType> (m_pEncStrength->GetSelection () + 1));
		}
		else {
			pJobItem->SetPass (wxEmptyString);
			pJobItem->SetEncryptType (ENC_NO_ENC);
		}

		if (m_pCompress->GetValue ()) {
			pJobItem->SetDeflateType (DEF_DEFAULT);
		}
		else {
			pJobItem->SetDeflateType (DEF_NO_DEFLATE);
		}

		int select = m_pSplitSize->GetSelection ();
		switch (select)
		{
		case 0:
			pJobItem->SetSplitSize (-1);
			break;
		case 1:
			pJobItem->SetSplitSize (1 * 1024 * 1024);
			break;
		case 2:
			pJobItem->SetSplitSize (5 * 1024 * 1024);
			break;
		case 3:
			pJobItem->SetSplitSize (100 * 1024 * 1024);
			break;
		case 4:
			pJobItem->SetSplitSize (600 * 1024 * 1024);
			break;
		case 5:
			pJobItem->SetSplitSize (2 * 1024 * 1024 * 1024LL);
			break;
		}
	}

	pJobItem->Times (m_pTimesMerge->GetValue ());
	pJobItem->DescStr (m_pComment->GetValue ());
	return;
}

class GmFilterPanel : public GmOptionPage
{
public:
	GmFilterPanel (wxWindow * parent);
	GmFilterPanel () : GmOptionPage () {}
	bool IsExclude ()
	{
		return m_pExclude->GetValue () ? true : false;
	}
	void InitControlStatus (GmJobItem * pJobItem);
	void SetJobData (GmJobItem * pJobItem);
	bool CheckParams ();
private:
	void OnAddFileType (wxCommandEvent &);
	void OnDeleteFileType (wxCommandEvent &);
	void OnAddDirectory (wxCommandEvent &);
	void OnDeleteDirectory (wxCommandEvent &);
	void OnHandleItemSelectionChanged (wxListEvent&);
	void SetButtonStatus ();
	void RemoveFilter (wxListCtrl * pList);
private:
	enum {
		IDC_FILE_TYPE = 1000,
		IDC_FILE_TYPE_DELETE,
		IDC_DIRECTORY_NAME,
		IDC_DIRECTORY_NAME_DELETE,
		IDC_FILE_TYPE_LIST,
		IDC_DIRECTORY_TYPE_LIST,
	};

	wxRadioButton *			m_pExclude;
	wxListCtrl *			m_pFileTypeList;
	wxListCtrl *			m_pDirNameList;
	DECLARE_DYNAMIC_CLASS (GmFilterPanel)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmFilterPanel)
};

GmFilterPanel::GmFilterPanel (wxWindow * parent)
				: GmOptionPage (parent)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);

	wxBoxSizer * pBoxSizerLine = new wxBoxSizer (wxHORIZONTAL);
	m_pExclude = new wxRadioButton (this, wxID_ANY, _("IDS_EXCLUDE"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_pExclude->SetValue (true);
	pBoxSizerLine->Add (m_pExclude);
	pBoxSizerLine->Add (new wxRadioButton (this, wxID_ANY, _("IDS_INCLUDE")), 0, wxLEFT, 20);
	pBoxSizer->Add (pBoxSizerLine, 0, wxLEFT | wxTOP, 15);

	wxStaticBoxSizer * pFileStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_FILE_STR"));
	pBoxSizer->Add (pFileStaticSizer, 1, wxEXPAND | wxALL, 15);

	//
	// 文件
	//
	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (1, 2, 5, 5);
	pGridSizer->AddGrowableCol (0);
	pGridSizer->AddGrowableRow (0);
	pFileStaticSizer->Add (pGridSizer, 1, wxEXPAND);

	//
	// 文件列表
	//
	m_pFileTypeList = new wxListCtrl (this, IDC_FILE_TYPE_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxALWAYS_SHOW_SB);
	m_pFileTypeList->InsertColumn (0, _("IDS_FILE_NAME"), wxLIST_FORMAT_LEFT);
	m_pFileTypeList->InsertColumn (1, _("IDS_DIRECTORY"), wxLIST_FORMAT_LEFT, 180);
	pGridSizer->Add (m_pFileTypeList, 1, wxEXPAND);
	wxBoxSizer * pBoxSizerVert = new wxBoxSizer (wxVERTICAL);
	pBoxSizerVert->Add (new wxButton (this, IDC_FILE_TYPE, _("IDS_ADD")), 0, wxEXPAND | wxALIGN_TOP | wxBOTTOM, 5);
	pBoxSizerVert->Add (new wxButton (this, IDC_FILE_TYPE_DELETE, _("IDS_DELETE")), 0, wxEXPAND | wxALIGN_TOP);
	pGridSizer->Add (pBoxSizerVert, 0, wxALIGN_TOP);

	FindWindow (IDC_FILE_TYPE_DELETE)->Enable (false);

	//
	// 目录
	//
	wxStaticBoxSizer * pDirStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_DIRECTORY_STR"));
	pBoxSizer->Add (pDirStaticSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);
	pBoxSizerLine = new wxBoxSizer (wxHORIZONTAL);
	pDirStaticSizer->Add (pBoxSizerLine, 1, wxEXPAND);

	m_pDirNameList = new wxListCtrl (this, IDC_DIRECTORY_TYPE_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxALWAYS_SHOW_SB);
	m_pDirNameList->InsertColumn (0, _("IDS_FILE_NAME"), wxLIST_FORMAT_LEFT);
	m_pDirNameList->InsertColumn (1, _("IDS_DIRECTORY"), wxLIST_FORMAT_LEFT, 180);

	pBoxSizerLine->Add (m_pDirNameList, 1, wxEXPAND | wxRIGHT, 5);

	pBoxSizerVert = new wxBoxSizer (wxVERTICAL);
	pBoxSizerVert->Add (new wxButton (this, IDC_DIRECTORY_NAME, _("IDS_ADD")), 0, wxEXPAND | wxALIGN_TOP | wxBOTTOM, 5);
	pBoxSizerVert->Add (new wxButton (this, IDC_DIRECTORY_NAME_DELETE, _("IDS_DELETE")), 0, wxEXPAND | wxALIGN_TOP);
	pBoxSizerLine->Add (pBoxSizerVert, 0, wxALIGN_TOP);
	FindWindow (IDC_DIRECTORY_NAME_DELETE)->Enable (false);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

bool GmFilterPanel::CheckParams ()
{
	return true;
}

static bool GetFilterParams (wxListCtrl *pList, wxString & FileName, wxString &DirName, int Index)
{
	wxListItem item;
	item.SetId (Index);
	item.SetColumn (0);
	item.SetMask (wxLIST_MASK_TEXT);
	if (!pList->GetItem (item)) return false;
	FileName = item.GetText ();
	item.SetColumn (1);
	if (!pList->GetItem (item)) return false;
	DirName = item.GetText ();
	return true;
}

static void InsertFilter (wxListCtrl * pList, const wxString & FileName, const wxString & DirName)
{
	if (!DirName.IsEmpty () && !IsValidPath (DirName)) return;
	if (DirName.Find (wxT ("\\\\")) != wxString::npos ||
		DirName.Find (wxT ("//")) != wxString::npos) {
		wxString message (_("IDS_PATH_HAS_REDUNDANT_CHARS"));
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	int count = pList->GetItemCount ();
	for (long index = 0; index < count; ++index) {
		wxString FileName1, DirName1;
		if (!GetFilterParams (pList, FileName1, DirName1, index)) continue;
		if (FileName == FileName1 && IsSameDirName (DirName, DirName1)) {
			wxMessageBox (_("IDS_HAS_SAME_FILTER"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return;
		}
	}

	long index = pList->InsertItem (count, _(""));
	wxListItem item;
	item.SetId (index);

	item.SetColumn (0);
	item.SetMask (wxLIST_MASK_TEXT);
	item.SetText (FileName);
	pList->SetItem (item);

	item.SetColumn (1);
	item.SetText (DirName);
	pList->SetItem (item);
}

void GmFilterPanel::SetButtonStatus ()
{
	if (m_pFileTypeList->GetItemCount () > 0) {
		m_pFileTypeList->SetItemState (0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		FindWindow (IDC_FILE_TYPE_DELETE)->Enable (true);
	}
	else FindWindow (IDC_FILE_TYPE_DELETE)->Enable (false);

	if (m_pDirNameList->GetItemCount ()) {
		m_pDirNameList->SetItemState (0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		FindWindow (IDC_DIRECTORY_NAME_DELETE)->Enable (true);
	}
	else FindWindow (IDC_DIRECTORY_NAME_DELETE)->Enable (false);

	return;
}

void GmFilterPanel::InitControlStatus (GmJobItem * pJobItem)
{
	assert (pJobItem);
	vector<GmJobFilterItem*> & vpFilters = pJobItem->GetFilter ().vpFilters;
	if (vpFilters.empty ()) m_pExclude->SetValue (true);
	else m_pExclude->SetValue (pJobItem->GetFilter ().bExclude);
	
	size_t Filters = vpFilters.size ();
	for (size_t index = 0; index < Filters; ++index) {
		GmJobFilterItem * pfi = vpFilters[index];
		wxListCtrl * pList;
		if (pfi->IsDir) pList = m_pDirNameList; 
		else pList = m_pFileTypeList;
		InsertFilter (pList, pfi->FileName, pfi->DirName);
	}

	SetButtonStatus ();
	return;
}

static void GetFilters (wxListCtrl * pList, vector<GmJobFilterItem*> & vpFilters, bool bIsDir)
{
	for (long index = 0; index < pList->GetItemCount (); ++index) {
		wxString FileName, DirName;
		if (!GetFilterParams (pList, FileName, DirName, index)) continue;
		GmJobFilterItem * pfi = new GmJobFilterItem;
		pfi->IsDir = bIsDir;
		pfi->FileName = FileName;
		pfi->DirName = DirName;
		vpFilters.push_back (pfi);
	}

	return;
}

void GmFilterPanel::SetJobData (GmJobItem * pJobItem)
{
	assert (pJobItem);

	pJobItem->GetFilter ().bExclude = m_pExclude->GetValue ();
	vector<GmJobFilterItem*> & vpFilters = pJobItem->GetFilter ().vpFilters;
	for_each (vpFilters.begin (), vpFilters.end (), ReleaseMemory ());
	vpFilters.clear ();

	GetFilters (m_pFileTypeList, vpFilters, false);
	GetFilters (m_pDirNameList, vpFilters, true);
}

void GmFilterPanel::RemoveFilter (wxListCtrl * pList)
{
	long index = pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index != -1) {
		pList->DeleteItem (index);
	}

	SetButtonStatus ();
}

void GmFilterPanel::OnAddFileType (wxCommandEvent &event)
{
	GmDirFileFilterDialog dialog (this, _("IDS_FILE_FILTER_EXAMPLE"));
	if (dialog.ShowModal () == wxID_OK) {
		InsertFilter (m_pFileTypeList, dialog.GetFileName (), dialog.GetDirName ());
		SetButtonStatus ();
	}
}

void GmFilterPanel::OnDeleteFileType (wxCommandEvent &event)
{
	RemoveFilter (m_pFileTypeList);
}


void GmFilterPanel::OnAddDirectory (wxCommandEvent &event)
{
	GmDirFileFilterDialog dialog (this, _("IDS_FILE_FILTER_EXAMPLE"));
	if (dialog.ShowModal () == wxID_OK) {
		InsertFilter (m_pFileTypeList, dialog.GetFileName (), dialog.GetDirName ());
		SetButtonStatus ();
	}
}

void GmFilterPanel::OnDeleteDirectory (wxCommandEvent &)
{
	RemoveFilter (m_pDirNameList);
}

void GmFilterPanel::OnHandleItemSelectionChanged (wxListEvent&)
{
	bool enable = false;
	if (m_pFileTypeList->GetSelectedItemCount () > 0) enable = true;
	else  enable = false;
	FindWindow (IDC_FILE_TYPE_DELETE)->Enable (enable);

	if (m_pDirNameList->GetSelectedItemCount () > 0) enable = true;
	else enable = false;
	FindWindow (IDC_DIRECTORY_NAME_DELETE)->Enable (enable);
}


IMPLEMENT_DYNAMIC_CLASS (GmFilterPanel, GmOptionPage)
BEGIN_EVENT_TABLE(GmFilterPanel, GmOptionPage)
	EVT_BUTTON (IDC_FILE_TYPE, GmFilterPanel::OnAddFileType)
	EVT_BUTTON (IDC_FILE_TYPE_DELETE, GmFilterPanel::OnDeleteFileType)
	EVT_BUTTON (IDC_DIRECTORY_NAME, GmFilterPanel::OnAddDirectory)
	EVT_BUTTON (IDC_DIRECTORY_NAME_DELETE, GmFilterPanel::OnDeleteDirectory)
	EVT_LIST_ITEM_SELECTED (IDC_FILE_TYPE_LIST, GmFilterPanel::OnHandleItemSelectionChanged)
	EVT_LIST_ITEM_DESELECTED (IDC_FILE_TYPE_LIST, GmFilterPanel::OnHandleItemSelectionChanged)
	EVT_LIST_ITEM_SELECTED (IDC_DIRECTORY_TYPE_LIST, GmFilterPanel::OnHandleItemSelectionChanged)
	EVT_LIST_ITEM_DESELECTED (IDC_DIRECTORY_TYPE_LIST, GmFilterPanel::OnHandleItemSelectionChanged)
END_EVENT_TABLE()

class GmPlanPanel : public GmOptionPage
{
public:
	GmPlanPanel (wxWindow * parent);
	GmPlanPanel () : GmOptionPage () {}
	~GmPlanPanel ();
	void InitControlStatus (GmJobItem * pJobItem);
	void SetJobData (GmJobItem * pJobItem);
	bool CheckParams ();
private:
	void OnAtStart (wxCommandEvent &event);
	void OnAtShutDown (wxCommandEvent &event);
	void OnAtSomeTime (wxCommandEvent &event);
	void OnAddSomeTime (wxCommandEvent &event);
	void OnDeleteSomeTime (wxCommandEvent &event);
	void OnHandleItemSelectionChanged (wxListEvent &event);
	wxCheckBox *		m_pOnShutDown;
	wxCheckBox *		m_pOnStartUp;
	wxCheckBox *		m_pOnSomeTime;
	wxListCtrl *		m_pSomeTimeList;
	enum {
		IDC_ON_START = 1000,
		IDC_ON_SHUTDOWN,
		IDC_ON_SOMETIME,
		IDC_ADD_SOMETIME,
		IDC_DELETE_SOMETIME,
		IDC_SOME_TIME_LIST,
	};
private:
	DECLARE_DYNAMIC_CLASS (GmPlanPanel)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmPlanPanel)
};

IMPLEMENT_DYNAMIC_CLASS (GmPlanPanel, GmOptionPage)
BEGIN_EVENT_TABLE(GmPlanPanel, GmOptionPage)
	EVT_CHECKBOX (IDC_ON_START, GmPlanPanel::OnAtStart)
	EVT_CHECKBOX (IDC_ON_SHUTDOWN, GmPlanPanel::OnAtShutDown)
	EVT_CHECKBOX (IDC_ON_SOMETIME, GmPlanPanel::OnAtSomeTime)
	EVT_BUTTON (IDC_ADD_SOMETIME, GmPlanPanel::OnAddSomeTime)
	EVT_BUTTON (IDC_DELETE_SOMETIME, GmPlanPanel::OnDeleteSomeTime)
	EVT_LIST_ITEM_SELECTED (IDC_SOME_TIME_LIST, GmPlanPanel::OnHandleItemSelectionChanged)
	EVT_LIST_ITEM_DESELECTED (IDC_SOME_TIME_LIST, GmPlanPanel::OnHandleItemSelectionChanged)
END_EVENT_TABLE()

GmPlanPanel::GmPlanPanel (wxWindow * parent)
				: GmOptionPage (parent)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);

	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_CONFIG_PLAN"));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 20);
	m_pOnShutDown = new wxCheckBox (this, IDC_ON_START, _("IDS_ON_START_UP"));
	m_pOnStartUp = new wxCheckBox (this, IDC_ON_SHUTDOWN, _("IDS_ON_SHUTDOWN"));
	m_pOnSomeTime = new wxCheckBox (this, IDC_ON_SOMETIME, _("IDS_ON_SOMETIME"));
	m_pSomeTimeList = new wxListCtrl (this, IDC_SOME_TIME_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxALWAYS_SHOW_SB);
	m_pSomeTimeList->InsertColumn (0, _("IDS_BEGIN_TIME"), wxLIST_FORMAT_LEFT, 100);
	m_pSomeTimeList->InsertColumn (1, _("IDS_END_TIME"), wxLIST_FORMAT_LEFT, 100);
	m_pSomeTimeList->InsertColumn (2, _("IDS_INTERVAL"), wxLIST_FORMAT_LEFT, 180);

	pStaticSizer->Add (m_pOnShutDown, 0, wxLEFT | wxBOTTOM, 5);
	pStaticSizer->Add (m_pOnStartUp, 0, wxLEFT | wxBOTTOM, 5);
	pStaticSizer->Add (m_pOnSomeTime, 0, wxLEFT | wxBOTTOM, 5);
	m_pSomeTimeList->Enable (false);

	wxBoxSizer * pBoxSizerVert = new wxBoxSizer (wxVERTICAL);
	pBoxSizerVert->Add (new wxButton (this, IDC_ADD_SOMETIME, _("IDS_ADD")), 0, wxEXPAND | wxBOTTOM, 5);
	pBoxSizerVert->Add (new wxButton (this, IDC_DELETE_SOMETIME, _("IDS_DELETE")), 0, wxEXPAND | wxBOTTOM, 5);
	FindWindow (IDC_ADD_SOMETIME)->Enable (false);
	FindWindow (IDC_DELETE_SOMETIME)->Enable (false);

	wxBoxSizer * pBoxSizerHori = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizerHori->Add (m_pSomeTimeList, 1, wxEXPAND | wxLEFT, 20);
	pBoxSizerHori->Add (pBoxSizerVert, 0, wxLEFT, 5);
	pStaticSizer->Add (pBoxSizerHori, 1, wxEXPAND);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

GmPlanPanel::~GmPlanPanel ()
{
	int count = m_pSomeTimeList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		GmJobPlan * pItemData = (GmJobPlan*)m_pSomeTimeList->GetItemData (index);
		if (pItemData) delete pItemData;
	}
}

bool GmPlanPanel::CheckParams ()
{
	//
	// 没有参数需要确认。
	//
	return true;
}

static void AddPlanListItem (wxListCtrl * pList, GmJobPlan * pPlan)
{
	long count = pList->GetItemCount ();
	long index = pList->InsertItem (count, wxT (""));

	wxListItem item;
	item.SetId (index);

	item.SetColumn (0);
	item.SetMask (wxLIST_MASK_TEXT);
	item.SetText (wxDateTime (TimeTToWxDateTime (pPlan->StartTime)).Format ());
	pList->SetItem (item);

	item.SetColumn (1);
	item.SetText (wxDateTime (TimeTToWxDateTime (pPlan->EndTime)).Format ());
	pList->SetItem (item);

	item.SetColumn (2);
	if (pPlan->IntervalType == DAY_INTERVAL) {
		item.SetText (wxString::Format (_("IDS_DAYS_INTERVAL"), pPlan->WeekOrDay, pPlan->AtHour));
	}
	else {
		wxString str (_("IDS_WEEK_INTERVAL"));
		wxString week;
		if (pPlan->WeekOrDay & SUNDAY) week += wxString (_("IDS_SUNDAY")) + wxT (",");
		if (pPlan->WeekOrDay & MONDAY) week += wxString (_("IDS_MONDAY")) + wxT (",");
		if (pPlan->WeekOrDay & TUESDAY) week += wxString (_("IDS_TUESDAY")) + wxT (",");
		if (pPlan->WeekOrDay & WENDESDAY) week +=wxString ( _("IDS_WEDNESDAY")) + wxT (",");
		if (pPlan->WeekOrDay & THURSDAY) week += wxString (_("IDS_THURSDAY")) + wxT (",");
		if (pPlan->WeekOrDay & FRIDAY) week += wxString (_("IDS_FRIDAY")) + wxT (",");
		if (pPlan->WeekOrDay & SATURDAY) week += wxString (_("IDS_SATURDAY")) + wxT (",");
		item.SetText (wxString::Format (str.c_str (), week.c_str (), pPlan->AtHour));
	}
	pList->SetItem (item);
	pList->SetItemData (index,  (wxUIntPtr)pPlan);
}

void GmPlanPanel::InitControlStatus (GmJobItem * pJobItem)
{
	assert (pJobItem);
	m_pOnStartUp->SetValue (pJobItem->GetOptions () & GO_RUN_ATSTART);
	m_pOnShutDown->SetValue (pJobItem->GetOptions () & GO_RUN_ATSHUTDOWN);

	const vector<GmJobPlan*> & plans = pJobItem->GetPlans ();
	if (!plans.empty ()) {
		m_pOnSomeTime->SetValue (true);
		for (size_t index = 0; index < pJobItem->GetPlans ().size (); ++index) {
			AddPlanListItem (m_pSomeTimeList, new GmJobPlan (*plans[index]));
		}
		m_pSomeTimeList->Enable (true);
		m_pSomeTimeList->SetItemState (0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		FindWindow (IDC_ADD_SOMETIME)->Enable (true);
		FindWindow (IDC_DELETE_SOMETIME)->Enable (true);
	}
	else
		m_pOnSomeTime->SetValue (false);
	return;
}

void GmPlanPanel::SetJobData (GmJobItem * pJobItem)
{
	assert (pJobItem);
	if (m_pOnStartUp->GetValue ()) pJobItem->SetOption (GO_RUN_ATSTART);
	else pJobItem->ClearOption (GO_RUN_ATSTART);

	if (m_pOnShutDown->GetValue ()) pJobItem->SetOption (GO_RUN_ATSHUTDOWN);
	else pJobItem->ClearOption (GO_RUN_ATSHUTDOWN);

	pJobItem->ClearPlan ();
	if (m_pOnSomeTime->IsChecked ()) {
		for (long index = 0; index < m_pSomeTimeList->GetItemCount (); ++index)
			pJobItem->AddPlan (*(GmJobPlan*)m_pSomeTimeList->GetItemData (index));
	}

	if (m_pOnStartUp->IsChecked ())
		pJobItem->SetOption (GO_RUN_ATSTART);
	else
		pJobItem->ClearOption (GO_RUN_ATSTART);

	if (m_pOnShutDown->IsChecked ())
		pJobItem->SetOption (GO_RUN_ATSHUTDOWN);
	else
		pJobItem->ClearOption (GO_RUN_ATSHUTDOWN);

	return;
}

void GmPlanPanel::OnAtStart (wxCommandEvent &event)
{
}

void GmPlanPanel::OnAtShutDown (wxCommandEvent &event)
{
}

void GmPlanPanel::OnAtSomeTime (wxCommandEvent &event)
{
	bool value = false;
	if (m_pOnSomeTime->IsChecked ()) {
		value = true;
	}

	m_pSomeTimeList->Enable (value);
	FindWindow (IDC_ADD_SOMETIME)->Enable (value);

	if (m_pSomeTimeList->GetItemCount () == 0)
		FindWindow (IDC_DELETE_SOMETIME)->Enable (false);
	else
		FindWindow (IDC_DELETE_SOMETIME)->Enable (value);
}

void GmPlanPanel::OnAddSomeTime (wxCommandEvent &event)
{
	GmPlanSomeTimeDialog dialog (this, _("IDS_ADD_SOME_TIME"));
	if (dialog.ShowModal () == wxID_OK) {
		auto_ptr<GmJobPlan> plan (new GmJobPlan);
		plan->IntervalType = dialog.GetPlan (plan->StartTime
											, plan->EndTime
											, plan->WeekOrDay
											, plan->AtHour);

		AddPlanListItem (m_pSomeTimeList, plan.release ());
	}
}

void GmPlanPanel::OnDeleteSomeTime (wxCommandEvent &event)
{
	long index = m_pSomeTimeList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index != -1) {
		GmJobPlan * pjp = (GmJobPlan*)m_pSomeTimeList->GetItemData (index);
		m_pSomeTimeList->DeleteItem (index);
		if (pjp != 0) {
			delete pjp;
		}
	}

	if (m_pSomeTimeList->GetItemCount () == 0) {
		FindWindow (IDC_DELETE_SOMETIME)->Enable (false);
	}
	else {
		m_pSomeTimeList->SetItemState (0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		FindWindow (IDC_DELETE_SOMETIME)->Enable (true);
	}
}

void GmPlanPanel::OnHandleItemSelectionChanged (wxListEvent &event)
{
	long index = m_pSomeTimeList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1) {
		FindWindow (IDC_DELETE_SOMETIME)->Enable (false);
		return;
	}

	FindWindow (IDC_DELETE_SOMETIME)->Enable (true);
}

IMPLEMENT_DYNAMIC_CLASS (GmPlanSomeTimeDialog, wxDialog)
BEGIN_EVENT_TABLE(GmPlanSomeTimeDialog, wxDialog)
	EVT_COMBOBOX (IDC_INTERVAL, GmPlanSomeTimeDialog::OnChangeInterval)
	EVT_BUTTON(wxID_OK, GmPlanSomeTimeDialog::OnOK)
END_EVENT_TABLE()

GmPlanSomeTimeDialog::GmPlanSomeTimeDialog (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxHORIZONTAL, this, _("IDS_PLAN"));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 15);

	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (4, 3, 5, 5);
	pGridSizer->AddGrowableCol (1);
	pStaticSizer->Add (pGridSizer, 0, wxEXPAND | wxALL, 5);
	m_pFrom = new wxDatePickerCtrl (this, wxID_ANY, wxDateTime ().GetTimeNow ()
									, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
	m_pTo = new wxDatePickerCtrl (this, wxID_ANY, wxDateTime ().GetTimeNow ()
									, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);;

	//first row
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_FROM")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	pGridSizer->Add (m_pFrom);
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));

	//second row
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_TO")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	pGridSizer->Add (m_pTo);
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));

	// third row
	wxBoxSizer * pBoxSizerLine = new wxBoxSizer (wxHORIZONTAL);
	m_pDays = new wxSpinCtrl (this, wxID_ANY, wxT ("1"));
	m_pDays->SetRange (1, 100);
	
	wxArrayString items;
	items.Add (_("IDS_EVERY_DAY"));
	items.Add (_("IDS_EVERY_WEEK"));
	m_pInterval = new wxComboBox (this, IDC_INTERVAL, _("IDS_EVERY_DAY"), wxDefaultPosition
									, wxDefaultSize, items, wxCB_READONLY);
	wxBoxSizer * pSizerHour = new wxBoxSizer (wxHORIZONTAL);
	m_pHours = new wxSpinCtrl (this, wxID_ANY, wxT ("0"), wxDefaultPosition, wxSize (70, 21));
	pSizerHour->Add (m_pHours);
	pSizerHour->Add (new wxStaticText (this, wxID_ANY, _("IDS_HOUR")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	m_pHours->SetRange (0, 23);
	pBoxSizerLine->Add (new wxStaticText (this, wxID_ANY, _("IDS_EVERY")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	pBoxSizerLine->Add (m_pInterval);
	//
	// 这个Sizer里面的两个Sizer根据 m_pInterval 的不同显示来显示其中一个相应的Sizer。
	//
	wxBoxSizer * pHideShowSizer = new wxBoxSizer (wxVERTICAL);
	m_pDaysSizer = new wxBoxSizer (wxHORIZONTAL);
	m_pDaysSizer->Add (m_pDays, 0, wxEXPAND);

	m_pWeekDays = new wxGridSizer (3, 3, 5, 5);
	m_pWeekDays->Add (new wxCheckBox (this, IDC_SUNDAY, _("IDS_SUNDAY")));
	m_pWeekDays->Add (new wxCheckBox (this, IDC_MONDAY, _("IDS_MONDAY")));
	m_pWeekDays->Add (new wxCheckBox (this, IDC_TUESDAY, _("IDS_TUESDAY")));
	m_pWeekDays->Add (new wxCheckBox (this, IDC_WEDNESDAY, _("IDS_WEDNESDAY")));
	m_pWeekDays->Add (new wxCheckBox (this, IDC_THURSDAY, _("IDS_THURSDAY")));
	m_pWeekDays->Add (new wxCheckBox (this, IDC_FRIDAY, _("IDS_FRIDAY")));
	m_pWeekDays->Add (new wxCheckBox (this, IDC_SATURDAY, _("IDS_SATURDAY")));

	pHideShowSizer->Add (m_pDaysSizer, 1, wxEXPAND);
	pHideShowSizer->Add (m_pWeekDays, 1, wxEXPAND);
	pHideShowSizer->Show (m_pWeekDays, false, true);

	pGridSizer->Add (pBoxSizerLine);
	pGridSizer->Add (pHideShowSizer, 0, wxEXPAND);
	pGridSizer->Add (pSizerHour);

	//bottom.
	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 15);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnParent ();
}

void GmPlanSomeTimeDialog::OnOK (wxCommandEvent & event)
{
	ubyte8 begin = m_pFrom->GetValue ().GetValue ().GetValue ();
	ubyte8 end = m_pTo->GetValue ().GetValue ().GetValue ();
	if (begin >= end) {
		wxMessageBox (_("IDS_TIME_SEQUENCE_ERROR"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	wxDialog::EndModal (wxID_OK);
}

char GmPlanSomeTimeDialog::GetPlan (ubyte8 & begin, ubyte8 & end, ubyte4 & WeekOrDays, ubyte2 & AtHour)
{
	begin = m_pFrom->GetValue ().GetValue ().GetValue ();
	end = m_pTo->GetValue ().GetValue ().GetValue ();
	AtHour = m_pHours->GetValue ();
	if (m_pInterval->GetSelection () == 0) { //day.
		WeekOrDays = m_pDays->GetValue ();
		return DAY_INTERVAL;
	}

	WeekOrDays = 0;
	if (((wxCheckBox*)FindWindow (IDC_SUNDAY))->GetValue ()) WeekOrDays |= SUNDAY;
	else WeekOrDays &= ~SUNDAY;

	if (((wxCheckBox*)FindWindow (IDC_MONDAY))->GetValue ()) WeekOrDays |= MONDAY;
	else WeekOrDays &= ~MONDAY;

	if (((wxCheckBox*)FindWindow (IDC_TUESDAY))->GetValue ()) WeekOrDays |= TUESDAY;
	else WeekOrDays &= ~TUESDAY;

	if (((wxCheckBox*)FindWindow (IDC_WEDNESDAY))->GetValue ()) WeekOrDays |= WENDESDAY;
	else WeekOrDays &= ~WENDESDAY;

	if (((wxCheckBox*)FindWindow (IDC_THURSDAY))->GetValue ()) WeekOrDays |= THURSDAY;
	else WeekOrDays &= ~THURSDAY;

	if (((wxCheckBox*)FindWindow (IDC_FRIDAY))->GetValue ()) WeekOrDays |= FRIDAY;
	else WeekOrDays &= ~FRIDAY;

	if (((wxCheckBox*)FindWindow (IDC_SATURDAY))->GetValue ()) WeekOrDays |= SATURDAY;
	else WeekOrDays &= ~SATURDAY;

	return WEEK_INTERVAL;
}

void GmPlanSomeTimeDialog::OnChangeInterval (wxCommandEvent &event)
{
	wxSizer * pSizer = GetSizer ();
	if (m_pInterval->GetValue () == _("IDS_EVERY_DAY")) {
		pSizer->Show (m_pDaysSizer, true, true);
		pSizer->Show (m_pWeekDays, false, true);
	}
	else if (m_pInterval->GetValue () == _("IDS_EVERY_WEEK")) {
		pSizer->Show (m_pWeekDays, true, true);
		pSizer->Show (m_pDaysSizer, false, true);
	}

	pSizer->RecalcSizes ();
	Fit ();
	pSizer->Layout ();
}

BEGIN_EVENT_TABLE(GmOptionsDlg, wxDialog)
	EVT_BUTTON(wxID_OK, GmOptionsDlg::OnOK)
END_EVENT_TABLE()

GmOptionsDlg::GmOptionsDlg (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);

	m_pOptions = new wxBookCtrl (this, wxID_ANY, wxDefaultPosition, wxSize (400, 400), wxBK_TOP);
	pBoxSizer->Add (m_pOptions, 1, wxEXPAND | wxALL, 3);

	wxImageList * pimglst = new wxImageList (16, 16);
	pimglst->Add (wxBitmap (config_xpm), wxColor (0, 0, 0));
	pimglst->Add (wxBitmap (filter_xpm), wxColor (0, 0, 0));
	pimglst->Add (wxBitmap (plan_xpm), wxColor (0, 0, 0));
	m_pOptions->AssignImageList (pimglst);

	GmGeneralOptionPanel * pGeneralPage = new GmGeneralOptionPanel (m_pOptions);
	m_pOptions->AddPage (pGeneralPage, _("IDS_GENERAL_OPTIONS"), false, 0);

	GmFilterPanel * pFilterPage = new GmFilterPanel (m_pOptions);
	m_pOptions->AddPage (pFilterPage, _("IDS_FILTER"), false, 1);

	GmPlanPanel * pPlanPage = new GmPlanPanel (m_pOptions);
	m_pOptions->AddPage (pPlanPage, _("IDS_PLAN"), false, 2);

	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxALL, 5);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND | wxRIGHT);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

void GmOptionsDlg::OnOK (wxCommandEvent &event)
{
	for (size_t index = 0; index < m_pOptions->GetPageCount (); ++index) {
		GmOptionPage * page = (GmOptionPage*)m_pOptions->GetPage (index);
		if (page == 0) continue;
		if (!page->CheckParams ()) return;
	}

	wxDialog::EndModal (wxID_OK);
}

void GmOptionsDlg::InitControlStatus (GmJobItem * pJobItem)
{
	assert (pJobItem);
	for (size_t index = 0; index < m_pOptions->GetPageCount (); ++index) {
		GmOptionPage * page = (GmOptionPage*)m_pOptions->GetPage (index);
		if (page == 0) continue;
		page->InitControlStatus (pJobItem);
	}
}

void GmOptionsDlg::SetJobData (GmJobItem * pJobItem)
{
	for (size_t index = 0; index < m_pOptions->GetPageCount (); ++index) {
		GmOptionPage * page = (GmOptionPage*)m_pOptions->GetPage (index);
		if (page == 0) continue;
		page->SetJobData (pJobItem);
	}
}

bool CheckJobNameAndDest (const wxString & jobname, wxString & dest)
{
	if (jobname.IsEmpty ()) {
		wxMessageBox (_("IDS_NO_JOB_NAME"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	if (!IsValidPath (dest) ||
		dest.Find (wxT ("\\\\")) != wxString::npos ||
		dest.Find (wxT ("//")) != wxString::npos) {
		wxString message (_("IDS_PATH_HAS_REDUNDANT_CHARS"));
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	if (!IsValidName (jobname)) {
		wxString message = wxString::Format (_("IDS_INVALID_JOB_NAME"), wxFileName::GetForbiddenChars ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	return true;
}

GmRestorePropertyDlg::GmRestorePropertyDlg (wxWindow * parent, const GmStoreEntry & entry, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_NODE_PROPERTIES"));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 15);

	GmLeafInfo info;
	entry.GetLeafInfo (info);

	int type = entry.GetType ();
	int lines = (type == GNT_ROOT) ? 2 : 7;
	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (lines, 2, 5, 5);
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, entry.GetName ()));
	pStaticSizer->Add (pGridSizer);
	pGridSizer->AddGrowableCol (1);
	// ONE
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_DIRECTORY_PATH")));
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, entry.GetPath (GmStoreEntry::PATH_FULL)));

	if (type != GNT_ROOT) {
		// TWO
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_CREATE_TIME")));
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, info.CreateTime == 0 ? wxT ("") :
												wxDateTime (TimeTToWxDateTime(info.CreateTime)).Format ()));

		// THREE
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_ACCESS_TIME")));
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, info.AccessTime == 0 ? wxT ("") 
													: wxDateTime (TimeTToWxDateTime(info.AccessTime)).Format ()));

		// FOUR
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_MODIFY_TIME")));
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, info.ModifyTime == 0 ? wxT ("") :
													wxDateTime (TimeTToWxDateTime(info.ModifyTime)).Format ()));

		// FIVE
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_STORE_TIME")));
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, info.StoreTime == 0 ? wxT ("") :
													wxDateTime (TimeTToWxDateTime(info.StoreTime)).Format ()));

		// SIX
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_CRC_VALUE")));
		wxString crc = wxString::Format (wxT ("%08X"), info.CRC);
		pGridSizer->Add (new wxStaticText (this, wxID_ANY, crc));
	}

	// SEVEN
	GmDirectoryInfo di = entry.GetDirectoryInfo ();
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_DIRECTORY_INFO")));
	wxString nodeinfos = wxString::Format (_("IDS_DIRECTORY_INFOS"), di.Dirs, di.Files, di.Size, di.CompressSize);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, nodeinfos));

	//bottom.
	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnParent ();
}

///////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(GmProgOptionsDlg, wxDialog)
	EVT_BUTTON (wxID_OK, GmProgOptionsDlg::OnOK)
END_EVENT_TABLE()

GmProgOptionsDlg::GmProgOptionsDlg (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxSize (300, 100), wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxHORIZONTAL, this, _("IDS_PROG_CONFIGURE"));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 15);

	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (2, 5, 5);
	pGridSizer->AddGrowableCol (1);
	pStaticSizer->Add (pGridSizer, 0, wxEXPAND | wxALL, 5);

	//---
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_PROG_PASSWORD")), 0, wxALIGN_RIGHT);
	pGridSizer->Add (new wxTextCtrl (this, IDC_PASSWORD, wxT (""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD)
					, 1, wxEXPAND);

	//---
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));
	pGridSizer->Add (new wxCheckBox (this, IDC_MINI_TO_TRAY, _("IDS_MINIMIZE_TO_TASKBAR")), 1, wxEXPAND);

	//---
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));
	pGridSizer->Add (new wxCheckBox (this, IDC_START_WITH_OS, _("IDS_START_WITH_WINDOWS")), 1, wxEXPAND);

	//lang
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_PROG_DEFAULT_LANG")), 0, wxALIGN_RIGHT);
	wxBoxSizer * pSizer = new wxBoxSizer (wxHORIZONTAL);
	pSizer->Add (new wxRadioButton (this
										, IDC_LANG_SIMPLIFIED_CHINESE
										, _("IDS_SIMPLIFIED_CHINESE")
										, wxDefaultPosition
										, wxDefaultSize
										, wxRB_GROUP), 0, wxRIGHT, 20);
	pSizer->Add (new wxRadioButton (this, IDC_LANG_TRADITIONAL_CHINESE, _("IDS_TRADITIONAL_CHINESE")), 0, wxRIGHT, 20);
	pSizer->Add (new wxRadioButton (this, IDC_LANG_ENGLISH, _("IDS_ENGLISH")), 0, wxRIGHT);
	pGridSizer->Add (pSizer);

	//format
	pSizer = new wxBoxSizer (wxHORIZONTAL);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_PROG_DEFAULT_JOB_TYPE")), 0, wxALIGN_RIGHT);
	pSizer->Add (new wxRadioButton (this
										, IDC_FORMAT_COPY
										, GetDupTypeStr ()
										, wxDefaultPosition
										, wxDefaultSize
										, wxRB_GROUP), 0, wxRIGHT, 20);

	pSizer->Add (new wxRadioButton (this, IDC_FORMAT_SYNC, GetSyncTypeStr ()), 0, wxRIGHT, 20);
	pSizer->Add (new wxRadioButton (this, IDC_FORMAT_ZIP, GetZipTypeStr ()), 0, wxRIGHT, 20);
	pSizer->Add (new wxRadioButton (this, IDC_FORMAT_7Z, Get7zTypeStr ()), 0, wxRIGHT);
	pGridSizer->Add (pSizer);

	//how to get data back
	pSizer = new wxBoxSizer (wxHORIZONTAL);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_PROG_DEFAULT_SNAPMODE")), 0, wxALIGN_RIGHT);
	pSizer->Add (new wxRadioButton (this
										, IDC_GET_DATA_SNAPMODE
										, _("IDS_DEFAULT_SNAPMODE_TRUE")
										, wxDefaultPosition
										, wxDefaultSize
										, wxRB_GROUP), 0, wxRIGHT, 20);
	pSizer->Add (new wxRadioButton (this, IDC_GET_DATA_SNAPMODE_FALSE, _("IDS_DEFAULT_SNAPMODE_FALSE")), 1, wxEXPAND);
	pGridSizer->Add (pSizer);

	// log and report
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_LOG_REPORT_LIMIT")), 0, wxALIGN_RIGHT);
	wxSpinCtrl * p = new wxSpinCtrl (this, IDC_LOG_REPORT_LIMIT);
	p->SetRange (10, 1000);
	pGridSizer->Add (p);

	//bottom.
	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 15);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	InitControls ();
	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnParent ();
}

void GmProgOptionsDlg::OnOK (wxCommandEvent &event)
{
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	wxString password = ((wxTextCtrl *)FindWindow (IDC_PASSWORD))->GetLabel ();
	if (!password.IsEmpty () && password.Length () < 4) {
		wxMessageBox (_("IDS_PASSWORD_LENGTH_IS_NOT_ENOUGH"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	GmPassword::GetInstance ()->WritePassword (password);

	if (((wxCheckBox *)FindWindow (IDC_MINI_TO_TRAY))->IsChecked ()) 
		pConfig->SetMinimizeToTaskBar (true);
	else
		pConfig->SetMinimizeToTaskBar (false);

	wxString app = GmAppInfo::GetFullAppName ();
	app = EnclosureWithQuotMark (app);
	bool bResult;
	if (event.IsChecked ())
		bResult = AddSystemStartItem (app);
	else
		bResult = RemoveSystemStartItem (app);

	if (bResult) {
		if (((wxCheckBox *)FindWindow (IDC_START_WITH_OS))->IsChecked ()) {
			pConfig->SetStartWithSystem (true);
		}
		else {
			pConfig->SetStartWithSystem (false);
		}
	}

	wxString type = GetZipTypeStr ();
	if (((wxRadioButton *)FindWindow (IDC_FORMAT_COPY))->GetValue ())
		type = GetDupTypeStr ();
	else if (((wxRadioButton *)FindWindow (IDC_FORMAT_SYNC))->GetValue ())
		type = GetSyncTypeStr ();
	else if (((wxRadioButton *)FindWindow (IDC_FORMAT_ZIP))->GetValue ())
		type = GetZipTypeStr ();
	else if (((wxRadioButton *)FindWindow (IDC_FORMAT_7Z))->GetValue ())
		type = Get7zTypeStr ();

	pConfig->SetDefaultType (type);

	GmLangID langid = pConfig->GetLangID ();
	if (((wxRadioButton *)FindWindow (IDC_LANG_SIMPLIFIED_CHINESE))->GetValue ())
		langid = LANGUAGE_SIMPLIFIED_CHINESE;
	else if (((wxRadioButton *)FindWindow (IDC_LANG_TRADITIONAL_CHINESE))->GetValue ())
		langid = LANGUAGE_TRADITIONAL_CHINESE;
	else if (((wxRadioButton *)FindWindow (IDC_LANG_ENGLISH))->GetValue ())
		langid = LANGUAGE_ENGLISH;

	pConfig->SetLangID (langid);

	bool bSnapMode = true;
	if (((wxRadioButton *)FindWindow (IDC_GET_DATA_SNAPMODE))->GetValue ())
		bSnapMode = true;
	else if (((wxRadioButton *)FindWindow (IDC_GET_DATA_SNAPMODE_FALSE))->GetValue ())
		bSnapMode = false;

	pConfig->SetSnapMode (bSnapMode);
	pConfig->SetLogReportLimit (((wxSpinCtrl*)FindWindow (IDC_LOG_REPORT_LIMIT))->GetValue ());
	wxDialog::EndModal (wxID_OK);
}

void GmProgOptionsDlg::InitControls ()
{
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	wxString passwd = GmPassword::GetInstance ()->GetPassword ();
	if (!passwd.IsEmpty ())
		((wxTextCtrl *)FindWindow (IDC_PASSWORD))->SetLabel (passwd);

	((wxCheckBox *)FindWindow (IDC_MINI_TO_TRAY))->SetValue (pConfig->GetMinimizeToTaskBar ());
	((wxCheckBox *)FindWindow (IDC_START_WITH_OS))->SetValue (pConfig->GetStartWithSystem ());

	wxString DefType = pConfig->GetDefaultType ();
	if (DefType == GetDupTypeStr ())  ((wxRadioButton *)FindWindow (IDC_FORMAT_COPY))->SetValue (true);
	else if (DefType == GetSyncTypeStr ())  ((wxRadioButton *)FindWindow (IDC_FORMAT_SYNC))->SetValue (true);
	else if (DefType == GetZipTypeStr ()) ((wxRadioButton *)FindWindow (IDC_FORMAT_ZIP))->SetValue (true);
	else if (DefType == Get7zTypeStr ())  ((wxRadioButton *)FindWindow (IDC_FORMAT_7Z))->SetValue (true);

	GmLangID langid = pConfig->GetLangID ();
	if (langid == LANGUAGE_SIMPLIFIED_CHINESE)
		((wxRadioButton *)FindWindow (IDC_LANG_SIMPLIFIED_CHINESE))->SetValue (true);
	else if (langid == LANGUAGE_TRADITIONAL_CHINESE)
		((wxRadioButton *)FindWindow (IDC_LANG_TRADITIONAL_CHINESE))->SetValue (true);
	else if (langid == LANGUAGE_ENGLISH)
		((wxRadioButton *)FindWindow (IDC_LANG_ENGLISH))->SetValue (true);

	if (pConfig->GetSnapMode ()) ((wxRadioButton *)FindWindow (IDC_GET_DATA_SNAPMODE))->SetValue (true);
	else ((wxRadioButton *)FindWindow (IDC_GET_DATA_SNAPMODE_FALSE))->SetValue (true);

	((wxSpinCtrl*)FindWindow (IDC_LOG_REPORT_LIMIT))->SetValue (pConfig->GetLogReportLimit ());
}

BEGIN_EVENT_TABLE(GmPasswordDlg, wxDialog)
END_EVENT_TABLE()

GmPasswordDlg::GmPasswordDlg (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxSize (300, 100), wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxHORIZONTAL, this, wxT (""));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 15);

	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_PROG_PASSWORD")), 0, wxALIGN_RIGHT);
	pStaticSizer->Add (new wxTextCtrl (this, IDC_PASSWORD, wxT (""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD)
					, 1, wxEXPAND);
	//bottom.
	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 15);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnParent ();
}

wxString GmPasswordDlg::GetPassword () const
{
	return ((wxTextCtrl*)FindWindow (IDC_PASSWORD))->GetLabel ();
}