//
// restore node base class
// Author:yeyuqun@163.com
// 2009-11-8
//
class GmWriter;
#ifndef __GM_RESTORE_NODE_H__
#define __GM_RESTORE_NODE_H__

enum GmRestNodeType {
	GNT_ROOT,
	GNT_SOURCE,
	GNT_DIR,
	GNT_FILE,
};

class GmHandleDataThread;

struct ENGINE_EXPORT GmStoreEntry
{
	GmStoreEntry (GmStoreEntry * parent, GmRestNodeType Type);
	virtual	~GmStoreEntry ();
	//
	// Entry Name
	//
	virtual wxString GetName (void) const = 0;
	//
	// SubNode Statistics.
	//
	virtual GmDirectoryInfo GetDirectoryInfo () const = 0;
	//
	// Get Node itself.
	//
	virtual const GmEntryBase * GetEntry () const = 0;
	//
	// Restore Node.
	//
	virtual void GetDataBack (GmHandleDataThread * pThread) const = 0;
	//
	// 取得以 GmLeafInfo 表示的结点信息。
	//
	virtual void GetLeafInfo (GmLeafInfo & info) const = 0;
public:
	//
	// No virtual function.
	//
	GmStoreEntry * GetParent () const { return m_pParent; }
	const vector<GmStoreEntry*> & Expand ();
	//
	// Entry Type
	//
	GmRestNodeType GetType (void) const { return m_Type; }
	const GmStoreEntry * GetRootEntry () const;
	enum PathType {
		PATH_FULL, // with file's name;
		PATH_DIR_PREFIX, // without file's name.
		PATH_SOURCE_PATH,
	};

	wxString GetPath (PathType type) const;
protected:
	//
	// SubNode
	//
	virtual void DoExpand (vector<GmStoreEntry*> & vpsubs) = 0;
	vector<GmStoreEntry*>		m_pSubNodes;
	GmStoreEntry *				m_pParent;
	GmRestNodeType				m_Type;
	bool						m_bExpanded;
};
/////////////////////////////////////////////////////////////
class ENGINE_EXPORT GmUifRootStoreEntry : public GmStoreEntry
{
	virtual wxString GetName (void) const;
	virtual void DoExpand (vector<GmStoreEntry*> & vpsubs);
	virtual GmDirectoryInfo GetDirectoryInfo () const;
	virtual const GmEntryBase * GetEntry () const;
	virtual void GetDataBack (GmHandleDataThread * pThread) const;
	virtual void GetLeafInfo (GmLeafInfo & info) const;
private:
	GmUifSourceVectorT *		m_pUifSourceVec;
	GmUifRootEntry *		m_root;
	GmUnitedIndexFile *		m_pUif;
public:
	GmUnitedIndexFile * GetUifFile () { return m_pUif; }
	GmUifRootEntry * GetUifRoot () { return m_root; };
	GmUifRootStoreEntry (GmUifRootPairT & rp, GmUnitedIndexFile * pUif);
	~GmUifRootStoreEntry ();
};

/////////////////////////////////////////////////////
class ENGINE_EXPORT GmDirStoreEntry : public GmStoreEntry
{
	virtual wxString GetName (void) const;
	virtual void DoExpand (vector<GmStoreEntry*> & vpsubs);
	virtual GmDirectoryInfo GetDirectoryInfo () const;
	virtual const GmEntryBase * GetEntry () const;
	virtual void GetDataBack (GmHandleDataThread * pThread) const;
	virtual void GetLeafInfo (GmLeafInfo & info) const;
	GmUifDirectory *		m_pUifDirSub;
	void ExpandSub (vector<GmStoreEntry*> & vpsubs, const GmUifDirectory * sp);
public:
	GmDirStoreEntry (GmUifDirectory * sp, GmStoreEntry *parent, GmRestNodeType Type = GNT_DIR);
	~GmDirStoreEntry ();
};

////////////////////////////////////////////////////////////////////////

class ENGINE_EXPORT GmUifSourceStoreEntry : public GmDirStoreEntry
{
	virtual wxString GetName (void) const;
	virtual const GmEntryBase * GetEntry () const;
private:
	GmUifSourceEntry *		m_pSource;
public:
	GmUifSourceStoreEntry (GmUifSourcePairT * sp, GmStoreEntry *parent);
	~GmUifSourceStoreEntry ();
};

///////////////////////////////////////////////
class ENGINE_EXPORT GmUifFileStoreEntry : public GmStoreEntry
{
	virtual wxString GetName (void) const;
	virtual void DoExpand (vector<GmStoreEntry*> & vpsubs);
	virtual GmDirectoryInfo GetDirectoryInfo () const;
	virtual const GmEntryBase * GetEntry () const;
	virtual void GetDataBack (GmHandleDataThread * pThread) const;
	virtual void GetLeafInfo (GmLeafInfo & info) const;
private:
	GmSnapNode *		m_pUifFile;
public:
	GmUifFileStoreEntry (GmSnapNode * sp, GmStoreEntry * parent);
	~GmUifFileStoreEntry ();
};

/////////////////////////////////////////////////////////////
class ENGINE_EXPORT GmZipDirStoreEntry : public GmStoreEntry
{
	virtual wxString GetName (void) const;
	virtual void DoExpand (vector<GmStoreEntry*> & vpsubs);
	virtual GmDirectoryInfo GetDirectoryInfo () const;
	virtual const GmEntryBase * GetEntry () const;
	virtual void GetDataBack (GmHandleDataThread * pThread) const;
	virtual void GetLeafInfo (GmLeafInfo & info) const
	{	info = *m_pDirectory->pMySelf; }
protected:
	void DoExpand (vector<GmStoreEntry*> & vpsubs, const GmZipDirectory *);
private:
	const GmZipDirectory *			m_pDirectory;
public:
	GmZipDirStoreEntry (const GmZipDirectory * pDirectory, GmStoreEntry * parent, GmRestNodeType Type = GNT_DIR);
	~GmZipDirStoreEntry ();
};
/////////////////////////////////////////////////////
class ENGINE_EXPORT GmZipFileStoreEntry : public GmStoreEntry
{
	virtual wxString GetName (void) const;
	virtual void DoExpand (vector<GmStoreEntry*> & vpsubs);
	virtual GmDirectoryInfo GetDirectoryInfo () const;
	virtual const GmEntryBase * GetEntry () const;
	virtual void GetDataBack (GmHandleDataThread * pThread) const;
	virtual void GetLeafInfo (GmLeafInfo & info) const
	{	info = *m_pCDSEntry; }
private:
	GmCDSEntry *			m_pCDSEntry;
public:
	GmZipFileStoreEntry (GmCDSEntry * pCDSEntry, GmStoreEntry * parent);
	~GmZipFileStoreEntry ();
};
/////////////////////////////////////////////////////////
class ENGINE_EXPORT GmZipRootStoreEntry : public GmZipDirStoreEntry
{
	virtual wxString GetName (void) const;
	virtual const GmEntryBase * GetEntry () const { return 0; }
	virtual void GetLeafInfo (GmLeafInfo & info) const {}
private:
	const ZipTreeT *			m_root;
	wxString					m_zipfile;
	auto_ptr<GmZipDistiller>	m_distiller;
public:
	GmZipDistiller * GetDistiller (GmMonitor * pMonitor) const;
	GmZipRootStoreEntry (const ZipTreeT * root, const wxString & zipfile);
	~GmZipRootStoreEntry ();
};
/////////////////////////////////////////////////////////
// Root tree creator.
//
class GmZipDistiller;
class ENGINE_EXPORT GmRootCreator
{
public:
	GmRootCreator (const wxString & filename);
	//
	// bSnapTree 只针对 UIF 文件有用。
	//
	const vector<GmStoreEntry*> & GetRoots (bool bSnapTree = false);
	~GmRootCreator ();
	bool HasSnapTree () { return m_bTimeTree; }
	bool IsZipTree () const { return m_uif.get () == 0 ? true : false; }
private:
	void GetUifRoots ();
	void CreateUifRoots ();
	void GetUifSnapRoots ();
	void CreateSnapRoots ();
	void GetZipRoots (GmZipDistiller & distiller);

	vector<GmStoreEntry*>	m_roots;
	//
	// uif file
	//
	auto_ptr<GmUnitedIndexFile> m_uif;
	vector<GmUifRootPairT>		m_vpRootTrees; // 单个时间点的树，用于Snap树，如果生成的是
												// Snap 树，这个树应该不为空，Snap 树的资源由GmStoreEntry 来释放
												// 而这个树需要自己释放，如果生成的就是单时间树，那么这个值应该为空
												// 其占用的资源由 GmStoreEntry 释放。
	//
	// zip file
	//
	wxString					m_filename;
	bool						m_bTimeTree;
};

#endif //__GM_RESTORE_NODE_H__