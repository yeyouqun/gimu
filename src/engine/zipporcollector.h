//
// zippor collector implementation.
// Author:yeyouqun@163.com
// 2010-4-14
//
#include <engine/collector.h>
#ifndef __GM_ZIPPOR_COLLECTOR_H__
#define __GM_ZIPPOR_COLLECTOR_H__

class GmZipCreator;
struct ENGINE_EXPORT GmZipCollector: public GmCollector
{
	auto_ptr<GmZipCreator> m_Zippor;
	virtual void OnSource (const GmUifSourceEntry & entry);
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir);
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath, const GmSnapNode * pLeafNode);
	virtual void HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile);
public:
	GmZipCreator & GetZippor () const;
	GmZipCollector (const wxString & szZipFile, const GmZipParams & params, GmMonitor * pMonitor);
	~GmZipCollector ();
};

#endif //__GM_ZIPPOR_COLLECTOR_H__