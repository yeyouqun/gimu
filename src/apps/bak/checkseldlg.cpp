//
// file select dialog's definition that with checkbox ctrl.
// Author:yeyouqun@163.com
// 2010-2-8
//

#include <gmpre.h>
#include <wx/treectrl.h>
#include <wx/image.h>
#include <wx/statline.h>
#include "engineinc.h"
#include "checkrestdlg.h"
#include "checkseldlg.h"
#include "optionsdlg.h"
#include "res/button/folder.xpm"
#include <wx/xml/xml.h>
#include "appconfig.h"
#include "gmutil.h"
#include <wx/splash.h>
#include "gimuapp.h"

BEGIN_EVENT_TABLE(GmFileSelectDialog, wxDialog)
	EVT_BUTTON(IDC_JOB_OPTIONS, GmFileSelectDialog::OnOptions)
	EVT_BUTTON(IDC_JOB_DEST_SEL_BUTT, GmFileSelectDialog::OnSelectDirectory)
	EVT_BUTTON(wxID_OK, GmFileSelectDialog::OnOK)
	EVT_COMBOBOX (IDC_JOB_FORMAT, GmFileSelectDialog::OnSelectFileFormat)
END_EVENT_TABLE()


struct GmSelectItemData : public wxTreeItemData
{
	wxString	m_path;
	bool		m_bfile;
	GmSelectItemData (const wxString & path, bool bFile = false) : m_path (path), m_bfile (bFile)
	{
	}
};

GmFileSelectDialog::GmFileSelectDialog (wxWindow * parent, const wxString & title, GmJobItem * pJobItem/* = 0*/)
					: wxDialog (parent, wxID_ANY, title
								, wxDefaultPosition, wxDefaultSize, wxNO_3D | wxDEFAULT_DIALOG_STYLE)
					, m_pTempJobItem (0)
{
	if (pJobItem != 0) {
		m_pTempJobItem = new GmJobItem;
		*m_pTempJobItem = *pJobItem;
	}

	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	m_tree = new GmSysFileTreeCtrl (this, IDC_CHECK_BOX_TREE, wxDefaultPosition, wxSize (500, 300)
										, wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_ROW_LINES);
	pBoxSizer->Add (m_tree, 1, wxEXPAND | wxALL, 15);

	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxLEFT | wxRIGHT, 15);

	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (3, 3, 5, 5);
	wxStaticBoxSizer * pStaticBoxSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_JOB_DATA_BOX"));
	pStaticBoxSizer->Add (pGridSizer, 1, wxEXPAND);
	pBoxSizer->Add (pStaticBoxSizer, 0, wxEXPAND | wxALL, 15);
	//
	// 第一行
	//
	pGridSizer->AddGrowableCol (1);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_JOB_NAME")), 0, wxRIGHT | wxALIGN_RIGHT, 5);
	m_JobName = new wxTextCtrl (this, IDC_JOB_NAME, _(""));
	pGridSizer->Add (m_JobName, 1, wxEXPAND);
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);

	//
	// 第二行
	//
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_SEL_DIRECTORY")), 0, wxRIGHT | wxALIGN_RIGHT, 5);
	wxArrayString items;
	pConfig->GetUsedDirectory (items);
	m_JobDest = new wxComboBox (this, IDC_JOB_DEST, wxT (""), wxDefaultPosition, wxDefaultSize, items);
	m_JobDest->SetValue (wxEmptyString);

	pGridSizer->Add (m_JobDest, 1, wxEXPAND);
	wxBitmap bitmap (folder_xpm);
	wxImage image = bitmap.ConvertToImage ();
	image.SetMaskColour (0, 0, 0);
	wxBitmapButton * pb = new wxBitmapButton (this, IDC_JOB_DEST_SEL_BUTT, wxBitmap (image));
	pGridSizer->Add (pb);

	//
	// 第三行
	//
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_OUTPUT_FORMAT")), 0, wxRIGHT | wxALIGN_RIGHT, 5);
	
	items.Clear ();
	GetSupportFormat (items);
	m_OutFormat = new wxComboBox (this, IDC_JOB_FORMAT, _(""), wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
	m_OutFormat->SetValue (pConfig->GetDefaultType ());

	pGridSizer->Add (m_OutFormat, 1, wxEXPAND);
	pGridSizer->Add (new wxButton (this, IDC_JOB_OPTIONS, _("IDS_JOB_OPTIONS")), 0, wxEXPAND | wxALIGN_RIGHT);

	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	wxCheckBox * run = new wxCheckBox (this, IDC_RUN_AT_ONCE, _("IDS_RUN_AT_ONCE"));
	pBoxSizer2->Add (run, 0, wxRIGHT | wxLEFT, 20);
	run->SetValue (true);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND | wxRIGHT);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);
	InitControlStatus (m_pTempJobItem);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

GmFileSelectDialog::~GmFileSelectDialog ()
{
	if (m_pTempJobItem != 0) delete m_pTempJobItem;
}

void GmFileSelectDialog::OnOptions (wxCommandEvent & event)
{
	GmOptionsDlg dlg (this, _("IDS_CONFIG_JOB"));
	wxString type = m_OutFormat->GetValue ();
	if (m_pTempJobItem == 0) {
		m_pTempJobItem = CreateDefaultJobItem (m_JobName->GetValue (), m_JobDest->GetValue ());
		SetNewJobItemWithDemandParams (m_pTempJobItem, m_JobName->GetValue (), m_JobDest->GetValue (), type);
	}

	dlg.InitControlStatus (m_pTempJobItem);
	if (dlg.ShowModal () == wxID_OK) {
		dlg.SetJobData (m_pTempJobItem);
	}
}

void GmFileSelectDialog::OnSelectDirectory (wxCommandEvent & event)
{
	m_OutFormat->GetValue ();
	wxDirDialog dlg (this);
	if (dlg.ShowModal () == wxID_OK) {
		m_JobDest->SetLabel (dlg.GetPath ());
	}
}

void GmFileSelectDialog::OnSelectFileFormat (wxCommandEvent & event)
{
	if (m_pTempJobItem == 0) return;

	wxString oldtype = m_pTempJobItem->GetJobString ();
	wxString newtype = m_OutFormat->GetValue ();

	if (oldtype == newtype) return;
	
	//
	// 如果改变了类型，原来的设置基本无用。
	//
	bool newiscopytype = newtype.CmpNoCase (GetSyncTypeStr ()) == 0 || newtype.CmpNoCase (GetDupTypeStr ()) == 0;
	m_pTempJobItem->SetSplitSize (-1);
	m_pTempJobItem->SetJobType (GetJobTypeValue (newtype));
	m_pTempJobItem->SetEncryptType (ENC_NO_ENC);

	if (newiscopytype) {
		//
		// 不支持压缩。
		//
		m_pTempJobItem->SetDeflateType (DEF_NO_DEFLATE);
	}
	else {
		//
		//　默认支持默认等级的压缩。
		//
		m_pTempJobItem->SetDeflateType (DEF_DEFAULT);
	}
}

void GmFileSelectDialog::InitControlStatus (GmJobItem * pJobItem)
{
	if (pJobItem == 0)
		return;

	vector<GmJobSource*> & sources = pJobItem->GetSources ();
	for (size_t index = 0; index < sources.size (); ++index) {
		if (!m_tree->SetNodeSelected (sources[index]->szPath)) {
			delete sources[index];
			sources[index] = 0;
		}
	}

	vector<GmJobSource*>::iterator pos = remove (sources.begin (), sources.end (), (GmJobSource*)0);
	sources.erase (pos, sources.end ());

	m_JobName->SetValue (pJobItem->Name ());
	m_OutFormat->SetValue (pJobItem->GetJobString ());
	m_JobDest->SetValue (pJobItem->DestPath ());

	//
	// 一个任务只能有一种格式。
	//
	m_OutFormat->Enable (false);
}

bool GmFileSelectDialog::RunAtOnce () const
{
	if (((wxCheckBox *)FindWindow (IDC_RUN_AT_ONCE))->IsChecked ()) 
		return true;

	return false;
}

void GmFileSelectDialog::OnOK (wxCommandEvent & event)
{
	if (!CheckJobNameAndDest (m_JobName->GetValue (), m_JobDest->GetValue ()))
		return;

	if (!IsThisTypeSupported (m_OutFormat->GetValue ()))
		return;

	vector<wxString> strs;
	m_tree->GetSelectedPathes (strs);
	if (strs.empty ()) {
		wxMessageBox (_("IDS_SELECT_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	if (m_pTempJobItem == 0) m_pTempJobItem = CreateDefaultJobItem (m_JobName->GetValue (), m_JobDest->GetValue ());
	//
	// 防止同一个源被加多次。
	//
	m_pTempJobItem->ClearSources ();
	for (size_t index = 0; index < strs.size (); ++index) {
		m_pTempJobItem->AddSource (strs[index]);
	}

	SetNewJobItemWithDemandParams (m_pTempJobItem
								, m_JobName->GetValue ()
								, m_JobDest->GetValue ()
								, m_OutFormat->GetValue ());
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	pConfig->AddUsedDirectory (m_JobDest->GetValue ());

	wxDialog::EndModal (wxID_OK);
}

const GmJobItem & GmFileSelectDialog::GetJobItem () const
{
	assert (m_pTempJobItem);
	return *m_pTempJobItem;
}

void GmSysFileTreeCtrl::GetSelectedPathes (vector<wxString> & pathes)
{
	vector<wxTreeItemData*> vitems;
	GetFullSelectedItems (vitems);
	for (size_t index = 0; index < vitems.size (); ++index) {
		GmSelectItemData * pData = (GmSelectItemData*)vitems[index];
		if (pData == 0)
			continue;

		if (!pData->m_path.IsEmpty ())
			pathes.push_back (pData->m_path);
	}
}

IMPLEMENT_DYNAMIC_CLASS (GmSysFileTreeCtrl, GmCheckTreeCtrl)
BEGIN_EVENT_TABLE(GmSysFileTreeCtrl, GmCheckTreeCtrl)
	EVT_TREE_ITEM_ACTIVATED (wxID_ANY, GmSysFileTreeCtrl::OnExpandItem)
	EVT_TREE_ITEM_EXPANDING (wxID_ANY, GmSysFileTreeCtrl::OnExpandingItem)
	EVT_TREE_ITEM_RIGHT_CLICK (wxID_ANY, GmSysFileTreeCtrl::OnMouseRight)
	EVT_MENU (IDC_SELECT, GmSysFileTreeCtrl::OnSelectItem)
	EVT_MENU (IDC_DESELECT, GmSysFileTreeCtrl::OnDeSelectItem)
	EVT_MENU (IDC_NODE_PROPERTIES, GmSysFileTreeCtrl::OnShowProperties)
END_EVENT_TABLE()

GmSysFileTreeCtrl::GmSysFileTreeCtrl (wxWindow* parent
									, wxWindowID id
									, const wxPoint& pos
									, const wxSize& size
									, long style
									, const wxValidator& validator
									, const wxString& name)
									: GmCheckTreeCtrl (parent, id, pos, size, style, validator, name)
{
	AddRootNodes (GetRootItem ());
}

void GmSysFileTreeCtrl::AddRootNodes (wxTreeItemId parent)
{
	wxTreeItemId theItem = AppendItem (parent, _("IDS_DESKTOP"), II_DESKTOP, -1, new GmSelectItemData (wxEmptyString));
	AddDeskTopItems (theItem);

	theItem = AppendItem (parent, _("IDS_MY_COMPUTER"), II_MY_COMPUTER, -1, new GmSelectItemData (wxEmptyString));
	AddDiskItems (theItem);
	wxTreeItemId MyComputerItem = theItem;

	theItem = AppendItem (parent, _("IDS_NETWORK"), II_NETWORK, -1, new GmSelectItemData (wxEmptyString));
	AddNetworkItems (theItem);

	theItem = AppendItem (parent, _("IDS_MY_DOCS"), II_FOLDER, -1, new GmSelectItemData (GetMyDocFolder ()));
	AddForgedNode (theItem);
	Expand (MyComputerItem);
	//
	//  如果支持插件，请在下面加入代码。
	//
}


void GmSysFileTreeCtrl::AddDeskTopItems (wxTreeItemId parent)
{
	wxString desktop = GetDesktopFolder ();
	if (desktop.IsEmpty ())
		return;

	if (!parent.IsOk ())
		return;

	GmSelectItemData * pData = (GmSelectItemData*)GetItemData (parent);
	if (pData != 0)
		delete pData;

	SetItemData (parent, new GmSelectItemData (desktop));
	AddDirectoryItems (desktop, parent);
}

void GmSysFileTreeCtrl::AddDirectoryItems (const wxString & dir, wxTreeItemId parent)
{
	wxDir directory (dir);
	wxString DirOrFile;
	bool bEnumerateOK = directory.GetFirst (&DirOrFile);

	int image = GetItemImage (parent);
	bool bSelected = false;
	if (image == -1 || image == II_DESKTOP_SELECTED || image == II_DISK_SELECTED || image == II_FOLDER_SELECTED)
		bSelected = true;

	while (bEnumerateOK) {
		wxString szFullPath = AssemblePath (dir, DirOrFile);
		if (IsFile (szFullPath)) {
			AppendItem (parent, DirOrFile, bSelected ? II_FILE_SELECTED : II_FILE
											, -1, new GmSelectItemData (szFullPath, true));
		}
		else {
			wxTreeItemId theItem = AppendItem (parent, DirOrFile, bSelected ? II_FOLDER_SELECTED : II_FOLDER
														, -1, new GmSelectItemData (szFullPath));
			AddForgedNode (theItem);
		}

		bEnumerateOK = directory.GetNext (&DirOrFile);
	};

}

void GmSysFileTreeCtrl::OnExpandingItem (wxTreeEvent & event)
{
	//
	// 在此接口中的函数，不再调用 Expand 方法。
	//
	wxTreeItemId item = event.GetItem ();
	if (!item.IsOk ())
		return;

	GmSelectItemData * pData = (GmSelectItemData*)GetItemData (item);
	if (pData == 0 || pData->m_bfile)
		return;


	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (item, cookie);
	//
	// 已经展开过了。
	//
	if (!child.IsOk () || GetItemImage (child) != -1)
		return;

	DeleteChildren (item);
	if (GetItemImage (item) == II_MY_COMPUTER) {
		//
		// 没有选择状态，即不可选择状态，其子结点就是使用默认状态。
		//
		AddDiskItems (item);
	}
	else if (GetItemImage (item) == II_NETWORK) {
		//
		// 没有选择状态，即不可选择状态，其子结点就是使用默认状态。
		//
		AddNetworkItems (item);
	}
	else {
		if (!pData->m_path.IsEmpty ())
			AddDirectoryItems (pData->m_path, item);
	}
}
void GmSysFileTreeCtrl::AddNetworkItems (wxTreeItemId parent)
{
	AddForgedNode (parent);
}

void GmSysFileTreeCtrl::AddDiskItems (wxTreeItemId parent)
{
	unsigned drives = GetLogicalDrives();
	for(int drive = 0; drives != 0; drives >>= 1, ++drive) {
		if ((drives & 0x1) != 0) {
			wxString vol = (char_t)('A' + drive);
			vol += wxT (":");
			unsigned type = GetDriveType(vol.c_str ());
			if (type == DRIVE_REMOVABLE || type == DRIVE_FIXED || type == DRIVE_REMOTE) {
				wxTreeItemId theItem = AppendItem (parent, vol, II_DISK, -1, new GmSelectItemData (vol));
				AddForgedNode (theItem);
			}
			else if (type == DRIVE_CDROM) {
				wxTreeItemId theItem = AppendItem (parent, vol, II_CD, -1, new GmSelectItemData (vol));
				AddForgedNode (theItem);
			}
		}
	}
}

void GmSysFileTreeCtrl::OnExpandItem (wxTreeEvent & event)
{
	wxTreeItemId item = event.GetItem ();
	if (!item.IsOk ())
		return;

	if (IsExpanded (item)) {
		Toggle (item);
		return;
	}

	Expand (item);
}

bool GmSysFileTreeCtrl::SetNodeSelected (wxTreeItemId item, const wxString & pathfile)
{
	if (pathfile.IsEmpty ()) return false;
	if (!item.IsOk ()) return false;

	Expand (item);
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (item, cookie);
	while (child.IsOk ()) {
		int image = GetItemImage (child);
		GmSelectItemData * pData = (GmSelectItemData*)GetItemData (child);
		if (pData != 0) {
			if (pData->m_path == pathfile) {
				int image = GetSelectedImage (child);
				SetSubSuperSelected (child, image);
				return true;
			}
			else if (pathfile.StartsWith (pData->m_path)) {
				return SetNodeSelected (child, pathfile);
			}
		}

		child = GetNextChild (child, cookie);
	}

	return false;
}

bool GmSysFileTreeCtrl::SetNodeSelected (const wxString & pathfile)
{
	if (pathfile.IsEmpty ()) return false;

	wxTreeItemId item = GetRootItem ();
	if (!item.IsOk ()) return false;

	wxTreeItemId subroot;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild (item, cookie);
	while (child.IsOk ()) {
		int image = GetItemImage (child);
		GmSelectItemData * pData = (GmSelectItemData*)GetItemData (child);
		if (pData != 0) {
			if (pData->m_path == pathfile) {
				int image = GetSelectedImage (child);
				SetSubSuperSelected (child, image);
				return true;
			}
			else if (pathfile.StartsWith (pData->m_path)) {
				return SetNodeSelected (child, pathfile);
			}
		}
		else if (image == II_MY_COMPUTER || image == II_NETWORK) {
			return SetNodeSelected (child, pathfile);
		}

		child = GetNextChild (child, cookie);
	}

	return false;
}

void GmSysFileTreeCtrl::OnMouseRight (wxTreeEvent & event)
{
	if (!event.GetItem ().IsOk ()) return;

	SelectItem (event.GetItem ());
	wxMenu popup;
	popup.Append (IDC_SELECT, _("IDS_SELECT_IT"));
	popup.Append (IDC_DESELECT, _("IDS_DESELECT_IT"));
	popup.Append (IDC_NODE_PROPERTIES, _("IDS_GET_FILE_PROPERTIES"));
	int image = GetItemImage (event.GetItem ());
	bool EnaSelected = !IsSelectedImage (image);
	popup.Enable (IDC_SELECT, EnaSelected);
	popup.Enable (IDC_DESELECT, !EnaSelected);
	PopupMenu (&popup);
	event.Skip ();
}


void GmSysFileTreeCtrl::OnSelectItem (wxCommandEvent & event)
{
	wxTreeItemId item = GetSelection ();
	if (!item.IsOk ()) return;
	SetSubSuperSelected (item, GetSelectedImage (item));
}

void GmSysFileTreeCtrl::OnDeSelectItem (wxCommandEvent & event)
{
	wxTreeItemId item = GetSelection ();
	if (!item.IsOk ()) return;
	SetSubSuperUnSelected (item, GetUnSelectedImage (item));
}

void GmSysFileTreeCtrl::OnShowProperties (wxCommandEvent &event)
{
	wxTreeItemId item = GetSelection ();
	if (!item.IsOk ()) return;

	wxString path;
	GmSelectItemData * pData = (GmSelectItemData*)GetItemData (item);
	if (pData != 0) path = pData->m_path;

	SHELLEXECUTEINFO   execInfo;
	execInfo.fMask = SEE_MASK_INVOKEIDLIST;  
	execInfo.cbSize = sizeof(execInfo);
	execInfo.hwnd = (HWND)wxGetApp ().GetTopWindow ()->GetHandle ();
	execInfo.lpVerb = wxT ("properties");  
	execInfo.lpFile = path.c_str ();   //文件全路径
	execInfo.lpParameters = NULL;
	execInfo.lpDirectory = NULL;
	execInfo.nShow = SW_SHOWNORMAL;
	execInfo.hProcess = NULL;
	execInfo.lpIDList = 0;
	execInfo.hInstApp = 0;
	ShellExecuteEx(&execInfo); 
}

void SetNewJobItemWithDemandParams (GmJobItem * pNewJob
									, const wxString & szName
									, const wxString & szDest
									, const wxString & szTypeStr)
{
	pNewJob->Name (szName);
	pNewJob->DestPath (szDest);
	pNewJob->ClearOption (GO_TYPE_MASK);
	pNewJob->SetSplitSize (-1);
	pNewJob->SetEncryptType (ENC_NO_ENC);
	pNewJob->SetJobType (GetJobTypeValue (szTypeStr));

	if (szTypeStr.CmpNoCase (GetSyncTypeStr ()) == 0) {
		// sync
		pNewJob->SetOption (GO_TYPE_SYNC);
		pNewJob->SetDeflateType (DEF_NO_DEFLATE);
	}
	else if (szTypeStr.CmpNoCase (GetDupTypeStr ()) == 0) {
		//dup
		pNewJob->SetOption (GO_TYPE_DUP);
		pNewJob->SetDeflateType (DEF_NO_DEFLATE);
	}
	else if (szTypeStr.CmpNoCase (GetZipTypeStr ()) == 0) {
		// zip
		pNewJob->SetOption (GO_TYPE_ZIP);
		pNewJob->SetDeflateType (DEF_DEFAULT);
	}
	else if (szTypeStr.CmpNoCase (Get7zTypeStr ()) == 0) {
		// 7z
		pNewJob->SetOption (GO_TYPE_7Z);
		pNewJob->SetDeflateType (DEF_DEFAULT);
	}
}

