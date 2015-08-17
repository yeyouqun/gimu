//
// job list's implementation.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include <wx/xml/xml.h>
#include <wx/treectrl.h>
#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/brush.h>

#include "engineinc.h"
#include "gimuframe.h"
#include "jobstatistics.h"
#include "joblist.h"
#include "res/list_res/normal.xpm"
#include "res/list_res/abnormal.xpm"
#include "res/list_res/paused.xpm"
#include "res/list_res/running.xpm"
#include "monitorimpl.h"
#include "checkseldlg.h"
#include "checkrestdlg.h"
#include <engine/log.h>
#include <engine/report.h>
#include "appconfig.h"
#include "res/report.xpm"
#include "res/log.xpm"
#include "gimudefs.h"

class GmTransStaticText : public wxStaticText
{
public:
	GmTransStaticText (wxWindow* parent
						, wxWindowID id
						, const wxString& label
						, const wxPoint& pos = wxDefaultPosition
						, const wxSize& size = wxDefaultSize
						, long style = 0
						, const wxString& name = wxT ("staticText"))
						: wxStaticText (parent, id, label, pos, size, style | wxTRANSPARENT_WINDOW, name)
	{
	}
	GmTransStaticText () : wxStaticText () {}
private:
	virtual WXHBRUSH MSWControlColor (WXHDC pDC, WXHWND hWnd);
};

WXHBRUSH GmTransStaticText::MSWControlColor (WXHDC pDC, WXHWND hWnd)
{
	SetBkMode ((HDC)pDC, TRANSPARENT);
	return (*wxTRANSPARENT_BRUSH).GetResourceHandle ();
}

enum ListImageIndex
{
	LII_NORMAL,
	LII_ABNORMAL,
	LII_PAUSED,
	LII_RUNNING,
};

#define GAUGE_POS	2

class GmListCtrl : public wxListCtrl
{
public:
	GmListCtrl (GmListWindow* parent
		, wxWindowID id
		, const wxPoint& pos = wxDefaultPosition
		, const wxSize& size = wxDefaultSize
		, long style = wxLC_ICON
		, const wxValidator& validator = wxDefaultValidator
		, const wxString& name = wxListCtrlNameStr);
	GmListCtrl () : wxListCtrl (), m_pParent (0) {}
	~GmListCtrl ();
	void AdjustGaugePos ();
private:
	friend class GmListWindow;
	enum {
		IDC_POP_RUN_JOB = 1000,
		IDC_POP_STOP_JOB,
		IDC_POP_RESUME_JOB,
		IDC_POP_DELETE_JOB,
		IDC_POP_PAUSE_JOB,
		IDC_POP_JOB_OPTION,
		IDC_POP_GET_DATA_BACK,
		IDC_POP_CREATE_NEW_JOB,
		IDC_POP_OPEN_ARCHIVE,
		IDC_POP_OPEN_DEST,
		IDC_POP_SHOW_LOGS,
		IDC_POP_SHOW_REPORTS,
		IDC_POP_EXPORT_SELECTED_JOB_LOGS,
		IDC_POP_EXPORT_SELECTED_JOB_REPORTS,
	};
	void OnMouseLeftDown (wxMouseEvent &event);
	void OnScrollList (wxScrollWinEvent & event);
	void OnListSize (wxSizeEvent &event);
	void OnDragColumn (wxListEvent &event);
	void OnRightClickList (wxMouseEvent & event);
	void OnRightClickListItem (wxListEvent &event);
	//
	void OnPopRunJob (wxCommandEvent &event);
	void OnPopStopJob (wxCommandEvent &event);
	void OnPopResumeJob (wxCommandEvent &event);
	void OnPopDeleteJob (wxCommandEvent &event);
	void OnPauseJob (wxCommandEvent &event);
	void OnPopJobOptions (wxCommandEvent &event);
	void OnPopJobDest (wxCommandEvent &event);
	void OnPopGetDataBack (wxCommandEvent &event);
	void OnPopCreateNew (wxCommandEvent &event);
	void OnPopOpenArchive (wxCommandEvent &event);
	void OnPopShowLogs (wxCommandEvent &event);
	void OnPopShowReports (wxCommandEvent &event);
	void OnPopExportLogsXML (wxCommandEvent &event);
	void OnPopExportLogsTXT (wxCommandEvent &event);
	void OnPopExportReportsXML (wxCommandEvent &event);
	void OnPopExportReportsTXT (wxCommandEvent &event);
	GmListWindow *		m_pParent;
private:
	DECLARE_DYNAMIC_CLASS (GmListCtrl)
	DECLARE_EVENT_TABLE ()
	DECLARE_NO_COPY_CLASS(GmListCtrl)
};

GmListCtrl::GmListCtrl (GmListWindow* parent
						, wxWindowID id
						, const wxPoint& pos
						, const wxSize& size
						, long style
						, const wxValidator& validator
						, const wxString& name)
						: wxListCtrl (parent, id, pos, size, style, validator, name)
						, m_pParent (parent)
{
	assert (m_pParent);
}

static inline void ClearListItemData (JobListItemData * pData)
{
	if (pData == 0) return;
	if (pData->pRootCreator)
		delete pData->pRootCreator;
	if (pData->pMonitor != 0)
		delete pData->pMonitor;
	delete pData;
}

GmListCtrl::~GmListCtrl ()
{
	int count = GetItemCount ();
	for (int index = 0; index < count; ++index) {
		JobListItemData * pItemData = (JobListItemData*)GetItemData (index);
		ClearListItemData (pItemData);
	}
}

IMPLEMENT_DYNAMIC_CLASS (GmListCtrl, wxListCtrl)
BEGIN_EVENT_TABLE(GmListCtrl, wxListCtrl)
	EVT_SCROLLWIN (GmListCtrl::OnScrollList)
	EVT_SIZE (GmListCtrl::OnListSize)
	EVT_LIST_COL_BEGIN_DRAG (wxID_ANY, GmListCtrl::OnDragColumn)
	EVT_LIST_COL_DRAGGING (wxID_ANY, GmListCtrl::OnDragColumn)
	EVT_LIST_COL_END_DRAG (wxID_ANY, GmListCtrl::OnDragColumn)
	EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, GmListCtrl::OnRightClickListItem) 
	EVT_RIGHT_UP(GmListCtrl::OnRightClickList)
	// POP UP EVENT HANDLER.
	EVT_MENU (IDC_POP_RUN_JOB, GmListCtrl::OnPopRunJob)
	EVT_MENU (IDC_POP_STOP_JOB, GmListCtrl::OnPopStopJob)
	EVT_MENU (IDC_POP_RESUME_JOB, GmListCtrl::OnPopResumeJob)
	EVT_MENU (IDC_POP_DELETE_JOB, GmListCtrl::OnPopDeleteJob)
	EVT_MENU (IDC_POP_PAUSE_JOB, GmListCtrl::OnPauseJob)
	EVT_MENU (IDC_POP_JOB_OPTION, GmListCtrl::OnPopJobOptions)
	EVT_MENU (IDC_POP_OPEN_DEST, GmListCtrl::OnPopJobDest)
	EVT_MENU (IDC_POP_GET_DATA_BACK, GmListCtrl::OnPopGetDataBack)
	EVT_MENU (IDC_POP_CREATE_NEW_JOB, GmListCtrl::OnPopCreateNew)
	EVT_MENU (IDC_POP_OPEN_ARCHIVE, GmListCtrl::OnPopOpenArchive)
	EVT_MENU (IDC_POP_SHOW_LOGS, GmListCtrl::OnPopShowLogs)
	EVT_MENU (IDC_POP_SHOW_REPORTS, GmListCtrl::OnPopShowReports)
	EVT_MENU (IDC_POP_EXPORT_SELECTED_JOB_LOGS, GmListCtrl::OnPopExportLogsTXT)
	EVT_MENU (IDC_POP_EXPORT_SELECTED_JOB_REPORTS, GmListCtrl::OnPopExportReportsTXT)
END_EVENT_TABLE()

void GmListCtrl::OnMouseLeftDown (wxMouseEvent &event)
{
	wxGauge * pGauge = (wxGauge*)event.GetEventObject ();
	int count = GetItemCount ();
	for (int index = 0; index < count; ++index) {
		JobListItemData * pData = (JobListItemData *)GetItemData (index);
		if (pData != 0 && pData->pGauge == pGauge) {
			SetItemState (index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			break;
		}
	}

	event.Skip ();
}

void GmListCtrl::OnListSize (wxSizeEvent &event)
{
	AdjustGaugePos ();
}


void GmListCtrl::OnPopDeleteJob (wxCommandEvent & event)
{
	m_pParent->OnDeleteJob ();
}


void GmListCtrl::OnPopRunJob (wxCommandEvent &event)
{
	m_pParent->RunSelectedJob ();
}

void GmListCtrl::OnPopStopJob (wxCommandEvent &event)
{
	m_pParent->OnStopJob ();
}

void GmListCtrl::OnPopResumeJob (wxCommandEvent &event)
{
	m_pParent->OnPauseResume ();
}

void GmListCtrl::OnPauseJob (wxCommandEvent &event)
{
	m_pParent->OnPauseResume (event);
}

void GmListCtrl::OnPopJobOptions (wxCommandEvent &event)
{
	m_pParent->SetSelectedJobOptions ();
}

void GmListCtrl::OnPopJobDest (wxCommandEvent &event)
{
	m_pParent->OnJobDest ();
}

void GmListCtrl::OnPopGetDataBack (wxCommandEvent &event)
{
	m_pParent->GetDataBack ();
}

void GmListCtrl::OnPopCreateNew (wxCommandEvent &event)
{
	m_pParent->OnCreateNew ();
}

void GmListCtrl::OnPopOpenArchive (wxCommandEvent &event)
{
	m_pParent->GetDataBack ();
}

void GmListCtrl::OnPopShowLogs (wxCommandEvent &event)
{
	m_pParent->OnShowLogs ();
}

void GmListCtrl::OnPopShowReports (wxCommandEvent &event)
{
	m_pParent->OnShowReports ();
}

void GmListCtrl::OnPopExportLogsTXT (wxCommandEvent &event)
{
	m_pParent->OnExportLogsTXT ();
}

void GmListCtrl::OnPopExportReportsTXT (wxCommandEvent &event)
{
	m_pParent->OnExportReportsTXT ();
}


void GmListCtrl::OnRightClickListItem (wxListEvent &event)
{
	long index = event.GetIndex ();
	if (index >= 0) {
		wxImage image (log_xpm);
		image.SetMaskColour (0, 0, 0);
		image = image.Scale (16, 16);
		wxBitmap logbitmap (image);

		wxImage rimage (report_xpm);
		rimage.SetMaskColour (0, 0, 0);
		rimage = rimage.Scale (16, 16);
		wxBitmap reportbitmap (rimage);

		wxMenu popup;
		SetItemState (index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		wxListItem item;
		item.SetId (index);
		item.SetMask (wxLIST_MASK_IMAGE);
		if (GetItem (item)) {
			int image = item.GetImage ();
			if (image == LII_NORMAL) {
				popup.Append (IDC_POP_RUN_JOB, _("IDS_RUN_JOB"));
				popup.Append (IDC_POP_DELETE_JOB, _("IDS_DELETE"));
				popup.Append (IDC_POP_JOB_OPTION, _("IDS_OPTIONS_MENU_ITEM"));
			}
			else if (image == LII_PAUSED) {
				popup.Append (IDC_POP_STOP_JOB, _("IDS_STOP"));
				popup.Append (IDC_POP_RESUME_JOB, _("IDS_RESUME"));
			}
			else if (image == LII_RUNNING) {
				popup.Append (IDC_POP_STOP_JOB, _("IDS_STOP"));
				popup.Append (IDC_POP_PAUSE_JOB, _("IDS_PAUSE"));
			}
			else assert (false);
			popup.Append (IDC_POP_OPEN_DEST, _("IDS_POP_OPEN_DEST"));
			popup.AppendSeparator ();
			popup.Append (IDC_POP_GET_DATA_BACK, _("IDS_GET_DATA_BACK"));
			// REPORT AND LOGS
			wxMenuItem * pItem = popup.Append (IDC_POP_SHOW_LOGS, _("IDS_POP_SHOW_LOGS"));
			pItem->SetBitmap (logbitmap);
			pItem = popup.Append (IDC_POP_SHOW_REPORTS, _("IDS_POP_SHOW_REPORTS"));
			pItem->SetBitmap (reportbitmap);
			pItem = popup.Append (IDC_POP_EXPORT_SELECTED_JOB_LOGS, _("IDS_POP_EXPORT_LOGS"));
			pItem->SetBitmap (logbitmap);
			pItem = popup.Append (IDC_POP_EXPORT_SELECTED_JOB_REPORTS, _("IDS_POP_EXPORT_REPORTS"));
			pItem->SetBitmap (reportbitmap);
		}

		popup.AppendSeparator ();
		popup.Append (IDC_POP_CREATE_NEW_JOB, _("IDS_CREATE_NEW_ARCH"));
		popup.Append (IDC_POP_OPEN_ARCHIVE, _("IDS_OPEN_ARCH"));
		PopupMenu (&popup);
	}

	event.Skip ();
}

void GmListCtrl::OnRightClickList (wxMouseEvent & event)
{
	wxMenu popup;
	popup.AppendSeparator ();
	popup.Append (IDC_POP_CREATE_NEW_JOB, _("IDS_CREATE_NEW_ARCH"));
	popup.Append (IDC_POP_OPEN_ARCHIVE, _("IDS_OPEN_ARCH"));
	PopupMenu (&popup);
	event.Skip ();
}

void GmListCtrl::OnDragColumn (wxListEvent &event)
{
	AdjustGaugePos ();
	event.Skip ();
}

void GmListCtrl::AdjustGaugePos ()
{
	int count = GetItemCount ();
	for (int index = 0; index < count; ++index) {
		JobListItemData * pItemData = (JobListItemData*)GetItemData (index);
		wxGauge * pGauge = pItemData->pGauge;
		if (pGauge == 0)
			continue;

		wxRect rect;
		bool result = GetItemRect(index, rect);
		if (!result)
			continue;

		wxPoint point;
		result = GetItemPosition(index, point);
		if (!result)
			continue;

		int x = rect.x;
		for (int i = 0; i < GAUGE_POS; ++i)
			x += GetColumnWidth (i);

		pGauge->Move (x, point.y);
		pGauge->SetSize (wxSize (GetColumnWidth (GAUGE_POS) - 2, rect.GetHeight () - 2));
	}
}

void GmListCtrl::OnScrollList (wxScrollWinEvent & event)
{
	int EventType = event.GetEventType ();
	if (EventType != wxEVT_SCROLLWIN_THUMBTRACK && EventType != wxEVT_SCROLLWIN_THUMBRELEASE) {
		auto_ptr<wxEvent> evt (event.Clone ());
		evt->SetEventType (wxEVT_SCROLLWIN_THUMBTRACK);
		AddPendingEvent (*evt.get ());
		event.Skip ();
	}
	else {
		AdjustGaugePos ();
		event.Skip ();
	}
}

///////////////////////////////////////////////////////////////////////////////////

typedef void (wxEvtHandler::*GmJobEventFunction)(GmJobEvent&);

#define EVT_JOB(evtid,fn)																	\
	wx__DECLARE_EVT0 (evtid,(wxObjectEventFunction) (wxEventFunction) (GmJobEventFunction)	\
												wxStaticCastEvent(GmJobEventFunction, & fn))

BEGIN_EVENT_TABLE(GmListWindow, wxPanel)
	EVT_BUTTON(IDC_STOP_JOB, GmListWindow::OnStopJob)
	EVT_BUTTON(IDC_PAUSE_RESUME, GmListWindow::OnPauseResume)
	EVT_BUTTON(IDC_CLEAR_PAGE_CONTENT, GmListWindow::OnClearPageContent)
	///JOB EVENT HANDLER.
	EVT_JOB(GmEventHandleFile, GmListWindow::OnHandleFile)
	EVT_JOB(GmEventHandleDir, GmListWindow::OnHandleDir)
	EVT_JOB(GmEventHandleSize, GmListWindow::OnHandleSize)
	EVT_JOB(GmEventHandleMessage, GmListWindow::OnHandleMessage)
	EVT_JOB(GmEventHandleBeginAnalysis, GmListWindow::OnHandleBeginAnalysis)
	EVT_JOB(GmEventHandleEndAnalysis, GmListWindow::OnHandleEndAnalysis)
	EVT_JOB(GmEventHandleError, GmListWindow::OnHandleError)
	EVT_JOB(GmEventHandleWarn, GmListWindow::OnHandleWarn)
	EVT_JOB(GmEventHandleBegin, GmListWindow::OnHandleBegin)
	EVT_JOB(GmEventHandleEnd, GmListWindow::OnHandleEnd)
	EVT_JOB(GmEventHandleOverwrite1, GmListWindow::OnHandleOverwrite1)
	EVT_JOB(GmEventHandleOverwrite2, GmListWindow::OnHandleOverwrite2)
	EVT_JOB(GmEventHandleManualStop, GmListWindow::OnHandleManualStop)
	EVT_JOB(GmEventHandleDirectoryInfo, GmListWindow::OnHandleDirectoryInfo)
	EVT_TIMER(IDC_PULSE_TIMER, GmListWindow::OnHandleTimerEvent)
	EVT_LIST_ITEM_SELECTED (IDC_JOB_LIST_ID, GmListWindow::OnHandleItemSelectionChanged)
	EVT_LIST_ITEM_DESELECTED (IDC_JOB_LIST_ID, GmListWindow::OnHandleItemSelectionChanged)
	EVT_LIST_ITEM_ACTIVATED (IDC_JOB_LIST_ID,  GmListWindow::OnDBClickItem)
END_EVENT_TABLE()

GmListWindow::GmListWindow (GmAppFrame * m_pMainFrame, wxSplitterWindow * pParent)
							: wxPanel (pParent, wxID_ANY)
							, m_pMainFrame (m_pMainFrame)
							, m_ImageList (16, 16)
							, m_timer (this, IDC_PULSE_TIMER)
{
	m_timer.Start (2000);
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	m_pList = new GmListCtrl (this, IDC_JOB_LIST_ID, wxDefaultPosition, wxDefaultSize
								, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_SINGLE_SEL);

	m_pList->InsertColumn (0, _("IDS_LIST_NAME"), wxLIST_FORMAT_LEFT);
	m_pList->InsertColumn (1, _("IDS_LIST_CREATE_TIME"), wxLIST_FORMAT_LEFT, 90);
	m_pList->InsertColumn (2, _("IDS_LIST_PROGRESS"), wxLIST_FORMAT_LEFT, 150);
	m_pList->InsertColumn (3, _("IDS_DEST_PATH"), wxLIST_FORMAT_LEFT, 110);
	m_pList->InsertColumn (4, _("IDS_LIST_DESCRIPTION"), wxLIST_FORMAT_LEFT, 170);
	m_pList->InsertColumn (5, _("IDS_OUT_FORMAT"), wxLIST_FORMAT_LEFT, 80);
	m_pList->InsertColumn (6, _("IDS_LIST_NEXTTIME"), wxLIST_FORMAT_LEFT, 150);

	m_ImageList.Add (wxBitmap (normal_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (abnormal_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (paused_xpm), wxColor (0, 0, 0));
	m_ImageList.Add (wxBitmap (running_xpm), wxColor (0, 0, 0));
	m_pList->SetImageList (&m_ImageList, wxIMAGE_LIST_SMALL);

	try {
		GetJobsFromJobFile (GetJobContainerFile (), m_vpJobs);
	} catch (GmException &e) {
		wxString message = e.GetMessage ();
		wxString ErrMessage = wxString::Format (_("IDS_LOAD_JOB_ITEM_ERROR"), message.c_str ());
		wxMessageBox (ErrMessage, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
	}

	for (size_t index = 0; index < m_vpJobs.size (); ++index) {
		AddJobItem (m_vpJobs[index]);
	}

	wxStaticBoxSizer * pStaticBoxSizer = new wxStaticBoxSizer (wxHORIZONTAL, this);
	pStaticBoxSizer->Add (new wxButton (this, IDC_STOP_JOB,  _("IDS_STOP")
						, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 0, wxRIGHT, 20);
	pStaticBoxSizer->Add (new wxButton (this, IDC_PAUSE_RESUME, _("IDS_PAUSE")
						, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 0, wxRIGHT, 20);
	pStaticBoxSizer->Add (new wxButton (this, IDC_CLEAR_PAGE_CONTENT, _("IDS_CLEAR_PAGE_CONTENT")
						, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 0, wxRIGHT, 20);
	m_JobText = new wxStaticText (this, wxID_ANY, wxT (""));
	pStaticBoxSizer->Add (m_JobText, 1, wxEXPAND | wxALIGN_CENTER);
	FindWindow (IDC_STOP_JOB)->Enable (false);
	FindWindow (IDC_PAUSE_RESUME)->Enable (false);

	pBoxSizer->Add (m_pList, 1, wxEXPAND);
	pBoxSizer->Add (pStaticBoxSizer, 0, wxEXPAND | wxALL, 1);
	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
}

void GmListWindow::RunSelectedJob ()
{
	wxListItem item;
	item.SetMask (wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
	if (!GetSelectedItem (item)) {
		wxMessageBox (_("IDS_NO_SELECTED_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	int image = item.GetImage ();
	JobListItemData * pItemData = (JobListItemData*) (item.GetData ());
	if (image == LII_NORMAL || image == LII_ABNORMAL) {
		StartTheJob (pItemData->pJob, pItemData);
	}
	else {
		PauseOrResumeJob (item);
	}

	return;
}

void GmListWindow::RunNamedJob (const wxString & name, bool bShowError)
{
	int index = FindItem (name);
	if (index < 0) {
		if (bShowError) {
			wxString message = wxString::Format (_("IDS_NO_NAMED_JOB"), name.c_str ());
			wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		}
		return;
	}

	if (m_pList->SetItemState (index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED))
		RunSelectedJob ();
}

void GmListWindow::RunShutdownJobs ()
{
	int count = m_pList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		item.SetId (index);
		if (!m_pList->GetItem (item)) continue;
		JobListItemData * pData = (JobListItemData *)item.GetData ();
		if (pData->pJob != 0 && pData->pJob->RunShutdown ())
			RunNamedJob (pData->pJob->Name (), false);
	}

	GmAppFrame::SetShutdownWhenJobsFinishedFlag (true);
}

void GmListWindow::RunStartJobs ()
{
	int count = m_pList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		item.SetId (index);
		if (!m_pList->GetItem (item)) continue;
		JobListItemData * pData = (JobListItemData *)item.GetData ();
		if (pData->pJob != 0 && pData->pJob->RunStart ())
			RunNamedJob (pData->pJob->Name (), false);
	}
}

bool GmListWindow::HasJobsRunStart ()
{
	int count = m_pList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		item.SetId (index);
		if (!m_pList->GetItem (item)) continue;
		JobListItemData * pData = (JobListItemData *)item.GetData ();
		if (pData->pJob != 0 && pData->pJob->RunStart ())
			return true;
	}

	return false;
}

bool GmListWindow::HasJobsRunShutDown ()
{
	int count = m_pList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		item.SetId (index);
		if (!m_pList->GetItem (item)) continue;
		JobListItemData * pData = (JobListItemData *)item.GetData ();
		if (pData->pJob != 0 && pData->pJob->RunShutdown ())
			return true;
	}

	return false;
}

void GmListWindow::SetSelectedJobOptions ()
{
	wxListItem item;
	item.SetMask (wxLIST_MASK_DATA);
	if (!GetSelectedItem (item)) {
		wxMessageBox (_("IDS_NO_SELECTED_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	JobListItemData * pItemData = (JobListItemData*) (item.GetData ());
	if (pItemData->pJob == 0 || item.GetImage () == LII_PAUSED || item.GetImage () == LII_RUNNING) {
		wxMessageBox (_("IDS_JOB_IS_RUNNING"), _("IDS_ERROR_INFO"), wxOK | wxICON_INFORMATION);
		return;
	}

	GmJobOptionConfDlg optdlg (this, _("IDS_CONFIG_JOB"), pItemData->pJob);
	optdlg.CenterOnScreen ();
	if (optdlg.ShowModal () == wxID_OK) {
		*pItemData->pJob = optdlg.GetJobItem ();
	}
}

void GmListWindow::PauseOrResumeJob (wxListItem & item)
{
	EnableStopPause (true);
	int image = item.GetImage ();
	JobListItemData * pItemData = (JobListItemData*) (item.GetData ());
	if (image == LII_PAUSED) {
		if (pItemData->pThread != 0 && pItemData->pThread->IsPaused ()) {
			pItemData->pThread->Resume ();
			item.SetColumn (0);
			item.SetMask (wxLIST_MASK_IMAGE);
			item.SetImage (LII_RUNNING);
			m_pList->SetItem (item);
			FindWindow (IDC_PAUSE_RESUME)->SetLabel (_("IDS_PAUSE"));
			//
			// change tool bar image status;
			//
		}
	}
	else if (image == LII_RUNNING) {
		if (pItemData->pThread != 0 && pItemData->pThread->IsRunning ()) {
			pItemData->pThread->Pause ();
			item.SetColumn (0);
			item.SetMask (wxLIST_MASK_IMAGE);
			item.SetImage (LII_PAUSED);
			m_pList->SetItem (item);
			FindWindow (IDC_PAUSE_RESUME)->SetLabel (_("IDS_RESUME"));
			//
			// change tool bar image status;
			//
		}
	}

	return;
}

JobListItemData * GmListWindow::AddJobItem (GmJobItem * pJob, bool push /* = false */)
{
	int count = m_pList->GetItemCount ();
	m_pList->InsertItem (count, _(""), LII_NORMAL);

	wxListItem item;
	item.SetId (count);

	item.SetColumn (0);
	item.SetMask (wxLIST_MASK_TEXT);
	item.SetText (pJob->Name ());
	m_pList->SetItem (item);

	item.SetMask (wxLIST_MASK_TEXT);
	item.SetColumn (1);
	item.SetText (wxDateTime ((time_t)pJob->GetTime ()).Format ());
	m_pList->SetItem (item);

	wxRect rect;
	m_pList->GetItemRect(count, rect);
	int x = 0;
	for (int i = 0; i < GAUGE_POS; ++i)
		x += m_pList->GetColumnWidth (i);

	wxGauge * pGauge = new wxGauge (m_pList
									, wxID_ANY
									, 100
									, wxPoint (x, rect.y)
									, wxSize (m_pList->GetColumnWidth (GAUGE_POS) - 2
									, rect.GetHeight () - 2));
	wxStaticText * percent = new GmTransStaticText (pGauge, wxID_ANY, wxT ("0 %"));
	pGauge->Connect (wxEVT_LEFT_DOWN, (wxObjectEventFunction)&GmListCtrl::OnMouseLeftDown, (wxObject*)0, m_pList);
	percent->CenterOnParent ();
	percent->SetBackgroundColour (pGauge->GetBackgroundColour ());
	JobListItemData * pItemData = new JobListItemData;
	pItemData->pGauge = pGauge;
	pItemData->pJob = pJob;
	pItemData->pPercent = percent;
	m_pList->SetItemPtrData (count, (wxUIntPtr)pItemData);

	item.SetColumn (3);
	item.SetText (pJob->DestPath ());
	m_pList->SetItem (item);

	item.SetColumn (4);
	if (!pJob->DescStr ().IsEmpty ())
		item.SetText (pJob->DescStr ());
	else
		item.SetText (wxT ("(No Desc)"));

	m_pList->SetItem (item);

	item.SetColumn (5);
	int Type = pJob->GetJobType ();
	if (Type == GO_TYPE_SYNC) {
		item.SetText (GetSyncTypeStr ());
	}
	else if (Type == GO_TYPE_DUP) {
		item.SetText (GetDupTypeStr ());
	}
	else if (Type == GO_TYPE_ZIP) {
		item.SetText (GetZipTypeStr ());
	}
	else if (Type == GO_TYPE_7Z) {
		item.SetText (Get7zTypeStr ());
	}
	else {
		item.SetText (wxT ("Unknown"));
	}

	m_pList->SetItem (item);

	item.SetColumn (6);
	ubyte8 next = GetNextExecTime (pJob);
	if (next == 0)
		item.SetText (wxT("NONE"));
	else
		item.SetText (wxDateTime (TimeTToWxDateTime (next)).Format ());

	m_pList->SetItem (item);
	if (push) m_vpJobs.push_back (pJob);

	m_pList->SetItemBackgroundColour (count, wxColour (179, 236, 255));
	return pItemData;
}

void GmListWindow::EnableStopPause (bool value)
{
	FindWindow (IDC_STOP_JOB)->Enable (value);
	FindWindow (IDC_PAUSE_RESUME)->Enable (value);
}


GmListWindow::~GmListWindow ()
{
	try {
		AddJobsToJobFile (GetJobContainerFile (), m_vpJobs);
	}
	catch (GmException &e) {
		wxString message = e.GetMessage ();
		wxString ErrMessage = wxString::Format (_("IDS_SAVE_JOB_ITEM_ERROR"), message.c_str ());
		wxMessageBox (ErrMessage, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
	}

	GmAutoClearVector<GmJobItem> acgj (m_vpJobs);
}

void GmListWindow::StartGetDataJob (GmJobItem * pJob
									, JobListItemData * pData
									, const wxString szDestPath
									, int Option
									, const vector<GmStoreEntry*> & items)
{
	int Type = pJob->GetJobType ();
	if (Type == GO_TYPE_SYNC) {
		pData->pMonitor = new GmJobMonitor (this, pData);
		pData->pThread = new GmHandleSyncThread (items, szDestPath, Option, pJob, pData->pMonitor);
	}
	else if (Type == GO_TYPE_DUP) {
		pData->pMonitor = new GmJobMonitor (this, pData);
		pData->pThread = new GmHandleDupThread  (items, szDestPath, Option, pJob, pData->pMonitor);
	}
	else if (Type == GO_TYPE_ZIP) {
		pData->pMonitor = new GmJobMonitor (this, pData);
		pData->pThread = new GmHandleZipThread  (items, szDestPath, Option, pJob, pData->pMonitor);
	}
	else if (Type == GO_TYPE_7Z) {
		pData->pMonitor = new GmJobMonitor (this, pData);
		pData->pThread = new GmHandle7ZThread  (items, szDestPath, Option, pJob, pData->pMonitor);
	}
	else {
		assert (false);
	}

	if (pData->pThread) {
		pData->pThread->Create ();
		pData->pThread->Run ();
		EnableStopPause (true);

		int index = FindItem (pData);
		if (index == -1) return;
		wxListItem item;
		item.SetColumn (0);
		item.SetId (index);
		item.SetMask (wxLIST_MASK_IMAGE);
		item.SetImage (LII_RUNNING);
		m_pList->SetItem (item);
	}
}

void GmListWindow::StartTheJob (GmJobItem * pJobItem, JobListItemData * pItemData)
{
	int Type = pJobItem->GetJobType ();
	if (Type == GO_TYPE_SYNC) {
		pItemData->pMonitor = new GmJobMonitor (this, pItemData);
		pItemData->pThread = new GmHandleSyncThread (pJobItem, pItemData->pMonitor);
	}
	else if (Type == GO_TYPE_DUP) {
		pItemData->pMonitor = new GmJobMonitor (this, pItemData);
		pItemData->pThread = new GmHandleDupThread (pJobItem, pItemData->pMonitor);
	}
	else if (Type == GO_TYPE_ZIP) {
		pItemData->pMonitor = new GmJobMonitor (this, pItemData);
		pItemData->pThread = new GmHandleZipThread (pJobItem, pItemData->pMonitor);
	}
	else if (Type == GO_TYPE_7Z) {
		pItemData->pMonitor = new GmJobMonitor (this, pItemData);
		pItemData->pThread = new GmHandle7ZThread (pJobItem, pItemData->pMonitor);
	}
	else {
		assert (false);
		return;
	}

	if (pItemData->pThread) {
		pItemData->pThread->Create ();
		pItemData->pThread->Run ();
		EnableStopPause (true);

		int index = FindItem (pItemData);
		if (index == -1) return;
		wxListItem item;
		item.SetColumn (0);
		item.SetId (index);
		item.SetMask (wxLIST_MASK_IMAGE);
		item.SetImage (LII_RUNNING);
		m_pList->SetItem (item);
	}
}

void GmListWindow::CreateNewJob (GmJobItem * pNewJob)
{
	JobListItemData * pItemData = AddJobItem (pNewJob, true);
	StartTheJob (pNewJob, pItemData);
	return;
}

void GmListWindow::OnStopJob (wxCommandEvent &event)
{
	OnStopJob ();
}

void GmListWindow::OnPauseResume ()
{
	wxListItem item;
	item.SetMask (wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
	if (!GetSelectedItem (item)) {
		wxMessageBox (_("IDS_NO_SELECTED_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	PauseOrResumeJob (item);
}

void GmListWindow::OnClearPageContent (wxCommandEvent &event)
{
	GmStatisticsWindow * plogwnd = m_pMainFrame->GetStatisticsWindow ();
	wxTextCtrl * pPage = plogwnd->GetCurrentPage ();
	if (pPage == 0) return;

	pPage->Clear ();
}

void GmListWindow::OnPauseResume (wxCommandEvent &event)
{
	OnPauseResume ();
}

void GmListWindow::OnHandleFile (GmJobEvent &event)
{
	//
	// do nothing now, fix me later.
	//
}

void GmListWindow::OnHandleDir (GmJobEvent &event)
{
	//
	// do nothing now, fix me later.
	//
}

void GmListWindow::OnHandleSize (GmJobEvent &event)
{
	ubyte8 value = event.m_InfoLast.Size;
	float percent = (value/(float)event.m_Info.Size) * 100;
	value = percent;
	event.m_pItemData->pGauge->SetValue ((int)value);
	wxString message = wxString::Format (wxT ("%.2f %%"), percent);
	event.m_pItemData->pPercent->SetLabel (wxT (""));
	event.m_pItemData->pPercent->SetLabel (message);
}

void GmListWindow::OnHandleMessage (GmJobEvent &event)
{
	m_pMainFrame->GetStatisticsWindow ()->ChangeToPage (LOG_PAGE); //REPORT_PAGE
	wxTextCtrl * page = m_pMainFrame->GetStatisticsWindow ()->GetLogPage ();
	wxString message = wxString::Format (_("IDS_JOB_MESSAGE")
										, event.m_pItemData->pJob->Name ().c_str ()
										, event.message);
	page->AppendText (message);
	page->AppendText (wxT ("\n"));
}

void GmListWindow::OnHandleBeginAnalysis (GmJobEvent &event)
{
	event.m_pItemData->pGauge->Pulse ();
	event.m_pItemData->pPercent->SetLabel (wxT ("0 %"));
}

void GmListWindow::OnHandleEndAnalysis (GmJobEvent &event)
{
	event.m_pItemData->pGauge->SetValue (0);
	event.m_pItemData->bEndAnalysis = true;
}

void GmListWindow::OnHandleError (GmJobEvent &event)
{
	wxDateTime now;
	now.SetToCurrent ();
	wxString message = wxString::Format (_("IDS_STOP_BY_ERROR")
										, event.m_pItemData->pJob->Name ().c_str ()
										, event.message.c_str ()
										, now.Format ().c_str ());
	m_JobText->SetLabel (message);
	StopJob (event);
}

void GmListWindow::OnHandleWarn (GmJobEvent &event)
{
	//
	// do nothing now, fix me later.
	//
	m_pMainFrame->GetStatisticsWindow ()->ChangeToPage (LOG_PAGE); //REPORT_PAGE
	wxTextCtrl * page = m_pMainFrame->GetStatisticsWindow ()->GetLogPage ();
	wxString message = wxString::Format (_("IDS_JOB_WARN")
										, event.m_pItemData->pJob->Name ().c_str ()
										, event.message);
	page->AppendText (message);
	page->AppendText (wxT ("\n"));
}

void GmListWindow::OnHandleBegin (GmJobEvent &event)
{
	//
	// do nothing now, fix me later.
	//
	m_pMainFrame->GetStatisticsWindow ()->ChangeToPage (LOG_PAGE); //REPORT_PAGE
	wxTextCtrl * page = m_pMainFrame->GetStatisticsWindow ()->GetLogPage ();
	wxString message = wxString::Format (_("IDS_BEGIN_JOB"), event.m_pItemData->pJob->Name ().c_str ());
	page->AppendText (message);
	page->AppendText (wxT ("\n"));
}

void GmListWindow::StopJob (GmJobEvent &event)
{
	event.m_pItemData->pGauge->SetValue (0);
	event.m_pItemData->pPercent->SetLabel (wxT ("0 %"));

	int index = FindItem (event.m_pItemData);
	if (index == -1) goto End;
	JobListItemData * pData = event.m_pItemData;

	event.m_pItemData->bEndAnalysis = false;
	if (pData->pJob == 0) goto End;
	if (pData->pJob->DestPath ().IsEmpty ()) {
		//
		// 临时任务
		//
		RemoveTempItem (index, pData);
	}
	else {
		//
		// 正常任务
		//
		if (pData->pRootCreator) delete pData->pRootCreator;
		pData->pRootCreator = 0;
		if (pData->pMonitor) delete pData->pMonitor;
		pData->pMonitor = 0;

		pData->pThread = 0; // 会在线程结束时，自动删除。
		// pData->pJob 会在 m_vpJobs 这个结构里清除
		// pData->pGauge 会在父窗口中被清除。
		wxListItem item;
		item.SetColumn (0);
		item.SetId (index);
		item.SetMask (wxLIST_MASK_IMAGE);
		item.SetImage (LII_NORMAL);
		m_pList->SetItem (item);
	}

	EnableStopPause (false);
End:
	wxCommandEvent cmd (GmEVT_JOB_FINISHED, wxID_ANY);
	cmd.SetEventObject (this);
	AddPendingEvent (cmd);
}

void GmListWindow::OnHandleEnd (GmJobEvent &event)
{
	wxDateTime now;
	now.SetToCurrent ();
	wxString message = wxString::Format (_("IDS_STOP_NORMALLY")
										, event.m_pItemData->pJob->Name ().c_str ()
										, now.Format ().c_str ());
	event.m_pItemData->pGauge->SetValue (100);
	event.m_pItemData->pPercent->SetLabel (wxT ("100 %"));
	m_JobText->SetLabel (message);
	StopJob (event);
}

void GmListWindow::GetDataBack ()
{
	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	wxString filename;
	JobListItemData * pData;
	bool bIsTempJob = true;
	if (index == -1) {
		//
		// 选择一个文件进行打开，如果选择了一个项目，则打开项目对应的文件。
		//
		wxFileDialog fd (this, _("IDS_MY_DOCS")
						, wxEmptyString
						, wxEmptyString
						, wxT ("UIF(*.uif)|*.uif|ZIP(*.zip)|*.zip")); //|7Z(*.7z)|*.7z|RAR(*.rar)|*.rar
		fd.CenterOnScreen ();
		if (fd.ShowModal () == wxID_CANCEL) return;
		filename = fd.GetPath ();
		pData = CreateTempJob (filename);
		if (pData == 0) return;
	}
	else {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		if (!GetSelectedItem (item)) return;

		pData = (JobListItemData*) (item.GetData ());
		filename = pData->pJob->GetUifName ();
		if (!wxFile::Exists (filename)) {
			wxString message = wxString::Format (_("IDS_NO_UIF_EXIST"), filename.c_str ());
			wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return;
		}

		if (filename.IsEmpty ()) return;
	}

	pData->pRootCreator = new GmRootCreator (filename);
	ShowGetDataBackDialog (pData);
	return;
}

void GmListWindow::GetDataBack (const wxString & szFileName)
{
	if (szFileName.IsEmpty ()) return;
	if (!(wxFileName::FileName (szFileName).IsAbsolute ())) return;
	if (!wxFile::Exists (szFileName)) {
		wxString message = wxString::Format (_("IDS_NO_UIF_EXIST"), szFileName.c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	JobListItemData * pData = CreateTempJob (szFileName);
	if (pData == 0) return;
	pData->pRootCreator = new GmRootCreator (szFileName);
	ShowGetDataBackDialog (pData);
	return;
}

void GmListWindow::ShowGetDataBackDialog (JobListItemData * pData)
{
	GmRestoreDialog dialog (this, _("IDS_SELECT_ITEM"));
	assert (pData->pRootCreator);
	pData->szDestPath.Clear ();
	dialog.ShowTree (pData->pRootCreator);
	dialog.CentreOnScreen ();
	if (dialog.ShowModal () == wxID_OK) {
		//
		// 启动恢复任务，完成后判断是临时项，则删除这个临时项，其他只是删除树结点创建器。
		//
		pData->szDestPath = dialog.GetDestPath ();
		int Option = dialog.GetRestoreOption ();
		vector<GmStoreEntry*> items;
		dialog.GetSelectedItems (items);
		StartGetDataJob (pData->pJob, pData, pData->szDestPath, Option, items);
	}
	else {
		//
		// 删除临时项
		//
		int index = FindItem (pData);
		if (index == -1) return;
		if (pData->pRootCreator) delete pData->pRootCreator;
		pData->pRootCreator = 0;
		if (pData->pJob->IsTempJob ()) RemoveTempItem (index, pData);
	}

	return;
}

void GmListWindow::RemoveTempItem (int index, JobListItemData * pData)
{
	m_pList->DeleteItem (index);
	pData->pGauge->Disconnect (wxEVT_LEFT_DOWN, (wxObjectEventFunction)&GmListCtrl::OnMouseLeftDown, (wxObject*)0, m_pList);
	pData->pGauge->GetParent ()->RemoveChild (pData->pGauge);
	delete pData->pGauge;
	if (pData->pJob != 0) delete pData->pJob;
	if (pData->pMonitor != 0) delete pData->pMonitor;
	delete pData;
}

int GmListWindow::FindItem (JobListItemData *pData)
{
	int count = m_pList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		item.SetId (index);
		if (!m_pList->GetItem (item)) continue;
		JobListItemData * pData2 = (JobListItemData *)item.GetData ();
		if (pData2 == pData)
			return index;
	}

	return -1;
}

int GmListWindow::FindItem (const wxString & name)
{
	int count = m_pList->GetItemCount ();
	for (int index = 0; index < count; ++index) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		item.SetId (index);
		if (!m_pList->GetItem (item)) continue;
		JobListItemData * pData2 = (JobListItemData *)item.GetData ();
		if (pData2->pJob != 0 && pData2->pJob->Name () == name)
			return index;
	}

	return -1;
}

JobListItemData * GmListWindow::CreateTempJob (const wxString & filename)
{
	int count = m_pList->GetItemCount ();
	for (int i = 0; i < count; ++i) {
		wxListItem item;
		item.SetId (i);
		item.SetMask (wxLIST_MASK_DATA);
		if (m_pList->GetItem (item)) {
			JobListItemData * pItemData = (JobListItemData*) (item.GetData ());
			if (pItemData->pJob->GetUifName ().CmpNoCase (filename) == 0) {

				return pItemData;
			}
		}
	}

	GmJobItem * pJob = CreateDefaultJobItem (_("IDS_TEMP_NAME"), _(""));
	JobListItemData * pData;
	try {
		pJob->SetOption (GetArchiveType (filename));
		//
		// 临时任务不会加到列表对应的 m_vpJobs 中，这样其不会导致下次启动还存留，并且在任务完成后，或者
		// 不执行任务时就把这个任务，连同这个pJob 都删除 。
		// 
		pData = AddJobItem (pJob);
		pJob->SetTempJob (true);
	}
	catch (GmException &e) {
		wxMessageBox (e.GetMessage (), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return 0;
	}

	return pData;
}


void GmListWindow::OnDeleteJob ()
{
	wxListItem item;
	item.SetMask (wxLIST_MASK_DATA);
	if (!GetSelectedItem (item)) return;

	JobListItemData * pData = (JobListItemData *)item.GetData ();
	if (pData) {
		wxGauge * pGauge = pData->pGauge;
		m_pList->DeleteItem (item.GetId ());
		if (pData->pRootCreator) delete pData->pRootCreator;
		pGauge->Disconnect (wxEVT_LEFT_DOWN, (wxObjectEventFunction)&GmListCtrl::OnMouseLeftDown, (wxObject*)0, m_pList);
		pGauge->GetParent ()->RemoveChild (pGauge);
		delete pGauge;
		if (pData->pJob != 0) {
			vector<GmJobItem*>::iterator pos = std::remove (m_vpJobs.begin (), m_vpJobs.end (), pData->pJob);
			m_vpJobs.erase (pos, m_vpJobs.end ());
			delete pData->pJob;
		}

		if (pData->pMonitor != 0) delete pData->pMonitor;
		m_pList->AdjustGaugePos ();
		delete pData;
	}
}

void GmListWindow::OnStopJob ()
{
	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1)
		return;

	JobListItemData * pItemData = (JobListItemData*)m_pList->GetItemData (index);
	if (pItemData == 0)
		return;

	if (pItemData->pMonitor)
		pItemData->pMonitor->SetStop ();

	EnableStopPause (false);
	return;
}

void GmListWindow::OnPauseJob (wxCommandEvent &event)
{
	OnPauseResume (event);
}

void GmListWindow::OnCreateNew ()
{
	GmFileSelectDialog dialog (this, _("IDS_SELECT_ITEM"));
	dialog.CenterOnScreen ();
	if (dialog.ShowModal () == wxID_OK) {
		if (HasSameJob (dialog.GetJobItem ().Name ())) {
			wxMessageBox (_("IDS_HAS_SAME_JOB"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return;
		}

		auto_ptr<GmJobItem> NewJob (CreateDefaultJobItem (wxEmptyString, wxEmptyString));
		GmJobItem * pItem = NewJob.get ();
		*pItem = dialog.GetJobItem ();
		JobListItemData * pItemData = AddJobItem (pItem, true);
		if (dialog.RunAtOnce ()) {
			StartTheJob (pItem, pItemData);
		}

		NewJob.release ();
	}
}

void GmListWindow::OnOpenArchive ()
{
	GetDataBack ();
}

bool GmListWindow::HasRunJob ()
{
	int count = m_pList->GetItemCount ();
	for (int i = 0; i < count; ++i) {
		wxListItem item;
		item.SetId (i);
		item.SetMask (wxLIST_MASK_IMAGE);
		if (m_pList->GetItem (item)) {
			int image = item.GetImage ();
			if (image == LII_PAUSED || image == LII_RUNNING)
				return true;
		}
	}

	return false;
}

void GmListWindow::OnHandleTimerEvent (wxTimerEvent &event)
{
	if (GmAppFrame::GetShutdownJobsFinishedFlag () &&  !HasRunJob ()) {
		wxShutdown (wxSHUTDOWN_POWEROFF);
		return;
	}

	if (GmAppFrame::GetExitWhenJobsFinishedFlag () && !HasRunJob ()) {
		m_pMainFrame->Destroy ();
		return;
	}

	int count = m_pList->GetItemCount ();
	for (int i = 0; i < count; ++i) {
		wxListItem item;
		item.SetId (i);
		item.SetMask (wxLIST_MASK_DATA | wxLIST_MASK_IMAGE);
		if (m_pList->GetItem (item)) {
			JobListItemData * pItemData = (JobListItemData*) (item.GetData ());
			if (pItemData->pMonitor != 0 && !(pItemData->bEndAnalysis) && item.GetImage () != LII_PAUSED) {
				pItemData->pGauge->Pulse ();
				pItemData->pPercent->SetLabel (wxT ("0 %"));
			}

			if (pItemData->pMonitor == 0) {
				//
				// 任务未执行。
				//
				GmJobItem * pItem = (GmJobItem *)pItemData->pJob;
				if (pItem == 0) continue;
				if (IsThisJobAtScheduleTime (pItem)) {
					RunNamedJob (pItem->Name (), false);
				}
			}
		}
	}

	return;
}

void GmListWindow::OnHandleItemSelectionChanged (wxListEvent &event)
{
	wxCommandEvent cmd (GmEVT_JOBITEM_CHANGED, wxID_ANY);
	event.SetEventObject (this);
	AddPendingEvent (cmd);

	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1) {
		EnableStopPause (false);
		return;
	}

	wxListItem item;
	item.SetMask (wxLIST_MASK_IMAGE);
	bool bEnable = false;
	if (GetSelectedItem (item)) {
		bEnable = true;
		if (item.GetImage () == LII_PAUSED) {
			FindWindow (IDC_PAUSE_RESUME)->SetLabel (_("IDS_RESUME"));
		}
		else if (item.GetImage () == LII_RUNNING) {
			FindWindow (IDC_PAUSE_RESUME)->SetLabel (_("IDS_PAUSE"));
		}
		else bEnable = false;
	}
	else {
		bEnable = false;
	}

	EnableStopPause (bEnable);
	return;
}

void GmListWindow::OnDBClickItem (wxListEvent &event)
{
	SetSelectedJobOptions ();
}

void GmListWindow::OnHandleOverwrite1 (GmJobEvent &event)
{
	GmOverwriteDlg::OverwriteBlob & blob = *static_cast<GmOverwriteDlg::OverwriteBlob*> (event.m_EventData);
	GmOverwriteDlg dlg (this, blob.m_FileName, blob.m_New, blob.m_Old, _("IDS_REPLACE_FILE_DLG_TITLE"));
	dlg.ShowModal ();

	blob.m_RetCode = dlg.GetReturnCode ();
	blob.m_IsOK = true;
}

void GmListWindow::OnHandleOverwrite2 (GmJobEvent &event)
{
}

void GmListWindow::OnHandleManualStop (GmJobEvent &event)
{
	wxDateTime now;
	now.SetToCurrent ();
	wxString message = wxString::Format (_("IDS_STOP_MANUALLY")
										, event.m_pItemData->pJob->Name ().c_str ()
										, now.Format ().c_str ());
	m_JobText->SetLabel (message);
	StopJob (event);
}

void GmListWindow::OnHandleDirectoryInfo (GmJobEvent &event)
{
	wxString op = event.m_pItemData->pRootCreator == 0 ? _("IDS_STORE_DATA_TO_TYPE") : _("IDS_GET_DATA_BACK_TYPE");
	wxString dest = event.m_pItemData->pRootCreator == 0 ? event.m_pItemData->pJob->DestPath ()
														: event.m_pItemData->szDestPath;
	wxString message = wxString::Format (_("IDS_JOB_STATISTICS")
										, event.m_pItemData->pJob->Name ().c_str ()
										, op.c_str (), dest.c_str ()
										, event.m_Info.Dirs
										, event.m_Info.Files
										, MakeCapacityString (event.m_Info.Size).c_str ());
	GmStatisticsWindow * plogwnd = m_pMainFrame->GetStatisticsWindow ();
	plogwnd->ChangeToPage (LOG_PAGE); //REPORT_PAGE
	wxTextCtrl * pPage = plogwnd->GetLogPage ();
	pPage->AppendText (message);
	pPage->AppendText (wxT ("\n"));
}

bool GmListWindow::GetSelectedItem (wxListItem & item)
{
	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1) return false;
	item.SetId (index);
	if (!m_pList->GetItem (item)) return false;
	return true;
}

void GmListWindow::OnShowLogs (bool all)
{
	wxString JobName;
	if (!all) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		if (!GetSelectedItem (item)) {
			wxMessageBox (_("IDS_NO_SELECTED_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return;
		}

		JobListItemData * pData = (JobListItemData*) (item.GetData ());
		JobName = pData->pJob->Name ();
	}

	vector<GmLogItem*> logs;
	GmAutoClearVector<GmLogItem> aclogs (logs);
	GetLogsOfJob (JobName, GetLogFile (), logs);

	GmStatisticsWindow * plogwnd = m_pMainFrame->GetStatisticsWindow ();
	plogwnd->ChangeToPage (LOG_PAGE);
	wxTextCtrl * pPage = plogwnd->GetLogPage ();
	pPage->Clear ();

	for (size_t index = 0; index < logs.size (); ++index) {
		GmLogItem * p = logs[index];
		pPage->AppendText (GetLogString (*p));
	}
}

void GmListWindow::OnShowReports (bool all)
{
	wxString JobName;
	if (!all) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		if (!GetSelectedItem (item)) {
			wxMessageBox (_("IDS_NO_SELECTED_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return;
		}

		JobListItemData * pData = (JobListItemData*) (item.GetData ());
		JobName = pData->pJob->Name ();
	}

	vector<GmReportItem*> reports;
	GmAutoClearVector<GmReportItem> acreports (reports);
	GetReportsOfJob (JobName, GetReportFile (), reports);
	GmStatisticsWindow * plogwnd = m_pMainFrame->GetStatisticsWindow ();
	plogwnd->ChangeToPage (REPORT_PAGE);
	wxTextCtrl * pPage = plogwnd->GetReportPage ();
	pPage->Clear ();
	for (size_t index = 0; index < reports.size (); ++index) {
		GmReportItem * p = reports[index];
		wxString message = wxT ("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
		message += wxString::Format (_("IDS_JOB_REPORT_MESSAGE"), p->JobName.c_str (), (int)index);
		message += wxT ("\n");
		message += GetReportString (*p);
		pPage->AppendText (message);
	}
}

void GmListWindow::OnExportLogsTXT (bool all)
{
	wxString files (wxT ("Txt File (*.txt)|*.txt"));
	wxString FileName;
	wxString JobName;
	if (!GetJobNameAndSaveName (files, FileName, JobName, all)) return;
	ExportLogsToTXT (JobName, GetLogFile (), FileName);
}

bool GmListWindow::GetJobNameAndSaveName (const wxString & files
											, wxString & FileName
											, wxString & JobName
											, bool all)
{
	JobName.Clear ();
	if (!all) {
		wxListItem item;
		item.SetMask (wxLIST_MASK_DATA);
		if (!GetSelectedItem (item)) {
			wxMessageBox (_("IDS_NO_SELECTED_ITEM"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return false;
		}

		JobListItemData * pData = (JobListItemData*) (item.GetData ());
		JobName = pData->pJob->Name ();
	}

	wxFileDialog dlg (this, _(""), wxEmptyString, wxEmptyString, files, wxFD_SAVE);
	if (dlg.ShowModal () == wxID_CANCEL) return false;
	FileName = dlg.GetPath ();
	if (FileName.IsEmpty ()) return false;
	return true;
}

void GmListWindow::OnExportReportsTXT (bool all)
{
	wxString files (wxT ("Text File (*.txt)|*.txt"));
	wxString FileName;
	wxString JobName;
	if (!GetJobNameAndSaveName (files, FileName, JobName, all)) return;
	ExportReportsToTXT (JobName, GetReportFile (), FileName);
}

void GmListWindow::OnJobDest ()
{
	wxListItem item;
	item.SetMask (wxLIST_MASK_DATA);
	if (!GetSelectedItem (item)) return;

	JobListItemData * pItemData = (JobListItemData*) (item.GetData ());
	if (pItemData->pJob != 0) {
		wxString dest = AssemblePath (pItemData->pJob->DestPath (), pItemData->pJob->Name ());
		if (!wxDir::Exists (dest)) return;
		wxString explorer = (wxT ("explorer.exe ") + dest);
		wxExecute (explorer);
	}
}

bool GmListWindow::HasSelectedItem ()
{
	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1) return false;
	return true;
}

bool GmListWindow::SelectedIsRun ()
{
	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1) return false;

	wxListItem item;
	item.SetMask (wxLIST_MASK_IMAGE);
	if (!GetSelectedItem (item)) return false;
	return (item.GetImage () == LII_RUNNING || item.GetImage () == LII_PAUSED) ? true : false;
}

bool GmListWindow::SelectedIsPaused ()
{
	long index = m_pList->GetNextItem (-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (index == -1) return false;

	wxListItem item;
	item.SetMask (wxLIST_MASK_IMAGE);
	if (!GetSelectedItem (item)) return false;
	return (item.GetImage () == LII_PAUSED) ? true : false;
}

bool GmListWindow::HasSameJob (const wxString & JobName) const
{
	for (size_t index = 0; index < m_vpJobs.size (); ++index) {
		GmJobItem * pItem = m_vpJobs[index];
		if (pItem->Name () == JobName) return true;
	}

	return false;
}

JobListItemData * GmListWindow::GetJobData (int index)
{
	if (index < 0 || index > m_pList->GetItemCount ())
		return 0;

	return (JobListItemData*)m_pList->GetItemData (index);
}

int GmListWindow::GetTotalJobs ()
{
	return m_pList->GetItemCount ();
}

DEFINE_EVENT_TYPE(GmEVT_JOBITEM_CHANGED);
DEFINE_EVENT_TYPE(GmEVT_JOB_FINISHED);
DEFINE_EVENT_TYPE(GmEVT_SHOW_UIF_FILE_TREE);