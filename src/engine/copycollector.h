//
//
// copier implementation.
// Author:yeyouqun@163.com
// 2010-4-14
//
#include <engine/collector.h>
#ifndef __GM_SYNCHRONIZIER_H__
#define __GM_SYNCHRONIZIER_H__
class GmWriter;
class GmUnitedIndexFile;

class GmCopyCollector : public GmCollector
{
	GmWriter *		m_pWriter;
	virtual void OnSource (const GmUifSourceEntry & entry);
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir);
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath, const GmSnapNode * pLeafNode);
	virtual void HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile);
public:
	GmCopyCollector (GmWriter * pWriter, GmMonitor * pMonitor);
	virtual ~GmCopyCollector ();
};

class GmUnitedIndexFile;
class GmJobItem;
void GetDataBackupFromDupDirectory (GmUnitedIndexFile * pUifFile
									, GmUifRootEntry * root
									, GmUifDirectory * pSnapDir
									, const wxString & path
									, int Option
									, GmWriter * pWriter
									, GmMonitor * pMonitor);
void GetDataBackupFromDupDirectory (GmUnitedIndexFile * pUifFile
									, GmUifRootEntry * root
									, GmSnapNode * pFile
									, const wxString & path
									, int Option
									, GmWriter * pWriter
									, GmMonitor * pMonitor);
#endif //__GM_SYNCHRONIZIER_H__