//
// Gimu united index file format block definition.
// Author:yeyuqun@163.com
// 2009-11-8
//
#include <engine/monitor.h>
#ifndef __UIF_BLOCK_H__
#define __UIF_BLOCK_H__

#define UIF_TABLE_BLOCK (6 * 1024)
#define UIF_DATA_BLOCK	(3 * 1024 * 1024)
#define CUR_VERSION		0x0100

enum TraversalMtd
{
	TM_DEPTH_FIRST = 0x0,
	TM_WIDTH_FIRST = 0x1,
	TM_NONE,
};

enum Signature {
	SG_UIF_ROOT_ENTRY 			= 0x0101474d,
	SG_UIF_SOURCE_ENTRY			= 0x0609474d,
	SG_UIF_LEAF_NODE_ENTRY		= 0x0204474d,
	SG_UIF_DIR_NODE_ENTRY		= 0x0203474d,
	SG_UIF_END_DIR_ENTRY		= 0x0202474d,
	SG_DATASET_ENTRY			= 0x0508474d,
	SG_UIF_ENTRY				= 0x55494646,
};
enum GmDataSetType
{
	ST_TYPE_NONE,
	ST_TYPE_SYNC,
	ST_TYPE_DUP,
	ST_TYPE_ZIP,
	ST_TYPE_7Z,
};

enum GmRootEntryType
{
	GRET_TYPE_FULL,
	GRET_TYPE_INC,
};

#define ROOT_ENTRY_REMOVED ((ubyte2)-1)

struct GmUifRootEntry : public GmEntryBase
{
	enum { RESERVED_CHARS = 8, };
	GmUifRootEntry ();
	GmUifRootEntry (const GmUifRootEntry &entry);
	GmUifRootEntry & operator = (const GmUifRootEntry &entry);
	ubyte4				Signature;
	ubyte2				EntryType; ////-1 means this entry is removed.
	ubyte2				EntryDataType;
	ubyte2				SourceNum;
	ubyte2				DataSetNum;
	ubyte2				ExtendDataLen; //64k is maximum;
	ubyte2				TraverseMtd;
	ubyte4				SourceDataLen;
	ubyte4				DataSetDataLen;
	ubyte4				Flags;
	ubyte8				EntryTime;
	ubyte8				DataOffset;
	ubyte8				SourceOffset;
	ubyte8				DataSetOffset;
	ubyte8				ExtendDataOffset;
	char				Reserved[RESERVED_CHARS];

	virtual ubyte4 ReadEntry (const char * pBuffer);
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return 76;
	}
private:
	void CopyEntry (const GmUifRootEntry & entry);
};

inline bool operator == (const GmUifRootEntry & left, const GmUifRootEntry & right)
{
	return memcmp (&left, &right, sizeof (GmUifRootEntry)) == 0 ? true : false;
}


struct GmDataSetEntry : public GmEntryBase
{
	GmDataSetEntry () : Signature (SG_DATASET_ENTRY)
						, Flags (0) 
						, RefCount (1)
						{ }
	GmDataSetEntry (const GmDataSetEntry & entry);
	GmDataSetEntry & operator = (const GmDataSetEntry & entry);
	ubyte4				Signature;
	ubyte4				Flags;
	ubyte2				RefCount;
	ubyte2				SetNameLen;
	ubyte2				SetExtLen;
	string				SetName;
	vector<char>		vcExtendData;
	virtual ubyte4 ReadEntry (const char * pBuffer);
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return 14 + (ubyte4)SetName.length () + (ubyte4)vcExtendData.size ();
	}
private:
	void CopyEntry (const GmDataSetEntry & entry);
};

enum GmSourceType
{
	GST_DIRECTORY,
	GST_FILE,
};

//
//  所有结点的最高 8 位是保留位。另 24 位留给将来的功能位。
//

struct GmUifSourceEntry : public GmEntryBase
{
	GmUifSourceEntry ()	: Signature (SG_UIF_SOURCE_ENTRY)
							, Flags (0) {}
	GmUifSourceEntry (const GmUifSourceEntry & entry);
	GmUifSourceEntry & operator = (const GmUifSourceEntry & entry);

	ubyte4				Signature;
	ubyte4				Flags;
	ubyte2				SourceType;
	ubyte2				SourceNameLen;
	ubyte2				SourceBlockNum;
	ubyte2				SourceExtLen;
	ubyte8				DataOffset;
	string				SourceName;
	vector<ubyte4>		SourceBlocks;
	vector<char>		vcExtendData;
	virtual ubyte4 ReadEntry (const char * pBuffer);	
	virtual ubyte4 WriteEntry (char * pBuffer) const;	
	virtual ubyte4 GetSize () const
	{ 
		return SIZET_TO_UBYTE4 (24 + SourceName.length () + SourceBlocks.size () * sizeof (ubyte4) + vcExtendData.size ());
	}
private:
	void Copy (const GmUifSourceEntry & entry);
};

struct GmLeafEntry : public GmEntryBase
{
	GmLeafEntry () : Signature (SG_UIF_LEAF_NODE_ENTRY)
					, Flags (0)
					, Attribute (0)
					, StartDataSet (0) 
					, StartOffset (0)
					, FileSize (0)
					, CompressSize (0)
					, Extent (0) {}
	ubyte4				Signature;
	ubyte4				Flags; //用于一些标志位。
	ubyte4				Attribute;
	ubyte4				StartDataSet;
	ubyte8				StartOffset;
	ubyte8				FileSize;
	ubyte8				CompressSize;
	ubyte8				CreateTime;
	ubyte8				ModifyTime;
	ubyte8				AccessTime;
	ubyte8				Extent;
	ubyte2				LeafNameLen;
	ubyte2				LeafExtLen;
	string				LeafName;
	vector<char>		vcExtendData;
	virtual ubyte4 ReadEntry (const char * pBuffer);	
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return SIZET_TO_UBYTE4 (76 + LeafName.length () + vcExtendData.size ());
	}
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmLeafEntry)
};

struct GmUifDirEntry : public GmEntryBase
{
	GmUifDirEntry () : Signature (SG_UIF_DIR_NODE_ENTRY)
						, Flags (0) {}
	ubyte4				Signature;
	ubyte4				Flags; //用于一些标志位。
	ubyte4				Attribute;
	ubyte8				CreateTime;
	ubyte8				ModifyTime;
	ubyte8				AccessTime;
	ubyte2				DirNameLen;
	ubyte2				ExtLen;
	string				DirName;
	vector<char>		vcExtendData;
	virtual ubyte4 ReadEntry (const char * pBuffer);	
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return SIZET_TO_UBYTE4 (40 + DirName.length () + vcExtendData.size ());
	}
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmUifDirEntry)
};

struct GmUifEndDirEntry
{
	GmUifEndDirEntry () {}
	ubyte4 ReadEntry (const char * pBuffer)
	{
		return sizeof (SG_UIF_END_DIR_ENTRY);
	}

	byte4 WriteEntry (char * pBuffer) const
	{
		return UByte4ToChars (pBuffer, SG_UIF_END_DIR_ENTRY);
	}
	ubyte4 GetSize () const
	{
		return sizeof (SG_UIF_END_DIR_ENTRY);
	}
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmUifEndDirEntry)
};
	
struct GmUIFEntry
{
	enum { UIF_HEAD_LEN = 32, };
	GmUIFEntry () : Signature (SG_UIF_ENTRY)
					, Version (CUR_VERSION)
	{
		memset (Reserved, -1, 26);
	}
	ubyte4				Signature;
	ubyte2				Version;
	char				Reserved[26];
	ubyte4 ReadEntry (const char * pBuffer)
	{
		const char * pBase = pBuffer;
		pBuffer += CharsToUByte4 (pBuffer, &Signature);
		pBuffer += CharsToUByte2 (pBuffer, &Version);
		memcpy (Reserved, pBuffer, 26);
		pBuffer += 26;
		return (ubyte4)(pBuffer - pBase);
	}

	ubyte4 WriteEntry (char * pBuffer) const
	{
		char * pBase = pBuffer;
		pBuffer += UByte4ToChars (pBuffer, Signature);
		pBuffer += UByte2ToChars (pBuffer, Version);
		memcpy (pBuffer, Reserved, 26);
		pBuffer += 26;
		return (ubyte4)(pBuffer - pBase);
	}
	ubyte2 GetSize () const
	{
		return 32;
	}
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmUIFEntry)
};

struct GmUIFTableEntry
{
	ubyte8				NextTable;
	ubyte2				Entries;
	GmUIFTableEntry () : NextTable (0), Entries (0) {}
	virtual ubyte4 ReadEntry (const char * pBuffer)
	{
		pBuffer += CharsToUByte8 (pBuffer, &NextTable);
		pBuffer += CharsToUByte2 (pBuffer, &Entries);
		return static_cast<ubyte4> (GetSize ());
	}

	virtual ubyte4 WriteEntry (char * pBuffer) const
	{
		pBuffer += UByte8ToChars (pBuffer, NextTable);
		pBuffer += UByte2ToChars (pBuffer, Entries);
		return static_cast<ubyte4> (GetSize ());
	}

	ubyte4 GetSize () const
	{
		return 10;
	}
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmUIFTableEntry)
};

struct GmCDSEntry;
struct ENGINE_EXPORT GmLeafInfo
{
	GmLeafInfo ();
	GmLeafInfo (const GmUifDirEntry & entry);
	GmLeafInfo & operator = (const GmUifDirEntry & entry);
	GmLeafInfo (const GmLeafEntry & entry);
	GmLeafInfo & operator = (const GmLeafEntry & entry);
	GmLeafInfo (const GmCDSEntry & entry);
	GmLeafInfo & operator = (const GmCDSEntry & entry);
	//GmLeafInfo (const GmLeafInfo &);
	//GmLeafInfo & operator = (const GmLeafInfo &);

	wxString Name;
	ubyte8 ModifyTime;
	ubyte8 CreateTime; // 文件的创建时间，只在对应的结点中有效，在这样的结点中设为 ModifyTime。
	ubyte8 AccessTime; // 文件的访问时间，只在对应的结点中有效，在这样的结点中设为 ModifyTime。
	ubyte8 FileSize;
	ubyte8 CompressSize; // 文件的压缩后大小，只在对应的结点中有效。
	ubyte4 Attribute;
	ubyte4 StartDataSet; // 文件所在的文件集，只在对应的结点中有效。
	ubyte8 StartOffset; // 文件开始偏移，只在对应的结点中有效。
	ubyte8 Extent;	// 文件内容的范围，只在对应的结点中有效。
	ubyte4 CRC;			// 只针对有CRC值的Zip结点，只在对应的结点中有效，目录无此参数。
	ubyte8 StoreTime; // 保存这个结点的时间，只在对应的结点中有效，比如备份时间。
private:
	void Copy (const GmLeafInfo &);
};

void CheckSignature (ubyte4 ActualValue, ubyte4 TrueValue);
void ThrowReadError (const wxString &, ubyte4, ubyte4);

#endif //__UIF_BLOCK_H__