//
// synchronizer implementation.
// Author:yeyouqun@163.com
// 2010-4-15
//
#include <engine/collector.h>
#ifndef __GM_SYNCHRONIZIER_H__
#define __GM_SYNCHRONIZIER_H__

struct GmSyncCollector : public GmCollector
{
	ACE_Message_Block block;
	GmSyncCollector (GmMonitor * pMonitor);
	virtual ~GmSyncCollector ();
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath);
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath);
	virtual void OnError (const wxString & Message);
	virtual void OnWarn (const wxString & Warn);
};

#endif //__GM_SYNCHRONIZIER_H__