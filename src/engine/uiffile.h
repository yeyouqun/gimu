//
// Definition for united index file format
// auther:yeyouqun@163.com
// 2009-11-8
//
//
#include <engine/gmiostream.h>


#ifndef __GIMU_UNITED_INDEX_FILE_H__
#define __GIMU_UNITED_INDEX_FILE_H__

class ENGINE_EXPORT GmUnitedIndexFile
{
public:
	//
	// 创建一个统一索引文件
	// @param szIndexFile	文件名，可以用不同的扩展名，程序关联的扩展名与应用相关。
	// @param pMonitor		处理统一索引文件时的监视器。
	// @excepton			不丢出异常。
	//
	GmUnitedIndexFile (const wxString & szIndexFile, GmMonitor * pMonitor);
	
	//
	// 取得某个索引文件的所有根索引入口。
	//
	const vector<GmUifRootEntry*> & GetAllRootEntries ();

	//
	// 新建一个新的根结点
	// @param Type					UIF 根类型，用户指定
	// @param EntryDataType			数据类型，用户指定
	//
	//
	void CreateRootEntry (GmRootEntryType Type, ubyte2 EntryDataType, ubyte2 TraverseMtd, ubyte8 Time);

	//
	// 结束一个根结点数据的添加
	// @param ExtendData			用户的扩展数据。
	//
	//
	//
	void EndRootEntry (const vector<char> & ExtendData = vector<char> ());

	//
	// 添加一个UIF数据源
	// @param Name				UIF 数据源的名字
	// @param Type				UIF数据源的类型，是路径，还是全文件名。
	// @param ExtendData		用户扩展数据。
	//
	void AddUifSource (const wxString & Name
						, GmSourceType Type
						, const vector<char> & ExtendData = vector<char> ());

	//
	// 添加UIF的叶子结点
	// @param NodeInfo			叶子结点的相关信息
	// @param ExtendData		叶子结点的扩展数据
	//
	void AddUifLeaf (const GmLeafInfo & NodeInfo, ubyte4 Flags = 0, const vector<char> & ExtendData = vector<char> ());

	//
	// 添加UIF的目录结点
	// @param NodeInfo			目录结点的相关信息，只取相关信息
	// @param ExtendData		目录结点的扩展数据
	//
	void AddUifDir (const GmLeafInfo & NodeInfo, ubyte4 Flags = 0, const vector<char> & ExtendData = vector<char> ());

	//
	// 结束UIF的目录结点，在添加完一个Dir结点后必须调用此接口，否则数据则无法正确写入及读取
	//
	void EndUifDir ();
	
	//
	//  添加数据集，如果没有数据集，则不需要调用。
	//
	void AddDataSet (GmDataSetEntry * pDataSet);

	//
	// 整理UIF文件
	//
	void Tidy ();

	//
	// 删除某个根结点及其所有数据，特别的的，在调用（1次，或者连续多次）本接口后，后续调用的接口
	// 只有 Tidy () ，否则其中的数据已经是不一致的了。
	//
	int RemoveUifRootEntry (size_t nIndex, vector<GmDataSetEntry*> & vpDataSet);

	//
	// 取得与根结点的相关信息，Uif数据源，Uif数据集，及扩展数据。
	//
	void GetAllUifSource (const GmUifRootEntry & Entry, vector<GmUifSourceEntry*> & vpSources);
	void GetAllUifDataSet (const GmUifRootEntry & Entry, vector<GmDataSetEntry*> & vpDataSet);
	void GetAllUifExtendData (const GmUifRootEntry & Entry, vector<char> & vcExtendData);
	//
	// 取得名字
	//
	const wxString & GetIndexFileName () const { return m_szIndexFile; }

	//
	// 取得Uif根结点树。
	//
	void GetUifRootTree (const GmUifRootEntry & Entry, GmUifRootPairT & tree);
	//
	// 析构函数
	//
	void Close ();
	void ClearUif ();
	~GmUnitedIndexFile ();
private:
	void CreateFirstTable (GmOutputStream & Writer);
	ubyte8 CreateNewTableAndChangeTableChain (ACE_Message_Block &, ubyte8 OldPos, GmOutputStream &);
	ubyte4 ReadTables (vector<pair<ubyte8, ACE_Message_Block *> > &);
	bool ReadUifRootEntryVector (vector<pair<ubyte8, ACE_Message_Block *> > * pTables);
	void OpenOrCreateUif ();
	void AddUifRootToTable (const GmUifRootEntry & Entry);
	void CreateEmptyTable (ACE_Message_Block & Block);
	void GetUifRootTable (ACE_Message_Block & Block, const GmUIFTableEntry & Entry, vector<GmUifRootEntry*> & Entries);
	void WriteTableToFile (GmOutputStream & Writer, ubyte4 pos, const ACE_Message_Block & Block);

	//
	//  读取树数据的接口。
	//
	template <int Value>
	void ReadTree (const GmUifRootEntry &
					, std::vector<GmUifSourcePairT*> &
					, vector<GmUifSourceEntry*> &
					, Int2Type<Value>);
	template <int Value>
	void ReadTree (size_t &
				, vector<ubyte4> &
				, GmUifDirectory *
				, const GmUifRootEntry & Entry
				, Int2Type<Value>);

	friend void HandleTheNextDirectory (GmUnitedIndexFile * pThis
								   , size_t & index
								   , vector<ubyte4> & blocks
								   , vector<GmUifDirectory*> & UNUSED (DirsNeedRead)
								   , GmUifDirectory* pNextDirectory
								   , const GmUifRootEntry & Entry
								   , Int2Type <TM_DEPTH_FIRST> i2t);

	friend void HandleTheseNextDirs (GmUnitedIndexFile * pThis
							   , vector<GmUifDirectory*> & DirsNeedRead
							   , size_t & index
							   , vector<ubyte4> &blocks
							   , const GmUifRootEntry & Entry
							   , Int2Type <TM_WIDTH_FIRST> i2t);
	void Finalize (); // must call at last or EndRootEntry ();
private:
	// data members.
	wxString				m_szIndexFile;
	GmMonitor				* m_pMonitor;
	ACE_Message_Block		m_DataBlock;
	vector<GmUifRootEntry*> m_pRootEntries;

	struct GmUifStatus
	{
		GmUifStatus () : pCurRootEntry (0)
		{
			LastTable.first = 0;
			LastTable.second = 0;
		}
		~GmUifStatus ()
		{
			Clear ();
		}

		void Clear ()
		{
			if (pCurRootEntry != 0)
				delete pCurRootEntry;
			pCurRootEntry = 0;

			if (Reader.IsOpened ())
				Reader.Close ();

			if (Writer.IsOpened ())
				Writer.Close ();

			for_each (vpSources.begin (), vpSources.end (), ReleaseMemory ());
			vpSources.clear ();
			for_each (vpDataSets.begin (), vpDataSets.end (), ReleaseMemory ());
			vpDataSets.clear ();

			if (LastTable.second != 0)
				delete LastTable.second;

			LastTable.second = 0;
			LastTable.first = 0;
		}

		GmInputStream						Reader;
		GmOutputStream						Writer;
		pair<ubyte8, ACE_Message_Block *>	LastTable;
		GmUifRootEntry *					pCurRootEntry;
		vector<GmUifSourceEntry*>			vpSources;
		vector<GmDataSetEntry*>				vpDataSets;
	}m_GmUifStatus;
};


void ENGINE_EXPORT AddTheseTreeToUifFile (GmUifSourceVectorT & trees
										, GmUnitedIndexFile & uif
										, GmRootEntryType Type
										, ubyte2 EntryDataType
										, ubyte2 TraverseMtd
										, ubyte8 Time
										, vector<GmDataSetEntry*> & vpDataSets = vector<GmDataSetEntry*> ()
										, vector<char> & ExtendData = vector<char> ());

//
// 这两个接口，必须是从后往前分析，即后备份的数据做为左边的参数，而之前备份的树做为后边的参数。
// 如果需要对多个时间点进行分析，则需要用之前生成的树做为左参数，而更早的一个时间树做为右参数传入分析。
// 最终生成一棵最新时间的备份树，通过这个时间树，可以进行备份时的分析。由于分析时，所有操作都只在内存中进行，
// 不在涉及到磁盘的读写，因此理论上这种分析方式将对整个执行产生较好的性能。（因为在产生系统当前树时，还是需要读取
// 磁盘来获取文件系统中文件的时间等数据。
//
inline const string & GetNodeName (GmSnapNode * pNode)
{
	return pNode->pLeafNode->LeafName;
}

inline const string & GetNodeName (GmUifDirectory* pNode)
{
	return pNode->pMySelf->DirName;
}

inline const string & GetNodeName (GmUifSourcePairT * pNode)
{
	return pNode->first->SourceName;
}

void MergeToLatestTree (GmUifSourceVectorT &, GmUifSourceVectorT &);
//
// 这两个接口，合并成生每个时间点的最新时间点树，这样可以集中取得在某个时间点可以取得的一致的文件数据，这里称为 Snap 树。
// 他的执行方法时，从最早的时间点开始分析，然后依次向更近的时间点进行分析。更近的时间点的分析以前一个最新
// 时间树做为右参数，而自己做为左参数传入分析。如果多个时间点同时参考到一个文件结点，则这个结点的指针将在多个
// 时间点中出现，因此，新生成的时间点在释放内存时，只释放在分析时新生成的目录项，则文件则不需要理会。真正
// 的资源管理应该让单时间树来进行管理。
//
void CopyToLatestTree (GmUifSourceVectorT &, const GmUifSourceVectorT &);
void CopyToLatestTree (GmUifRootPairT & left, const GmUifRootPairT & right);

inline ubyte4 GetNodeSize (GmSnapNode * pNode)
{
	return pNode->pLeafNode->FileSize;
}

inline ubyte4 GetNodeCompressSize (GmSnapNode * pNode)
{
	return pNode->pLeafNode->CompressSize;
}

void GetDirectoryInfo (const GmUifDirectory * pDirNode, GmDirectoryInfo & infos);
void GetUifRootTreeDirectoryInfo (const GmUifSourceVectorT & trees, GmDirectoryInfo & infos);

GmDirectoryInfo GetUifRootTreeDirectoryInfo (const GmUifRootPairT &);

void ClearEmptyUifTreeDirectory (vector<GmUifDirectory*> & vpSubDirs);
void ClearEmtpySourceTree (GmUifSourceVectorT & tree);
void ClearEmptyRootTreePair (GmUifRootPairT & RootTree);

GmUifRootEntry* FromTimeToRootEntry (const vector<GmUifRootEntry*> & roots, ubyte8 time);
void GetAllSetName (GmUnitedIndexFile & uif, vector<wxString> & vszSetName);
#endif //__GIMU_UNITED_INDEX_FILE_H__