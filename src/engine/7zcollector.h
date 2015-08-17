//
// 7z collector implementation.
// Author:yeyouqun@163.com
// 2010-4-16
//
#include <engine/collector.h>

#ifndef __GM_7Z_COLLECTOR_H__
#define __GM_7Z_COLLECTOR_H__

class Gm7zCollector: public GmCollector
{
	virtual void OnSource (const GmUifSourceEntry & entry);
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath);
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath);
	virtual void HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile);
public:
	Gm7zCollector (GmMonitor * pMonitor);
	~Gm7zCollector ();
};

#endif //__GM_7Z_COLLECTOR_H__