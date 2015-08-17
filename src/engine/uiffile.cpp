//
// Definition for united index file format
// auther:yeyouqun@163.com
// 2009-11-8
//
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/exception.h>
#include <engine/uiffile.h>
#include <engine/monitor.h>

struct GmAutoClearTable
{
	vector<pair<ubyte8, ACE_Message_Block *> > & Tables;
	GmAutoClearTable (vector<pair<ubyte8, ACE_Message_Block *> > & Tables) : Tables (Tables) {}
	~GmAutoClearTable ()
	{
		for (size_t index = 0; index < Tables.size (); ++index)
			if (Tables[index].second != 0)
				delete Tables[index].second;

		Tables.clear ();
	}
};

GmUnitedIndexFile::GmUnitedIndexFile (const wxString & szIndexFile, GmMonitor * pMonitor)
					: m_szIndexFile (szIndexFile)
					, m_pMonitor (pMonitor)
					, m_DataBlock (UIF_DATA_BLOCK)
{
	m_szIndexFile.Replace (wxT ("\\\\"), wxT ("\\"));
	m_szIndexFile.Replace (wxT ("//"), wxT ("/"));
}

bool GmUnitedIndexFile::ReadUifRootEntryVector (vector<pair<ubyte8, ACE_Message_Block *> > * pTables)
{
	vector<pair<ubyte8, ACE_Message_Block *> > & Tables = *pTables;
	ReadTables (Tables);
	GmUIFTableEntry TableEntry;
	const ubyte4 ThreshHode = (UIF_TABLE_BLOCK - TableEntry.GetSize ()) / GmUifRootEntry ().GetSize ();

	bool bNeedNewTable = false;
	for (size_t index = 0; index < Tables.size (); ++index) {
		bNeedNewTable = false;
		ACE_Message_Block & Block = *Tables[index].second;
		ubyte4 Size = TableEntry.ReadEntry (Block.rd_ptr ());
		Block.rd_ptr (Size);
		GetUifRootTable (Block, TableEntry, m_pRootEntries);
		//
		// 最后一个表满才需要新表。
		//
		if (TableEntry.Entries >= ThreshHode)
			bNeedNewTable = true;
	}

	return bNeedNewTable;
}

const vector<GmUifRootEntry*> & GmUnitedIndexFile::GetAllRootEntries ()
{
	if (!wxFileName::FileExists (m_szIndexFile)) {
		for_each (m_pRootEntries.begin (), m_pRootEntries.end (), ReleaseMemory ());
		m_pRootEntries.clear ();
		return m_pRootEntries;
	}

	vector<pair<ubyte8, ACE_Message_Block *> > Tables;
	GmAutoClearTable act (Tables);

	if (m_pRootEntries.empty ()) {
		ReadUifRootEntryVector (&Tables);
	}
	
	return m_pRootEntries;
}

void GmUnitedIndexFile::CreateEmptyTable (ACE_Message_Block & Block)
{
	assert (Block.size () == UIF_TABLE_BLOCK);
	Block.reset ();
	GmUIFTableEntry entry;
	memset (&entry, 0, sizeof (entry));
	ubyte4 pos = entry.WriteEntry (Block.base ());
	Block.wr_ptr (pos);
	memset (Block.wr_ptr (), -1, Block.space ());
}

void GmUnitedIndexFile::GetUifRootTable (ACE_Message_Block & Block
										   , const GmUIFTableEntry & Entry
										   , vector<GmUifRootEntry*> & Entries)
{
	assert (Block.length () == UIF_TABLE_BLOCK - Entry.GetSize ());
	for (ubyte4 index = 0; index < Entry.Entries; ++index) {
		ubyte4 signature;
		ubyte2 EntryType;

		CharsToUByte4 (Block.rd_ptr (), &signature);
		if (signature == -1) break;
		CharsToUByte2 (Block.rd_ptr () + sizeof (signature), &EntryType);
		if (EntryType == ROOT_ENTRY_REMOVED) {
			Block.rd_ptr (GmUifRootEntry().GetSize ());
			continue;
		}

		GmUifRootEntry * pRoot = new GmUifRootEntry;
		ubyte4 EntrySize = pRoot->ReadEntry (Block.rd_ptr ());
		Block.rd_ptr (EntrySize);
		Entries.push_back (pRoot);
	}
}

void GmUnitedIndexFile::WriteTableToFile (GmOutputStream & Writer, ubyte4 pos, const ACE_Message_Block & Block)
{
	assert (Block.size () == UIF_TABLE_BLOCK);
	assert (Writer.IsOpened ());
	Writer.Seek (pos, wxFromStart);
	Writer.Write (Block.base (), SIZET_TO_UBYTE4 (Block.size ()));
	Writer.Flush ();
}

ubyte8 GmUnitedIndexFile::CreateNewTableAndChangeTableChain (ACE_Message_Block & LastTable
														   , ubyte8 OldPos
														   , GmOutputStream & Writer)
{
	assert (Writer.IsOpened ());

	//
	// 更改表值并写入表
	//
	GmUIFTableEntry TableEntry;
	TableEntry.ReadEntry (LastTable.base ());
	TableEntry.NextTable = Writer.SeekEnd ();
	TableEntry.WriteEntry (LastTable.base ());

	Writer.Seek (OldPos, wxFromStart);
	Writer.Write (LastTable.base (), SIZET_TO_UBYTE4 (LastTable.size ()));
	Writer.Flush ();

	Writer.SeekEnd ();
	ubyte8 NewPos = Writer.Tell ();
	LastTable.reset ();

	CreateEmptyTable (LastTable);
	WriteTableToFile (Writer, NewPos, LastTable);
	return NewPos;
}

void GmUnitedIndexFile::CreateFirstTable (GmOutputStream & Writer)
{
	GmUIFEntry Entry;
	char Buffer[GmUIFEntry::UIF_HEAD_LEN];
	Entry.WriteEntry ((char*)Buffer);
	Writer.Write ((char*)Buffer, GmUIFEntry::UIF_HEAD_LEN);
	m_GmUifStatus.LastTable.first = GmUIFEntry::UIF_HEAD_LEN;
	if (m_GmUifStatus.LastTable.second == 0)
		m_GmUifStatus.LastTable.second = new ACE_Message_Block (UIF_TABLE_BLOCK);

	m_GmUifStatus.LastTable.second->reset ();
	CreateEmptyTable (*m_GmUifStatus.LastTable.second);
	WriteTableToFile (Writer, m_GmUifStatus.LastTable.first, *m_GmUifStatus.LastTable.second);
}

void GmUnitedIndexFile::OpenOrCreateUif ()
{
	GmOutputStream & Writer = m_GmUifStatus.Writer;
	assert (!Writer.IsOpened ());

	if (!wxFileName::FileExists (m_szIndexFile)) {
		wxFile (m_szIndexFile, wxFile::write_append);
		Writer.Open (m_szIndexFile.c_str ());
		CreateFirstTable (Writer);
		return;
	}
	else if (wxFile (m_szIndexFile).Length () == 0) {
		Writer.Open (m_szIndexFile.c_str ());
		CreateFirstTable (Writer);
	}

	vector<pair<ubyte8, ACE_Message_Block *> > Tables;
	GmAutoClearTable act (Tables);

	bool bNewTable = ReadUifRootEntryVector (&Tables);
	if (!Writer.IsOpened ())
		Writer.Open (m_szIndexFile.c_str ());

	//
	// 将最后一个表保留下来，以便随时更新最近的一个 UIF 表。
	//
	m_GmUifStatus.LastTable.first = Tables.back ().first;
	if (m_GmUifStatus.LastTable.second == 0) {
		m_GmUifStatus.LastTable.second = Tables.back ().second;
		Tables.pop_back ();
	}
	else {
		ACE_Message_Block & src = *Tables.back ().second;
		ACE_Message_Block & dest = *m_GmUifStatus.LastTable.second;

		//
		// 将源的内存块的内部位置也复制过来。
		//
		dest.copy (src.base (), src.size ());
		dest.rd_ptr (dest.base () + (src.rd_ptr () - src.base ()));
		dest.wr_ptr (dest.base () + (src.wr_ptr () - src.base ()));
	}

	if (bNewTable)  {
		//
		// 改写前一个表的链表值。
		//
		m_GmUifStatus.LastTable.first = CreateNewTableAndChangeTableChain (*m_GmUifStatus.LastTable.second
																	, m_GmUifStatus.LastTable.first, Writer);
	}
}

void GmUnitedIndexFile::CreateRootEntry (GmRootEntryType Type
										 , ubyte2 EntryDataType
										 , ubyte2 TraverseMtd
										 , ubyte8 Time)
{
	Finalize ();
	assert (m_GmUifStatus.pCurRootEntry == 0);
	m_GmUifStatus.pCurRootEntry = new GmUifRootEntry;
	GmUifRootEntry & root = *m_GmUifStatus.pCurRootEntry;
	OpenOrCreateUif ();
	root.EntryType = Type;
	root.EntryDataType = EntryDataType;
	root.EntryTime = Time;
	root.TraverseMtd = TraverseMtd;
	m_GmUifStatus.Writer.SeekEnd ();
	root.DataOffset = m_GmUifStatus.Writer.Tell ();
}

template <typename WriteType>
static ubyte4 WriteDataToFile (const vector<WriteType*> & vData, GmOutputStream & Writer)
{
	ubyte4 DataLen = 0;
	for (size_t index = 0; index < vData.size (); ++index)
		DataLen += vData[index]->GetSize ();

	if (DataLen != 0) {
		ACE_Message_Block block (DataLen);
		for (size_t index = 0; index < vData.size (); ++index) {
			ubyte4 length = vData[index]->WriteEntry (block.wr_ptr ());
			block.wr_ptr (length);
		}

		Writer.Write (block.rd_ptr (), SIZET_TO_UBYTE4 (block.length ()));
	}

	return DataLen;
}

static void WriteSourceSetExtendDataToFile (GmUifRootEntry & root
											, vector<GmUifSourceEntry*> &vpSources
											, vector<GmDataSetEntry*> & vpDataSets
											, const vector<char> & ExtendData
											, GmOutputStream & Writer)
{
	root.SourceNum = static_cast<ubyte2> (vpSources.size ());
	root.SourceOffset = Writer.Tell ();
	root.SourceDataLen = WriteDataToFile (vpSources, Writer);

	root.DataSetNum = static_cast<ubyte2> (vpDataSets.size ());
	root.DataSetOffset = Writer.Tell ();
	root.DataSetDataLen = WriteDataToFile (vpDataSets, Writer);
	//
	// 写数据集
	//
	root.ExtendDataLen = static_cast<ubyte2> (ExtendData.size ());
	root.ExtendDataOffset = Writer.Tell ();
	if (ExtendData.size () > 0)
		Writer.Write (&ExtendData[0], SIZET_TO_UBYTE4 (ExtendData.size ()));

	return;
}

void GmUnitedIndexFile::EndRootEntry (const vector<char> & ExtendData)
{
	//
	// 多次调用 EndRootEntry 时可以。
	//
	if (m_GmUifStatus.pCurRootEntry == 0)
		return;

	GmOutputStream & Writer = m_GmUifStatus.Writer;
	assert (m_GmUifStatus.vpSources.size () > 0);
	assert (m_GmUifStatus.Writer.IsOpened ());

	if (m_GmUifStatus.vpSources.size () != 0 && m_DataBlock.length () > 0) {
		//
		// 将数据块添加到上一个数据源的块数据中。
		//
		ubyte4 Length = SIZET_TO_UBYTE4 (m_DataBlock.length ());
		m_GmUifStatus.Writer.Write (m_DataBlock.rd_ptr (), Length);
		m_GmUifStatus.vpSources.back ()->SourceBlocks.push_back (Length);
		m_DataBlock.reset ();
	}
	
	GmUifRootEntry & root = *m_GmUifStatus.pCurRootEntry;
	//
	// 写数据源。
	//
	WriteSourceSetExtendDataToFile (root, m_GmUifStatus.vpSources
									, m_GmUifStatus.vpDataSets, ExtendData, Writer);

	//
	// 将Root项写入表中。
	//
	GmUIFTableEntry TableEntry;
	TableEntry.ReadEntry (m_GmUifStatus.LastTable.second->base ());
	ubyte4 pos = TableEntry.GetSize ();
	pos += (TableEntry.Entries) * (m_GmUifStatus.pCurRootEntry->GetSize ());
	m_GmUifStatus.pCurRootEntry->WriteEntry (m_GmUifStatus.LastTable.second->base () + pos);

	TableEntry.Entries ++;
	TableEntry.WriteEntry (m_GmUifStatus.LastTable.second->base ());

	Writer.Seek (m_GmUifStatus.LastTable.first, wxFromStart);
	Writer.Write (m_GmUifStatus.LastTable.second->base (), SIZET_TO_UBYTE4 (m_GmUifStatus.LastTable.second->size ()));
	m_pRootEntries.push_back (m_GmUifStatus.pCurRootEntry);
	Finalize ();
}

void GmUnitedIndexFile::AddUifSource (const wxString & Name
									, GmSourceType Type
									, const vector<char> & ExtendData/* = vector<char> ()*/)
{
	assert (m_GmUifStatus.Writer.IsOpened ());
	vector<GmUifSourceEntry*> & sources = m_GmUifStatus.vpSources;
	if (sources.size () != 0 && m_DataBlock.length () > 0) {
		//
		// 将数据块添加到上一个数据源的块数据中。
		//
		ubyte4 Length = SIZET_TO_UBYTE4 (m_DataBlock.length ());
		m_GmUifStatus.Writer.Write (m_DataBlock.rd_ptr (), Length);
		sources.back ()->SourceBlocks.push_back (Length);
		m_DataBlock.reset ();
	}

	GmUifSourceEntry * pSource = new GmUifSourceEntry;
	sources.push_back (pSource);

	pSource->vcExtendData = ExtendData;
	pSource->SourceType = Type;
	pSource->SourceName = ToStlString (Name);
	m_GmUifStatus.Writer.SeekEnd ();
	pSource->DataOffset = m_GmUifStatus.Writer.Tell ();
}

template <typename NodeType>
static inline void AddNodeToBlock (NodeType & Node
								   , ACE_Message_Block & Block
								   , GmUifSourceEntry & source
								   , GmOutputStream & Writer)
{
	ubyte4 Length = Node.GetSize ();
	if (Block.space () < Length) {
		Length = SIZET_TO_UBYTE4 (Block.length ());
		Writer.Write (Block.rd_ptr (), Length);
		source.SourceBlocks.push_back (Length);
		Block.reset ();
	}

	Length = Node.WriteEntry (Block.wr_ptr ());
	Block.wr_ptr (Length);
}

void GmUnitedIndexFile::AddUifLeaf (const GmLeafInfo & NodeInfo
									, ubyte4 Flags/* = 0*/
									, const vector<char> & ExtendData/* = vector<char> ()*/)
{
	if (m_GmUifStatus.vpSources.empty () || m_GmUifStatus.pCurRootEntry == 0)
		return;

	assert (m_GmUifStatus.Writer.IsOpened ());
	GmUifSourceEntry & source = *m_GmUifStatus.vpSources.back ();

	GmLeafEntry Node;
	Node.Flags = Flags;
	Node.Attribute		= NodeInfo.Attribute;
	Node.StartDataSet	= NodeInfo.StartDataSet;
	Node.StartOffset	= NodeInfo.StartOffset;
	Node.FileSize		= NodeInfo.FileSize;
	Node.CompressSize	= NodeInfo.CompressSize;
	Node.CreateTime		= NodeInfo.CreateTime;
	Node.ModifyTime		= NodeInfo.ModifyTime;
	Node.AccessTime		= NodeInfo.AccessTime;
	Node.Extent			= NodeInfo.Extent;
	Node.LeafName		= ToStlString (NodeInfo.Name);
	Node.vcExtendData	= ExtendData;

	AddNodeToBlock (Node, m_DataBlock, source, m_GmUifStatus.Writer);
}

void GmUnitedIndexFile::AddUifDir (const GmLeafInfo & NodeInfo
								   , ubyte4 Flags/* = 0*/
								   , const vector<char> & ExtendData/* = vector<char> ()*/)
{
	if (m_GmUifStatus.vpSources.empty () || m_GmUifStatus.pCurRootEntry == 0)
		return;

	assert (m_GmUifStatus.Writer.IsOpened ());
	GmUifSourceEntry & source = *m_GmUifStatus.vpSources.back ();

	GmUifDirEntry Node;
	Node.Flags			= Flags;
	Node.Attribute		= NodeInfo.Attribute;
	Node.CreateTime		= NodeInfo.CreateTime;
	Node.ModifyTime		= NodeInfo.ModifyTime;
	Node.AccessTime		= NodeInfo.AccessTime;
	Node.DirName		= ToStlString (NodeInfo.Name);
	Node.vcExtendData	= ExtendData;

	AddNodeToBlock (Node, m_DataBlock, source, m_GmUifStatus.Writer);
}

void GmUnitedIndexFile::EndUifDir ()
{
	if (m_GmUifStatus.vpSources.empty () || m_GmUifStatus.pCurRootEntry == 0)
		return;

	assert (m_GmUifStatus.Writer.IsOpened ());
	GmUifSourceEntry & source = *m_GmUifStatus.vpSources.back ();

	GmUifEndDirEntry Node;
	AddNodeToBlock (Node, m_DataBlock, source, m_GmUifStatus.Writer);
}

void GmUnitedIndexFile::AddDataSet (GmDataSetEntry * pDataSet)
{
	if (m_GmUifStatus.vpSources.empty () || m_GmUifStatus.pCurRootEntry == 0)
		return;
	assert (m_GmUifStatus.Writer.IsOpened ());
	m_GmUifStatus.vpDataSets.push_back (pDataSet);
}

void GmUnitedIndexFile::Tidy ()
{
	m_GmUifStatus.Writer.Close ();
	Close ();
	if (m_pRootEntries.empty ()) {
		vector<pair<ubyte8, ACE_Message_Block *> > Tables;
		GmAutoClearTable act (Tables);
		ReadUifRootEntryVector (&Tables);
	}

	if (m_pRootEntries.size () == 0)
		return;

	
	//
	// 迫使重新读取UIF根项，以反映真实变化。
	//
	GmAutoClearVector<GmUifRootEntry> acvr (m_pRootEntries);
	time_t time = wxDateTime::GetTimeNow ();
	wxString szTempUif = wxULongLong (time).ToString ();
	wxString Path = GetPathName (m_szIndexFile);
	szTempUif = AssemblePath (Path, szTempUif);

	GmInputStream & Reader = m_GmUifStatus.Reader;
	if (!Reader.IsOpened () && !Reader.Open (m_szIndexFile.c_str ())) {
		wxString message (_("IDS_OPEN_FILE_FAILED"));
		wxString ThisMessage = wxString::Format (message.c_str ()
												, m_szIndexFile.c_str ()
												, wxSysErrorMsg (wxSysErrorCode ()));
		throw GmException (ThisMessage);
	}

	const ubyte4 ThreshHode = (UIF_TABLE_BLOCK - GmUIFTableEntry ().GetSize ()) / GmUifRootEntry ().GetSize ();
	try {
		if (wxFile::Exists (szTempUif)) {
			wxRemoveFile (szTempUif);
		}

		wxFile (szTempUif, wxFile::write_append);
		GmOutputStream Writer;
		Writer.Open (szTempUif);
		CreateFirstTable (Writer);

		GmUIFTableEntry TableEntry;
		pair<ubyte8, ACE_Message_Block *> & LastTable = m_GmUifStatus.LastTable;
		ACE_Message_Block & DataBlock = m_DataBlock;

		LastTable.second->reset ();
		LastTable.second->wr_ptr (TableEntry.GetSize ());
		DataBlock.reset ();

		ubyte4 TotalEntries = 0;
		for (size_t index = 0; index < m_pRootEntries.size (); ++index) {
			vector<GmUifSourceEntry*> vpSources;
			vector<GmDataSetEntry*> vpDataSets;
			GmAutoClearVector<GmUifSourceEntry> acvs (vpSources);
			GmAutoClearVector<GmDataSetEntry> acvd (vpDataSets);
			vector<char> vcExtendData;
			GmUifRootEntry & RootEntry = *m_pRootEntries[index];
			GetAllUifSource (RootEntry, vpSources);
			GetAllUifDataSet (RootEntry, vpDataSets);
			GetAllUifExtendData (RootEntry, vcExtendData);

			GmUifRootEntry NewRootEntry = RootEntry;
			++TableEntry.Entries;
			++TotalEntries;
			
			ubyte8 Pos = Writer.SeekEnd ();
			NewRootEntry.DataOffset = Pos;

			for (size_t si = 0; si < vpSources.size (); ++si) {
				Pos = Writer.Tell ();
				const vector<ubyte4> & SourceBlocks = vpSources[si]->SourceBlocks;
				Reader.Seek (vpSources[si]->DataOffset, wxFromStart);
				vpSources[si]->DataOffset = Pos;//New Position

				for (size_t sib = 0; sib < SourceBlocks.size (); ++sib) {
					ubyte4 Length = SourceBlocks[sib];
					ubyte4 BytesRead = Reader.Read (DataBlock.base (), Length);
					if (BytesRead != Length)
						ThrowReadError (m_szIndexFile, Length, BytesRead);

					Writer.Write (DataBlock.base (), Length);
				}
			}

			WriteSourceSetExtendDataToFile (NewRootEntry, vpSources, vpDataSets, vcExtendData, Writer);
			ubyte4 Size = NewRootEntry.WriteEntry (LastTable.second->wr_ptr ());
			LastTable.second->wr_ptr (Size);

			if ((TableEntry.Entries % ThreshHode) == 0 && // 表项超临界值
				TotalEntries < m_pRootEntries.size ()) { // 并且其数量不会刚好相等，就不会有多余的空表产生。
				//
				// 需要别一个表。
				//
				TableEntry.NextTable = Writer.Tell ();
				TableEntry.WriteEntry (LastTable.second->base ());
				ubyte4 NewPos = CreateNewTableAndChangeTableChain (*LastTable.second, LastTable.first, Writer);
				LastTable.first = NewPos;
				//
				// 重置。
				//
				TableEntry.Entries = 0;
				TableEntry.NextTable = 0;
				LastTable.second->reset ();
				LastTable.second->wr_ptr (TableEntry.GetSize ());
			}
		}
		//
		// 将最后一个表写入文件。
		//
		TableEntry.WriteEntry (LastTable.second->base ());
		WriteTableToFile (Writer, LastTable.first, *LastTable.second);
	}
	catch (GmException &) {
		//
		// 如果中途出现错误，整理不成功，因此整理要么成功，要么失败。
		//
		wxRemoveFile (szTempUif);
		throw;
	}

	Reader.Close ();
	wxRemoveFile (m_szIndexFile);
	wxRenameFile (szTempUif, m_szIndexFile);
	return;
}

ubyte4 GmUnitedIndexFile::ReadTables (vector<pair<ubyte8, ACE_Message_Block *> > &Tables)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;

	if (!Reader.IsOpened () && !Reader.Open (m_szIndexFile.c_str ())) {
		wxString message (_("IDS_OPEN_FILE_FAILED"));
		wxString ThisMessage = wxString::Format (message.c_str ()
												, m_szIndexFile.c_str ()
												, wxSysErrorMsg (wxSysErrorCode ()));
		throw GmException (ThisMessage);
	}

	char Buffer[GmUIFEntry::UIF_HEAD_LEN];
	Reader.Seek (0, wxFromStart);
	ubyte4 ReadBytes = Reader.Read (Buffer, GmUIFEntry::UIF_HEAD_LEN);
	if (ReadBytes != GmUIFEntry::UIF_HEAD_LEN)
		ThrowReadError (m_szIndexFile, GmUIFEntry::UIF_HEAD_LEN, ReadBytes);

	GmUIFEntry UifEntry;
	UifEntry.ReadEntry (Buffer);
	if (UifEntry.Signature != SG_UIF_ENTRY) {
		throw GmException (_("IDS_UNRECOGNIZIBLE_UIF"));
	}

	if (UifEntry.Version > CUR_VERSION) {
		throw GmException (_("IDS_UNSUPPORT_UIF_VERSION"));
	}

	ubyte8 TablePos = GmUIFEntry::UIF_HEAD_LEN;
	for (;;) {
		GmUIFTableEntry Table;
		pair<ubyte8, ACE_Message_Block *> EntryTable;
		EntryTable.first = TablePos;
		EntryTable.second = new ACE_Message_Block (UIF_TABLE_BLOCK);
		ACE_Message_Block & Block = *EntryTable.second;
		Tables.push_back (EntryTable);

		ReadBytes = Reader.Read (Block.base (), UIF_TABLE_BLOCK);
		if (ReadBytes != UIF_TABLE_BLOCK)
			ThrowReadError (m_szIndexFile, UIF_TABLE_BLOCK, ReadBytes);

		Block.wr_ptr (ReadBytes);
		ubyte4 Size = Table.ReadEntry (Block.rd_ptr ());
		if (Table.NextTable == 0)
			break;
		TablePos = Table.NextTable;
		Reader.Seek (Table.NextTable, wxFromStart);
	}

	Reader.Close ();
	return (ubyte4)Tables.size ();
}

template <typename FileIOType>
struct GmAutoCloseFile
{
	FileIOType & FileIo;
	GmAutoCloseFile (FileIOType & FileIo) : FileIo (FileIo) {}
	~GmAutoCloseFile () { FileIo.Close (); }
};

int GmUnitedIndexFile::RemoveUifRootEntry (size_t nIndex, vector<GmDataSetEntry*> & vpDataSet)
{
	assert (!m_GmUifStatus.Writer.IsOpened ());
	assert (!m_GmUifStatus.Reader.IsOpened ());
	GmAutoCloseFile<GmOutputStream> acfo (m_GmUifStatus.Writer);
	GmAutoCloseFile<GmInputStream> acfi (m_GmUifStatus.Reader);

	GmOutputStream & Writer = m_GmUifStatus.Writer;
	if (nIndex >= m_pRootEntries.size ())
		return 1;

	GmUifRootEntry root = *m_pRootEntries[nIndex];
	GetAllUifDataSet (root, vpDataSet);

	vector<pair<ubyte8, ACE_Message_Block *> > Tables;
	GmAutoClearTable act (Tables);
	ReadTables (Tables);
	const ubyte4 ThreshHode = (UIF_TABLE_BLOCK - GmUIFTableEntry ().GetSize ()) / GmUifRootEntry ().GetSize ();

	for (size_t index = 0; index < Tables.size (); ++index) {
		GmUIFTableEntry TableEntry;
		pair<ubyte8, ACE_Message_Block *> & Table = Tables[index];
		ACE_Message_Block & Block = *Table.second;
		ubyte4 Size = TableEntry.ReadEntry (Block.rd_ptr ());
		Block.rd_ptr (Size);
		for (ubyte4 RootIndex = 0; RootIndex < TableEntry.Entries; ++RootIndex) {
			GmUifRootEntry RootEntry;
			ubyte4 EntrySize = RootEntry.ReadEntry (Block.rd_ptr ());
			if (RootEntry.EntryType == ROOT_ENTRY_REMOVED)
				continue;

			if (RootEntry == root) {
				//
				// 将整个表写入文件。
				//
				RootEntry.EntryType = ROOT_ENTRY_REMOVED;
				RootEntry.WriteEntry (Block.rd_ptr ());
				if (!Writer.Open (m_szIndexFile.c_str ()))
					return 0;

				Writer.Seek (Table.first, wxFromStart);
				Writer.Write (Block.base (), SIZET_TO_UBYTE4 (Block.size ()));
				return 1;
			}
			else {
				Block.rd_ptr (EntrySize);
			}
		}
	}

	return 0;
}

struct GetNodeInfo
{
	const GmUifRootEntry & Entry;
	GetNodeInfo (const GmUifRootEntry & Entry) : Entry (Entry) {}
	template<typename DataType>
	ubyte8 GetOffset ();
	template<typename DataType>
	ubyte4 GetLength ();
	template<typename DataType>
	bool GetData(ACE_Message_Block & Block, vector<DataType> &vpDataSet);
	template<typename DataType>
	ubyte4 GetNumber ();

	//
	// 特化实现。
	//
	template<>
	ubyte8 GetOffset<char> ()
	{
		return Entry.ExtendDataOffset;
	}
	template<>
	ubyte4 GetLength<char> ()
	{
		return Entry.ExtendDataLen;
	}
	template<>
	bool GetData<char> (ACE_Message_Block & Block, vector<char> &vpDataSet)
	{
		vpDataSet.assign (Block.base (), Block.base () + Block.size ());
		return true;
	}

	template<>
	ubyte4 GetNumber<char> ()
	{
		return Entry.ExtendDataLen;
	}
	//===========
	template<>
	ubyte8 GetOffset<GmUifSourceEntry*> ()
	{
		return Entry.SourceOffset;
	}
	template<>
	ubyte4 GetLength<GmUifSourceEntry*> ()
	{
		return Entry.SourceDataLen;
	}
	template<>
	bool GetData<GmUifSourceEntry*> (ACE_Message_Block & Block, vector<GmUifSourceEntry*> &vpSources)
	{
		GmUifSourceEntry * pSource = new GmUifSourceEntry;
		vpSources.push_back (pSource);
		ubyte4 Len = pSource->ReadEntry (Block.rd_ptr ());
		Block.rd_ptr (Len);
		return false;
	}
	template<>
	ubyte4 GetNumber<GmUifSourceEntry*> ()
	{
		return Entry.SourceNum;
	}
	//===========
	template<>
	ubyte8 GetOffset<GmDataSetEntry*> ()
	{
		return Entry.DataSetOffset;
	}
	template<>
	ubyte4 GetLength<GmDataSetEntry*> ()
	{
		return Entry.DataSetDataLen;
	}
	template<>
	bool GetData<GmDataSetEntry*> (ACE_Message_Block & Block, vector<GmDataSetEntry*> &vpDataSet)
	{
		GmDataSetEntry * pDataSet = new GmDataSetEntry;
		vpDataSet.push_back (pDataSet);
		ubyte4 Len = pDataSet->ReadEntry (Block.rd_ptr ());
		Block.rd_ptr (Len);
		return false;
	}
	template<>
	ubyte4 GetNumber<GmDataSetEntry*> ()
	{
		return Entry.DataSetNum;
	}
};

template <typename DataType>
static void ReadData (const GmUifRootEntry & Entry, GmInputStream & Reader, const wxString & szIndexFile, vector<DataType> & Data)
{
	if (!Reader.IsOpened () && !Reader.Open (szIndexFile.c_str ())) {
		wxString message (_("IDS_OPEN_FILE_FAILED"));
		wxString ThisMessage = wxString::Format (message.c_str ()
												, szIndexFile.c_str ()
												, wxSysErrorMsg (wxSysErrorCode ()));
		throw GmException (ThisMessage);
	}

	GetNodeInfo Info (Entry);
	ubyte4 DataLen = Info.GetLength<DataType> ();
	if (DataLen > 0) {
		Reader.Seek (Info.GetOffset<DataType> (), wxFromStart);
		ACE_Message_Block Block (DataLen);
		ubyte4 BytesRead = Reader.Read (Block.base (), DataLen);
		if (DataLen != BytesRead) {
			ThrowReadError (szIndexFile, DataLen, BytesRead);
		}

		Block.wr_ptr (BytesRead);

		for (ubyte4 index = 0; index < Info.GetNumber<DataType> (); ++index) {
			if (Info.GetData <DataType> (Block, Data))
				break;
		}
	}

	return;
}

void GmUnitedIndexFile::GetAllUifSource (const GmUifRootEntry & Entry, vector<GmUifSourceEntry*> & vpSources)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;
	ReadData (Entry, Reader, m_szIndexFile, vpSources);
}

void GmUnitedIndexFile::GetAllUifDataSet (const GmUifRootEntry & Entry, vector<GmDataSetEntry*> & vpDataSet)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;
	ReadData (Entry, Reader, m_szIndexFile, vpDataSet);
}

void GmUnitedIndexFile::GetAllUifExtendData (const GmUifRootEntry & Entry, vector<char> & vcExtendData)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;
	ReadData (Entry, Reader, m_szIndexFile, vcExtendData);
}

void HandleTheNextDirectory (GmUnitedIndexFile * pThis
							   , size_t & index
							   , vector<ubyte4> & blocks
							   , vector<GmUifDirectory*> & UNUSED (DirsNeedRead)
							   , GmUifDirectory * pNextDirectory
							   , const GmUifRootEntry & Entry
							   , Int2Type <TM_DEPTH_FIRST> i2t)
{
	pThis->ReadTree (index, blocks, pNextDirectory, Entry, i2t);
}

void HandleTheNextDirectory (GmUnitedIndexFile * UNUSED (pThis)
							   , size_t & UNUSED (index)
							   , vector<ubyte4> & UNUSED (blocks)
							   , vector<GmUifDirectory*> & DirsNeedRead
							   , GmUifDirectory* pNextDirectory
							   , const GmUifRootEntry & Entry
							   , Int2Type <TM_WIDTH_FIRST> UNUSED (i2t))
{
	DirsNeedRead.push_back (pNextDirectory);
}

void HandleTheseNextDirs (GmUnitedIndexFile * UNUSED (pThis)
							   , vector<GmUifDirectory*> & UNUSED (DirsNeedRead)
							   , size_t & UNUSED (index)
							   , vector<ubyte4> & UNUSED (blocks)
							   , const GmUifRootEntry & Entry
							   , Int2Type <TM_DEPTH_FIRST> UNUSED (i2t))
{
	// Do nothing.
}

void HandleTheseNextDirs (GmUnitedIndexFile * pThis
							   , vector<GmUifDirectory*> & DirsNeedRead
							   , size_t & index
							   , vector<ubyte4> &blocks
							   , const GmUifRootEntry & Entry
							   , Int2Type <TM_WIDTH_FIRST> i2t)
{
	for (size_t dnri = 0; dnri < DirsNeedRead.size (); ++dnri)
		pThis->ReadTree (index, blocks, DirsNeedRead[dnri], Entry, i2t);
}

template <int Value>
struct ReturnOrBreak
{
	// break default.
	enum { RETURN = 1 };
};

template <>
struct ReturnOrBreak<TM_WIDTH_FIRST>
{
	// break default.
	enum { RETURN = 0 };
};

template <int Value>
void GmUnitedIndexFile::ReadTree (size_t & index
								  , vector<ubyte4> & blocks
								  , GmUifDirectory * pDirectory
								  , const GmUifRootEntry & Entry
								  , Int2Type<Value> i2t)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;
	vector<GmUifDirectory*> DirsNeedRead;
	while (true) {
		ubyte4 signature;
		if (m_DataBlock.length () == 0) {
			if (index >= blocks.size ()) {
				//
				// 一定出问题了。
				//
				wxString message (_("IDS_WRONG_UIF_DATA"));
				throw GmException (message);
			}

			m_DataBlock.reset ();
			ubyte4 BytesToRead = blocks[index++];
			ubyte4 BytesRead = Reader.Read (m_DataBlock.wr_ptr (), BytesToRead);
			if (BytesRead != BytesToRead) {
				ThrowReadError (m_szIndexFile, BytesToRead, BytesRead);
			}

			m_DataBlock.wr_ptr (BytesRead);
		}

		ubyte4 size = CharsToUByte4 (m_DataBlock.rd_ptr (), &signature);
		//
		// 只可能有三种签名，目录尾，文件，目录
		//
		if (signature == SG_UIF_END_DIR_ENTRY) {
			//
			// 遇到目录，如果是深度，则返回一级，否则读取下一级。
			//
			m_DataBlock.rd_ptr (size);
			if (ReturnOrBreak<Value>::RETURN) return;
			else break;
		}
		else if (signature == SG_UIF_LEAF_NODE_ENTRY) {
			//
			// 在这里不会有异常出现，所以不需要 auto_ptr 的帮助。
			//
			GmLeafEntry * pEntry = new GmLeafEntry;
			size = pEntry->ReadEntry (m_DataBlock.rd_ptr ());
			m_DataBlock.rd_ptr (size);
			pDirectory->AddLeafEntry (pEntry, Entry.EntryTime);
		}
		else if (signature == SG_UIF_DIR_NODE_ENTRY) {
			//
			// 在这里不会有异常出现，所以不需要 auto_ptr 的帮助。
			//
			GmUifDirEntry * pEntry = new GmUifDirEntry;
			GmUifDirectory * pNextDirectory = new GmUifDirectory;
			pNextDirectory->pMySelf = pEntry;
			size = pEntry->ReadEntry (m_DataBlock.rd_ptr ());
			m_DataBlock.rd_ptr (size);
			pDirectory->AddDirNode (pNextDirectory);
			HandleTheNextDirectory (this, index, blocks, DirsNeedRead, pNextDirectory, Entry, i2t);
		}
		else {
			wxString message (_("IDS_WRONG_UIF_DATA"));
			throw GmException (message);
		}
	}

	HandleTheseNextDirs (this, DirsNeedRead, index, blocks, Entry, i2t);
}

template <int Value>
void GmUnitedIndexFile::ReadTree (const GmUifRootEntry & Entry
								, GmUifSourceVectorT & tree
								, vector<GmUifSourceEntry*> &vpSources
								, Int2Type<Value> i2t)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;
	for (size_t index = 0; index < vpSources.size (); ++index) {
		m_DataBlock.reset ();
		GmUifSourceEntry & source = *vpSources[index];
		//
		// 所有对像需要自动清理。
		//
		auto_ptr<GmUifSourcePairT> SourceTree (new GmUifSourcePairT);
		auto_ptr<GmUifDirectory> DirTree (new GmUifDirectory);

		Reader.Seek (source.DataOffset, wxFromStart);
		size_t block = 0;
		ReadTree (block, source.SourceBlocks, DirTree.get (), Entry, i2t);

		SourceTree->first = &source;
		SourceTree->second = DirTree.release ();
		tree.push_back (SourceTree.release ());
	}
}

void GmUnitedIndexFile::GetUifRootTree (const GmUifRootEntry & Entry, GmUifRootPairT & tree)
{
	GmInputStream & Reader = m_GmUifStatus.Reader;
	if (!Reader.IsOpened () && !Reader.Open (m_szIndexFile.c_str ())) {
		wxString message (_("IDS_OPEN_FILE_FAILED"));
		wxString ThisMessage = wxString::Format (message.c_str ()
												, m_szIndexFile.c_str ()
												, wxSysErrorMsg (wxSysErrorCode ()));
		throw GmException (ThisMessage);
	}

	auto_ptr<GmUifRootEntry> RootEntry (new GmUifRootEntry);
	auto_ptr<GmUifSourceVectorT> RootTree (new GmUifSourceVectorT);
	vector<GmUifSourceEntry*> vpSources;
	GetAllUifSource (Entry, vpSources);
	GmAutoClearVector<GmUifSourceEntry> acvs (vpSources);

	if (Entry.TraverseMtd == TM_DEPTH_FIRST) {
		ReadTree (Entry, *RootTree.get (), vpSources, Int2Type<TM_DEPTH_FIRST> ());
	}
	else if (Entry.TraverseMtd == TM_WIDTH_FIRST) {
		ReadTree (Entry, *RootTree.get (), vpSources, Int2Type<TM_WIDTH_FIRST> ());
	}
	else
		return;

	Reader.Close ();
	acvs.Clear ();
	tree.first = RootEntry.release ();
	*tree.first = Entry;
	tree.second = RootTree.release ();
}

void GmUnitedIndexFile::Close ()
{
	for_each (m_pRootEntries.begin (), m_pRootEntries.end (), ReleaseMemory ());
	m_pRootEntries.clear ();
}

void GmUnitedIndexFile::Finalize ()
{
	//
	// 强迫重新读取数据。
	//
	for_each (m_pRootEntries.begin (), m_pRootEntries.end (), ReleaseMemory ());
	m_pRootEntries.clear ();
	//
	// pCurRootEntry 一定要清零。
	//
	m_GmUifStatus.pCurRootEntry = 0;
	m_GmUifStatus.Clear ();

	m_DataBlock.reset ();
}

void GmUnitedIndexFile::ClearUif ()
{
	if (m_GmUifStatus.Writer.IsOpened ())
		m_GmUifStatus.Writer.Close ();

	if (m_GmUifStatus.Reader.IsOpened ())
		m_GmUifStatus.Reader.Close ();

	if (!wxFileName::FileExists (m_szIndexFile))
		return;

	if (!wxRemoveFile (m_szIndexFile)) {
		wxString message = wxString::Format (_("IDS_CANT_REMOVE_FILE")
											, m_szIndexFile.c_str ()
											, wxSysErrorMsg (wxSysErrorCode ()));
		throw GmException (message);
	}
	return;
}

GmUnitedIndexFile::~GmUnitedIndexFile ()
{
	Close ();
}

void CheckSignature (ubyte4 ActualValue, ubyte4 TrueValue)
{
	if (ActualValue != TrueValue) {
		wxString format (_("IDS_SIG_CHECK"));
		throw GmException (wxString::Format (format.c_str (), TrueValue, ActualValue));
	}
}

////////////////////////
// 生成一个时间点的数据。
//
static inline void AddFileNodeToUif (GmUnitedIndexFile & uif, GmLeafEntry * pFileNode)
{
	GmLeafInfo Info (*pFileNode);
	uif.AddUifLeaf (Info, pFileNode->Flags, pFileNode->vcExtendData);
}

struct AddLeafNode 
{
	GmUnitedIndexFile & uif;
	AddLeafNode (GmUnitedIndexFile & uif) : uif (uif) {}
	void operator () (const GmSnapNode * pFileNode)
	{
		const GmLeafEntry * pLeafNode = pFileNode->pLeafNode;
		GmLeafInfo Info (*pLeafNode);
		uif.AddUifLeaf (Info, pLeafNode->Flags, pLeafNode->vcExtendData);
	}
};

static inline void AddDirectoryNodeToUif (GmUnitedIndexFile & uif, GmUifDirectory * pDirNode)
{
	GmUifDirEntry * pDirEntry = pDirNode->pMySelf;
	GmLeafInfo Info (*pDirEntry);
	uif.AddUifDir (Info, pDirEntry->Flags, pDirEntry->vcExtendData);
}

void AddDirectoryToUif (GmUnitedIndexFile & uif, GmUifDirectory * pDirNode, Int2Type<TM_DEPTH_FIRST> i2t)
{
	for (size_t index = 0; index < pDirNode->vpSubDirs.size (); ++index) {
		AddDirectoryNodeToUif (uif, pDirNode->vpSubDirs[index]);
		AddDirectoryToUif (uif, pDirNode->vpSubDirs[index], i2t);
	}

	for_each (pDirNode->vpSubFiles.begin (), pDirNode->vpSubFiles.end (), AddLeafNode (uif));
	uif.EndUifDir ();
}

void AddDirectoryToUif (GmUnitedIndexFile & uif, GmUifDirectory * pDirNode, Int2Type<TM_WIDTH_FIRST> i2t)
{
	for (size_t index = 0; index < pDirNode->vpSubDirs.size (); ++index) {
		AddDirectoryNodeToUif (uif, pDirNode->vpSubDirs[index]);
	}

	for_each (pDirNode->vpSubFiles.begin (), pDirNode->vpSubFiles.end (), AddLeafNode (uif));
	uif.EndUifDir ();

	for (size_t index = 0; index < pDirNode->vpSubDirs.size (); ++index) {
		AddDirectoryToUif (uif, pDirNode->vpSubDirs[index], i2t);
	}
}

void AddDirectoryToUif (GmUnitedIndexFile & uif, GmUifDirectory* pDirNode, ubyte2 TraverseMtd)
{
	if (TraverseMtd == TM_DEPTH_FIRST) {
		AddDirectoryToUif (uif, pDirNode, Int2Type<TM_DEPTH_FIRST> ());
	}
	else if (TraverseMtd == TM_WIDTH_FIRST) {
		AddDirectoryToUif (uif, pDirNode, Int2Type<TM_WIDTH_FIRST> ());
	}

	uif.EndUifDir ();
}

struct AddSetToUif 
{
	GmUnitedIndexFile & uif;
	AddSetToUif (GmUnitedIndexFile & uif) : uif (uif) {}
	void operator () (const GmDataSetEntry * pDataSet)
	{
		GmDataSetEntry * pNewDataSet = new GmDataSetEntry;
		*pNewDataSet = *pDataSet;
		uif.AddDataSet (pNewDataSet);
	}
};

void AddTheseTreeToUifFile (GmUifSourceVectorT & trees
							, GmUnitedIndexFile & uif
							, GmRootEntryType Type
							, ubyte2 EntryDataType
							, ubyte2 TraverseMtd
							, ubyte8 Time
							, vector<GmDataSetEntry*> & vpDataSets/* = vector<GmDataSetEntry*> ()*/
							, vector<char> & ExtendData/* = vector<char> ()*/)
{
	if ((TraverseMtd != TM_DEPTH_FIRST) && (TraverseMtd != TM_WIDTH_FIRST))
		return;

	uif.CreateRootEntry (Type, EntryDataType, TraverseMtd, Time);
	for (size_t index = 0; index < trees.size (); ++index) {
		GmUifSourceEntry & source = * trees[index]->first;
		uif.AddUifSource (ToWxString (source.SourceName), (GmSourceType)source.SourceType, source.vcExtendData);
		GmUifDirectory* pDirNode = trees[index]->second;
		AddDirectoryToUif (uif, pDirNode, TraverseMtd);
	}

	for_each (vpDataSets.begin (), vpDataSets.end (), AddSetToUif (uif));
	uif.EndRootEntry (ExtendData);
}

///////////////////////////////////////////////////////////////////////
// 关键算法
// 这些算法用来产生快照树。有合并操作，既如果被分析的树中没有找到相应的结点，这些结点将会
// 迁移到这个被分析的树中，并将结点从所在的原树中删除。这个算法用在生成一个单独快照树，用来进行
// 进一步的分析过程中。
//

template <typename FileNodeType>
void DoCalcTreeNode (std::vector<FileNodeType*> & vpTarget, std::vector<FileNodeType*> & vpRight);

inline void CalcNextLevel (GmUifDirectory* pTarget, GmUifDirectory* pRight)
{
	DoCalcTreeNode (pTarget->vpSubDirs, pRight->vpSubDirs);
	DoCalcTreeNode (pTarget->vpSubFiles, pRight->vpSubFiles);
}

inline void CalcNextLevel (GmSnapNode * pTarget, GmSnapNode * pRight)
{
	return;
}

inline void CalcNextLevel (GmUifSourcePairT * pTarget, GmUifSourcePairT * pRight)
{
	CalcNextLevel (pTarget->second, pRight->second);
}

template <typename FileNodeType>
void DoCalcTreeNode (std::vector<FileNodeType*> & vpTarget, std::vector<FileNodeType*> & vpRight)
{
	std::vector<FileNodeType*> vpTransNodes;

	for (size_t rindex = 0; rindex < vpRight.size (); ++rindex) {
		bool found = false;

		for (size_t tindex = 0; tindex < vpTarget.size (); ++tindex) {
			if (GetNodeName (vpTarget[tindex]) == GetNodeName (vpRight[rindex])) {
				CalcNextLevel (vpTarget[tindex], vpRight[rindex]);
				found = true;
				break;
			}
		}

		if (!found) {
			vpTransNodes.push_back (vpRight[rindex]);
			vpRight[rindex] = 0;
		}
	}

	std::vector<FileNodeType*>::iterator endpos = std::remove (vpRight.begin (), vpRight.end (), (FileNodeType*)0);
	vpRight.erase (endpos, vpRight.end ());
	vpTarget.insert (vpTarget.end (), vpTransNodes.begin (), vpTransNodes.end ());
	return;
}

void MergeToLatestTree (std::vector<GmUifSourcePairT*> & vpTargetTree, std::vector<GmUifSourcePairT*> & vpRightTree)
{
	DoCalcTreeNode (vpTargetTree, vpRightTree);
}

////////////////////////////////////////////////////////////////////////////
//// 取得树的一些信息或者清除空目录。
GmDirectoryInfo GetUifRootTreeDirectoryInfo (const GmUifRootPairT & tree)
{
	GmDirectoryInfo infos;
	GetUifRootTreeDirectoryInfo (*tree.second, infos);
	return infos;
}


void ClearEmptyUifTreeDirectory (vector<GmUifDirectory*> & vpSubDirs)
{
	for (size_t index = 0; index < vpSubDirs.size (); ++index) {
		GmUifDirectory* pDirNode = vpSubDirs[index];
		ClearEmptyUifTreeDirectory (pDirNode->vpSubDirs);
		if (pDirNode->vpSubDirs.empty () && pDirNode->vpSubFiles.empty ()) {
			vpSubDirs[index] = 0;
			delete pDirNode;
		}
	}

	vector<GmUifDirectory*>::iterator pos = 
			std::remove (vpSubDirs.begin (), vpSubDirs.end (), (GmUifDirectory*)0);
	vpSubDirs.erase (pos, vpSubDirs.end ());
}

void ClearEmtpySourceTree (GmUifSourceVectorT & tree)
{
	for (size_t index = 0; index < tree.size (); ++index) {
		GmUifSourcePairT * pNode = tree[index];
		ClearEmptyUifTreeDirectory (pNode->second->vpSubDirs);
		if (pNode->second->vpSubDirs.empty () && pNode->second->vpSubFiles.empty ()) {
			tree[index] = 0;
			delete pNode->first;
			delete pNode->second;
			delete pNode;
		}
	}

	GmUifSourceVectorT::iterator pos = 
		std::remove (tree.begin (), tree.end (), (GmUifSourceVectorT::value_type)0);
	tree.erase (pos, tree.end ());
}

void ClearEmptyRootTreePair (GmUifRootPairT & RootTree)
{
	ClearEmtpySourceTree (*RootTree.second);
}

////////////////////////////////////////////////////////////////////////////
// 只分析树，但不合并树，用来对最新数据进行备份分析，以期取得
// 需要进行备份的树。
//
inline void AnalysisNextLevel (GmUifDirectory*, const GmUifDirectory*);
inline void AnalysisNextLevel (GmSnapNode *&, const GmSnapNode *);

inline void SetInTargetTreeFlag (GmUifDirectory* entry)
{
	if (entry && entry->pMySelf) entry->pMySelf->Flags |= NODE_IN_TARGET_TREE;
}

inline void SetInTargetTreeFlag (GmSnapNode * entry)
{
	if (entry && entry->pLeafNode) entry->pLeafNode->Flags |= NODE_IN_TARGET_TREE;
}

template <typename FileNodeType>
void UseTheseNodes (std::vector<FileNodeType*> & vpOriNodes)
{
	return;
}

void UseTheseNodes (std::vector<GmSnapNode*> & vpOriNodes)
{
	std::vector<GmSnapNode*>::iterator pos = std::remove (vpOriNodes.begin (), vpOriNodes.end (), (GmSnapNode*)0);
	vpOriNodes.erase (pos, vpOriNodes.end ());
	return;
}

//
// 本接口用来分析当前的从系统取得的分析树，与之前备份产生的最新时间点树进行分析。
//
template <typename TreeNodeType>
void DoAnalysisNextLevelNodes (std::vector<TreeNodeType*> & vpOriNodes
							   , const std::vector<TreeNodeType*> & vpRefNodes)
{
	for (size_t oriindex = 0; oriindex < vpOriNodes.size (); ++oriindex) {
		for (size_t refindex = 0; refindex < vpRefNodes.size (); ++refindex) {
			if (GetNodeName (vpOriNodes[oriindex]) == GetNodeName (vpRefNodes[refindex])) {
				//
				// 对于没有找到的目录，则保留，否则向下分析，对于找到的文件，需要看分析条件来确定去留。
				// 在本库中，则是根据更改时间来确定的，保留时间更新的。
				//
				AnalysisNextLevel (vpOriNodes[oriindex], vpRefNodes[refindex]);
				//
				// 参考 defs.h 文件中对 NODE_IN_TARGET_TREE 所做的说明。
				//
				SetInTargetTreeFlag (vpRefNodes[refindex]);
				break;
			}
		}
	}

	UseTheseNodes (vpOriNodes);
}

inline void AnalysisNextLevel (GmUifDirectory * pOriDirectory, const GmUifDirectory * pRefDirectory)
{
	DoAnalysisNextLevelNodes (pOriDirectory->vpSubDirs, pRefDirectory->vpSubDirs);
	DoAnalysisNextLevelNodes (pOriDirectory->vpSubFiles, pRefDirectory->vpSubFiles);
}

inline void AnalysisNextLevel (GmUifSourcePairT * pOriTree, const GmUifSourcePairT * pRefTree)
{
	AnalysisNextLevel (pOriTree->second, pRefTree->second);
}

inline void AnalysisNextLevel (GmSnapNode * & pOriLeaf, const GmSnapNode *pRefLeaf)
{
	// 
	// 在本库中，则是根据更改时间来确定的，保留时间更新的。被分析的树的结点不晚于（一般来说是等于）被参考的结点，
	// 那么说明这个结点是旧的结点，不再需要操作。
	//
	if (pOriLeaf->pLeafNode->ModifyTime <= pRefLeaf->pLeafNode->ModifyTime) {
		delete pOriLeaf;
		pOriLeaf = 0;
	}
}

//
// 用于分析最新树，这个接口用来在备份分析阶段。系统生成而来的树（OriTree），与原来UIF中取得的
// 快照树（RefTree）进行计算，需要进行操作的文件树。计算方式是这样的，如果 OriTree 中存在比 RefTree 新的结点
// 那么保留这个结点，否则删除它，这样剩下的树就是需要操作的最新树。下一次备份进行同样的操作。如果没有找到，那
// 么说明这个结点是新添加的，自然需要保留下来进行操作。
//
void AnalysisLatestTree (GmUifSourceVectorT& OriTree, const GmUifSourceVectorT & RefTree)
{
	for (size_t oriindex = 0; oriindex < OriTree.size (); ++oriindex) {
		for (size_t refindex = 0; refindex < RefTree.size (); ++refindex) {
			if (GetNodeName (OriTree[oriindex]) == GetNodeName (RefTree[refindex])) {
				AnalysisNextLevel (OriTree[oriindex], RefTree[refindex]);
				break;
			}
		}
		//
		// 没找到对应的结点，就不管他，保留下来。
		//
	}

	return;
}

////////////////////////////////////////////////////////////////////////////
template <typename TreeNodeType>
void DoCopyTreeNode (std::vector<TreeNodeType*> & vpTarget, const std::vector<TreeNodeType*> & vpRight);

inline void CopyNextLevel (GmUifDirectory* pTarget, const GmUifDirectory* pRight)
{
	DoCopyTreeNode (pTarget->vpSubDirs, pRight->vpSubDirs);
	DoCopyTreeNode (pTarget->vpSubFiles, pRight->vpSubFiles);
}

inline void CopyNextLevel (GmSnapNode * pTarget, const GmSnapNode * pRight)
{
	return;
}

inline void CopyNextLevel (GmUifSourcePairT * pTarget, const GmUifSourcePairT * pRight)
{
	CopyNextLevel (pTarget->second, pRight->second);
}

void AddOneNode (std::vector<GmUifDirectory*> & copies, const GmUifDirectory * node)
{
	auto_ptr<GmUifDirectory> next (new GmUifDirectory);
	next->bOnlyClearDir = true;
	next->pMySelf = node->pMySelf;
	CopyNextLevel (next.get (), node);
	copies.push_back (next.release ());
}

inline void AddOneNode (std::vector<GmSnapNode*> & copies, GmSnapNode * node)
{
	copies.push_back (node);
}

inline void AddOneNode (GmUifSourceVectorT & copies, GmUifSourcePairT * sp)
{
	auto_ptr<GmUifSourcePairT> sp2 (new GmUifSourcePairT);
	sp2->first = new GmUifSourceEntry;
	*(sp2->first) = *sp->first;

	sp2->second = new GmUifDirectory;
	sp2->second->bOnlyClearDir = true;

	CopyNextLevel (sp2->second, sp->second);
	copies.push_back (sp2.release ());
}

template <typename TreeNodeType>
void DoCopyTreeNode (std::vector<TreeNodeType*> & vpTarget, const std::vector<TreeNodeType*> & vpRight)
{
	std::vector<TreeNodeType*> copies;
	for (size_t rindex = 0; rindex < vpRight.size (); ++rindex) {
		bool found = false;

		for (size_t tindex = 0; tindex < vpTarget.size (); ++tindex) {
			if (GetNodeName (vpTarget[tindex]) == GetNodeName (vpRight[rindex])) {
				CopyNextLevel (vpTarget[tindex], vpRight[rindex]);
				found = true;
				break;
			}
		}

		if (!found) AddOneNode (copies, vpRight[rindex]);
	}

	vpTarget.insert (vpTarget.end (), copies.begin (), copies.end ());
	return;
}

//
// 复制合并树，用于生成快照树。
// 合并的算法是所有的叶结点都是共享同一个叶子结点，而目录结点则是需要自己生成，因此在释放这个树时，
// 只要释放这个目录结点就可以了。为了保证快照树是某个时间对应的目录快点，分析必须是从最后向前分析
//
void CopyToLatestTree (GmUifSourceVectorT & vpTargetTree, const GmUifSourceVectorT & vpRightTree)
{
	DoCopyTreeNode (vpTargetTree, vpRightTree);
}

/////=========================================================

GmUifRootEntry* FromTimeToRootEntry (const vector<GmUifRootEntry*> & roots, ubyte8 time)
{
	for (size_t index = 0; index < roots.size (); ++index)
		if (roots[index]->EntryTime == time) return roots[index];

	return 0;
}

void GetDirectoryInfo (const GmUifDirectory * pDirNode, GmDirectoryInfo & infos)
{
	infos.Dirs += SIZET_TO_UBYTE4 (pDirNode->vpSubDirs.size ());
	for (size_t dindex = 0; dindex < pDirNode->vpSubDirs.size (); ++dindex) {
		GetDirectoryInfo (pDirNode->vpSubDirs[dindex], infos);
	}

	infos.Files += SIZET_TO_UBYTE4 (pDirNode->vpSubFiles.size ());
	for (size_t findex = 0; findex < pDirNode->vpSubFiles.size (); ++findex) {
		infos.CompressSize += GetNodeCompressSize (pDirNode->vpSubFiles[findex]);
		infos.Size += GetNodeSize (pDirNode->vpSubFiles[findex]);
	}
}

void GetUifRootTreeDirectoryInfo (const GmUifSourceVectorT & trees, GmDirectoryInfo & infos)
{
	for (size_t tindex = 0; tindex < trees.size (); ++tindex) {
		GetDirectoryInfo (trees[tindex]->second, infos);
	}
}

void GetAllSetName (GmUnitedIndexFile & uif, vector<wxString> & vszSetName)
{
	const vector<GmUifRootEntry*> & roots = uif.GetAllRootEntries ();
	typedef vector<GmUifRootEntry*>::const_iterator cit;

	for (cit begin = roots.begin (); begin != roots.end (); ++begin) {
		vector<GmDataSetEntry*> vpDataSets;
		GmAutoClearVector<GmDataSetEntry> acvs (vpDataSets);
		uif.GetAllUifDataSet (**begin, vpDataSets);
		for (size_t index = 0; index < vpDataSets.size (); ++index) {
			vszSetName.push_back (ToWxString (vpDataSets[index]->SetName));
		}
	}
}

void CopyToLatestTree (GmUifRootPairT & left, const GmUifRootPairT & right)
{
	assert (left.second == 0);
	assert (left.first == 0);

	left.first = new GmUifRootEntry;
	*left.first = *right.first;

	left.second = new GmUifSourceVectorT;
	CopyToLatestTree (*left.second, *right.second);
}
