//
// monitor implemetation definition.
// Author:yeyouqun@163.com
// 2010-3-13
//
#include <gmpre.h>
#include <wx/statline.h>
#include "engineinc.h"
#include <wx/xml/xml.h>
#include "appconfig.h"
#include "monitorimpl.h"
#include "gimudefs.h"

DEFINE_EVENT_TYPE(GmEventHandleFile)
DEFINE_EVENT_TYPE(GmEventHandleDir)
DEFINE_EVENT_TYPE(GmEventHandleSize)
DEFINE_EVENT_TYPE(GmEventHandleMessage)
DEFINE_EVENT_TYPE(GmEventHandleBegin)
DEFINE_EVENT_TYPE(GmEventHandleEnd)
DEFINE_EVENT_TYPE(GmEventHandleBeginAnalysis)
DEFINE_EVENT_TYPE(GmEventHandleEndAnalysis)
DEFINE_EVENT_TYPE(GmEventHandleError)
DEFINE_EVENT_TYPE(GmEventHandleWarn)
DEFINE_EVENT_TYPE(GmEventHandleOverwrite1)
DEFINE_EVENT_TYPE(GmEventHandleOverwrite2)
DEFINE_EVENT_TYPE(GmEventHandleManualStop)
DEFINE_EVENT_TYPE(GmEventHandleDirectoryInfo)

GmJobMonitor::GmJobMonitor (wxWindow * parent, JobListItemData * pData)
										: m_pParent (parent)
										, m_pItemData (pData)
										, m_bReplaceAll (false)
{
	assert (m_pParent);
	assert (m_pItemData);
}


GmJobMonitor::~GmJobMonitor ()
{
}

void GmJobMonitor::HandleFile (const wxString & FileName)
{
	//
	//  暂时不处理
	//
	m_InfoLast.Files ++;
}

void GmJobMonitor::HandleDir (const wxString & DirName)
{
	//
	//  暂时不处理
	//
	m_InfoLast.Dirs ++;
}

wxString GmJobMonitor::HandlePassword ()
{
	return wxEmptyString;
}

void GmJobMonitor::AddPendingEvent (wxEventType type, const wxString & Message)
{
	GmJobEvent event(type);
	event.SetValue (Message, m_Info, m_InfoLast, m_pParent, m_pItemData);
	m_pParent->AddPendingEvent (event);
}

int GmJobMonitor::GetLogReportLimit () const
{
	GmAppConfig *pConfig = GmAppConfig::GetInstance ();
	return pConfig->GetLogReportLimit ();
}

BEGIN_EVENT_TABLE(GmOverwriteDlg, wxDialog)
	EVT_BUTTON (IDC_REPLACE_ALL, GmOverwriteDlg::OnReplaceAll)
END_EVENT_TABLE()

GmOverwriteDlg::GmOverwriteDlg (wxWindow * parent
							, const wxString & FileName
							, const GmLeafInfo & New
							, const GmLeafInfo & Old
							, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxString file = wxString::Format (_("IDS_REPLACE_FILE_DLG"), FileName.c_str ());
	wxStaticText * tc = new wxStaticText (this, wxID_ANY, file, wxDefaultPosition, wxSize (230, 50));
	pBoxSizer->Add (tc, 0, wxEXPAND | wxALL, 15);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this, _("IDS_NODE_PROPERTIES"));
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 15);

	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (3, 5, 5, 5);
	pStaticSizer->Add (pGridSizer);
	pGridSizer->AddGrowableCol (1);
	pGridSizer->AddGrowableCol (4);

	//line one
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_ORIGINAL_FILE")), 0, wxALIGN_RIGHT);
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, wxT ("|")));
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_DEST_FILE")), 0, wxALIGN_RIGHT);
	pGridSizer->Add (new wxBoxSizer (wxHORIZONTAL));
	//line two
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_FILE_SIZE")), 0, wxALIGN_RIGHT);
	wxString size = wxString::Format (wxT ("%I64d"), New.FileSize);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, size), 0, wxALIGN_RIGHT);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, wxT ("|")));
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_FILE_SIZE")), 0, wxALIGN_RIGHT);
	size = wxString::Format (wxT ("%I64d"), Old.FileSize);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, size), 0, wxALIGN_RIGHT);
	//line three
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_MODIFY_TIME")), 0, wxALIGN_RIGHT);
	wxString time = wxDateTime (TimeTToWxDateTime (New.ModifyTime)).Format ();
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, time), 0, wxALIGN_RIGHT);
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, wxT ("|")));
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_MODIFY_TIME")), 0, wxALIGN_RIGHT);
	time = wxDateTime (TimeTToWxDateTime (Old.ModifyTime)).Format ();
	pGridSizer->Add (new wxStaticText (this, wxID_ANY, time), 0, wxALIGN_RIGHT);

	//bottom.
	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer2->Add (new wxButton (this, IDC_REPLACE_ALL, _("IDS_REPLACE_ALL")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_REPLACE")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (this, wxID_CANCEL, _("IDS_REPLACE_NO")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	CenterOnScreen ();
}

void GmOverwriteDlg::OnReplaceAll (wxCommandEvent &event)
{
	wxDialog::EndModal (event.GetId ());
}

bool GmJobMonitor::OverwriteFile (const wxString & FileName, const GmLeafInfo & New, const GmLeafInfo & Old)
{
	if (m_bReplaceAll) return true;

	GmOverwriteDlg::OverwriteBlob blob (FileName, New, Old);
	GmJobEvent event(GmEventHandleOverwrite1);
	event.SetValue (wxT (""), m_Info, m_InfoLast, m_pParent, m_pItemData);
	event.m_EventData = &blob;
	m_pParent->AddPendingEvent (event);

	while (!blob.m_IsOK)
		Sleep (1000);

	int ret = blob.m_RetCode;
	if (ret == wxID_OK) {
		return true;
	}
	else if (ret == wxID_CANCEL) {
		return false;
	}
	else if (ret == GmOverwriteDlg::IDC_REPLACE_ALL) {
		m_bReplaceAll = true;
		return true;
	}

	return true;
}

bool GmJobMonitor::OverwriteFile (const wxString & FileName, ubyte8 ModifyTime, ubyte8 Size)
{
	wxString time = wxDateTime (TimeTToWxDateTime (ModifyTime)).Format ();
	wxString message = wxString::Format (_("IDS_SAME_FILE_EXISTS"), FileName.c_str (), time.c_str (), Size);
	return wxMessageBox (message, _("IDS_CONFIRM_PLEASE"), wxYES_NO, m_pParent);
}

wxString GmJobMonitor::GetReportFile () const
{
	return ::GetReportFile ();
}

wxString GmJobMonitor::GetLogFile () const
{ 
	return ::GetLogFile ();
}

void GmJobMonitor::HandleSize (ubyte4 Size)
{
	m_InfoLast.Size += Size;
	AddPendingEvent (GmEventHandleSize, wxT (""));
}

void GmJobMonitor::HandleMessage (const wxString & Message)
{
	AddPendingEvent (GmEventHandleMessage, Message);
}

wxString GmJobMonitor::SelectFile (const wxString & FileName)
{
	return wxEmptyString;
}

void GmJobMonitor::Begin (const wxString & Message)
{
	AddPendingEvent (GmEventHandleBegin, Message);
}

void GmJobMonitor::End (const wxString & Message)
{
	AddPendingEvent (GmEventHandleEnd, Message);
}

void GmJobMonitor::BeginAnalysis (const wxString & Message)
{
	AddPendingEvent (GmEventHandleBeginAnalysis, Message);
}

void GmJobMonitor::EndAnalysis (const wxString & Message)
{
	AddPendingEvent (GmEventHandleEndAnalysis, Message);
}

void GmJobMonitor::HandleError (const wxString &Message)
{
	AddPendingEvent (GmEventHandleError, Message);
}

void GmJobMonitor::HandleWarn (const wxString &Message)
{
	AddPendingEvent (GmEventHandleWarn, Message);
}

void GmJobMonitor::HandleManualStop (const wxString &Message)
{
	AddPendingEvent (GmEventHandleManualStop, Message);
}

void GmJobMonitor::DoHandleDirectoryInfo (const GmDirectoryInfo &)
{
	AddPendingEvent (GmEventHandleDirectoryInfo, wxT (""));
}

wxString GmJobMonitor::HandleSelectDir (const wxString & path)
{
	wxDirDialog dlg (m_pParent);
	if (dlg.ShowModal () == wxID_OK) {
		return dlg.GetPath ();
	}

	return wxEmptyString;
}

////////////////////////////////////////////////////////////////////////////////

void GmJobEvent::SetValue (const wxString Message
							, const GmDirectoryInfo & Info
							, const GmDirectoryInfo & InfoLast
							, const wxWindow * pParent
							, const JobListItemData * pItemData)
{
	message = Message;
	m_Info = Info;
	m_InfoLast = InfoLast;
	m_pParent = const_cast<wxWindow*> (pParent);
	m_pItemData = const_cast<JobListItemData*> (pItemData);
}

