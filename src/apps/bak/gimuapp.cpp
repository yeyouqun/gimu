//
// App definition.
// Author:yeyouqun@163.com
// 2009-11-15
//

#include <gmpre.h>
#include <wx/event.h>
#include <wx/splash.h>
#include <wx/cmdline.h>
#include <wx/xml/xml.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/region.h>
#include <wx/splash.h>
#include "gimusplash.h"
#include "engineinc.h"
#include "gimuapp.h"
#include "appconfig.h"
#include "res/gimu_splash.xpm"
#include "gmutil.h"
#include "optionsdlg.h"
#include "password.h"
#include "joblist.h"
#include "gimuframe.h"
#include "gimudefs.h"

#define GIMU_LANG_MO				wxT("gimu_simplified.mo")
#define ENGIN_LANG_MO				wxT("engine_simplified.mo")

#define GIMU_LANG_ENGLISH_MO		wxT("gimu_english.mo")
#define ENGIN_LANG_ENGLISH_MO		wxT("engine_english.mo")

#define GIMU_LANG_TRADITIONAL_MO	wxT("gimu_traditional.mo")
#define ENGIN_LANG_TRADITIONAL_MO	wxT("engine_traditional.mo")

IMPLEMENT_APP(GimuApp)
BEGIN_EVENT_TABLE(GimuApp, wxApp)
END_EVENT_TABLE()

//
// 1. gimu.exe -u xxx.uif (open uif file to show the file tree.
// 2. gimu.exe -m (start minimized)
// 3. gimu.exe -n job_xxx job_yyy (runing job of job_xxx and job_yyy)
// 4. gimu.exe -r/-s/-p
// -r: start the gimu and run the job that has the option that start the job when system logon first time.
// -s: start the gimu and run the job that has the option that start the job when system shutdown.
// -p: shutdown the system when all the running job finish.
// 
const static wxCmdLineEntryDesc CmdDesc [] = {
 {wxCMD_LINE_SWITCH, START_MINIMIZE, 0, wxT ("Start minimized."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_SWITCH, START_RUN_NAMED_JOBS, 0, wxT ("Start named job."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_SWITCH, EXIT_APP_AFTER_FINISH, 0, wxT ("Exit when finish all jobs."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_SWITCH, RUN_JOBS_WHEN_START, 0, wxT ("Start job when start system."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_SWITCH, SHUTDOWN_SYS_AFTER_FINISH, 0, wxT ("Shutdown system when finish all jobs."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_SWITCH, SHOW_UIF_DATA, 0, wxT ("Show uif file's content."), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_PARAM, 0, 0, wxT ("Named jobs or uif file."), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL}
, {wxCMD_LINE_NONE}
};

GimuApp::GimuApp ()
		: wxApp ()
{
	wxString szExePath = GmAppInfo::GetAppPath ();
	wxString szLockFile = AssemblePath (szExePath, LOCK_FILE);
	m_Locker.reset (new GmFileLocker (szLockFile));
}

bool GimuApp::OnInit (void)
{
	//if (!wxApp::OnInit())
	//	return false;

	GmAppConfig * pConfig = GmAppConfig::GetInstance ();
	if (pConfig == 0) return false;
	pConfig->OpenConfig (GetConfigDocFile ());

	wxString szExePath = GmAppInfo::GetAppPath ();
	m_Locale.Init();
	GmLangID langid = pConfig->GetLangID ();
	m_Locale.AddCatalogLookupPathPrefix (szExePath);
	SetLanguage (langid);

	if (GmPassword::GetInstance ()->HasPassword ()) {
		GmPasswordDlg dlg (0, _("IDS_PROG_PASSWORD"));
		dlg.ShowModal ();
		if (!GmPassword::GetInstance ()->CheckPassword (dlg.GetPassword ())) {
			wxMessageBox (_("IDS_PASSWORD_INCORRECT"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return false;
		}
	}

	if (!m_Locker->Lock ()) {
		wxMessageBox (_("IDS_NO_MUTIPLE_APP_ALLOWED"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	wxString daemon = AssemblePath (GmAppInfo::GetAppPath (), GimuDaemonExe);
	if (!IsDaemonInstalled (daemon)) {
		InstallDaemon (daemon);
	}

	GmAppFrame * pFrame = new GmAppFrame (_("IDS_APP_TITLE"));
	int x, y;
	wxDisplaySize (&x, &y);
	wxSize MinSize (x / 1.5, y / 1.5);
	pFrame->SetMinSize (MinSize);
	SetTopWindow (pFrame);

	wxCmdLineParser parser (wxApp::argc, wxApp::argv);
	parser.SetDesc (CmdDesc);
	parser.Parse (false);

	//
	// 如果这两个选项同时出现，不是设计结果。
	//
	bool bShowSplash = true;
	if (parser.Found (START_MINIMIZE)) {
		pFrame->ToTaskTray ();
		bShowSplash = false;
	}
	else if (parser.Found (SHOW_UIF_DATA)) {
		size_t count = parser.GetParamCount ();
		if (count >= 1) {
			wxString UifFileName = parser.GetParam (0);
			wxCommandEvent cmd (GmEVT_SHOW_UIF_FILE_TREE, wxID_ANY);
			cmd.SetEventObject (pFrame);
			cmd.SetString (UifFileName);
			pFrame->AddPendingEvent (cmd);
			bShowSplash = false;
		}
	}
	
	if (bShowSplash) {
		wxBitmap bitmap (gimu_splash_xpm);
		new GmSplashWindow (bitmap
							, 3000
							, pFrame
							, wxID_ANY
							, wxDefaultPosition
							, wxDefaultSize
							, wxSTAY_ON_TOP | wxFRAME_SHAPED);
		wxYield ();
	}
	else {
		pFrame->Show ();
		pFrame->Maximize ();
		pFrame->StartRegisterTimer ();
	}



	//
	// 需要先启动任务，后面的两个参数才会正确地执行，否则可能由于执行顺序的问题导致错误。
	// 尽管这种可能性很低。
	//
	if (parser.Found (START_RUN_NAMED_JOBS)) {
		GmListWindow * plw = pFrame->GetListWindow ();
		size_t count = parser.GetParamCount ();
		for (size_t i = 0; i < count; ++i) {
			wxString JobName = parser.GetParam (i);
			bool bShowError = false;
			plw->RunNamedJob (JobName, bShowError);
		}
	}

	if (parser.Found (EXIT_APP_AFTER_FINISH)) {
		GmAppFrame::SetExitWhenJobsFinishedFlag (true);
	}
	else {
		GmAppFrame::SetExitWhenJobsFinishedFlag (false);
	}

	if (parser.Found (SHUTDOWN_SYS_AFTER_FINISH)) {
		GmAppFrame::SetShutdownWhenJobsFinishedFlag (true);
	}
	else {
		GmAppFrame::SetShutdownWhenJobsFinishedFlag (false);
	}

	if (parser.Found (RUN_JOBS_WHEN_START)) {
		GmListWindow * plw = pFrame->GetListWindow ();
		plw->RunStartJobs ();
	}

	return true;
}

bool GimuApp::OnCmdLineError (wxCmdLineParser &)
{
	return true;
}

void GimuApp::OnInitCmdLine (wxCmdLineParser& parser)
{
	parser.SetDesc (CmdDesc);
}

void GimuApp::SetLanguage (GmLangID langid)
{
	if (langid == LANGUAGE_SIMPLIFIED_CHINESE) {
		m_Locale.AddCatalog (GIMU_LANG_MO);
		m_Locale.AddCatalog (ENGIN_LANG_MO);
	}
	else if (langid == LANGUAGE_TRADITIONAL_CHINESE) {
		m_Locale.AddCatalog (GIMU_LANG_TRADITIONAL_MO);
		m_Locale.AddCatalog (ENGIN_LANG_TRADITIONAL_MO);
	}
	else if (langid == LANGUAGE_ENGLISH) {
		m_Locale.AddCatalog (GIMU_LANG_ENGLISH_MO);
		m_Locale.AddCatalog (ENGIN_LANG_ENGLISH_MO);
	}
}

int GimuApp::OnExit (void)
{
	GmAppConfig::DeleteInstance ();
	m_Locker->Unlock ();
	GmPassword::DeleteInstance ();
	return 0;
}
