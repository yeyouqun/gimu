//
// 7z collector implementation.
// Author:yeyouqun@163.com
// 2010-4-16
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/analysis.h>
#include <engine/uiffile.h>
#include <engine/exception.h>
#include <engine/7zcollector.h>
#include <engine/reader.h>
#include <engine/writer.h>
#include <engine/zipfile.h>
#include <engine/execunit.h>
#include <engine/exception.h>

Gm7zCollector::Gm7zCollector (GmMonitor * pMonitor)
					: GmCollector (pMonitor)
{
}

Gm7zCollector::~Gm7zCollector ()
{
}

void Gm7zCollector::OnSource (const GmUifSourceEntry & entry)
{
}

bool Gm7zCollector::OnDir (const wxString & FullName, const wxString & szRelaPath)
{
	return true;
}

bool Gm7zCollector::OnFile (const wxString & FullName, const wxString & szRelaPath)
{
	return true;
}

void Gm7zCollector::HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile)
{
	//m_pWriter->Write (pData, Bytes);
}

////////////////////////////

GmHandle7ZThread::GmHandle7ZThread (const GmJobItem * pJob, GmMonitor * pMonitor)
					: GmHandleDataThread (pJob, pMonitor)
{
}

GmHandle7ZThread::GmHandle7ZThread (const vector<GmStoreEntry*> & vpStoreNodes
									, const wxString & szDestPath
									, int Option
									, const GmJobItem * pJob
									, GmMonitor * pMonitor)
									: GmHandleDataThread (vpStoreNodes
														, szDestPath
														, Option
														, pJob
														, pMonitor)
{
}

void GmHandle7ZThread::DoHandleData (GmUifSourceVectorT & vps, GmUnitedIndexFile * uif)
{
}

void GmHandle7ZThread::MergeDataToOne (GmUnitedIndexFile & uif)
{
}

void GmHandle7ZThread::GetDirDataBack (GmUifDirectory * pDir, const wxString & path, const GmStoreEntry * pEntry)
{
}

void GmHandle7ZThread::GetFileDataBack (GmSnapNode * pFile, const wxString & path, const GmStoreEntry * pEntry)
{
}

