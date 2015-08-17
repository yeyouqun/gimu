//
// Gimu Application definitions.
// Author:yeyouqun@163.com
// 2010-7-19
//
#ifndef __GM_GIMU_DEFS_H__
#define __GM_GIMU_DEFS_H__
#define LOCK_FILE wxT ("782D647D-F416-40fc-A253-06D92C066D9D")
#define GIMU_APP_NAME wxT ("gimu.exe")

inline wxString GetConfigDocFile ()
{
	return AssemblePath (GmAppInfo::GetAppPath (), wxT ("gimu_config.xml"));
}

inline wxString GetReportFile ()
{
	return AssemblePath (GmAppInfo::GetAppPath (), wxT ("gimu_reports_data.dat"));
}

inline wxString GetLogFile ()
{
	return AssemblePath (GmAppInfo::GetAppPath (), wxT ("gimu_logs_data.dat"));
}

inline wxString GetJobContainerFile ()
{
	return AssemblePath (GmAppInfo::GetAppPath (), wxT ("gimu_jobs_data.dat"));
}

#define START_MINIMIZE			wxT ("m")
#define START_RUN_NAMED_JOBS	wxT ("n")
#define EXIT_APP_AFTER_FINISH	wxT ("s")
#define SHUTDOWN_SYS_AFTER_FINISH wxT ("p")
#define RUN_JOBS_WHEN_START		wxT ("r")
#define SHOW_UIF_DATA			wxT ("u")

#endif //__GM_GIMU_DEFS_H__