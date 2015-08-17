//
// App definition.
// Author:yeyouqun@163.com
// 2009-11-15
//

#include <wx/splitter.h>
#include <wx/listbook.h>

#ifndef __GM_APP_FRAME_H__
#define __GM_APP_FRAME_H__

class GmTreeWindow;
class GmListWindow;
class GmStatisticsWindow;
class wxSplitterWindow;
class GmTaskbarIcon;

class GmAppFrame : public wxFrame
{
public:
	/**
	 * ctor
	 */
	GmAppFrame (const wxString& title);
				
	/**
	 * dtor
	 */
	~GmAppFrame (void);

	void OnPaint(wxPaintEvent &);
	GmListWindow * GetListWindow ();
	GmTreeWindow * GetTreeWindow ();
	GmStatisticsWindow * GetStatisticsWindow ();
	void StartRegisterTimer ();
	void ToTaskTray ();
	static void SetExitWhenJobsFinishedFlag (bool exit) { ExitWhenJobFinished = exit; }
	static bool GetExitWhenJobsFinishedFlag () { return ExitWhenJobFinished; }
	static void SetShutdownWhenJobsFinishedFlag (bool shutdown) { ShutdownWhenJobFinished = shutdown; }
	static bool GetShutdownJobsFinishedFlag () { return ShutdownWhenJobFinished; }
protected:
	enum COMMAND_ID
	{
		CMD_CREATE_NEW = 1000,
		CMD_OPEN_ARCHIVE,
		CMD_RUN_TASK,
		CMD_OPTIONS,
		CMD_CONFIGURE,
		CMD_PROG_CONFIGURE,
		CMD_SET_PASSWORD,
		CMD_MINIMIZE_TO_TASKBAR,
		CMD_START_WITH_WINDOWS,
		CMD_REGISTER,
		CMD_HELP,
		CMD_ABOUT,
		IDC_TOOLWND,
		CMD_REGISTER_ONESHOT_TIMER,
		CMD_DEFAULT_COPY,
		CMD_DEFAULT_SYNC,
		CMD_DEFAULT_ZIP,
		CMD_DEFAULT_7Z,
		CMD_DEFAULT_LANG_SIMPLIFIED,
		CMD_DEFAULT_LANG_TRADITIONAL,
		CMD_DEFAULT_LANG_ENGLISH,
		CMD_DEFAULT_SNAPMODE_TRUE,
		CMD_DEFAULT_SNAPMODE_FALSE,
		CMD_RUN_SELECTED_JOB,
		CMD_GET_DATA_BACK,
		CMD_DELETE_JOB,
		CMD_OPEN_DEST,
		CMD_SHOW_SELECTED_JOB_LOGS,
		CMD_SHOW_SELECTED_JOB_REPORTS,
		CMD_SHOW_ALL_JOB_LOGS,
		CMD_SHOW_ALL_JOB_REPORTS,
		CMD_EXPORT_SELECTED_JOB_LOGS,
		CMD_EXPORT_SELECTED_JOB_REPORTS,
		CMD_EXPORT_ALL_JOB_LOGS,
		CMD_EXPORT_ALL_JOB_REPORTS,
	};
	void InitMenu ();
	void InitToolBar ();
	void InitSplitWindows ();

	void OnCreateNewFile (wxCommandEvent &event);
	void OnOpenFile (wxCommandEvent &event);
	void OnRun (wxCommandEvent &event);
	void OnOptions (wxCommandEvent &event);
	void OnConfigure (wxCommandEvent &event);
	void OnConfigureProg (wxCommandEvent &event);
	void OnRegister (wxCommandEvent &event);
	void OnHelp (wxCommandEvent &event);
	void OnAbout (wxCommandEvent &event);
	void OnRegisterTimer (wxTimerEvent &);
	void OnWndSize (wxSizeEvent &);
	void OnWndMinimize (wxIconizeEvent &);
	void OnDefaultType (wxCommandEvent &event);
	void OnDefaultLang (wxCommandEvent &event);
	void OnSetDefaultSnapMode (wxCommandEvent &event);
	void OnExitGimu (wxCommandEvent &event);
	void OnSetPassword (wxCommandEvent &event);
	void OnMinimizeToTaskTray (wxCommandEvent &event);
	void OnStartWithSystem (wxCommandEvent &event);
	//job
	void OnRunSelectedJob (wxCommandEvent &event);
	void OnGetSelectedJobDataBack (wxCommandEvent &event);
	void OnDeleteJob (wxCommandEvent &event);
	void OnOpenDestPathOfJob (wxCommandEvent &event);
	void OnShowOrExportLogsOrReports (wxCommandEvent &event);
	void OnCloseGimu (wxCloseEvent &event);
	void OnMenuOpenInit (wxMenuEvent& event);
	void OnMouseEvent (wxMouseEvent &event);
	void OnActivate (wxActivateEvent&event);
private:
	//menu.
	wxMenu *				m_ArchiveMenu;
	wxMenu *				m_pOptionMenu;
	wxMenu *				m_pJobMenu;
	wxMenu *				m_pHelpMenu;
	void EnableOrDisableJobMenu (wxMenu * pMenu);
	void SetOptionMenus (wxMenu * pMenu);
	void OnListChanged (wxCommandEvent&event);
	void OnJobFinished (wxCommandEvent&event);
	void OnShowUifFileTree (wxCommandEvent&event);
	void RestoreAllWindowNormalSize ();
	friend class GmTaskbarIcon;
private:
	GmTreeWindow *			m_pLeftWindow;
	GmListWindow *			m_pUpRightWindow;
	GmStatisticsWindow *	m_pDownRightWindow;
	wxSplitterWindow *		m_pHoriSplitter;
	wxSplitterWindow *		m_pVertiSplitter;
	GmTaskbarIcon *			m_pTaskbarIcon;
	wxTimer					m_timer;
	static bool				ExitWhenJobFinished;
	static bool				ShutdownWhenJobFinished;
	DECLARE_EVENT_TABLE ()
}; // class GmAppFrame

#endif //__GM_APP_FRAME_H__