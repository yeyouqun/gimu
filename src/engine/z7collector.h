//
// 7z collector implementation.
// Author:yeyouqun@163.com
// 2010-4-16
//
#include <engine/collector.h>
#ifndef __GM_7Z_COLLECTOR_H__
#define __GM_7Z_COLLECTOR_H__

struct Gm7zCollector: public GmCollector
{
	ACE_Message_Block block;
	Gm7zCollector (GmMonitor * pMonitor);
	~Gm7zCollector ();
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath);
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath);
	virtual void OnError (const wxString & Message);
	virtual void OnWarn (const wxString & Warn);
};

#endif //__GM_7Z_COLLECTOR_H__