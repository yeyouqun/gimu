//
// zippor collector implementation.
// Author:yeyouqun@163.com
// 2010-4-14
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/uifalgo.h>
#include <engine/util.h>
#include <engine/analysis.h>
#include <engine/zipfile.h>
#include <engine/uiffile.h>
#include <engine/exception.h>
#include <engine/zipporcollector.h>
#include <engine/gmiostream.h>
#include <engine/job.h>
#include <engine/execunit.h>
#include <engine/writer.h>
#include <engine/storeentry.h>
#include <engine/reader.h>

void InitZipParams (GmZipParams & params, const GmJobItem * pJob)
{
	params.bCompress = pJob->GetDeflateType () == DEF_NO_DEFLATE ? false : true;
	params.bEncode = pJob->GetEncryptType () == ENC_NO_ENC ? false : true;
	if (params.bEncode) {
		if (pJob->GetPass ().IsEmpty ()) {
			throw GmException (_("IDS_NULL_PASSWORD"));
		}
		params.KeySize = pJob->GetEncryptType () == ENC_AES128 ? ZKS128 : 
						pJob->GetEncryptType () == ENC_AES192 ? ZKS192 : ZKS256;
		params.szPassword = ToStlString (pJob->GetPass ());
	}

	if (params.bCompress) params.ZipLevel = pJob->GetDeflateType ();
	
	params.SplitSize = pJob->GetSplitSize ();
	params.Flags = 0;
}


GmZipCollector::GmZipCollector (const wxString & szZipFile
								, const GmZipParams & params
								, GmMonitor * pMonitor) 
								: GmCollector (pMonitor, true)
{
	m_Zippor.reset (new GmZipCreator (szZipFile, params, pMonitor));
}

void GmZipCollector::OnSource (const GmUifSourceEntry & entry)
{
}

bool GmZipCollector::OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir)
{
	GmUifDirEntry entry;
	if (!GetDirectoryNode (entry, FullName, GetFileName (FullName)))
		return false;

	GmLeafInfo info = entry;
	m_Zippor->AddDirectory (szRelaPath, info);
	return true;
}

bool GmZipCollector::OnFile (const wxString & FullName, const wxString & szRelaPath, const GmSnapNode * pLeafNode)
{
	GmLeafEntry entry;
	if (!GetFileNode (entry, FullName, GetFileName (FullName)))
		return false;

	GmLeafInfo info = entry;
	m_Zippor->BeginAddFile (szRelaPath, info);
	HandleFile (FullName);
	m_Zippor->EndAddFile ();
	return true;
}

GmZipCreator & GmZipCollector::GetZippor () const
{
	return *m_Zippor.get ();
}

void GmZipCollector::HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile)
{
	m_Zippor->AddFileBlock (pData, Bytes, bEndFile);
}

GmZipCollector::~GmZipCollector ()
{
}

wxString GetZipName (const GmJobItem * pJob, const wxDateTime & now)
{
	wxString szZipName;
	wxDateTime::Tm tmdata = now.GetTm ();
	wxString szNameDateStr = wxString::Format (wxT ("%s_%d-%d-%d-%d-%d-%d")
											, pJob->Name ().c_str ()
											, tmdata.year
											, tmdata.mon + 1
											, tmdata.mday
											, tmdata.hour
											, tmdata.min
											, tmdata.sec);

	szZipName = wxString::Format (wxT ("%s\\%s\\%s.zip")
											, pJob->DestPath ().c_str ()
											, pJob->Name ().c_str ()
											, szNameDateStr.c_str ());

	int index = 0;
	while (wxFile::Exists (szZipName)) {
		szZipName = wxString::Format (wxT ("%s\\%s\\%s(%d).zip")
											, pJob->DestPath ().c_str ()
											, pJob->Name ().c_str ()
											, szNameDateStr.c_str ()
											, index);
	}

	szZipName.Replace (wxT ("\\\\"), wxT ("\\"));
	szZipName.Replace (wxT ("//"), wxT ("/"));
	return szZipName;
}

class GmMergeZipWriter : public GmWriter
{
	ACE_Message_Block &	m_block;
	GmZipCreator &		m_ZipCreator;
	virtual unsigned Write (const char * pData, unsigned Length)
	{
		if (m_block.length () > 0)
			m_ZipCreator.AddFileBlock (m_block.base (), SIZET_TO_UBYTE4 (m_block.length ()));
		m_block.reset ();
		assert (m_block.space () >= Length);
		m_block.copy (pData, Length);
		return Length;
	}
	virtual ubyte8 Seek (ubyte8 pos, wxSeekMode from)
	{
		assert (false);
		return -1;
	}
	virtual bool Rebind (const wxString & fileName, const GmLeafInfo * pInfo)
	{
		m_block.reset ();
		return true;
	}
	virtual bool CreateDir (const wxString & name)
	{
		assert (false);
		return false;
	}
	virtual void SetFileAttr (const ubyte8 & ct, const ubyte8 & mt, const ubyte8 & at, ubyte4 attribute)
	{
		return;
	}
	virtual bool SetDestPath (const wxString & path)
	{
		assert (false);
		return false;
	}
	virtual bool Exists (const wxString & FileName) const
	{
		assert (false);
		return false;
	}
	virtual bool RemoveFile (const wxString & FileName) const
	{
		assert (false);
		return false;
	}
	virtual bool RemoveDir (const wxString & DirName) const
	{
		assert (false);
		return false;
	}
public:
	virtual void Close ()
	{
		assert (m_block.length ());
		if (m_block.length () > 0)
			m_ZipCreator.AddFileBlock (m_block.base (), SIZET_TO_UBYTE4 (m_block.length ()), true);
		m_block.reset ();
	}
	GmMergeZipWriter (ACE_Message_Block & block, GmZipCreator &	ZipCreator)
		: m_block (block)
		, m_ZipCreator (ZipCreator)
	{
	}
	~GmMergeZipWriter ()
	{
	}
};

GmCDSEntry * FindEntry (const ZipTreeT & tree, const string & file);
/////////////////////////////////////////////////////////////////////////
struct GmMergeZipCollector: public GmCollector
{
	GmZipCreator &			m_zippor;
	ACE_Message_Block		m_block;
	GmUnitedIndexFile *		m_pUifFile;
	GmMonitor *				m_pMonitor;
	const vector<GmUifRootEntry *> &m_roots;

	virtual void OnSource (const GmUifSourceEntry & entry) {}
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir);
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath, const GmSnapNode * pLeafNode);
	virtual void HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile) {}
public:
	GmMergeZipCollector (GmZipCreator & zippor, GmUnitedIndexFile * pUifFile, GmMonitor * pMonitor)
								: GmCollector (pMonitor, true)
								, m_zippor (zippor)
								, m_block (READ_BLOCK_LEN)
								, m_pUifFile (pUifFile)
								, m_pMonitor (pMonitor)
								, m_roots (pUifFile->GetAllRootEntries ())
	{}
	~GmMergeZipCollector ()
	{
		GmZipDistillerBuffer::DeleteInstance ();
	}
};

bool GmMergeZipCollector::OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir)
{
	GmLeafInfo info;
	if (pDir->pMySelf == 0) {
		GmUifDirEntry entry;
		if (!GetDirectoryNode (entry, FullName, GetFileName (FullName)))
			return false;
		info = entry;
	}
	else info = *(pDir->pMySelf);
	m_zippor.AddDirectory (szRelaPath, info);
	return true;
}

bool GmMergeZipCollector::OnFile (const wxString & FullName
								  , const wxString & szRelaPath
								  , const GmSnapNode * pLeafNode)
{
	GmLeafInfo info = *(pLeafNode->pLeafNode);
	m_zippor.BeginAddFile (szRelaPath, info);

	GmMergeZipWriter writer (m_block, m_zippor);
	ubyte8 time = pLeafNode->Time;
	GmUifRootEntry * pRoot = FromTimeToRootEntry (m_roots, time);
	if (pRoot == 0) return false;

	GetDataBackFromZipArchive (m_pUifFile
							, pRoot
							, const_cast<GmSnapNode*> (pLeafNode)
							, szRelaPath
							, 0
							, &writer
							, m_pMonitor);
	m_zippor.EndAddFile ();
	return true;
}

/////////////////////////////////////////////////////

GmHandleZipThread::GmHandleZipThread (const GmJobItem * pJob, GmMonitor * pMonitor)
						: GmHandleDataThread (pJob, pMonitor)
{
}

GmHandleZipThread::GmHandleZipThread (const vector<GmStoreEntry*> & vpStoreNodes
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

void GmHandleZipThread::DoHandleData (GmUifSourceVectorT & vps, GmUnitedIndexFile * uif)
{
	if (vps.empty ()) return;
	wxDateTime now;
	now.SetToCurrent ();
	//
	// handle file data.
	//
	wxString ZipName = GetZipName (m_pJob, now);
	GmZipParams params;
	InitZipParams (params, m_pJob);

	GmZipCollector zipcollector (ZipName, params, m_pMonitor);
	GmZipCreator & creator = zipcollector.GetZippor ();

	creator.BeginCreate ();
	TraverseTheDirForCollector (vps, zipcollector);
	creator.EndCreate (wxString ());
	return;
}


void GmHandleZipThread::MergeDataToOne (GmUnitedIndexFile & uif)
{
	// to do:implement it.
	const vector<GmUifRootEntry*> & roots = uif.GetAllRootEntries ();
	ubyte4 MergeTimes = m_pJob->Times ();
	if (roots.size () <= MergeTimes) return;

	//
	// 需要合并。
	//
	vector<wxString> vszFiles;
	GetAllSetName (uif, vszFiles);

	GmUifRootPairT ResultRootPair;
	GmAutoClearRootPairTree acur (ResultRootPair);
	MergeTree (ResultRootPair, uif);

	wxDateTime now;
	now.SetToCurrent ();
	//
	// 将数据恢复同一个ZIP（一组）文件中。
	//
	RestoreToOneZipFile (uif, ResultRootPair, now);

	uif.ClearUif ();
	AddTreeToUif (now, *ResultRootPair.second, uif);

	//
	// 删除数据。
	//
	m_pMonitor->HandleMessage (_("IDS_REMOVING_FILE"));
	for (size_t index = 0; index < vszFiles.size (); ++index) {
		if (!wxRemoveFile (vszFiles.at (index))) {
			wxString message = wxString::Format (_("IDS_CANT_REMOVE_FILE")
												, vszFiles.at (index).c_str ()
												, GetSystemError ().c_str ());
			if (m_pMonitor) m_pMonitor->HandleMessage (message);
		}
	}
}

void GmHandleZipThread::RestoreToOneZipFile (GmUnitedIndexFile & uif
											 , const GmUifRootPairT & ResultRootPair
											 , const wxDateTime & now)
{
	const vector<GmUifRootEntry*> & roots = uif.GetAllRootEntries ();

	wxString ZipName = GetZipName (m_pJob, now);
	GmZipParams params;
	InitZipParams (params, m_pJob);

	m_pMonitor->ClearAllFile ();
	GmZipCreator creator (ZipName, params, m_pMonitor);
	creator.BeginCreate ();

	GmMergeZipCollector collector (creator, &uif, m_pMonitor);
	TraverseTheDirForCollector (*ResultRootPair.second, collector);

	creator.EndCreate (wxEmptyString);
}

void GetDataBackFromZipArchive (GmUnitedIndexFile * pUifFile
								 , GmUifRootEntry * root
								 , GmUifDirectory * pTimeDir
								 , const wxString & path
								 , int Option
								 , GmWriter * pWriter
								 , GmMonitor * pMonitor);

void GmHandleZipThread::GetDirDataBack (GmUifDirectory * pDir, const wxString & path, const GmStoreEntry * pEntry)
{
	GmUifRootStoreEntry * root = (GmUifRootStoreEntry*)pEntry->GetRootEntry ();
	GmUnitedIndexFile * pUifFile = root->GetUifFile ();
	GmUifRootEntry * pUifRoot = root->GetUifRoot ();

	const GmStoreEntry * ste = pEntry;
	while (ste->GetType () != GNT_SOURCE) ste = ste->GetParent ();
	const GmUifSourceEntry * entry = (GmUifSourceEntry*)(ste->GetEntry ());

	GmWriter * pWriter = GetDataBackWriter (pEntry, entry); 
	GetDataBackFromZipArchive (pUifFile, pUifRoot, pDir, path, m_Options, pWriter, m_pMonitor);
}

void GmHandleZipThread::GetFileDataBack (GmSnapNode * pFile, const wxString & path, const GmStoreEntry * pEntry)
{
	GmUifRootStoreEntry * root = (GmUifRootStoreEntry*)pEntry->GetRootEntry ();
	GmUnitedIndexFile * pUifFile = root->GetUifFile ();
	GmUifRootEntry * pUifRoot = root->GetUifRoot ();
	
	const GmStoreEntry * ste = pEntry;
	while (ste->GetType () != GNT_SOURCE) ste = ste->GetParent ();
	const GmUifSourceEntry * entry = (GmUifSourceEntry*)(ste->GetEntry ());

	GmWriter * pWriter = GetDataBackWriter (pEntry, entry);
	GetDataBackFromZipArchive (pUifFile, pUifRoot, pFile, path, m_Options, pWriter, m_pMonitor);
}

void GmHandleZipThread::GetZipDirectoryDataBack (const GmZipDirectory * pDir, const GmStoreEntry * pEntry)
{
	const GmZipRootStoreEntry * root = (const GmZipRootStoreEntry*)pEntry->GetRootEntry ();
	GmZipDistiller * pDistiller = root->GetDistiller (m_pMonitor);
	GmWriter * pWriter = GetDataBackWriter (pEntry, 0); 
	pDistiller->RestoreDirectory (pDir, *pWriter);
}

void GmHandleZipThread::GetZipFileDataBack (GmCDSEntry * pFile, const GmStoreEntry * pEntry)
{
	GmZipRootStoreEntry * root = (GmZipRootStoreEntry*)pEntry->GetRootEntry ();
	GmZipDistiller * pDistiller = root->GetDistiller (m_pMonitor);
	GmWriter * pWriter = GetDataBackWriter (pEntry, 0); 
	pDistiller->RestoreFile (pFile, *pWriter);
}
