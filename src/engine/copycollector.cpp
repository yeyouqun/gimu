//
// copier implementation.
// Author:yeyouqun@163.com
// 2010-4-14
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/uifblock.h>
#include <engine/uifalgo.h>
#include <engine/gmiostream.h>
#include <engine/copycollector.h>
#include <engine/analysis.h>
#include <engine/uiffile.h>
#include <engine/reader.h>
#include <engine/exception.h>
#include <engine/job.h>
#include <engine/writer.h>
#include <engine/zipfile.h>
#include <engine/execunit.h>
#include <engine/storeentry.h>

inline static string 
GetDestRelaPath (const GmUifSourceEntry & entry)
{
	//
	// 各个数据源的位置都加了一个数据源的 CRC32 目录，为了防止同名文件的覆盖。因为在不同数据源进行复制时
	// 会导致不同层级的目录复制在同一层级的目的地，如果数据数源是一个文件数据源（文件数据源只在目的地根目录
	// 下直接生成文件），那么有可能在目的地有文件名冲突。数据源是一个目录数据源也是一样，如果最后一级目录相同
	// 也会出现相同的情况。我们尽力避免。如果无法避免，我们不复制并且通知用户。
	//
	wxString srcpath = GetCrc32String (entry.SourceName.c_str (), entry.SourceName.length ());
	return ToStlString (srcpath);
}

GmCopyCollector::GmCopyCollector (GmWriter * pWriter
								  , GmMonitor * pMonitor)
					: GmCollector (pMonitor)
					, m_pWriter (pWriter)
{
}

GmCopyCollector::~GmCopyCollector ()
{
}

void GmCopyCollector::OnSource (const GmUifSourceEntry & entry)
{
	//
	// 虽然这样的方法不能完全杜绝不同数据源下存在相同名字的文件而导致出错的问题，但已经把可能性降到最低。
	//
	wxString crcpath = GetCrc32String (entry.SourceName.c_str (), entry.SourceName.length ());
	wxString path = AssemblePath (m_pWriter->GetDestPath (), crcpath);
	if (!m_pWriter->SetDestPath (path)) {
		//
		// 如果没有成功创建最上层目录，后面的操作都将失败。
		//
		wxString message = wxString::Format (_("IDS_CANT_CREATE_DEST_FILE")
											, path.c_str ()
											, GetSystemError ().c_str ());
		throw GmException (message);
	}
}

bool GmCopyCollector::OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir)
{
	GmUifDirEntry entry;
	if (!GetDirectoryNode (entry, FullName, GetFileName (FullName)))
		return false;
	GmLeafInfo info = entry;
	if (!m_pWriter->CreateDir (szRelaPath)) return false;
	m_pWriter->SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
	return true;
}

bool GmCopyCollector::OnFile (const wxString & FullName, const wxString & szRelaPath, const GmSnapNode * pLeafNode)
{
	GmLeafEntry entry;
	wxString FileName = GetFileName (FullName);
	if (!GetFileNode (entry, FullName, FileName))
		return false;

	GmLeafInfo info = entry;
	wxString szDestPath = AssemblePath (szRelaPath, FileName);
	if (m_pWriter->Exists (szDestPath)) {
		wxString message = wxString::Format (wxT ("IDS_HAS_SAME_FILE_ON_DEST"), szDestPath.c_str ());
		OnMessage (message);
		return false;
	}

	if (!m_pWriter->Rebind (szDestPath, &info)) {
		return false;
	}

	HandleFile (FullName);
	m_pWriter->Close ();
	m_pWriter->SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
	return true;
}

void GmCopyCollector::HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile)
{
	m_pWriter->Write (pData, Bytes);
}


//////////////////////////////////////////////////////////////////////////////////////////


wxString GetJobDestPath (GmUnitedIndexFile * pUifFile
							   , const GmUifRootEntry & root
							   , GmMonitor * pMonitor)
{
	vector<GmDataSetEntry*> vpsets;
	GmAutoClearVector<GmDataSetEntry> acvpsets (vpsets);
	pUifFile->GetAllUifDataSet (root, vpsets);
	if (vpsets.empty ()) return wxEmptyString;

	wxString szSourcePath = ToWxString (vpsets.at (0)->SetName);
	if (!wxDir::Exists (szSourcePath)) {
		wxString path = pMonitor->HandleSelectDir (szSourcePath);
		if (path.IsEmpty ()) {
			szSourcePath.clear ();
		}
		else {
			szSourcePath = path;
		}
	}

	return szSourcePath;
}

bool RestoreDupFile (ACE_Message_Block & block
					 , const wxString & szPathFile
					 , GmSnapNode * pFile
					 , GmWriter * pWriter
					 , GmReader * pReader
					 , GmMonitor * pMonitor)
{
	GmLeafInfo info (*pFile->pLeafNode);
	if (!pWriter->Rebind (szPathFile, &info)) {
		wxString message = wxString::Format (_("IDS_CANT_CREATE_DEST_FILE")
											, szPathFile.c_str ()
											, GetSystemError ().c_str ());
		if (pMonitor != 0) pMonitor->HandleWarn (message);
		return false;
	}

	ubyte8 TotalSize = 0;
	if (!pReader->Rebind (szPathFile, TotalSize)) {
		wxString message = wxString::Format (_("IDS_DIR_OR_FILE_NOT_EXISTS")
									, szPathFile.c_str (), szPathFile.c_str ());
		if (pMonitor != 0) pMonitor->HandleWarn (message);
		return false;
	}


	ubyte8 HandleSize = 0;
	while (true) {
		if (pMonitor != 0 && pMonitor->IsStopped ())
			throw GmStopProcedureException ();

		ubyte4 BytesRead = pReader->Read (block.base (), block.size ());
		HandleSize += BytesRead;
		pWriter->Write (block.base (), BytesRead);
		if (HandleSize >= TotalSize) break;
	}

	pWriter->Close ();
	pReader->Close ();
	pWriter->SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
	return true;
}

class GmMergeDupDataHandler : public GmNodeHandler
{
	GmWriter *						m_pWriter;
	wxString						m_szTimeFoler;

	GmUnitedIndexFile *				m_pUifFile;
	GmMonitor *						m_pMonitor;
	const vector<GmUifRootEntry*> & m_roots;

	typedef std::pair<wxString, wxString> move_file_t;
	std::list<move_file_t>			m_MovedFile;

	bool HandleNode (GmUifDirectory * pDir, const string & path)
	{
		wxString szNewPath = AssemblePath (m_szTimeFoler, ToWxString (path));
		if (!m_pWriter->CreateDir (szNewPath)) {
			wxString message = wxString::Format  (_("IDS_ROLLING_BACK_DIR")
													, GetSystemError ().c_str ()
													, szNewPath.c_str ());
			if (m_pMonitor) m_pMonitor->HandleMessage (message);
			RollbackMovedFiles ();
			message = wxString::Format (_("IDS_CANT_CREATE_DIR")
										, szNewPath.c_str ()
										, GetSystemError ().c_str ());
			throw GmException (message);
		}

		if (pDir->pMySelf != 0) {
			GmLeafInfo info = *(pDir->pMySelf);
			m_pWriter->SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
			pDir->pMySelf->Flags |= NODE_COLLECTED_FLAG;
		}
		return true;
	}

	bool HandleNode (GmSnapNode * pFile, const string & path)
	{
		wxString szNewPath = ToWxString (AssemblePath (path, GetNodeName (pFile)));
		wxDateTime time (TimeTToWxDateTime (pFile->Time));
		wxString szOldPath = AssemblePath (GetTimeFolderName (time), szNewPath);
		szNewPath = AssemblePath (m_szTimeFoler, szNewPath);

		if (!m_pWriter->MoveFileTo (szNewPath, szOldPath)) {
			wxString message = wxString::Format  (_("IDS_ROLLING_BACK_FILE")
											, szOldPath.c_str ()
											, szNewPath.c_str ());
			if (m_pMonitor) m_pMonitor->HandleMessage (message);
			RollbackMovedFiles ();
			message = wxString::Format (_("IDS_CANT_MOVE_FILE")
											, szOldPath.c_str ()
											, szNewPath.c_str ()
											, GetSystemError ().c_str ());
			throw GmException (message);
		}
		else {
			m_MovedFile.push_back (std::make_pair (szNewPath, szOldPath));
		}

		pFile->pLeafNode->Flags |= NODE_COLLECTED_FLAG;
		return true;
	}
	void RollbackMovedFiles ()
	{
		typedef std::list<move_file_t>::iterator iterator;
		for (iterator begin = m_MovedFile.begin (); begin != m_MovedFile.end (); ++begin) {
			const move_file_t & value = *begin;
			if (!m_pWriter->MoveFileTo (value.second, value.first)) {
				wxString message = wxString::Format (_("IDS_ROLLBACK_MOVE_FILE_ERROR")
												, value.first.c_str ()
												, value.second.c_str ()
												, GetSystemError ().c_str ());
				throw GmException (message);
			}
		}

		try {
			m_pWriter->RemoveDir (m_szTimeFoler);
		} catch (...) {}
	}
public:
	GmMergeDupDataHandler (GmUnitedIndexFile * pUifFile
							, GmWriter * pWriter
							, GmMonitor * pMonitor
							, const wxString & TimeFolder)
		: m_pWriter (pWriter)
		, m_pUifFile (pUifFile)
		, m_pMonitor (pMonitor)
		, m_roots (pUifFile->GetAllRootEntries ())
		, m_szTimeFoler (TimeFolder)
	{
		assert (pWriter);
	}

	~GmMergeDupDataHandler ()
	{
	}
};

static inline wxString GetDestPathFile (GmSnapNode * pFile, const wxString & szRelaPath)
{
	return AssemblePath (szRelaPath, ToWxString (GetNodeName (pFile)));
}

static inline wxString GetSourcePathFile (const wxString & DestTimePath
										  , const wxString & CrcPath
										  , const wxString & PathFileName
										  , const wxString & FileName)
{
	wxString szSourcePathFile = AssemblePath (DestTimePath, CrcPath);
	return AssemblePath (szSourcePathFile, PathFileName);
}

static inline wxString GetDirSourceRelaPath (const wxString & path) // 带有目录数据源最后一级目录的相对路径。
{
	int pos;
	if ((pos = path.Find ('\\')) != wxNOT_FOUND || (pos = path.Find ('/')) != wxNOT_FOUND) {
		return path.Mid (pos + 1);
	}
	
	return wxEmptyString;
}

class GmDupGetDataBackHandler : public GmNodeHandler
{
	GmUnitedIndexFile *				m_pUifFile;
	const vector<GmUifRootEntry *> &m_roots;
	const GmUifSourceEntry *		m_pEntry;
	GmWriter *						m_pWriter;
	GmHandleDupThread *				m_pThread;
	ACE_Message_Block				m_block;
	ubyte4							m_Options;
private:
	bool HandleNode (GmUifDirectory * pDir, const string & path)
	{
		if (!m_pWriter->CreateDir (ToWxString (path))) return false;
		if (pDir->pMySelf != 0) {
			GmLeafInfo info = *(pDir->pMySelf);
			m_pWriter->SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
		}
		return true;
	}

	bool HandleNode (GmSnapNode * pFile, const string & path)
	{
		wxString szFileName = ToWxString (GetNodeName (pFile));
		wxString szRelaPathFile = AssemblePath (ToWxString (path), szFileName);

		ubyte8 time = pFile->Time;
		GmUifRootEntry * pRoot = FromTimeToRootEntry (m_roots, time);
		if (pRoot == 0) return false;

		GmReader * pReader = m_pThread->GetJobReader (m_pUifFile, pRoot, m_pEntry);
		if (!RestoreDupFile (m_block, szRelaPathFile, pFile
							, m_pWriter, pReader, m_pThread->GetMonitor ())) return false;
		return true;
	}
public:
	GmDupGetDataBackHandler (GmUnitedIndexFile * pUifFile
							, GmWriter * pWriter
							, GmHandleDupThread * pThread
							, ubyte4 Options
							, const GmUifSourceEntry * entry)
		: GmNodeHandler ()
		, m_pUifFile (pUifFile)
		, m_roots (pUifFile->GetAllRootEntries ())
		, m_pEntry (entry)
		, m_pWriter (pWriter)
		, m_pThread (pThread)
		, m_block (READ_BLOCK_LEN)
		, m_Options (Options)
	{
	}
};

void GmHandleDupThread::MergeDataToOne (GmUnitedIndexFile & uif)
{
	const vector<GmUifRootEntry*> & roots = uif.GetAllRootEntries ();
	ubyte4 MergeTimes = m_pJob->Times ();
	if (roots.size () <= MergeTimes) return;

	vector<wxString> vszTimeFolders;

	typedef vector<GmUifRootEntry*>::const_iterator cit;
	for (cit begin = roots.begin (); begin != roots.end (); ++begin) {
		vszTimeFolders.push_back (GetTimeFolderName (wxDateTime (TimeTToWxDateTime ((*begin)->EntryTime))));
	}

	wxDateTime now;
	now.SetToCurrent ();

	GmWriter * pWriter = GetJobWriter (m_pJob);

	GmUifRootPairT FinalRootPair;
	GmAutoClearRootPairTree acur (FinalRootPair);
	MergeTree (FinalRootPair, uif);

	//
	// handle file data.
	//
	wxString szTimeFolder = GetTimeFolderName (now);
	
	GmMergeDupDataHandler handler (&uif, pWriter, m_pMonitor, szTimeFolder);
	for (size_t index = 0; index < FinalRootPair.second->size (); ++index) {
		GmUifRootPairT::second_type value = FinalRootPair.second;
		assert ((value->at (index)->first));
		const GmUifSourceEntry & entry = *(value->at (index)->first);
		string path = GetDestRelaPath (entry);

		TraverseTree (value->at (index)->second, &handler, path, GmCommHandleFileType ());
	}

	RemoveNodeNotFinished (*FinalRootPair.second);

	m_pMonitor->ClearAllFile ();
	m_pMonitor->HandleNewFile (szTimeFolder);

	uif.ClearUif ();
	AddTreeToUif (now, *FinalRootPair.second, uif);

	//
	// delete that directory.
	//
	m_pMonitor->HandleMessage (_("IDS_REMOVING_FILE"));
	vector<wxString> NotRemovedDirs = vszTimeFolders;
	try {
		for (size_t index = 0; index < vszTimeFolders.size (); ++index) {
			pWriter->RemoveDir (vszTimeFolders[index]);
			NotRemovedDirs.erase (NotRemovedDirs.begin ());
		}
	}
	catch (GmException &e) {
		wxString DirName;
		for (size_t index = 0; index < NotRemovedDirs.size (); ++index) {
			DirName += NotRemovedDirs[index];
			DirName += wxT (",");
		}
		DirName = EnclosureWithQuotMark (DirName);
		wxString message = wxString::Format (_("IDS_CANT_REMOVE_DIR")
											, DirName.c_str ()
											, e.GetMessage ().c_str ());
		if (m_pMonitor) m_pMonitor->HandleMessage (message);
	}

	return;
}


GmHandleDupThread::GmHandleDupThread (const GmJobItem * pJob, GmMonitor * pMonitor)
					: GmHandleDataThread (pJob, pMonitor)
{
}

GmHandleDupThread::GmHandleDupThread (const vector<GmStoreEntry*> & vpStoreNodes
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

void GmHandleDupThread::DoHandleData (GmUifSourceVectorT & vps, GmUnitedIndexFile * uif)
{
	wxDateTime now;
	now.SetToCurrent ();

	GmWriter * pWriter = GetJobWriter (m_pJob);
	wxString szTimeFolder = GetTimeFolderName (now);
	if (!pWriter->SetDestPath (AssemblePath (pWriter->GetDestPath (), szTimeFolder))) {
		wxString message = wxString::Format (_("IDS_CANT_CREATE_DIR"), szTimeFolder.c_str (), GetSystemError ().c_str ());
		throw GmException (message);
	}

	GmCopyCollector collector (pWriter, m_pMonitor);
	TraverseTheDirForCollector (vps, collector);
	m_pMonitor->HandleNewFile (szTimeFolder);
	return;
}

void GmHandleDupThread::GetDirDataBack (GmUifDirectory * pDir, const wxString & path, const GmStoreEntry * pEntry)
{
	GmUifRootStoreEntry * root = (GmUifRootStoreEntry*)pEntry->GetRootEntry ();
	GmUnitedIndexFile * pUifFile = root->GetUifFile ();
	const vector<GmUifRootEntry*> & roots = pUifFile->GetAllRootEntries ();
	GmUifRootEntry * pUifRoot = root->GetUifRoot ();

	const GmStoreEntry * ste = pEntry;
	while (ste->GetType () != GNT_SOURCE) ste = ste->GetParent ();
	const GmUifSourceEntry * entry = (GmUifSourceEntry*)(ste->GetEntry ());

	GmWriter * pWriter = GetDataBackWriter (pEntry, entry);
	//
	// 取得该目录对应的数据源存储的目录。
	//
	GmDupGetDataBackHandler handler (pUifFile, pWriter, this, m_Options, entry);
	TraverseTree (pDir, &handler, ToStlString (path), GmCommHandleFileType ());
}

void GmHandleDupThread::GetFileDataBack (GmSnapNode * pFile, const wxString & path, const GmStoreEntry * pEntry)
{
	const wxString & szRelaPath = path;
	GmUifRootStoreEntry * root = (GmUifRootStoreEntry*)pEntry->GetRootEntry ();
	GmUnitedIndexFile * pUifFile = root->GetUifFile ();
	GmUifRootEntry * pUifRoot = root->GetUifRoot ();

	const GmStoreEntry * ste = pEntry;
	while (ste->GetType () != GNT_SOURCE) ste = ste->GetParent ();
	const GmUifSourceEntry * entry = (GmUifSourceEntry*)(ste->GetEntry ());

	GmWriter * pWriter = GetDataBackWriter (pEntry, entry);

	wxString szFileName = ToWxString (GetNodeName (pFile));
	wxString szRelaPathFile = AssemblePath (szRelaPath, szFileName);

	string srcpath = entry->SourceName;
	wxString crcpath = GetCrc32String (srcpath.c_str (), srcpath.length ());
	ACE_Message_Block block (READ_BLOCK_LEN);
	
	GmReader * pReader = GetJobReader (pUifFile, pUifRoot, entry);
	RestoreDupFile (block, szRelaPathFile, pFile, pWriter, pReader, m_pMonitor);
}


////////////////////////////////////////////////////////////////////////////////////
// 同步任务。
//
GmHandleSyncThread::GmHandleSyncThread (const GmJobItem * pJob, GmMonitor * pMonitor)
					: GmHandleDupThread (pJob, pMonitor)
{
}

GmHandleSyncThread::GmHandleSyncThread (const vector<GmStoreEntry*> & vpStoreNodes
									, const wxString & szDestPath
									, int Option
									, const GmJobItem * pJob
									, GmMonitor * pMonitor)
									: GmHandleDupThread (vpStoreNodes
														, szDestPath
														, Option
														, pJob
														, pMonitor)
{
}


class GmSyncRemoveFilesHandler : public GmNodeHandler
{
	GmWriter &			m_Writer;
	GmMonitor *			m_pMonitor;

	bool HandleNode (GmUifDirectory * pDir, const string & path)
	{
		if (pDir->pMySelf == 0) return true;
		if (pDir->pMySelf->Flags & NODE_IN_TARGET_TREE) return true;
		wxString szPath = ToWxString (AssemblePath (path, GetNodeName (pDir)));
		if (!m_Writer.RemoveDir (szPath)) {
			wxString message = wxString::Format (_("IDS_CANT_REMOVE_DIR")
												, szPath.c_str ()
												, GetSystemError ().c_str ());
			if (m_pMonitor) m_pMonitor->HandleMessage (message);
		}
		return true;
	}

	bool HandleNode (GmSnapNode * pFile, const string & path)
	{
		if (pFile->pLeafNode->Flags & NODE_IN_TARGET_TREE) return true;
		wxString szFile = ToWxString (AssemblePath (path, GetNodeName (pFile)));
		if (!m_Writer.RemoveFile (szFile)) {
			wxString message = wxString::Format (_("IDS_CANT_REMOVE_DIR")
												, szFile.c_str ()
												, GetSystemError ().c_str ());
			if (m_pMonitor) m_pMonitor->HandleMessage (message);
		}
		return true;
	}
public:
	GmSyncRemoveFilesHandler (GmWriter & Writer, GmMonitor * pMonitor)
		: m_Writer (Writer)
		, m_pMonitor (pMonitor)
	{
	}
};

void GmHandleSyncThread::DoHandleDataTrees (const GmUifSourceVectorT & now
											, const GmUifSourceVectorT & former)
{
	if ((m_pJob->GetOptions () & GO_SYNC_REMOVED_FILE) == 0) return;
	//
	// 需要同步删除的文件，在 former 树中，被设置 NODE_IN_TARGET_TREE 的结点说明在源树中，不需要处理。
	//
	wxString dir = GetDestPath (*m_pJob);
	GmLocalWriter writer (m_pMonitor, 0);
	writer.SetDestPath (dir);

	GmSyncRemoveFilesHandler handler (writer, m_pMonitor);
	for (size_t index = 0; index < former.size (); ++index) {
		GmUifSourceVectorT::value_type source = former[index];
		assert (source);
		const GmUifSourceEntry & entry = *(source->first);
		string path = GetDestRelaPath (entry);

		TraverseTree (source->second, &handler, path, GmCommHandleFileType ());
	}
}

void GmHandleSyncThread::DoHandleData (GmUifSourceVectorT & vps, GmUnitedIndexFile * uif)
{
	wxDateTime now;
	now.SetToCurrent ();
	//
	// handle file data.
	//
	GmWriter * pWriter = GetJobWriter (m_pJob);
	m_pMonitor->HandleNewFile (pWriter->GetDestPath ());
	GmCopyCollector collector (pWriter, m_pMonitor);
	TraverseTheDirForCollector (vps, collector);
	RemoveNodeNotFinished (vps);
}

void GmHandleSyncThread::MergeDataToOne (GmUnitedIndexFile & uif)
{
	const vector<GmUifRootEntry*> & roots = uif.GetAllRootEntries ();
	if (roots.size () <= 1) return;

	vector<wxString> vszFiles;
	GetAllSetName (uif, vszFiles);

	wxDateTime now;
	now.SetToCurrent ();

	wxString dir = GetDestPath (*m_pJob);
	GmUifRootPairT FinalRootPair;
	GmAutoClearRootPairTree acur (FinalRootPair);
	MergeTree (FinalRootPair, uif);

	m_pMonitor->ClearAllFile ();
	m_pMonitor->HandleNewFile (dir);

	uif.ClearUif ();
	AddTreeToUif (now, *FinalRootPair.second, uif);

	return;
}
