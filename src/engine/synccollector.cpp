//
// synchronizer implementation.
// Author:yeyouqun@163.com
// 2010-4-15
//
#include <gmpre.h>
#include <engine/synccollector.h>

GmSyncCollector::GmSyncCollector (GmMonitor * pMonitor)
					: GmCollector (pMonitor)
					, block (READ_BLOCK_LEN)
{
}

GmSyncCollector::~GmSyncCollector ()
{
}

bool GmSyncCollector::OnDir (const wxString & FullName, const wxString & szRelaPath)
{
	return true;
}

bool GmSyncCollector::OnFile (const wxString & FullName, const wxString & szRelaPath)
{
	return true;
}

void GmSyncCollector::OnError (const wxString & Message)
{
}

void GmSyncCollector::OnWarn (const wxString & Warn)
{
}
