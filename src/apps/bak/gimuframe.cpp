//
// App definition.
// Author:yeyouqun@163.com
// 2009-11-15
//

#include <gmpre.h>
#include <wx/treectrl.h>
#include <wx/panel.h>
#include <wx/xml/xml.h>
#include <wx/image.h>
#include <wx/splash.h>
#include <wx/taskbar.h>
#include <wx/listctrl.h>

#include "engineinc.h"
#include "gimuframe.h"
#include "checkseldlg.h"
#include "joblist.h"
#include "jobtree.h"
#include "jobstatistics.h"
#include "checkrestdlg.h"
#include "toolbarwnd.h"
#include "appconfig.h"
#include "regsoft.h"
#include "about.h"
#include "optionsdlg.h"
//
// tool bar
//
#include "res/run.xpm"
#include "res/configure.xpm"
#include "res/create_new.xpm"
#include "res/options.xpm"
#include "res/open_archive.xpm"
#include "res/report.xpm"
#include "res/log.xpm"

#include "gmutil.h"
#include "password.h"
#include <resource.h>
#include "gimuapp.h"
#include "monitorimpl.h"
#include "gimudefs.h"

class GmTaskbarIcon : public wxTaskBarIcon
{
public:
	GmTaskbarIcon (GmAppFrame * parent) : m_parent (parent)
								, wxTaskBarIcon () {}
private:
	enum {
		IDC_TASKBAR_RESTORE_APP = 10001,
		IDC_TASKBAR_EXIT_APP,
	};
	void ShowMain ();
	void OnMenuRestore (wxCommandEvent &event);
	void OnMenuExit (wxCommandEvent &event);
	void OnOneOrDbClick (wxTaskBarIconEvent &event);
	void OnMouseHover (wxTaskBarIconEvent &event);
	GmAppFrame * m_parent;
	virtual wxMenu *CreatePopupMenu();
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(GmTaskbarIcon, wxTaskBarIcon)
    EVT_MENU (IDC_TASKBAR_RESTORE_APP, GmTaskbarIcon::OnMenuRestore)
    EVT_MENU (IDC_TASKBAR_EXIT_APP, GmTaskbarIcon::OnMenuExit)
	EVT_TASKBAR_LEFT_UP (GmTaskbarIcon::OnOneOrDbClick)
	EVT_TASKBAR_LEFT_DCLICK (GmTaskbarIcon::OnOneOrDbClick)
	EVT_TASKBAR_MOVE (GmTaskbarIcon::OnMouseHover)
END_EVENT_TABLE()

void GmTaskbarIcon::OnMouseHover (wxTaskBarIconEvent &event)
{
	if (!m_parent->IsIconized ())
		return;

	GmListWindow * listwnd = m_parent->GetListWindow ();
	wxString message (_("IDS_APP_TITLE"));

	int count = listwnd->GetTotalJobs ();
	wxString gauge;
	for (int i = 0; i < count; ++i) {
		JobListItemData * data = listwnd->GetJobData (i);
		if (data != 0 && data->pMonitor != 0 && data->bEndAnalysis) {
			wxString direction = data->pRootCreator == 0 ? _("IDS_SAVE_JOB_DATA_TO") : _("IDS_GET_JOB_DATA_BACK");
			wxString job = wxString::Format (_("IDS_TASKICON_MESSAGE")
				, data->pJob->Name ().c_str (), direction.c_str (), data->pGauge->GetValue ());
			gauge += job;
			gauge += wxT ("\n");
		}
	}

	if (!gauge.IsEmpty ()) message = gauge;
	SetIcon (m_parent->GetIcon (), message);
}

void GmTaskbarIcon::ShowMain ()
{
	m_parent->Show ();
	m_parent->Maximize ();
	m_parent->RestoreAllWindowNormalSize ();
	RemoveIcon ();
}

void GmTaskbarIcon::OnMenuRestore (wxCommandEvent &event)
{
	if (m_parent->IsIconized () || !m_parent->IsShown ()) {
		ShowMain ();
	}
}

wxMenu * GmTaskbarIcon::CreatePopupMenu()
{
	wxMenu * popup = new wxMenu;
	popup->Append (IDC_TASKBAR_RESTORE_APP, _("IDS_SHOW_MAINFRAME"));
	popup->Append (IDC_TASKBAR_EXIT_APP, _("IDS_EXIT_GIMU"));
	return popup;
}

void GmTaskbarIcon::OnMenuExit (wxCommandEvent &event)
{
	m_parent->Destroy ();
}

void GmTaskbarIcon::OnOneOrDbClick (wxTaskBarIconEvent &event)
{
	if (m_parent->IsIconized () || !m_parent->IsShown ()) {
		ShowMain ();
	}
}

BEGIN_EVENT_TABLE(GmAppFrame, wxFrame)
	EVT_PAINT (GmAppFrame::OnPaint)
	EVT_CLOSE (GmAppFrame::OnCloseGimu)
	EVT_MENU (CMD_CREATE_NEW, GmAppFrame::OnCreateNewFile)
	EVT_MENU (CMD_OPEN_ARCHIVE, GmAppFrame::OnOpenFile)
	EVT_MENU (CMD_RUN_TASK, GmAppFrame::OnRun)
	EVT_MENU (CMD_OPTIONS, GmAppFrame::OnOptions)
	EVT_MENU (CMD_CONFIGURE, GmAppFrame::OnConfigure)
	EVT_MENU (CMD_PROG_CONFIGURE, GmAppFrame::OnConfigureProg)
	EVT_COMMAND  (wxID_ANY, GmEVT_TOOLWND_RPOG_CONF, GmAppFrame::OnConfigureProg)
	EVT_MENU (CMD_REGISTER, GmAppFrame::OnRegister)
	EVT_MENU (CMD_HELP, GmAppFrame::OnHelp)
	EVT_MENU (CMD_ABOUT, GmAppFrame::OnAbout)
	EVT_SIZE (GmAppFrame::OnWndSize)
	EVT_ICONIZE (GmAppFrame::OnWndMinimize) 
	EVT_TIMER(CMD_REGISTER_ONESHOT_TIMER, GmAppFrame::OnRegisterTimer)
	EVT_MENU (CMD_DEFAULT_COPY, GmAppFrame::OnDefaultType)
	EVT_MENU (CMD_DEFAULT_SYNC, GmAppFrame::OnDefaultType)
	EVT_MENU (CMD_DEFAULT_ZIP, GmAppFrame::OnDefaultType)
	EVT_MENU (CMD_DEFAULT_7Z, GmAppFrame::OnDefaultType)
	EVT_MENU (CMD_DEFAULT_LANG_SIMPLIFIED, GmAppFrame::OnDefaultLang)
	EVT_MENU (CMD_DEFAULT_LANG_TRADITIONAL, GmAppFrame::OnDefaultLang)
	EVT_MENU (CMD_DEFAULT_LANG_ENGLISH, GmAppFrame::OnDefaultLang)
	EVT_MENU (CMD_DEFAULT_SNAPMODE_TRUE, GmAppFrame::OnSetDefaultSnapMode)
	EVT_MENU (CMD_DEFAULT_SNAPMODE_FALSE, GmAppFrame::OnSetDefaultSnapMode)
	EVT_MENU (wxID_EXIT, GmAppFrame::OnExitGimu)
	EVT_MENU (CMD_SET_PASSWORD, GmAppFrame::OnSetPassword)
	EVT_MENU (CMD_MINIMIZE_TO_TASKBAR, GmAppFrame::OnMinimizeToTaskTray)
	EVT_MENU (CMD_START_WITH_WINDOWS, GmAppFrame::OnStartWithSystem)
	//task
	EVT_MENU (CMD_RUN_SELECTED_JOB, GmAppFrame::OnRunSelectedJob)
	EVT_MENU (CMD_GET_DATA_BACK, GmAppFrame::OnGetSelectedJobDataBack)
	EVT_MENU (CMD_DELETE_JOB, GmAppFrame::OnDeleteJob)
	EVT_MENU (CMD_OPEN_DEST, GmAppFrame::OnOpenDestPathOfJob)
	EVT_MENU (CMD_SHOW_SELECTED_JOB_LOGS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_SHOW_SELECTED_JOB_REPORTS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_SHOW_ALL_JOB_LOGS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_SHOW_ALL_JOB_REPORTS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_EXPORT_SELECTED_JOB_LOGS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_EXPORT_SELECTED_JOB_REPORTS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_EXPORT_ALL_JOB_LOGS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU (CMD_EXPORT_ALL_JOB_REPORTS, GmAppFrame::OnShowOrExportLogsOrReports)
	EVT_MENU_OPEN (GmAppFrame::OnMenuOpenInit)
	EVT_COMMAND  (wxID_ANY, GmEVT_JOBITEM_CHANGED, GmAppFrame::OnListChanged)
	EVT_COMMAND  (wxID_ANY, GmEVT_JOB_FINISHED, GmAppFrame::OnJobFinished)
	EVT_COMMAND  (wxID_ANY, GmEVT_SHOW_UIF_FILE_TREE, GmAppFrame::OnShowUifFileTree)
	EVT_MOUSE_EVENTS (GmAppFrame::OnMouseEvent)
	EVT_ACTIVATE_APP (GmAppFrame::OnActivate)
	EVT_ACTIVATE (GmAppFrame::OnActivate) 
END_EVENT_TABLE()

GmAppFrame::GmAppFrame (const wxString& title)
			: wxFrame ((wxFrame *) NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
			, m_timer (this, CMD_REGISTER_ONESHOT_TIMER)
			, m_pTaskbarIcon (0)
{
	SetIcon (wxICON (gimu));
	CreateStatusBar(3);
	InitMenu ();
	InitToolBar ();
	InitSplitWindows ();
	Centre ();
}

void GmAppFrame::OnCreateNewFile (wxCommandEvent &event)
{
	GetListWindow ()->OnCreateNew ();
}

void GmAppFrame::OnOpenFile (wxCommandEvent &event)
{
	GetListWindow ()->GetDataBack ();
}

void GmAppFrame::OnRun (wxCommandEvent &event)
{
	GetListWindow ()->RunSelectedJob ();
}

void GmAppFrame::OnOptions (wxCommandEvent &event)
{
	GetListWindow ()->SetSelectedJobOptions ();
}

void GmAppFrame::OnConfigure (wxCommandEvent &event)
{
	GmProgOptionsDlg dlg (this, _("IDS_PROG_CONFIGURE"));
	if (dlg.ShowModal () == wxID_OK) {
		GmAppConfig * pConfig = GmAppConfig::GetInstance ();
		GetTreeWindow ()->SetDefaultType (pConfig->GetDefaultType ());
	}
}

void GmAppFrame::OnConfigureProg (wxCommandEvent &event)
{
	GmProgOptionsDlg dlg (this, _("IDS_PROG_CONFIGURE"));
	if (dlg.ShowModal () == wxID_OK) {
		GmAppConfig * pConfig = GmAppConfig::GetInstance ();
		GetTreeWindow ()->SetDefaultType (pConfig->GetDefaultType ());
	}
}

void GmAppFrame::OnRegister (wxCommandEvent &event)
{
	wxMessageBox (ToWxString (__FUNCTION__));
}

void GmAppFrame::OnHelp (wxCommandEvent &event)
{
	wxString szHelpPath = GmAppInfo::GetAppPath ();
	szHelpPath = AssemblePath (szHelpPath, wxT ("gimu.chm"));
	szHelpPath = EnclosureWithQuotMark (szHelpPath);
	if (!wxFile::Exists (szHelpPath)) return;

	szHelpPath = wxT ("hh.exe ") + szHelpPath;
	wxExecute (szHelpPath);
}

void GmAppFrame::OnAbout (wxCommandEvent &event)
{
	GmAboutDialog dlg (this, _("IDS_ABOUT_GMSOFT"));
	dlg.ShowModal ();
}

void GmAppFrame::InitSplitWindows ()
{
	m_pHoriSplitter = new wxSplitterWindow (this, wxID_ANY);
	m_pLeftWindow = new GmTreeWindow (this, m_pHoriSplitter);
	m_pVertiSplitter = new wxSplitterWindow (m_pHoriSplitter, wxID_ANY);

	m_pUpRightWindow = new GmListWindow (this, m_pVertiSplitter);
	m_pDownRightWindow = new GmStatisticsWindow (this, m_pVertiSplitter);

	m_pLeftWindow->Show(true);
	m_pUpRightWindow->Show(true);
	m_pDownRightWindow->Show(true);

	m_pHoriSplitter->SplitVertically (m_pLeftWindow, m_pVertiSplitter);
	m_pVertiSplitter->SplitHorizontally (m_pUpRightWindow, m_pDownRightWindow);
	m_pHoriSplitter->SetMinimumPaneSize (80);
	m_pHoriSplitter->SetSashGravity (0.12);

	m_pVertiSplitter->SetSashGravity (0.45);
	m_pVertiSplitter->SetMinimumPaneSize (70);
}

void GmAppFrame::InitMenu ()
{
#define ITEM_WIDTH_HEIGHT 16
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	wxMenuBar * pMenuBar = new wxMenuBar ();
	wxMenu * pMenu = new wxMenu ();
	m_ArchiveMenu = pMenu;
	//
	// 文档
	//
	pMenuBar->Append (pMenu, _("IDS_ARCH_MENU"));
	//
	// 主要是设定图像的MenuItem的宽度，这样才能正常显示。
	//
	wxMenuItem * ptempitem = pMenu->Append (-1, wxT ("xxx"));
	ptempitem->SetMarginWidth (ITEM_WIDTH_HEIGHT);
	pMenu->Destroy (ptempitem);
	pMenu->Append (CMD_CREATE_NEW, _("IDS_CREATE_NEW_ARCH"));
	pMenu->Append (CMD_OPEN_ARCHIVE, _("IDS_OPEN_ARCH"));
	pMenu->AppendSeparator ();
	pMenu->Append (wxID_EXIT, _("IDS_EXIT_GIMU"));

	//
	// 设置
	//
	pMenu = new wxMenu ();
	m_pOptionMenu = pMenu;
	pMenuBar->Append (pMenu, _("IDS_CONFIGURE_MENU"));

	pMenu->Append (CMD_PROG_CONFIGURE, _("IDS_PROG_OPTIONS_ITEM"));
	pMenu->Append (CMD_SET_PASSWORD, _("IDS_SET_PASSWORD"));
	pMenu->AppendSeparator ();
	pMenu->AppendCheckItem (CMD_MINIMIZE_TO_TASKBAR, _("IDS_MINIMIZE_TO_TASKBAR"));
	pMenu->AppendCheckItem (CMD_START_WITH_WINDOWS, _("IDS_START_WITH_WINDOWS"));
	pMenu->Check (CMD_MINIMIZE_TO_TASKBAR, pConfig->GetMinimizeToTaskBar ());
	pMenu->Check (CMD_START_WITH_WINDOWS, pConfig->GetStartWithSystem ());
	pMenu->AppendSeparator ();

	wxMenu * pFormatMenu = new wxMenu;
	//
	// 主要是设定Radio 类型的MenuItem的宽度，这样才能正常显示。
	//
	ptempitem = pFormatMenu->Append (-1, wxT ("xxx"));
	ptempitem->SetMarginWidth (ptempitem->GetDefaultMarginWidth ());
	pFormatMenu->Destroy (ptempitem);
	pFormatMenu->AppendRadioItem (CMD_DEFAULT_COPY, GetDupTypeStr ());
	pFormatMenu->AppendRadioItem (CMD_DEFAULT_SYNC, GetSyncTypeStr ());
	pFormatMenu->AppendRadioItem (CMD_DEFAULT_ZIP, GetZipTypeStr ());
	pFormatMenu->AppendRadioItem (CMD_DEFAULT_7Z, Get7zTypeStr ());
	pMenu->AppendSubMenu (pFormatMenu, _("IDS_DEFAULT_JOB_TYPE"));

	wxMenu * pLang = new wxMenu;
	pLang->AppendRadioItem (CMD_DEFAULT_LANG_SIMPLIFIED, _("IDS_SIMPLIFIED_CHINESE"));
	pLang->AppendRadioItem (CMD_DEFAULT_LANG_TRADITIONAL, _("IDS_TRADITIONAL_CHINESE"));
	pLang->AppendRadioItem (CMD_DEFAULT_LANG_ENGLISH, _("IDS_ENGLISH"));
	pMenu->AppendSubMenu (pLang, _("IDS_DEFAULT_LANG"));

	wxMenu * pSnapMode = new wxMenu;
	pSnapMode->AppendRadioItem (CMD_DEFAULT_SNAPMODE_TRUE, _("IDS_DEFAULT_SNAPMODE_TRUE"));
	pSnapMode->AppendRadioItem (CMD_DEFAULT_SNAPMODE_FALSE, _("IDS_DEFAULT_SNAPMODE_FALSE"));
	pMenu->AppendSubMenu (pSnapMode, _("IDS_DEFAULT_SNAPMODE"));

	//
	// 作业
	//
	wxImage image (log_xpm);
	image.SetMaskColour (0, 0, 0);
	image = image.Scale (ITEM_WIDTH_HEIGHT, ITEM_WIDTH_HEIGHT);
	wxBitmap logbitmap (image);

	wxImage rimage (report_xpm);
	rimage.SetMaskColour (0, 0, 0);
	rimage = rimage.Scale (ITEM_WIDTH_HEIGHT, ITEM_WIDTH_HEIGHT);
	wxBitmap reportbitmap (rimage);

	wxImage oa (open_archive_xpm);
	oa.SetMaskColour (0, 0, 0);
	oa = oa.Scale (ITEM_WIDTH_HEIGHT, ITEM_WIDTH_HEIGHT);
	wxBitmap oab (oa);

	pMenu = new wxMenu ();
	m_pJobMenu = pMenu;

	ptempitem = pMenu->Append (-1, wxT ("xxx"));
	ptempitem->SetMarginWidth (ITEM_WIDTH_HEIGHT);
	pMenu->Destroy (ptempitem);

	pMenuBar->Append (pMenu, _("IDS_TASK_MENU"));
	pMenu->Append (CMD_OPTIONS, _("IDS_OPTIONS_MENU_ITEM"));
	pMenu->Append (CMD_RUN_SELECTED_JOB, _("IDS_RUN_JOB"));
	pMenu->Append (CMD_GET_DATA_BACK, _("IDS_GET_DATA_BACK"))->SetBitmap (oab);
	pMenu->Append (CMD_DELETE_JOB, _("IDS_DELETE"));
	pMenu->Append (CMD_OPEN_DEST, _("IDS_POP_OPEN_DEST"));
	pMenu->AppendSeparator ();
	// SHOW LOGS AND REPORTS
	wxMenu * pExp = new wxMenu;
	pExp->Append (CMD_SHOW_SELECTED_JOB_LOGS, _("IDS_JOB_SELECTED"));
	pExp->Append (CMD_SHOW_ALL_JOB_LOGS, _("IDS_ALL_JOBS"));
	wxMenuItem * log = pMenu->AppendSubMenu (pExp, _("IDS_SHOW_LOGS"));
	log->SetBitmap (wxNullBitmap);
	log->SetBitmap (logbitmap);

	pExp = new wxMenu;
	pExp->Append (CMD_SHOW_SELECTED_JOB_REPORTS, _("IDS_JOB_SELECTED"));
	pExp->Append (CMD_SHOW_ALL_JOB_REPORTS, _("IDS_ALL_JOBS"));
	wxMenuItem * report = pMenu->AppendSubMenu (pExp, _("IDS_SHOW_REPORTS"));
	report->SetBitmap (reportbitmap);

	pMenu->AppendSeparator ();
	// EXPORT REPORTS AND LOGS
	pExp = new wxMenu;
	pExp->Append (CMD_EXPORT_SELECTED_JOB_LOGS, _("IDS_EXPORT_JOB_SELECTED"));
	pExp->Append (CMD_EXPORT_ALL_JOB_LOGS, _("IDS_EXPORT_ALL_JOBS"));
	log = pMenu->AppendSubMenu (pExp, _("IDS_MENU_EXPORT_LOGS"));
	log->SetBitmap (logbitmap);

	pExp = new wxMenu;
	pExp->Append (CMD_EXPORT_SELECTED_JOB_REPORTS, _("IDS_EXPORT_JOB_SELECTED"));
	pExp->Append (CMD_EXPORT_ALL_JOB_REPORTS, _("IDS_EXPORT_ALL_JOBS"));
	report = pMenu->AppendSubMenu (pExp, _("IDS_MENU_EXPORT_REPORTS"));
	report->SetBitmap (reportbitmap);
	pExp = new wxMenu;

	//
	// 帮助
	//
	pMenu = new wxMenu;
	m_pHelpMenu = pMenu;
	pMenuBar->Append (pMenu, _("IDS_HELP_MENU"));
	pMenu->Append (CMD_HELP, _("IDS_HELP_MENU_ITEM_HELP"));
	pMenu->Append (CMD_ABOUT, _("IDS_HELP_MENU_ITEM_ABOUT"));

	SetOptionMenus (m_pOptionMenu);
	SetMenuBar (pMenuBar);
}

void GmAppFrame::InitToolBar ()
{
	wxToolBar * pToolBar = new wxToolBar (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT);
	SetToolBar (pToolBar);
#define BITMAP_PIXELS 16
#define TOOL_NUMBER 5
	wxBitmap bitmaps[TOOL_NUMBER] = {wxBitmap (create_new_xpm)
									, wxBitmap (open_archive_xpm)
									, wxBitmap (run_xpm)
									, wxBitmap (options_xpm)
									, wxBitmap (configure_xpm)
									};


	for (int index = 0; index < TOOL_NUMBER; ++index) {
		wxImage image = bitmaps[index].ConvertToImage ();
		image.Scale (BITMAP_PIXELS, BITMAP_PIXELS);
		image.SetMaskColour (0, 0, 0);
		bitmaps[index] = wxBitmap (image);
	}

	pToolBar->AddSeparator ();
	pToolBar->AddTool (CMD_CREATE_NEW, wxT (""), bitmaps[0], _("IDS_CREATE_NEW_TIP"));
	pToolBar->AddTool (CMD_OPEN_ARCHIVE, wxT (""), bitmaps[1], _("IDS_OPEN_ARCH_TIP"));
	pToolBar->AddTool (CMD_RUN_TASK, wxT (""), bitmaps[2], _("IDS_RUN_TIP"));
	pToolBar->AddSeparator ();
	pToolBar->AddTool (CMD_OPTIONS, wxT (""), bitmaps[3], _("IDS_OPTIONS_TIP"));
	pToolBar->AddTool (CMD_CONFIGURE, wxT (""), bitmaps[4], _("IDS_CONFIGURE_TIP"));
	pToolBar->AddSeparator ();
	pToolBar->SetToolBitmapSize (wxSize (BITMAP_PIXELS, BITMAP_PIXELS));

	GmToolBarWnd * pToolWnd = new GmToolBarWnd (pToolBar, IDC_TOOLWND);
	pToolBar->AddControl (pToolWnd);
	pToolBar->Realize ();
}


GmAppFrame::~GmAppFrame (void)
{
	//
	// 先处理自己的事情。
	//
	if (m_pTaskbarIcon) delete m_pTaskbarIcon;
}

void GmAppFrame::OnPaint(wxPaintEvent & event)
{
	wxPaintDC dc(this);
	PrepareDC(dc);
}

GmListWindow * GmAppFrame::GetListWindow ()
{
	return m_pUpRightWindow;
}

GmTreeWindow * GmAppFrame::GetTreeWindow ()
{
	return m_pLeftWindow;
}

GmStatisticsWindow * GmAppFrame::GetStatisticsWindow ()
{
	return m_pDownRightWindow;
}

void GmAppFrame::OnWndSize (wxSizeEvent &event)
{
	event.Skip ();
	wxToolBar * pToolBar = GetToolBar ();
	if (pToolBar == 0) return;
	GmToolBarWnd * pToolWnd = (GmToolBarWnd*)pToolBar->FindControl (IDC_TOOLWND);
	if (pToolWnd == 0) return;

	wxPoint point = pToolWnd->GetPosition ();
	point.y = 2;
	pToolWnd->SetPosition (point);
	wxSize size = event.GetSize ();
	wxSize ts = pToolBar->GetToolSize ();
	int sep = pToolBar->GetToolSeparation ();
	wxSize margins = pToolBar->GetMargins ();
	int xmargin = (pToolBar->GetToolsCount () + 1) * margins.x;
	int x = size.GetWidth () - (ts.GetWidth () * 6);
	int y = ts.GetHeight () - 3;
	pToolWnd->SetSize (x, y);
}

void GmAppFrame::OnRegisterTimer (wxTimerEvent & event)
{
	/*
#if defined(ENTERPRISE_EDITION) || defined(PRO_EDITION)
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	if (!pConfig->CheckSerialNo ()) {
		Show ();
		Maximize ();
		GmRegSoftDialog dlg (this, _("IDS_INPUT_SN"));
		dlg.ShowModal ();
		if (pConfig->CheckSerialNo (dlg.GetSerialNo ())) {
			pConfig->SetSerialNo (dlg.GetSerialNo ());
		}
		else {
			wxMessageBox (_("IDS_SN_INCORRECT"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			Destroy ();
		}
	}
#endif //
	*/
}

void GmAppFrame::StartRegisterTimer ()
{
	m_timer.Start (5000, wxTIMER_ONE_SHOT);
}

void GmAppFrame::OnDefaultType (wxCommandEvent &event)
{
	int id = event.GetId ();
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	wxString type;
	if (CMD_DEFAULT_COPY == id) {
		type = GetDupTypeStr ();
		pConfig->SetDefaultType (type);
	}
	else if (CMD_DEFAULT_SYNC == id) {
		type = GetSyncTypeStr ();
		pConfig->SetDefaultType (type);
	}
	else if (CMD_DEFAULT_ZIP == id) {
		type = GetZipTypeStr ();
		pConfig->SetDefaultType (type);
	}
	else if (CMD_DEFAULT_7Z == id) {
		type = Get7zTypeStr ();
		pConfig->SetDefaultType (type);
	}

	GetTreeWindow ()->SetDefaultType (type);
}

void GmAppFrame::OnDefaultLang (wxCommandEvent &event)
{
	int id = event.GetId ();
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	GmLangID former = pConfig->GetLangID ();
	GmLangID idnow = former;
	if (CMD_DEFAULT_LANG_SIMPLIFIED == id)
		idnow = LANGUAGE_SIMPLIFIED_CHINESE;
	else if (CMD_DEFAULT_LANG_TRADITIONAL == id)
		idnow = LANGUAGE_TRADITIONAL_CHINESE;
	else if (CMD_DEFAULT_LANG_ENGLISH == id)
		idnow = LANGUAGE_ENGLISH;

	if (former == idnow) return;
	pConfig->SetLangID (idnow);
	if (GetListWindow ()->HasRunJob ()) {
		wxMessageBox (_("IDS_JOBS_IS_RUNNING"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	int result = wxMessageBox (_("IDS_RESET_LANG_RELAUNCH_APP"), _("IDS_CONFIRM_PLEASE"), wxYES_NO, this);
	if (result == wxYES) {
		wxString app = GmAppInfo::GetFullAppName ();
		app = EnclosureWithQuotMark (app);
		wxExecute (app);
		Destroy ();
	}

	return;
}

void GmAppFrame::OnSetDefaultSnapMode (wxCommandEvent &event)
{
	int id = event.GetId ();
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	bool mode = false;
	if (id == CMD_DEFAULT_SNAPMODE_TRUE)
		mode = true;
	else if (id == CMD_DEFAULT_SNAPMODE_FALSE)
		mode = false;

	pConfig->SetSnapMode (mode);
}

void GmAppFrame::OnExitGimu (wxCommandEvent &event)
{
	if (GetListWindow ()->HasRunJob ()) {
		wxMessageBox (_("IDS_JOBS_IS_RUNNING"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	Destroy ();
}

void GmAppFrame::OnSetPassword (wxCommandEvent &event)
{
	//
	// fix me.
	//
	GmPasswordDlg dlg (this, _("IDS_ENCRYPTION_PASSWD"));
	if (dlg.ShowModal () == wxID_OK) {
		wxString passwd = dlg.GetPassword ();
		if (!passwd.IsEmpty () && passwd.Length () < 4) {
			wxMessageBox (_("IDS_PASSWORD_LENGTH_IS_NOT_ENOUGH"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return;
		}

		GmPassword::GetInstance ()->WritePassword (passwd);
	}
}

void GmAppFrame::OnMinimizeToTaskTray (wxCommandEvent &event)
{
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	pConfig->SetMinimizeToTaskBar (event.IsChecked ());
}

void GmAppFrame::OnStartWithSystem (wxCommandEvent &event)
{
	wxString app = GmAppInfo::GetFullAppName ();
	app = EnclosureWithQuotMark (app);
	bool bResult;
	if (event.IsChecked ())
		bResult = AddSystemStartItem (app);
	else
		bResult = RemoveSystemStartItem (app);

	if (bResult) {
		GmAppConfig * pConfig = GmAppConfig::GetInstance ();
		pConfig->SetStartWithSystem (event.IsChecked ());
	}
}

void GmAppFrame::OnRunSelectedJob (wxCommandEvent &event)
{
	wxString label = m_pJobMenu->GetLabel (event.GetId ());
	if (label == _("IDS_PAUSE_JOB") || label == _("IDS_RESUME_JOB")) {
		GetListWindow ()->OnPauseResume ();
	}
	else {
		GetListWindow ()->RunSelectedJob ();
	}
}

void GmAppFrame::OnGetSelectedJobDataBack (wxCommandEvent &event)
{
	GetListWindow ()->GetDataBack ();
}

void GmAppFrame::OnDeleteJob (wxCommandEvent &event)
{
	GetListWindow ()->OnDeleteJob ();
}

void GmAppFrame::OnOpenDestPathOfJob (wxCommandEvent &event)
{
	GetListWindow ()->OnJobDest ();
}

void GmAppFrame::OnShowOrExportLogsOrReports (wxCommandEvent &event)
{
	int eventid = event.GetId ();
	GmListWindow * pwnd = GetListWindow ();
	if (eventid == CMD_SHOW_SELECTED_JOB_LOGS)
		pwnd->OnShowLogs ();
	if (eventid == CMD_SHOW_SELECTED_JOB_REPORTS)
		pwnd->OnShowReports ();
	if (eventid == CMD_SHOW_ALL_JOB_LOGS)
		pwnd->OnShowLogs (true);
	if (eventid == CMD_SHOW_ALL_JOB_REPORTS)
		pwnd->OnShowReports (true);
	if (eventid == CMD_EXPORT_SELECTED_JOB_LOGS)
		pwnd->OnExportLogsTXT ();
	if (eventid == CMD_EXPORT_SELECTED_JOB_REPORTS)
		pwnd->OnExportReportsTXT ();
	if (eventid == CMD_EXPORT_ALL_JOB_LOGS)
		pwnd->OnExportLogsTXT (true);
	if (eventid == CMD_EXPORT_ALL_JOB_REPORTS)
		pwnd->OnExportReportsTXT (true);

	return;
}

void GmAppFrame::OnCloseGimu (wxCloseEvent &event)
{
	//
	// 如果是关机事件，需要启动关机启动的任务，但是千万不能千万死循环，因为任务的执行完后，需要
	// 再关系，又再会发送关机消息。
	//
	if (event.GetEventType () == wxEVT_QUERY_END_SESSION &&
		!event.GetLoggingOff ())
	{
		static bool bHasRunShutDownJobs = false;
		if (!bHasRunShutDownJobs) {
			GetListWindow ()->RunShutdownJobs ();
			bHasRunShutDownJobs = true;
			event.Veto ();
			return;
		}

		if (GetListWindow ()->HasJobsRunStart ()) {
			wxString app = GmAppInfo::GetFullAppName ();
			app = EnclosureWithQuotMark (app);
			app += wxT (" -");
			app += RUN_JOBS_WHEN_START;
			app += wxT (" -");
			app += START_MINIMIZE;
			AddSystemStartItem (app);
		}
	}

	Destroy();
	event.Skip ();
}

void GmAppFrame::OnMenuOpenInit (wxMenuEvent& event)
{
	wxMenu * p = event.GetMenu ();
	if (p == m_ArchiveMenu) {
	}
	else if (p == m_pOptionMenu) {
		SetOptionMenus (p);
	}
	else if (p == m_pJobMenu) {
		EnableOrDisableJobMenu (p);
	}
	else if (p == m_pHelpMenu) {
	}

	event.Skip ();
}

void GmAppFrame::SetOptionMenus (wxMenu * pMenu)
{
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();

	pMenu->Check (CMD_MINIMIZE_TO_TASKBAR, pConfig->GetMinimizeToTaskBar ());
	pMenu->Check (CMD_START_WITH_WINDOWS, pConfig->GetStartWithSystem ());

	wxString DefType = pConfig->GetDefaultType ();
	if (DefType == GetDupTypeStr ())
		pMenu->Check (CMD_DEFAULT_COPY, true);
	else if (DefType == GetSyncTypeStr ())
		pMenu->Check (CMD_DEFAULT_SYNC, true);
	else if (DefType == GetZipTypeStr ())
		pMenu->Check (CMD_DEFAULT_ZIP, true);
	else if (DefType == Get7zTypeStr ())
		pMenu->Check (CMD_DEFAULT_7Z, true);

	GmLangID langid = pConfig->GetLangID ();
	if (langid == LANGUAGE_SIMPLIFIED_CHINESE)
		pMenu->Check (CMD_DEFAULT_LANG_SIMPLIFIED, true);
	else if (langid == LANGUAGE_TRADITIONAL_CHINESE)
		pMenu->Check (CMD_DEFAULT_LANG_TRADITIONAL, true);
	else if (langid == LANGUAGE_ENGLISH)
		pMenu->Check (CMD_DEFAULT_LANG_ENGLISH, true);

	if (pConfig->GetSnapMode ())
		pMenu->Check (CMD_DEFAULT_SNAPMODE_TRUE, true);
	else
		pMenu->Check (CMD_DEFAULT_SNAPMODE_FALSE, true);

	return;
}

void GmAppFrame::EnableOrDisableJobMenu (wxMenu * pMenu)
{
	GmListWindow * pwnd = GetListWindow ();
	bool bHasSelected = pwnd->HasSelectedItem ();
	pMenu->Enable (CMD_OPTIONS, bHasSelected);

	if (bHasSelected && pwnd->SelectedIsRun ()) {
		pMenu->Enable (CMD_RUN_SELECTED_JOB, bHasSelected);
		if (pwnd->SelectedIsPaused ()) {
			pMenu->SetLabel (CMD_RUN_SELECTED_JOB, _("IDS_RESUME_JOB"));
			pMenu->Enable (CMD_RUN_SELECTED_JOB, bHasSelected);
		}
		else {
			pMenu->SetLabel (CMD_RUN_SELECTED_JOB, _("IDS_PAUSE_JOB"));
			pMenu->Enable (CMD_RUN_SELECTED_JOB, bHasSelected);
		}
	}
	else {
		pMenu->SetLabel (CMD_RUN_SELECTED_JOB, _("IDS_RUN_JOB"));
		pMenu->Enable (CMD_RUN_SELECTED_JOB, bHasSelected);
	}

	pMenu->Enable (CMD_DELETE_JOB, bHasSelected);
	pMenu->Enable (CMD_OPEN_DEST, bHasSelected);
	pMenu->Enable (CMD_SHOW_SELECTED_JOB_LOGS, bHasSelected);
	pMenu->Enable (CMD_SHOW_SELECTED_JOB_REPORTS, bHasSelected);
	pMenu->Enable (CMD_EXPORT_SELECTED_JOB_LOGS, bHasSelected);
	pMenu->Enable (CMD_EXPORT_SELECTED_JOB_REPORTS, bHasSelected);
}

void GmAppFrame::OnListChanged (wxCommandEvent &event)
{
	wxToolBar * toolbar = GetToolBar ();
	if (toolbar == 0) return;
	GmListWindow * list = GetListWindow ();
	if (list->HasSelectedItem ()) {
		toolbar->EnableTool (CMD_OPTIONS, true);
		toolbar->EnableTool (CMD_RUN_TASK, true);
	}
	else {
		toolbar->EnableTool (CMD_OPTIONS, false);
		toolbar->EnableTool (CMD_RUN_TASK, false);
	}

	return;
}

void GmAppFrame::OnJobFinished (wxCommandEvent&event)
{
	if (GmAppFrame::GetShutdownJobsFinishedFlag () &&  !GetListWindow ()->HasRunJob ()) {
		wxShutdown (wxSHUTDOWN_POWEROFF);
	}

	if (GmAppFrame::GetExitWhenJobsFinishedFlag () && !GetListWindow ()->HasRunJob ()) {
		Destroy ();
	}
}

void GmAppFrame::OnShowUifFileTree (wxCommandEvent&event)
{
	wxString UifFileName = event.GetString ();
	GetListWindow ()->GetDataBack (UifFileName);
}

void GmAppFrame::OnWndMinimize (wxIconizeEvent &event)
{
	event.Skip ();
	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	if (pConfig->GetMinimizeToTaskBar ()) {
		Show (false);
		ToTaskTray ();
	}
}

void GmAppFrame::ToTaskTray ()
{
	if (m_pTaskbarIcon == 0) {
		m_pTaskbarIcon = new GmTaskbarIcon (this);
	}

	m_pTaskbarIcon->SetIcon (GetIcon (), _("IDS_APP_TITLE"));
}

void GmAppFrame::RestoreAllWindowNormalSize ()
{
	wxSize size = GetSize ();
	m_pHoriSplitter->SetSashPosition (size.x/3);
	m_pVertiSplitter->SetSashPosition (size.y/2);
}

void GmAppFrame::OnMouseEvent (wxMouseEvent &event)
{
	event.Skip ();
}

void GmAppFrame::OnActivate (wxActivateEvent&event)
{
	GmAppFrame::SetExitWhenJobsFinishedFlag (false);
}

bool GmAppFrame::ExitWhenJobFinished = false;
bool GmAppFrame::ShutdownWhenJobFinished = false;
