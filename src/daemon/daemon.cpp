//
// daemon's implementation, daemon server as a service.
// Author:yeyouqun@163.com
// 2010-6-27
//

#include <gmpre.h>
#include <winsvc.h>
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/gmiostream.h>
#include <engine/uifblock.h>
#include <engine/uifalgo.h>
#include <engine/uiffile.H>
#include <engine/job.h>
#include <engine/writer.h>
#include <engine/zipblock.h>
#include <engine/zipfile.h>
#include <engine/storeentry.h>
#include <engine/exception.h>
#include <engine/execunit.h>
#include <engine/reader.h>
#include "daemonutil.h"
#include "../apps/bak/gimudefs.h"

void GimuDaemonProc (DWORD argc, LPTSTR *argv);
BOOL UpdateServiceStatus (DWORD dwCurrentState
						, DWORD dwWin32ExitCode
						, DWORD dwServiceSpecificExitCode
						, DWORD dwWaitHint);
void KillService ();

SERVICE_TABLE_ENTRY  gimusvc []= {{GimuDaemonService , (LPSERVICE_MAIN_FUNCTION)GimuDaemonProc},
									{0, 0}};

int main (int argn, char ** argc)
{
	wxSleep (10);
	BOOL success = StartServiceCtrlDispatcher(gimusvc);
	return 0;
}

extern DaemonContext ctx;
void DoDaemonTask ()
{
	bool bGimuIsRuning = false;
	ReadJobs ();
	while (ctx.bNotStop) {
		//
		// 
		//
		::Sleep (GimuJobScheduleDispersion * 1000);
		if (IsGimuAppStart ()) {
			bGimuIsRuning = true;
			continue;
		}
		else {
			//
			// 之前软件在执行，现在已经不在执行了，所以先更新一下Job内容。
			//
			if (bGimuIsRuning) ReadJobs ();
			bGimuIsRuning = false;
		}

		{
			vector<GmJobItem*> & jobs = ctx.Jobs;
			if (jobs.empty ()) continue;

			wxString jobsname;
			for (size_t index = 0; index < jobs.size (); ++index) {
				if (IsThisJobAtScheduleTime (jobs[index])) {
					jobsname += jobs[index]->Name ();
					jobsname += wxT (" ");
				}
			}

			//
			// 如果没有任务需要执行，那么就不需要启动 Gimu。
			//
			if (jobsname.IsEmpty ()) continue;

			wxString szExePath = GmAppInfo::GetAppPath ();
			wxString gimuexe = AssemblePath (szExePath, GIMU_APP_NAME);

			wxString format = wxT ("%s -");
			format += START_MINIMIZE;
			format += wxT (" -");
			format += START_RUN_NAMED_JOBS;
			format += wxT (" -");
			format += EXIT_APP_AFTER_FINISH;
			format += wxT (" ");
			format += wxT ("%s");
			wxString run = wxString::Format (format, gimuexe.c_str (), jobsname);
			wxExecute (run);
		}
	}

	UpdateServiceStatus (SERVICE_STOPPED, NO_ERROR, 0, 3000);
	return;
}

void GimuDaemonCtrlHandler(DWORD nControlCode)
{
	switch (nControlCode)
	{
		case SERVICE_CONTROL_SHUTDOWN:
			if (!IsGimuAppStart ()) {
				//
				// 如果软件没有启动，需要启动备份完成后才能关机。
				//
				vector<GmJobItem*> & jobs = ctx.Jobs;
				if (jobs.empty ()) break;

				wxString jobsname;
				for (size_t index = 0; index < jobs.size (); ++index) {
					if (IsThisJobAtScheduleTime (jobs[index])) {
						jobsname += jobs[index]->Name ();
						jobsname += wxT (" ");
					}
				}

				//
				// 如果没有任务需要执行，那么就不需要启动 Gimu。
				//
				if (jobsname.IsEmpty ()) break;

				AbortSystemShutdown (0);
				wxString szExePath = GmAppInfo::GetAppPath ();
				wxString gimuexe = AssemblePath (szExePath, GIMU_APP_NAME);

				wxString format = wxT ("%s -");
				format += START_MINIMIZE;
				format += wxT (" -");
				format += START_RUN_NAMED_JOBS;
				format += wxT (" -");
				format += SHUTDOWN_SYS_AFTER_FINISH;
				format += wxT (" ");
				format += wxT ("%s");
				wxString run = wxString::Format (format, gimuexe.c_str (), jobsname);
				wxExecute (run);
			}
			break;
		case SERVICE_CONTROL_STOP:
			UpdateServiceStatus (SERVICE_STOP_PENDING, NO_ERROR, 0, 3000);
			KillService();
			break;
		default:
			break;
	}

	return;
}

static SERVICE_STATUS_HANDLE DaemonHandle;
void GimuDaemonProc (DWORD argc, LPTSTR *argv)
{
	DaemonHandle = RegisterServiceCtrlHandler (GimuDaemonService, (LPHANDLER_FUNCTION)GimuDaemonCtrlHandler);
	if (DaemonHandle == 0) {
		return;
	}

	BOOL success = UpdateServiceStatus (SERVICE_RUNNING, NO_ERROR, 0, 3000);
	if(!success) {
		return;
	}

	DoDaemonTask ();
}

void KillService ()
{
	ctx.bNotStop = false;
	UpdateServiceStatus (SERVICE_STOPPED, NO_ERROR, 0, 0);
}

BOOL UpdateServiceStatus (DWORD dwCurrentState
						, DWORD dwWin32ExitCode
						, DWORD dwServiceSpecificExitCode
						, DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS nServiceStatus;
	nServiceStatus.dwServiceType	= SERVICE_WIN32_OWN_PROCESS;
	nServiceStatus.dwCurrentState	= dwCurrentState;
	nServiceStatus.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
	nServiceStatus.dwCheckPoint		= 0;
	nServiceStatus.dwWaitHint		= dwWaitHint;

	if(dwCurrentState == SERVICE_START_PENDING) {
		nServiceStatus.dwControlsAccepted = 0;
	}
	else {
		nServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	}

	if(dwServiceSpecificExitCode == NO_ERROR) {
		nServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	}
	else {
		nServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	}

	success = SetServiceStatus (DaemonHandle, &nServiceStatus);
	if(!success) {
		KillService ();
		return success;
	}

	return success;
}

