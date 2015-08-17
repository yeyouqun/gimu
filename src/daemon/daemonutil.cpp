//
// daemon's implementation, daemon server as a service.
// Author:yeyouqun@163.com
// 2010-7-19
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/gmiostream.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/job.h>
#include <engine/exception.h>
#include "../apps/bak/gimudefs.h"
#include "daemonutil.h"


bool IsGimuAppStart ()
{
	wxString szExePath = GmAppInfo::GetAppPath ();
	wxString szLockFile = AssemblePath (szExePath, LOCK_FILE);
	GmFileLocker flocker (szLockFile);
	if (!flocker.Lock ()) return true;
	return false;
}

DaemonContext::DaemonContext ()
				: bNotStop (true)
{
}

DaemonContext::~DaemonContext ()
{
	GmAutoClearVector<GmJobItem> acvj (Jobs);
}

DaemonContext ctx;

void ReadJobs ()
{
	bool bError = false;
	wxString message;
	try {
		vector<GmJobItem*> & jobs = ctx.Jobs;
		for_each (jobs.begin (), jobs.end (), ReleaseMemory ());
		jobs.clear ();
		GetJobsFromJobFile (GetJobContainerFile (), jobs);
	}
	catch (GmException &e) {
		message = wxString::Format (_T("Schedule %s job error, error is %s")
									, GimuDaemonService
									, e.GetMessage ().c_str ());
		bError = true;
	}
	catch (...) {
		message = wxString::Format (_T("Schedule %s job unknown error!"), GimuDaemonService);
		bError = true;
	}

	if (bError) {
		HANDLE es = RegisterEventSource (NULL, GimuDaemonService);
		if (NULL != es) {
			LPCTSTR lpszStrings[2];
			lpszStrings[0] = GimuDaemonService;
			lpszStrings[0] = message.c_str ();
			ReportEvent(es,					 // event log handle
						EVENTLOG_ERROR_TYPE, // event type
						0,                   // event category
						0,			         // event identifier
						NULL,                // no security identifier
						2,                   // size of lpszStrings array
						0,                   // no binary data
						lpszStrings,         // array of strings
						NULL);               // no binary data
			DeregisterEventSource (es);
		}
	}
}
