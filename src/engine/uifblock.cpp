//
// Gimu united index file format block definition.
// Author:yeyuqun@163.com
// 2009-11-8
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/exception.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/classbase.h>

GmUifRootEntry::GmUifRootEntry () : Signature (SG_UIF_ROOT_ENTRY)
									, EntryType (GRET_TYPE_FULL) ////-1 means this entry is removed.
									, EntryDataType (ST_TYPE_NONE)
									, SourceNum (0)
									, DataSetNum (0)
									, ExtendDataLen (0) //64k is maximum;
									, TraverseMtd (TM_NONE)
									, SourceDataLen (0)
									, DataSetDataLen (0)
									, Flags (0)
									, EntryTime (0)
									, DataOffset (0)
									, SourceOffset (0)
									, DataSetOffset (0)
									, ExtendDataOffset (0)
{
	memset (Reserved, 0, RESERVED_CHARS);
}

void GmUifRootEntry::CopyEntry (const GmUifRootEntry & entry)
{
	Signature = entry.Signature;
	EntryType = entry.EntryType;
	EntryDataType = entry.EntryDataType;
	SourceNum = entry.SourceNum;
	DataSetNum = entry.DataSetNum;
	ExtendDataLen = entry.ExtendDataLen;
	TraverseMtd = entry.TraverseMtd;
	SourceDataLen = entry.SourceDataLen;
	DataSetDataLen = entry.DataSetDataLen;
	Flags = entry.Flags;
	EntryTime = entry.EntryTime;
	DataOffset = entry.DataOffset;
	SourceOffset = entry.SourceOffset;
	DataSetOffset = entry.DataSetOffset;
	ExtendDataOffset = entry.ExtendDataOffset;
	memcpy (Reserved, entry.Reserved, RESERVED_CHARS);
}

GmUifRootEntry::GmUifRootEntry (const GmUifRootEntry &entry)
{
	CopyEntry (entry);
}

GmUifRootEntry & GmUifRootEntry::operator = (const GmUifRootEntry &entry)
{
	if (this == &entry) return *this;
	CopyEntry (entry);
	return *this;
}


ubyte4 GmUifRootEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	CheckSignature (SG_UIF_ROOT_ENTRY, Signature);
	pBuffer += CharsToUByte2 (pBuffer, &EntryType); ////-1 means this entry is removed.
	pBuffer += CharsToUByte2 (pBuffer, &EntryDataType);
	pBuffer += CharsToUByte2 (pBuffer, &SourceNum);
	pBuffer += CharsToUByte2 (pBuffer, &DataSetNum);
	pBuffer += CharsToUByte2 (pBuffer, &ExtendDataLen); //64k is maximum;
	pBuffer += CharsToUByte2 (pBuffer, &TraverseMtd);
	pBuffer += CharsToUByte4 (pBuffer, &SourceDataLen);
	pBuffer += CharsToUByte4 (pBuffer, &DataSetDataLen);
	pBuffer += CharsToUByte4 (pBuffer, &Flags);
	pBuffer += CharsToUByte8 (pBuffer, &EntryTime);
	pBuffer += CharsToUByte8 (pBuffer, &DataOffset);
	pBuffer += CharsToUByte8 (pBuffer, &SourceOffset);
	pBuffer += CharsToUByte8 (pBuffer, &DataSetOffset);
	pBuffer += CharsToUByte8 (pBuffer, &ExtendDataOffset);
	memcpy (&Reserved, pBuffer, RESERVED_CHARS);
	pBuffer += RESERVED_CHARS;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

ubyte4 GmUifRootEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte2ToChars (pBuffer, EntryType); ////-1 means this entry is removed.
	pBuffer += UByte2ToChars (pBuffer, EntryDataType);
	pBuffer += UByte2ToChars (pBuffer, SourceNum);
	pBuffer += UByte2ToChars (pBuffer, DataSetNum);
	pBuffer += UByte2ToChars (pBuffer, ExtendDataLen); //64k is maximum;
	pBuffer += UByte2ToChars (pBuffer, TraverseMtd);
	pBuffer += UByte4ToChars (pBuffer, SourceDataLen);
	pBuffer += UByte4ToChars (pBuffer, DataSetDataLen);
	pBuffer += UByte4ToChars (pBuffer, Flags);
	pBuffer += UByte8ToChars (pBuffer, EntryTime);
	pBuffer += UByte8ToChars (pBuffer, DataOffset);
	pBuffer += UByte8ToChars (pBuffer, SourceOffset);
	pBuffer += UByte8ToChars (pBuffer, DataSetOffset);
	pBuffer += UByte8ToChars (pBuffer, ExtendDataOffset);
	memcpy (pBuffer, (void*)Reserved, RESERVED_CHARS);
	pBuffer += RESERVED_CHARS;

	assert (((ubyte4)(pBuffer - pBase) == GetSize ()));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmDataSetEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	CheckSignature (SG_DATASET_ENTRY, Signature);
	pBuffer += CharsToUByte4 (pBuffer, &Flags);
	pBuffer += CharsToUByte2 (pBuffer, &RefCount);
	pBuffer += CharsToUByte2 (pBuffer, &SetNameLen);
	pBuffer += CharsToUByte2 (pBuffer, &SetExtLen);
	SetName.assign (pBuffer, SetNameLen);
	pBuffer += SetNameLen;
	vcExtendData.assign (pBuffer, pBuffer + SetExtLen);
	pBuffer += SetExtLen;

	assert (((ubyte4)(pBuffer - pBase) == GetSize ()));
	return (ubyte4) (pBuffer - pBase);
}

void GmDataSetEntry::CopyEntry (const GmDataSetEntry & entry)
{
	Signature	= entry.Signature;
	Flags		= entry.Flags;
	RefCount		= entry.RefCount;
	SetNameLen	= entry.SetNameLen;
	SetExtLen	= entry.SetExtLen;
	SetName		= entry.SetName;
	vcExtendData = entry.vcExtendData;
}

GmDataSetEntry::GmDataSetEntry (const GmDataSetEntry & entry)
{
	CopyEntry (entry);
}

GmDataSetEntry & GmDataSetEntry::operator = (const GmDataSetEntry & entry)
{
	if (this == &entry) return *this;
	CopyEntry (entry);
	return *this;
}

ubyte4 GmDataSetEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte4ToChars (pBuffer, Flags);
	pBuffer += UByte2ToChars (pBuffer, RefCount);

	const_cast<GmDataSetEntry*> (this)->SetNameLen = (ubyte2)SetName.length ();
	pBuffer += UByte2ToChars (pBuffer, SetNameLen);

	const_cast<GmDataSetEntry*> (this)->SetExtLen = (ubyte2)vcExtendData.size ();
	pBuffer += UByte2ToChars (pBuffer, SetExtLen);

	if (SetNameLen > 0)
		memcpy (pBuffer, &SetName[0], SetNameLen);
	pBuffer += SetNameLen;

	if (SetExtLen > 0)
		memcpy (pBuffer, &vcExtendData[0], SetExtLen);
	pBuffer += SetExtLen;

	assert (((ubyte4)(pBuffer - pBase) == GetSize ()));
	return (ubyte4) (pBuffer - pBase);
}

ubyte4 GmUifSourceEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	CheckSignature (SG_UIF_SOURCE_ENTRY, Signature);
	pBuffer += CharsToUByte4 (pBuffer, &Flags);
	pBuffer += CharsToUByte2 (pBuffer, &SourceType);
	pBuffer += CharsToUByte2 (pBuffer, &SourceNameLen);
	pBuffer += CharsToUByte2 (pBuffer, &SourceBlockNum);
	pBuffer += CharsToUByte2 (pBuffer, &SourceExtLen);
	pBuffer += CharsToUByte8 (pBuffer, &DataOffset);

	SourceName.assign (pBuffer, SourceNameLen);
	pBuffer += SourceNameLen;

	for (ubyte2 index = 0; index < SourceBlockNum; ++index) {
		ubyte4 Data;
		pBuffer += CharsToUByte4 (pBuffer, &Data);
		SourceBlocks.push_back (Data);
	}

	vcExtendData.assign (pBuffer, pBuffer + SourceExtLen);
	pBuffer += SourceExtLen;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

ubyte4 GmUifSourceEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte4ToChars (pBuffer, Flags);
	pBuffer += UByte2ToChars (pBuffer, SourceType);

	const_cast<GmUifSourceEntry*> (this)->SourceNameLen = (ubyte2)SourceName.length ();
	pBuffer += UByte2ToChars (pBuffer, SourceNameLen);

	const_cast<GmUifSourceEntry*> (this)->SourceBlockNum = (ubyte2)SourceBlocks.size ();
	pBuffer += UByte2ToChars (pBuffer, SourceBlockNum);

	const_cast<GmUifSourceEntry*> (this)->SourceExtLen = (ubyte2)vcExtendData.size ();
	pBuffer += UByte2ToChars (pBuffer, SourceExtLen);
	pBuffer += UByte8ToChars (pBuffer, DataOffset);

	if (SourceNameLen > 0)
		memcpy (pBuffer, &SourceName[0], SourceNameLen);
	pBuffer += SourceNameLen;

	for (ubyte2 index = 0; index < SourceBlockNum; ++index) {
		pBuffer += UByte4ToChars (pBuffer, SourceBlocks[index]);
	}

	if (SourceExtLen > 0)
		memcpy (pBuffer, &vcExtendData[0], SourceExtLen);
	pBuffer += SourceExtLen;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

GmUifSourceEntry::GmUifSourceEntry (const GmUifSourceEntry & entry)
{
	Copy (entry);
}

GmUifSourceEntry & GmUifSourceEntry::operator = (const GmUifSourceEntry & entry)
{
	if (this == &entry) return *this;
	Copy (entry);
	return *this;
}

void  GmUifSourceEntry::Copy (const GmUifSourceEntry & entry)
{
	Signature		= entry.Signature;   
	Flags			= entry.Flags;   
	SourceType		= entry.SourceType;  
	SourceNameLen	= entry.SourceNameLen;
	SourceBlockNum	= entry.SourceBlockNum;
	SourceExtLen	= entry.SourceExtLen;
	DataOffset		= entry.DataOffset;  
	SourceName		= entry.SourceName;  
	SourceBlocks	= entry.SourceBlocks;
	vcExtendData	= entry.vcExtendData;
	return;
}

ubyte4 GmLeafEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	CheckSignature (SG_UIF_LEAF_NODE_ENTRY, Signature);
	pBuffer += CharsToUByte4 (pBuffer, &Flags);
	pBuffer += CharsToUByte4 (pBuffer, &Attribute);
	pBuffer += CharsToUByte4 (pBuffer, &StartDataSet);
	pBuffer += CharsToUByte8 (pBuffer, &StartOffset);
	pBuffer += CharsToUByte8 (pBuffer, &FileSize);
	pBuffer += CharsToUByte8 (pBuffer, &CompressSize);
	pBuffer += CharsToUByte8 (pBuffer, &CreateTime);
	pBuffer += CharsToUByte8 (pBuffer, &ModifyTime);
	pBuffer += CharsToUByte8 (pBuffer, &AccessTime);
	pBuffer += CharsToUByte8 (pBuffer, &Extent);
	pBuffer += CharsToUByte2 (pBuffer, &LeafNameLen);
	pBuffer += CharsToUByte2 (pBuffer, &LeafExtLen);

	LeafName.assign (pBuffer, LeafNameLen);
	pBuffer += LeafNameLen;

	vcExtendData.assign (pBuffer, pBuffer + LeafExtLen);
	pBuffer += LeafExtLen;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

ubyte4 GmLeafEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte4ToChars (pBuffer, Flags);
	pBuffer += UByte4ToChars (pBuffer, Attribute);
	pBuffer += UByte4ToChars (pBuffer, StartDataSet);
	pBuffer += UByte8ToChars (pBuffer, StartOffset);
	pBuffer += UByte8ToChars (pBuffer, FileSize);
	pBuffer += UByte8ToChars (pBuffer, CompressSize);
	pBuffer += UByte8ToChars (pBuffer, CreateTime);
	pBuffer += UByte8ToChars (pBuffer, ModifyTime);
	pBuffer += UByte8ToChars (pBuffer, AccessTime);
	pBuffer += UByte8ToChars (pBuffer, Extent);
	
	const_cast<GmLeafEntry*> (this)->LeafNameLen = (ubyte2)LeafName.length ();
	pBuffer += UByte2ToChars (pBuffer, LeafNameLen);

	const_cast<GmLeafEntry*> (this)->LeafExtLen = (ubyte2)vcExtendData.size ();
	pBuffer += UByte2ToChars (pBuffer, LeafExtLen);

	if (LeafNameLen > 0)
		memcpy (pBuffer, &LeafName[0], LeafNameLen);
	pBuffer += LeafNameLen;

	if (LeafExtLen > 0)
		memcpy (pBuffer, &vcExtendData[0], LeafExtLen);
	pBuffer += LeafExtLen;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

ubyte4 GmUifDirEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte4ToChars (pBuffer, Flags);
	pBuffer += UByte4ToChars (pBuffer, Attribute);
	pBuffer += UByte8ToChars (pBuffer, CreateTime);
	pBuffer += UByte8ToChars (pBuffer, ModifyTime);
	pBuffer += UByte8ToChars (pBuffer, AccessTime);

	const_cast<GmUifDirEntry*> (this)->DirNameLen = (ubyte2)DirName.length ();
	pBuffer += UByte2ToChars (pBuffer, DirNameLen);

	const_cast<GmUifDirEntry*> (this)->ExtLen = (ubyte2)vcExtendData.size ();
	pBuffer += UByte2ToChars (pBuffer, ExtLen);

	if (DirNameLen > 0)
		memcpy (pBuffer, &DirName[0], DirNameLen);
	pBuffer += DirNameLen;

	if (ExtLen > 0)
		memcpy (pBuffer, &vcExtendData[0], ExtLen);
	pBuffer += ExtLen;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

ubyte4 GmUifDirEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	CheckSignature (SG_UIF_DIR_NODE_ENTRY, Signature);
	pBuffer += CharsToUByte4 (pBuffer, &Flags);
	pBuffer += CharsToUByte4 (pBuffer, &Attribute);
	pBuffer += CharsToUByte8 (pBuffer, &CreateTime);
	pBuffer += CharsToUByte8 (pBuffer, &ModifyTime);
	pBuffer += CharsToUByte8 (pBuffer, &AccessTime);
	pBuffer += CharsToUByte2 (pBuffer, &DirNameLen);
	pBuffer += CharsToUByte2 (pBuffer, &ExtLen);

	DirName.assign (pBuffer, DirNameLen);
	pBuffer += DirNameLen;

	vcExtendData.assign (pBuffer, pBuffer + ExtLen);
	pBuffer += ExtLen;

	assert ((GetSize () == (ubyte4) (pBuffer - pBase)));
	return (ubyte4) (pBuffer - pBase);
}

GmLeafInfo::GmLeafInfo () : Name ()
					, ModifyTime (0)
					, CreateTime (0)
					, AccessTime (0)
					, FileSize (0)
					, CompressSize (0)
					, Attribute (0)
					, StartDataSet (0)
					, StartOffset (0)
					, Extent (0)
					, CRC (0)
					, StoreTime (0)
{
}

void static inline CopyDirToInfoNode (GmLeafInfo & Info, const GmUifDirEntry & entry)
{
	Info.Name		= ToWxString (entry.DirName);
	Info.Attribute	= entry.Attribute;
	Info.AccessTime	= entry.AccessTime;
	Info.CreateTime	= entry.CreateTime;
	Info.ModifyTime	= entry.ModifyTime;
}

GmLeafInfo::GmLeafInfo (const GmUifDirEntry & entry)
					: FileSize (0)
					, CompressSize (0)
					, StartDataSet (0)
					, StartOffset (0)
					, Extent (0)
{
	CopyDirToInfoNode (*this, entry);
}

GmLeafInfo & GmLeafInfo::operator = (const GmUifDirEntry & entry)
{
	CopyDirToInfoNode (*this, entry);
	return *this;
}

void static inline CopyLeafToInfoNode (GmLeafInfo & Info, const GmLeafEntry & entry)
{
	Info.Name			= ToWxString (entry.LeafName);
	Info.Attribute		= entry.Attribute;
	Info.AccessTime		= entry.AccessTime;
	Info.CreateTime		= entry.CreateTime;
	Info.ModifyTime		= entry.ModifyTime;
	Info.CompressSize	= entry.CompressSize;
	Info.FileSize		= entry.FileSize;
	Info.StartDataSet	= entry.StartDataSet;
	Info.StartOffset	= entry.StartOffset;
	Info.Extent			= entry.Extent;
}

GmLeafInfo::GmLeafInfo (const GmLeafEntry & entry)
{
	CopyLeafToInfoNode (*this, entry);
}

//void GmLeafInfo::Copy (const GmLeafInfo & right)
//{
//	Name = right.Name;
//	ModifyTime = right.ModifyTime;
//	CreateTime = right.CreateTime;
//	AccessTime = right.AccessTime;
//	FileSize = right.FileSize;
//	CompressSize = right.CompressSize;
//	Attribute = right.Attribute;
//	StartDataSet = right.StartDataSet;
//	StartOffset = right.StartOffset;
//	Extent = right.Extent;
//}
//
//GmLeafInfo::GmLeafInfo (const GmLeafInfo &right)
//{
//	Copy (right);
//}
//
//GmLeafInfo & GmLeafInfo::operator = (const GmLeafInfo &right)
//{
//	if (this == &right) return *this;
//	Copy (right);
//	return *this;
//}

GmLeafInfo & GmLeafInfo::operator = (const GmLeafEntry & entry)
{
	CopyLeafToInfoNode (*this, entry);
	return *this;
}

void ThrowReadError (const wxString & File, ubyte4 BytesToRead, ubyte4 ReadBytes)
{
	wxString message (_("IDS_READ_FILE_FAILED"));
	wxString ThisMessage = wxString::Format (message.c_str (), File.c_str (), BytesToRead, ReadBytes);
	throw GmException (ThisMessage);
}