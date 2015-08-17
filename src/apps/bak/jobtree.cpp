//
// job tree's implementation.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include <wx/treectrl.h>
#include <wx/panel.h>
#include <wx/image.h>
#include "engineinc.h"
#include "gimuframe.h"
#include "jobtree.h"
#include "checkseldlg.h"
#include <wx/statline.h>
#include "res/button/folder.xpm"
#include "optionsdlg.h"
#include "joblist.h"
#include "monitorimpl.h"
#include <wx/xml/xml.h>
#include "appconfig.h"
#include "gmutil.h"

BEGIN_EVENT_TABLE(GmTreeWindow, wxPanel)
	EVT_BUTTON(IDC_FAST_LAUNCH_JOB, GmTreeWindow::OnFastLaunchJob)
	EVT_BUTTON(IDC_CLEAR_SEL, GmTreeWindow::OnClearSelect)
	EVT_BUTTON(IDC_JOB_DEST_SEL_BUTT, GmTreeWindow::OnSelectDirectory)
	EVT_COMBOBOX (IDC_JOB_FORMAT, GmTreeWindow::OnSelectFileFormat)
	EVT_TEXT(IDC_JOB_DEST, GmTreeWindow::OnSetDirectory)
	EVT_TEXT(IDC_JOB_NAME, GmTreeWindow::OnSetJobName) 
	EVT_COMMAND  (wxID_ANY, GmEVT_NODE_CHANGED, GmTreeWindow::OnTreeChanged)
END_EVENT_TABLE()

GmTreeWindow::GmTreeWindow (GmAppFrame * m_pMainFrame, wxSplitterWindow * pParent)
							: wxPanel (pParent, wxID_ANY)
							, m_pMainFrame (m_pMainFrame)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	m_pSystemFileTree = new GmSysFileTreeCtrl (this, wxID_ANY);
	pBoxSizer->Add (m_pSystemFileTree, 1, wxEXPAND);

	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxTOP | wxLEFT | wxBOTTOM, 5);

	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (4, 3, 5, 5);
	wxStaticBoxSizer * pStaticBoxSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_JOB_DATA_BOX"));
	pStaticBoxSizer->Add (pGridSizer, 1, wxEXPAND);
	pBoxSizer->Add (pStaticBoxSizer, 0, wxEXPAND | wxALL, 5);

	//
	// 第一行
	//
	pGridSizer->AddGrowableCol (1);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_JOB_NAME")), 0, wxRIGHT | wxALIGN_RIGHT, 5);
	m_JobName = new wxTextCtrl (this, IDC_JOB_NAME, _(""));
	pGridSizer->Add (m_JobName, 1, wxEXPAND);
	pGridSizer->Add (new wxBoxSizer (wxVERTICAL));

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
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));

	//
	// 第四行
	//
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));
	wxBoxSizer * pBoxSizerLine = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizerLine->Add (new wxButton (this, IDC_FAST_LAUNCH_JOB, _("IDS_FAST_LAUNCH_JOB")));
	pBoxSizerLine->Add (new wxButton (this, IDC_CLEAR_SEL, _("IDS_CLEAR_SEL")), 0, wxLEFT, 20);
	EnableFastLaunchButtons (false);

	pGridSizer->Add (pBoxSizerLine);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
}

void GmTreeWindow::OnSelectDirectory (wxCommandEvent & event)
{
	wxDirDialog dlg (this);
	if (dlg.ShowModal () == wxID_OK) {
		m_JobDest->SetLabel (dlg.GetPath ());
	}
}

void GmTreeWindow::OnSelectFileFormat (wxCommandEvent & event)
{
}

void GmTreeWindow::OnSetDirectory (wxCommandEvent & event)
{
}

void GmTreeWindow::OnSetJobName (wxCommandEvent & event)
{
}

void GmTreeWindow::EnableFastLaunchButtons (bool enable)
{
	FindWindow (IDC_FAST_LAUNCH_JOB)->Enable (enable);
	FindWindow (IDC_CLEAR_SEL)->Enable (enable);
}

void GmTreeWindow::OnFastLaunchJob (wxCommandEvent & event)
{
	if (!CheckJobNameAndDest (m_JobName->GetValue (), m_JobDest->GetValue ()))
		return;

	if (!IsThisTypeSupported (m_OutFormat->GetValue ()))
		return;

	vector<wxString> strs;
	m_pSystemFileTree->GetSelectedPathes (strs);
	if (strs.empty ()) {
		wxMessageBox (_("IDS_SELECT_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	GmListWindow * pListWnd = m_pMainFrame->GetListWindow ();
	if (pListWnd->HasSameJob (m_JobName->GetValue ())) {
		wxMessageBox (_("IDS_HAS_SAME_JOB"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	GmJobItem * pNewJob = CreateDefaultJobItem (m_JobName->GetValue (), m_JobDest->GetValue ());

	for (size_t index = 0; index < strs.size (); ++index)
		pNewJob->AddSource (strs[index]);

	wxString dest = m_JobDest->GetValue ();
	SetNewJobItemWithDemandParams (pNewJob, m_JobName->GetValue (), dest, m_OutFormat->GetValue ());

	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	bool bAdded = pConfig->AddUsedDirectory (dest);

	pListWnd->CreateNewJob (pNewJob);
	m_pSystemFileTree->ClearAllSelected ();
	EnableFastLaunchButtons (false);
	m_JobName->Clear ();
	if (bAdded)
		m_JobDest->Insert (dest, 0);

	m_JobDest->SetValue (wxEmptyString);

	return;
}

void GmTreeWindow::SetDefaultType (const wxString & type)
{
	m_OutFormat->SetValue (type);
}

void GmTreeWindow::OnClearSelect (wxCommandEvent & event)
{
	m_pSystemFileTree->ClearAllSelected ();
	EnableFastLaunchButtons (false);
}

void GmTreeWindow::OnTreeChanged (wxCommandEvent &event)
{
	vector<wxTreeItemData*> vitems;
	m_pSystemFileTree->GetFullSelectedItems (vitems);
	bool value = false;
	if (!vitems.empty ())
		value = true;

	EnableFastLaunchButtons (value);
	return;
}
