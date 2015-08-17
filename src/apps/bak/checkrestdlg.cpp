//
// file restoration dialog's definition that with checkbox ctrl.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include "checkrestdlg.h"
#include <wx/statline.h>
#include "checktreectrl.h"
#include "engineinc.h"
#include "res/button/folder.xpm"
#include <wx/xml/xml.h>
#include "appconfig.h"
#include "gmutil.h"
#include "optionsdlg.h"

struct GmStoreEntryData : public wxTreeItemData
{
	GmStoreEntryData (GmStoreEntry * pData) : m_pData (pData) {}
	GmStoreEntry *	m_pData;
};

class GmRestOptionsDialog : public wxDialog
{
public:
	GmRestOptionsDialog (wxWindow * parent, bool bNewPlace);
	GmRestOptionsDialog () : wxDialog () {}
	int GetRestoreOption () const;
private:
private:
	wxRadioButton *	m_pOvwrOld;
	wxRadioButton *	m_pSkip;
	wxRadioButton *	m_pOverWrite;
	wxRadioButton *	m_pAskMe;

	DECLARE_DYNAMIC_CLASS (GmRestOptionsDialog)
	DECLARE_EVENT_TABLE ()
	DECLARE_NO_COPY_CLASS(GmRestOptionsDialog)
};

int GmRestOptionsDialog::GetRestoreOption () const
{
	int option = 0;
	if (m_pOvwrOld->GetValue ()) {
		option |= RO_REPLACE_OLD;
	}
	else if (m_pSkip->GetValue ()) {
		option |= RO_SKIP;
	}
	else if (m_pOverWrite->GetValue ()) {
		option |= RO_OVERWRITE;
	}
	else {
		option |= RO_DECIDE_BY_USER;
	}

	return option;
}

GmRestOptionsDialog::GmRestOptionsDialog (wxWindow * parent, bool bNewPlace)
				: wxDialog (parent, wxID_ANY, _("IDS_REST_OPTIONS_DLG")
				, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_REST_OPTIONS_DLG"));
	pBoxSizer->Add (pStaticSizer, 0, wxEXPAND | wxALL, 15);

	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_IF_EXIST_FILE")), 0, wxLEFT | wxTOP, 15);

	wxBoxSizer * pBoxSizerVert = new wxBoxSizer (wxVERTICAL);
	pStaticSizer->Add (pBoxSizerVert, 0, wxLEFT, 30);
	m_pAskMe = new wxRadioButton (this, wxID_ANY, _("IDS_ASKME"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_pOvwrOld = new wxRadioButton (this, wxID_ANY, _("IDS_OVERWIRTE_OLD"));
	m_pSkip = new wxRadioButton (this, wxID_ANY, _("IDS_SKIP"));
	m_pOverWrite = new wxRadioButton (this, wxID_ANY, _("IDS_OVERWRITE"));

	pBoxSizerVert->Add (m_pAskMe, 0, wxTOP, 10);
	pBoxSizerVert->Add (m_pOvwrOld, 0, wxTOP, 10);
	pBoxSizerVert->Add (m_pSkip, 0, wxTOP, 10);
	pBoxSizerVert->Add (m_pOverWrite, 0, wxTOP | wxBOTTOM, 10);


	wxBoxSizer * pBoxSizerLine = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizerLine->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer->Add (pBoxSizerLine, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, 15);
	pBoxSizerLine->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizerLine->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxALIGN_RIGHT);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnParent ();
}

IMPLEMENT_DYNAMIC_CLASS (GmRestOptionsDialog, wxDialog)
BEGIN_EVENT_TABLE(GmRestOptionsDialog, wxDialog)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(GmRestoreDialog, wxDialog)
	EVT_BUTTON(wxID_OK, GmRestoreDialog::OnOK)
	EVT_RADIOBUTTON (IDC_TO_ORIGINAL, GmRestoreDialog::OnToOriginal)
	EVT_RADIOBUTTON (IDC_TO_SPECIFIED_PATH, GmRestoreDialog::OnToSpecifiedPath)
	EVT_BUTTON (IDC_REST_OPTIONS, GmRestoreDialog::OnSetOptions)
	EVT_BUTTON(IDC_JOB_DEST_SEL_BUTT, GmRestoreDialog::OnSelectDirectory)
	// tree
	EVT_TREE_ITEM_ACTIVATED (IDC_TREE_CTRL, GmRestoreDialog::OnExpandItem)
	EVT_TREE_ITEM_EXPANDING (IDC_TREE_CTRL, GmRestoreDialog::OnExpandingItem)
	//pop up menu.
	EVT_TREE_ITEM_RIGHT_CLICK (IDC_TREE_CTRL, GmRestoreDialog::OnMouseRight)
	EVT_MENU (IDC_SELECT, GmRestoreDialog::OnSelectItem)
	EVT_MENU (IDC_DESELECT, GmRestoreDialog::OnDeSelectItem)
	EVT_MENU (IDC_NODE_PROPERTIES, GmRestoreDialog::OnShowProperties)
END_EVENT_TABLE()

GmRestoreDialog::GmRestoreDialog (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title
						, wxDefaultPosition, wxDefaultSize, wxNO_3D | wxDEFAULT_DIALOG_STYLE)
					, m_option (RO_ORIGINAL_PLACE | RO_DECIDE_BY_USER)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	m_pTree = new GmCheckTreeCtrl (this, IDC_TREE_CTRL, wxDefaultPosition, wxSize (500, 300)
										, wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_ROW_LINES);
	pBoxSizer->Add (m_pTree, 1, wxEXPAND | wxALL, 15);

	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxLEFT | wxRIGHT, 15);

	wxStaticBoxSizer * pStaticBoxSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_JOB_DATA_BOX"));
	pBoxSizer->Add (pStaticBoxSizer, 0, wxEXPAND | wxALL, 15);
	pStaticBoxSizer->Add (new wxRadioButton (this, IDC_TO_ORIGINAL, _("IDS_DATA_TO_ORIGINAL")
											, wxDefaultPosition, wxDefaultSize, wxRB_GROUP)
											, 0, wxBOTTOM | wxTOP | wxLEFT, 10);

	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pStaticBoxSizer->Add (pBoxSizer2, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
	pBoxSizer2->Add (new wxRadioButton (this, IDC_TO_SPECIFIED_PATH, _("IDS_DATA_TO_SPECIFIED_PATH"))
										, 0, wxBOTTOM | wxLEFT | wxALIGN_CENTER_VERTICAL, 10);
	m_pPath = new wxTextCtrl (this, IDC_TO_SPECIFIED_PATH);
	pBoxSizer2->Add (m_pPath, 1, wxEXPAND | wxRIGHT, 5);
	wxBitmap bitmap (folder_xpm);
	wxImage image = bitmap.ConvertToImage ();
	image.SetMaskColour (0, 0, 0);
	wxBitmapButton * pb = new wxBitmapButton (this, IDC_JOB_DEST_SEL_BUTT, wxBitmap (image));
	pBoxSizer2->Add (pb, 0,  wxRIGHT, 5);
	pBoxSizer2->Add (new wxButton (this, IDC_REST_OPTIONS, _("IDS_REST_OPTIONS")), 0, wxLEFT, 5);

	m_pPath->Enable (false);
	pb->Enable (false);

	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);
	pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND | wxRIGHT);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_CANCEL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

void GmRestoreDialog::OnSelectDirectory (wxCommandEvent & event)
{
	wxDirDialog dlg (this);
	if (dlg.ShowModal () == wxID_OK) {
		m_pPath->SetLabel (dlg.GetPath ());
	}
}

void GmRestoreDialog::OnOK (wxCommandEvent &event)
{
	vector<wxTreeItemData*> vitems;
	m_pTree->GetFullSelectedItems (vitems);
	if (vitems.empty ()) {
		wxMessageBox (_("IDS_SELECT_STORE_ENTRY"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	if (m_pPath->IsEnabled () && !IsValidPath (m_pPath->GetValue ()))
		return;

	wxDialog::EndModal (wxID_OK);
}

void GmRestoreDialog::OnToOriginal (wxCommandEvent &event)
{
	m_pPath->Enable (false);
	FindWindow (IDC_JOB_DEST_SEL_BUTT)->Enable (false);
}

void GmRestoreDialog::OnToSpecifiedPath (wxCommandEvent &event)
{
	m_pPath->Enable (true);
	FindWindow (IDC_JOB_DEST_SEL_BUTT)->Enable (true);
}

void GmRestoreDialog::OnSetOptions (wxCommandEvent &event)
{
	bool bNewPlace = m_pPath->IsEnabled ();
	GmRestOptionsDialog dialog (this, bNewPlace);
	if (dialog.ShowModal () == wxID_OK) {
		m_option = dialog.GetRestoreOption ();
	}
}

int GmRestoreDialog::GetRestoreOption () const
{
	if (m_pPath->IsEnabled ())
		const_cast<GmRestoreDialog*> (this)->m_option |= RO_NEW_PLACE;
	else
		const_cast<GmRestoreDialog*> (this)->m_option &= ~RO_NEW_PLACE;

	return m_option;
}

void GmRestoreDialog::ShowTree (GmRootCreator* ptree)
{
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	const vector<GmStoreEntry*> & roots = ptree->GetRoots (pConfig->GetSnapMode ());
	bool bIsZipTree = ptree->IsZipTree ();
	if (bIsZipTree) {
		((wxRadioButton*)FindWindow (IDC_TO_ORIGINAL))->Enable (false);
		((wxRadioButton*)FindWindow (IDC_TO_SPECIFIED_PATH))->SetValue (true);
		m_pPath->Enable (true);
		FindWindow (IDC_JOB_DEST_SEL_BUTT)->Enable (true);
	}

	wxTreeItemId ri = m_pTree->GetRootItem ();
	m_pTree->DeleteChildren (ri);
	if (!ri.IsOk ())
		return;

	bool bHasTimeRoot = ptree->HasSnapTree ();
	for (size_t index = 0; index < roots.size (); ++index) {
		wxTreeItemId parent = m_pTree->AppendItem (ri
												, roots[index]->GetName ()
												, bHasTimeRoot ? II_TIMEPNT : II_FOLDER
												, -1
												, new GmStoreEntryData (roots[index]));
		m_pTree->AddForgedNode (parent);
		if (index == 0) m_pTree->Expand (parent);
	}
}

GmRestoreDialog::~GmRestoreDialog ()
{
}

void GmRestoreDialog::GetSelectedItems (vector<GmStoreEntry*> & items) const
{
	items.clear ();
	vector<wxTreeItemData*> vitems;
	m_pTree->GetFullSelectedItems (vitems);
	for (size_t index = 0; index < vitems.size (); ++index)
		if (((GmStoreEntryData*)vitems[index])->m_pData != 0)
			items.push_back (((GmStoreEntryData*)vitems[index])->m_pData);

	return;
}

wxString GmRestoreDialog::GetDestPath () const
{
	if (m_pPath->IsEnabled ())
		return m_pPath->GetLabel ();

	return wxEmptyString;
}

void GmRestoreDialog::OnExpandItem (wxTreeEvent & event)
{
	wxTreeItemId item = event.GetItem ();
	if (!item.IsOk ())
		return;

	if (m_pTree->IsExpanded (item)) {
		m_pTree->Toggle (item);
		return;
	}

	ExpandThisItem (item);
	m_pTree->Expand (item);
}

void GmRestoreDialog::OnExpandingItem (wxTreeEvent & event)
{
	wxTreeItemId item = event.GetItem ();
	if (!item.IsOk ())
		return;

	wxTreeItemIdValue cookie;
	wxTreeItemId child = m_pTree->GetFirstChild (item, cookie);
	if (!(!child.IsOk () || m_pTree->GetItemImage (child) != -1)) {
		ExpandThisItem (item);
	}
}

void GmRestoreDialog::ExpandThisItem (wxTreeItemId item)
{
	GmStoreEntryData * pData = (GmStoreEntryData*)m_pTree->GetItemData (item);
	if (pData == 0 || pData->m_pData == 0)
		return;

	m_pTree->DeleteChildren (item);
	const vector<GmStoreEntry*> & subs = pData->m_pData->Expand ();
	if (subs.empty ())
		return;

	int image = m_pTree->GetItemImage (item);
	bool bSelected = false;
	if (image == -1 || image == II_DESKTOP_SELECTED ||
		image == II_DISK_SELECTED || image == II_FOLDER_SELECTED || image == II_TIMEPNT_SELECTED)
		bSelected = true;

	for (size_t index = 0; index < subs.size (); ++index) {
		GmStoreEntry* sep = subs[index];
		int type = sep->GetType ();
		int image = -1;
		if (type == GNT_SOURCE) {
			//
			// 只有 UIF 文件才会有这个类型。
			//
			GmUifSourceEntry * source = (GmUifSourceEntry*)sep->GetEntry ();
			if (source->SourceType == GST_DIRECTORY) image = bSelected ? II_FOLDER_SELECTED : II_FOLDER;
			else image = bSelected ? II_FILE_SELECTED : II_FILE;
		}
		else if (type == GNT_DIR) image = bSelected ? II_FOLDER_SELECTED : II_FOLDER;
		else image = bSelected ? II_FILE_SELECTED : II_FILE;

		wxTreeItemId parent = m_pTree->AppendItem (item, sep->GetName (), image, -1, new GmStoreEntryData (sep));
		if (image == II_FOLDER || image == II_FOLDER_SELECTED) m_pTree->AddForgedNode (parent);
	}
}


void GmRestoreDialog::OnMouseRight (wxTreeEvent & event)
{
	if (!event.GetItem ().IsOk ()) return;

	m_pTree->SelectItem (event.GetItem ());
	wxMenu popup;
	popup.Append (IDC_SELECT, _("IDS_SELECT_IT"));
	popup.Append (IDC_DESELECT, _("IDS_DESELECT_IT"));
	popup.Append (IDC_NODE_PROPERTIES, _("IDS_GET_FILE_PROPERTIES"));
	int image = m_pTree->GetItemImage (event.GetItem ());
	bool EnaSelected = !IsSelectedImage (image);
	popup.Enable (IDC_SELECT, EnaSelected);
	popup.Enable (IDC_DESELECT, !EnaSelected);
	PopupMenu (&popup);
	event.Skip ();
}


void GmRestoreDialog::OnSelectItem (wxCommandEvent & event)
{
	wxTreeItemId item = m_pTree->GetSelection ();
	if (!item.IsOk ()) return;
	m_pTree->SetSubSuperSelected (item, GetSelectedImage (item));
}

void GmRestoreDialog::OnDeSelectItem (wxCommandEvent & event)
{
	wxTreeItemId item = m_pTree->GetSelection ();
	if (!item.IsOk ()) return;
	m_pTree->SetSubSuperUnSelected (item, GetUnSelectedImage (item));
}

void GmRestoreDialog::OnShowProperties (wxCommandEvent &event)
{
	wxTreeItemId item = m_pTree->GetSelection ();
	if (!item.IsOk ()) return;

	GmStoreEntryData * pData = (GmStoreEntryData*)m_pTree->GetItemData (item);
	GmRestorePropertyDlg dlg (this, *pData->m_pData, _("IDS_NODE_PROPERTIES"));
	dlg.ShowModal ();
}
