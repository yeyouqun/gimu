//
// job list's implementation.
// Author:yeyouqun@163.com
// 2010-2-8
//

#include <wx/image.h>
struct GmMonitor;

#ifndef __GM_JOB_LIST_H__
#define __GM_JOB_LIST_H__

class GmJobItem;
class GmListCtrl;
struct GmJobEvent;
class wxListItem;
class GmRootCreator;
struct JobListItemData;
class GmJobItem;
class GmStatisticsWindow;

class GmListWindow : public wxPanel
{
public:
	GmListWindow (GmAppFrame * m_pMainFrame, wxSplitterWindow * pParent);
	~GmListWindow ();
	JobListItemData * AddJobItem (GmJobItem * pJob, bool push = false);
	void RunSelectedJob ();
	void RunNamedJob (const wxString & name, bool bShowError);
	//
	// 执行关机时要执行的任务
	//
	void RunShutdownJobs ();
	//
	// 执行开机时要执行的任务
	//
	void RunStartJobs ();
	//
	// 判断有没有启动后执行的任务;
	//
	bool HasJobsRunStart ();
	//
	// 判断有没有关机时执行的任务;
	//
	bool HasJobsRunShutDown ();
	void CreateNewJob (GmJobItem * pNewJob);
	void SetSelectedJobOptions ();
	void DeleteJob ();
	void GetDataBack ();
	void GetDataBack (const wxString & szFileName);
	bool HasRunJob ();
	void OnDeleteJob ();
	// about logs and reports
	void OnShowLogs (bool all = false);
	void OnShowReports (bool all = false);
	void OnExportLogsTXT (bool all = false);
	void OnExportReportsTXT (bool all = false);
	// dest
	void OnJobDest ();
	bool HasSelectedItem ();
	bool SelectedIsRun ();
	bool SelectedIsPaused ();
	void OnPauseResume ();
	bool HasSameJob (const wxString & JobName) const;
	JobListItemData * GetJobData (int index);
	int GetTotalJobs ();
	void OnStopJob ();
	void OnCreateNew ();
	void OnOpenArchive ();
private:
	void ShowGetDataBackDialog (JobListItemData * pItemData);
	friend class GmListCtrl;
	void StartTheJob (GmJobItem * pJobItem, JobListItemData * pItemData);
	void StartGetDataJob (GmJobItem * pJob
									, JobListItemData * pData
									, const wxString szDestPath
									, int Option
									, const vector<GmStoreEntry*> & items);
	JobListItemData * CreateTempJob (const wxString & filename);
	vector<GmJobItem *>		m_vpJobs;
	enum {
		IDC_STOP_JOB = 10000,
		IDC_PAUSE_RESUME,
		IDC_CLEAR_PAGE_CONTENT,
		IDC_JOB_LIST_ID,
		IDC_PULSE_TIMER,
	};

	bool GetJobNameAndSaveName (const wxString & files, wxString & FileName, wxString & JobName, bool all);
	//event
	void OnPauseJob (wxCommandEvent &event);
	void OnStopJob (wxCommandEvent &event);
	void OnPauseResume (wxCommandEvent &event);
	void OnClearPageContent (wxCommandEvent &event);
	//my event

	void OnHandleFile (GmJobEvent &event);
	void OnHandleDir (GmJobEvent &event);
	void OnHandleSize (GmJobEvent &event);
	void OnHandleMessage (GmJobEvent &event);
	void OnHandleBeginAnalysis (GmJobEvent &event);
	void OnHandleEndAnalysis (GmJobEvent &event);
	void OnHandleError (GmJobEvent &event);
	void OnHandleWarn (GmJobEvent &event);
	void OnHandleBegin (GmJobEvent &event);
	void OnHandleEnd (GmJobEvent &event);
	void OnHandleOverwrite1 (GmJobEvent &event);
	void OnHandleOverwrite2 (GmJobEvent &event);
	void OnHandleManualStop (GmJobEvent &event);
	void OnHandleDirectoryInfo (GmJobEvent &event);
	void OnHandleTimerEvent (wxTimerEvent &event);
	void OnHandleItemSelectionChanged (wxListEvent &event);
	void OnDBClickItem (wxListEvent &event);
private:
	bool GetSelectedItem (wxListItem & item);
	int FindItem (JobListItemData *pData);
	int FindItem (const wxString & name);
	void RemoveTempItem (int index, JobListItemData * pData);
	void EnableStopPause (bool);
	void PauseOrResumeJob (wxListItem & item);
	void StopJob (GmJobEvent &event);
protected:
	GmAppFrame *	m_pMainFrame;
	GmListCtrl *	m_pList;
	wxStaticText *	m_JobText;
	wxTimer			m_timer;
	wxImageList	m_ImageList;
	DECLARE_EVENT_TABLE ()
};

DECLARE_GM_EVENT_TYPE (GmEVT_JOBITEM_CHANGED)
DECLARE_GM_EVENT_TYPE (GmEVT_JOB_FINISHED)
DECLARE_GM_EVENT_TYPE (GmEVT_SHOW_UIF_FILE_TREE)
#endif //__GM_JOB_LIST_H__