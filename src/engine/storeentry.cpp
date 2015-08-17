//
// restore node base class
// Author:yeyuqun@163.com
// 2010-3-25
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/monitor.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/uiffile.h>
#include <engine/zipblock.h>
#include <engine/writer.h>
#include <engine/zipfile.h>
#include <engine/storeentry.h>
#include <engine/execunit.h>
#include <engine/reader.h>

GmStoreEntry::GmStoreEntry (GmStoreEntry * parent, GmRestNodeType Type) 
						: m_pParent (parent)
						, m_Type (Type)
						, m_bExpanded (false)
{
}

GmStoreEntry::~GmStoreEntry ()
{
	GmAutoClearVector<GmStoreEntry> acvs (m_pSubNodes);
}

const vector<GmStoreEntry*> & GmStoreEntry::Expand ()
{
	if (!m_bExpanded) {
		DoExpand (m_pSubNodes);
		m_bExpanded = true;
	}

	return m_pSubNodes;
}

const GmStoreEntry * GmStoreEntry::GetRootEntry () const
{
	const GmStoreEntry * root = this;
	while (root->GetParent () != 0) root = root->GetParent ();
	return root;
}

wxString GmStoreEntry::GetPath (PathType type) const
{
	const GmStoreEntry * ste = this;
	if (ste->GetType () == GNT_ROOT) return wxEmptyString;

	wxString szFileName = ste->GetType () == GNT_SOURCE ? wxEmptyString : ste->GetName ();
	wxString szPrefix;
	if (ste->GetType () != GNT_SOURCE) {
		ste = ste->GetParent ();
		int nodetype = ste->GetType ();
		while (nodetype != GNT_SOURCE) {
			szPrefix = AssemblePath (ste->GetName (), szPrefix);
			ste = ste->GetParent ();
		}
	}

	const GmUifSourceEntry & entry = *((GmUifSourceEntry*)(ste->GetEntry ()));
	wxString szSrcPath;
	if (entry.SourceType == GST_DIRECTORY)
		szSrcPath = ToWxString (entry.SourceName);
	else
		szSrcPath = ToWxString (GetPathName (entry.SourceName));

	if (PATH_SOURCE_PATH == type) {
		return szSrcPath;
	}
	else if (PATH_DIR_PREFIX == type) {
		return szPrefix;
	}
	else if (PATH_FULL == type) {
		return AssemblePath (AssemblePath (szSrcPath, szPrefix), szFileName);
	}

	return wxEmptyString;
}

//////////////////////////////////////////////////////////////

GmUifRootStoreEntry::GmUifRootStoreEntry (GmUifRootPairT & rp
										, GmUnitedIndexFile * pUif) 
										: GmStoreEntry (0, GNT_ROOT)
										, m_pUifSourceVec (rp.second)
										, m_root (rp.first)
										, m_pUif (pUif)
{
}

void GmUifRootStoreEntry::GetLeafInfo (GmLeafInfo & info) const
{
	info.StoreTime = m_root->EntryTime;
}

GmUifRootStoreEntry::~GmUifRootStoreEntry ()
{
	ClearUifTree (m_pUifSourceVec);
	if (m_root) delete m_root;
}

wxString GmUifRootStoreEntry::GetName (void) const
{
	return wxDateTime (TimeTToWxDateTime (m_root->EntryTime)).Format ();
}

void GmUifRootStoreEntry::DoExpand (vector<GmStoreEntry*> & vpsubs)
{
	assert (vpsubs.empty ());

	for (size_t index = 0; index < m_pUifSourceVec->size (); ++index) {
		GmStoreEntry * pEntry = new GmUifSourceStoreEntry ((*m_pUifSourceVec)[index], this);
		vpsubs.push_back (pEntry);
	}
}

GmDirectoryInfo GmUifRootStoreEntry::GetDirectoryInfo () const
{
	GmDirectoryInfo info;

	GmUifRootPairT urp;
	urp.first = 0;
	urp.second = m_pUifSourceVec;
	info = GetUifRootTreeDirectoryInfo (urp);

	return info;
}

const GmEntryBase * GmUifRootStoreEntry::GetEntry () const
{
	return m_root;
}

void GmUifRootStoreEntry::GetDataBack (GmHandleDataThread * pThread) const
{
	const GmStoreEntry * pthis = this;
	const vector<GmStoreEntry*> & vpsubs = const_cast<GmStoreEntry*> (pthis)->Expand ();
	for (size_t index = 0; index < vpsubs.size (); ++index)
		vpsubs[index]->GetDataBack (pThread);
	return;
}

/////////////////////////////////////////////////
GmUifSourceStoreEntry::GmUifSourceStoreEntry (GmUifSourcePairT * sp
											  , GmStoreEntry *parent)
											  : GmDirStoreEntry (sp->second, parent, GNT_SOURCE)
											  , m_pSource (sp->first)
{
}

GmUifSourceStoreEntry::~GmUifSourceStoreEntry ()
{
	//
	// Do nothing.
	//
}

wxString GmUifSourceStoreEntry::GetName (void) const
{
	return ToWxString (m_pSource->SourceName);
}

const GmEntryBase * GmUifSourceStoreEntry::GetEntry () const
{
	return m_pSource;
}

////////////////////////////////////////////////
void GmDirStoreEntry::ExpandSub (vector<GmStoreEntry*> & vpsubs, const GmUifDirectory* sp)
{
	const vector<GmUifDirectory*> & dirs = sp->vpSubDirs;
	size_t size = dirs.size ();

	for (size_t index = 0; index < size; ++index) {
		vpsubs.push_back (new GmDirStoreEntry (dirs[index], this));
	}

	const vector<GmSnapNode*> & files = sp->vpSubFiles;
	size = files.size ();

	for (size_t index = 0; index < size; ++index) {
		vpsubs.push_back (new GmUifFileStoreEntry (files[index], this));
	}
}

void GmDirStoreEntry::DoExpand (vector<GmStoreEntry*> & vpsubs)
{
	assert (vpsubs.empty ());
	ExpandSub (vpsubs, m_pUifDirSub);
}

void GmDirStoreEntry::GetLeafInfo (GmLeafInfo & info) const
{
	const GmUifRootEntry & entry = *reinterpret_cast<const GmUifRootEntry*> (GetRootEntry ()->GetEntry ());
	if (m_pUifDirSub->pMySelf == 0) {
		info.StoreTime = entry.EntryTime;
	}
	else {
		info = *m_pUifDirSub->pMySelf;
		info.StoreTime = entry.EntryTime;
	}
	return;
}

GmDirectoryInfo GmDirStoreEntry::GetDirectoryInfo () const
{
	GmDirectoryInfo info;
	::GetDirectoryInfo (m_pUifDirSub, info);
	return info;
}
wxString GmDirStoreEntry::GetName (void) const
{
	return ToWxString (m_pUifDirSub->pMySelf->DirName);
}

const GmEntryBase * GmDirStoreEntry::GetEntry () const
{
	return m_pUifDirSub->pMySelf;
}

void GmDirStoreEntry::GetDataBack (GmHandleDataThread * pThread) const
{
	//
	// 如果是文件数据源的话，应该由下一级来恢复，虽然只有一个文件，但是保证了代码的一致性。
	//
	if (GNT_SOURCE == GetType () && ((GmUifSourceEntry*)GetEntry ())->SourceType == GST_FILE) {
		const GmStoreEntry * pthis = this;
		const vector<GmStoreEntry*> & vpsubs = const_cast<GmStoreEntry*> (pthis)->Expand ();
		for (size_t index = 0; index < vpsubs.size (); ++index)
			vpsubs[index]->GetDataBack (pThread);
	}
	else {
		//
		// 通过这个接口将不同的文件格式交由不同的文件处理器处理。
		//
		wxString pathsource = GetPath (PATH_DIR_PREFIX);
		pThread->GetDirDataBack (const_cast<GmDirStoreEntry*> (this)->m_pUifDirSub, pathsource, this);
	}

	return;
}

GmDirStoreEntry::GmDirStoreEntry (GmUifDirectory * sp
								  , GmStoreEntry *parent
								  , GmRestNodeType Type/* = GNT_DIR*/)
									: GmStoreEntry (parent, Type)
									, m_pUifDirSub (sp)
{
}

GmDirStoreEntry::~GmDirStoreEntry ()
{
	//
	// Do nothing now.
	//
}
////////////////////////////////////////////////
wxString GmUifFileStoreEntry::GetName (void) const
{
	return ToWxString (m_pUifFile->pLeafNode->LeafName);
}

void GmUifFileStoreEntry::GetLeafInfo (GmLeafInfo & info) const
{
	const GmUifRootEntry & entry = *reinterpret_cast<const GmUifRootEntry*> (GetRootEntry ()->GetEntry ());
	info = *m_pUifFile->pLeafNode;
	info.StoreTime = entry.EntryTime;
}

void GmUifFileStoreEntry::DoExpand (vector<GmStoreEntry*> & vpsubs)
{
	return;
}

GmDirectoryInfo GmUifFileStoreEntry::GetDirectoryInfo () const
{
	GmDirectoryInfo info;
	info.CompressSize = GetNodeCompressSize (m_pUifFile);
	info.Size = GetNodeSize (m_pUifFile);
	info.Files = 1;
	info.Dirs = 0;
	return info;
}

const GmEntryBase * GmUifFileStoreEntry::GetEntry () const
{
	return m_pUifFile->pLeafNode;
}

void GmUifFileStoreEntry::GetDataBack (GmHandleDataThread * pThread) const
{
	wxString pathsource = GetPath (PATH_DIR_PREFIX);
	pThread->GetFileDataBack (m_pUifFile, pathsource, this);
}

GmUifFileStoreEntry::GmUifFileStoreEntry (GmSnapNode * sp, GmStoreEntry * parent)
										: GmStoreEntry (parent, GNT_FILE)
										, m_pUifFile (sp)
{
}

GmUifFileStoreEntry::~GmUifFileStoreEntry ()
{
}

////////////////////////////////////////////////
wxString GmZipRootStoreEntry::GetName (void) const
{
	return GetFileName (m_zipfile);
}

GmZipDistiller * GmZipRootStoreEntry::GetDistiller (GmMonitor * pMonitor) const
{
	GmZipRootStoreEntry * pthis = const_cast <GmZipRootStoreEntry*> (this);
	if (pthis->m_distiller.get () == 0) pthis->m_distiller.reset (new GmZipDistiller (m_zipfile, pMonitor));
	return pthis->m_distiller.get ();
}

GmZipRootStoreEntry::GmZipRootStoreEntry (const ZipTreeT * root, const wxString & zipfile)
										: GmZipDirStoreEntry (root, 0, GNT_ROOT)
										, m_root (root)
										, m_zipfile (zipfile)
{
	assert (root);
}

GmZipRootStoreEntry::~GmZipRootStoreEntry ()
{
	delete m_root;
}
////////////////////////////////////////////////
wxString GmZipDirStoreEntry::GetName (void) const
{
	return ToWxString (m_pDirectory->PathName);
}

void GmZipDirStoreEntry::DoExpand (vector<GmStoreEntry*> & vpsubs)
{
	DoExpand (vpsubs, m_pDirectory);
}

void GmZipDirStoreEntry::DoExpand (vector<GmStoreEntry*> & vpsubs, const GmZipDirectory * pDirectory)
{
	assert (vpsubs.empty ());
	for (size_t index = 0; index < pDirectory->vpSubDirs.size (); ++index)
		vpsubs.push_back (new GmZipDirStoreEntry (pDirectory->vpSubDirs[index], this));
	for (size_t index = 0; index < pDirectory->vpSubFiles.size (); ++index)
		vpsubs.push_back (new GmZipFileStoreEntry (pDirectory->vpSubFiles[index], this));
}

GmDirectoryInfo GmZipDirStoreEntry::GetDirectoryInfo () const
{
	GmDirectoryInfo info;
	GetZipDirectoryInfo (m_pDirectory, info);
	return info;
}

const GmEntryBase * GmZipDirStoreEntry::GetEntry () const
{
	return m_pDirectory->pMySelf;
}

void GmZipDirStoreEntry::GetDataBack (GmHandleDataThread * pThread) const
{
	pThread->GetZipDirectoryDataBack (m_pDirectory, this);
}

GmZipDirStoreEntry::GmZipDirStoreEntry (const GmZipDirectory * pDirectory
										, GmStoreEntry * parent
										, GmRestNodeType Type/* = GNT_DIR*/)
						: GmStoreEntry (parent, Type)
						, m_pDirectory (pDirectory)
{
}

GmZipDirStoreEntry::~GmZipDirStoreEntry ()
{
	//
	// Do nothing now.
	//
}

////////////////////////////////////////////////
wxString GmZipFileStoreEntry::GetName (void) const
{
	return ToWxString (GetFileName (m_pCDSEntry->FileName));
}

void GmZipFileStoreEntry::DoExpand (vector<GmStoreEntry*> & vpsubs)
{
	return;
}

GmDirectoryInfo GmZipFileStoreEntry::GetDirectoryInfo () const
{
	GmDirectoryInfo info;
	info.CompressSize = m_pCDSEntry->CompressSize;
	info.Size = m_pCDSEntry->FileSize;
	info.Files = 1;
	info.Dirs = 0;
	return info;
}


const GmEntryBase * GmZipFileStoreEntry::GetEntry () const
{
	return m_pCDSEntry;
}

void GmZipFileStoreEntry::GetDataBack (GmHandleDataThread * pThread) const
{
	pThread->GetZipFileDataBack (m_pCDSEntry, this);
}

GmZipFileStoreEntry::GmZipFileStoreEntry (GmCDSEntry * pCDSEntry, GmStoreEntry * parent)
						: GmStoreEntry (parent, GNT_FILE)
						, m_pCDSEntry (pCDSEntry)
{
}

GmZipFileStoreEntry::~GmZipFileStoreEntry ()
{
	//
	// Do nothing now.
	//
}
//////////////////////////////////////////////
GmRootCreator::GmRootCreator (const wxString & filename)
						: m_filename (filename)
						, m_bTimeTree (false)
{
}

GmRootCreator::~GmRootCreator ()
{
	GmAutoClearVector<GmStoreEntry> acvr (m_roots);
	acvr.ReleaseNow ();
	for (size_t index = 0; index < m_vpRootTrees.size (); ++index) {
		ClearRootTree (&m_vpRootTrees[index]);
	}
}

const vector<GmStoreEntry*> & GmRootCreator::GetRoots (bool bSnapTree)
{
	wxFileName fn = wxFileName::FileName (m_filename);
	if (!fn.IsAbsolute ())
		return m_roots;

	if (m_roots.empty ()) {
		if (fn.GetExt ().CmpNoCase (wxT ("uif")) == 0) {
			if (bSnapTree) GetUifSnapRoots ();
			else GetUifRoots ();
			m_bTimeTree = true;
		}
		else if (fn.GetExt ().CmpNoCase (wxT ("zip")) == 0) {
			auto_ptr<GmZipDistiller> distiller (new GmZipDistiller (m_filename, 0));
			GetZipRoots (*distiller.get ());
			m_bTimeTree = false;
		}
		else if (fn.GetExt ().CmpNoCase (wxT ("7z")) == 0) {
			m_bTimeTree = false;
		}
	}

	return m_roots;
}

void GmRootCreator::CreateUifRoots ()
{
	assert (m_roots.empty ());

	if (m_uif.get () == 0) m_uif.reset (new GmUnitedIndexFile (m_filename, 0));

	const vector<GmUifRootEntry*> & items = m_uif->GetAllRootEntries ();
	for (size_t index = 0; index < items.size (); ++index) {
		GmUifRootPairT root;
		m_uif->GetUifRootTree (*items[index], root);
		//
		// 资源管理转交给 GmUifRootStoreEntry;
		//
		m_roots.push_back (new GmUifRootStoreEntry (root, m_uif.get ()));
	}
}

void GmRootCreator::GetUifRoots ()
{
	//
	// 空文件
	//
	if (m_uif.get () != 0 && m_roots.empty ())
		return;

	if (m_roots.empty ()) {
		CreateUifRoots ();
	}
}

void GmRootCreator::GetUifSnapRoots ()
{
	if (m_roots.empty ()) {
		CreateSnapRoots ();
	}
	return;
}

void GmRootCreator::CreateSnapRoots ()
{
	assert (m_roots.empty ());

	if (m_uif.get () == 0) m_uif.reset (new GmUnitedIndexFile (m_filename, 0));
	const vector<GmUifRootEntry*> & items = m_uif->GetAllRootEntries ();

	GmUifSourceVectorT * pPrevSnapTree = 0;
	for (size_t index = 0; index < items.size (); ++index) {
		GmUifRootPairT root;
		m_uif->GetUifRootTree (*items[index], root);
		m_vpRootTrees.push_back (root);

		GmUifRootPairT SnapTree;
		SnapTree.first = 0;
		SnapTree.second = 0;

		//
		// 行先复制本时间的Snap，再用这个Snap与之前的Snap进行合并。
		//
		CopyToLatestTree (SnapTree, root);
		if (pPrevSnapTree != 0) {
			CopyToLatestTree (*SnapTree.second, *pPrevSnapTree);
		}

		pPrevSnapTree = SnapTree.second;
		m_roots.push_back (new GmUifRootStoreEntry (SnapTree, m_uif.get ()));
	}
}

void GmRootCreator::GetZipRoots (GmZipDistiller & distiller)
{
	auto_ptr<ZipTreeT> tree (new ZipTreeT);
	distiller.GetFileTree (*tree.get ());
	//
	// 由 Root 来释放 Tree 资源。
	//
	m_roots.push_back (new GmZipRootStoreEntry (tree.release (), m_filename));
}
