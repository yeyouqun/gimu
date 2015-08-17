//
// zip header and its block definition.
// Author:yeyouqun@163.com
// 2009-11-8
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/encdeflate.h>
#include <engine/gmiostream.h>
#include <engine/writer.h>
#include <engine/zipwriter.h>
#include <engine/zipblock.h>
#include <engine/exception.h>
#include <engine/util.h>

void DosTimeToSystemTime (SYSTEMTIME & systemTime, ubyte2 time, ubyte2 date)
{
	systemTime.wSecond	= (time & 0x1F) * 2;
	systemTime.wMinute	= (time >> 5) & 0x3F;
	systemTime.wHour	= (time >> 11) & 0x1F;
	systemTime.wDay		= date & 0x1F;
	systemTime.wMonth	= (date >> 5) & 0xF;
	systemTime.wYear	= ((date >> 9) & 0x7F + 1980);
}

void DosTimeToWxDateTime (wxDateTime & time1, ubyte2 time, ubyte2 date)
{
	time1.SetYear (((date >> 9) & 0x7F + 1980));
	time1.SetMonth ((wxDateTime::Month) ((date >> 5) & 0xF));
	time1.SetDay (date & 0x1F);
	time1.SetHour ((time >> 11) & 0x1F);
	time1.SetMinute ((time >> 5) & 0x3F);
	time1.SetSecond ((time & 0x1F) * 2);
}

FILETIME DosDateToFileTime (ubyte2 time, ubyte2 date)
{
	SYSTEMTIME systemTime;
	memset (&systemTime, 0, sizeof (SYSTEMTIME));
	DosTimeToSystemTime (systemTime, time, date);

	FILETIME localTime, fileTime;
	SystemTimeToFileTime (&systemTime, &localTime);

	LocalFileTimeToFileTime(&localTime, &fileTime);
	return fileTime;
}
void DosDateToTm (struct tm * ptm, ubyte2 dosTime, ubyte2 dosDate)
{
	if (ptm != 0) {
		memset (ptm, 0, sizeof (struct tm));
		ptm->tm_hour = ((dosTime >> 11) & 0x1F);
		ptm->tm_min = ((dosTime >> 5) & 0x3F);
		ptm->tm_sec = (((dosTime >> 0) & 0x1F) * 2);

		ptm->tm_year = (((dosDate >> 9) + 1980 - 1900) & 0x7F);
		ptm->tm_mon = ((dosDate >> 5) & 0xF);
		ptm->tm_mday = ((dosDate >> 0) & 0x1F);
	}
}

wxString DosDateToString (ubyte2 time, ubyte2 date)
{
	struct tm tmv;
	DosDateToTm (&tmv, time, date);
	return wxString::Format (wxT("%d_%d_%d-%d_%d_%d"), tmv.tm_year + 1900, tmv.tm_mon, tmv.tm_mday
														, tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
}

void Ubyte8ToDosDate (ubyte8 time, ubyte2 & dosTime, ubyte2 & dosDate)
{
	time /= 1000;
	struct tm * ptm = localtime ((time_t*)&time);
	dosTime =  dosDate = 0;
	if (ptm != 0) {
		dosTime	|= ((ptm->tm_hour & 0x1F) << 11);
		dosTime	|= ((ptm->tm_min & 0x3F) << 5);
		dosTime	|= ((ptm->tm_sec / 2) & 0x1F);

		dosDate |= (((ptm->tm_year + 1900) - 1980) << 9);
		dosDate |= (ptm->tm_mon & 0xf) << 5;
		dosDate |= (ptm->tm_mday & 0x1f);
	}
	return;
}

void DosDateToUbyte8 (ubyte8 & dt, ubyte2 time, ubyte2 date)
{
	struct tm tmv;
	DosDateToTm (&tmv, time, date);
	dt = (ubyte8)mktime (&tmv);
	dt *= 1000;
}

ubyte4 GmFileHeaderEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte2ToChars (pBuffer, VerExtract);
	pBuffer += UByte2ToChars (pBuffer, GeneralFlags);
	pBuffer += UByte2ToChars (pBuffer, CompressMethod);
	pBuffer += UByte2ToChars (pBuffer, ModifyTime);
	pBuffer += UByte2ToChars (pBuffer, ModifyDate);
	pBuffer += UByte4ToChars (pBuffer, Crc32);
	pBuffer += UByte4ToChars (pBuffer, CompressSize);
	pBuffer += UByte4ToChars (pBuffer, FileSize);

	const_cast<GmFileHeaderEntry*> (this)->NameLength = SIZET_TO_UBYTE4 (FileName.length ());
	pBuffer += UByte2ToChars (pBuffer, NameLength);

	const_cast<GmFileHeaderEntry*> (this)->ExtraLen = (ubyte2)ExtraData.size ();
	pBuffer += UByte2ToChars (pBuffer, ExtraLen);

	if (NameLength > 0)
		memcpy (pBuffer, FileName.c_str (), NameLength);
	pBuffer += NameLength;

	if (ExtraLen > 0)
		memcpy (pBuffer, &ExtraData[0], ExtraLen);
	pBuffer += ExtraLen;

	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmFileHeaderEntry::ReadHead (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte2 (pBuffer, &VerExtract);
	pBuffer += CharsToUByte2 (pBuffer, &GeneralFlags);
	pBuffer += CharsToUByte2 (pBuffer, &CompressMethod);
	pBuffer += CharsToUByte2 (pBuffer, &ModifyTime);
	pBuffer += CharsToUByte2 (pBuffer, &ModifyDate);
	pBuffer += CharsToUByte4 (pBuffer, &Crc32);
	pBuffer += CharsToUByte4 (pBuffer, &CompressSize);
	pBuffer += CharsToUByte4 (pBuffer, &FileSize);
	pBuffer += CharsToUByte2 (pBuffer, &NameLength);
	pBuffer += CharsToUByte2 (pBuffer, &ExtraLen);
	assert (GmFileHeaderEntry::FIX_LEN == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmFileHeaderEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += ReadHead (pBuffer);

	if (NameLength > 0)
		FileName.assign (pBuffer, NameLength);
	pBuffer += NameLength;

	if (ExtraLen > 0)
		ExtraData.assign (pBuffer, pBuffer + ExtraLen);
	pBuffer += ExtraLen;

	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

void GmFileHeaderEntry::AddZip64Entry (const GmZip64Entry &entry)
{
	if (FileSize != -1) return;
	char * p = GetHead (GmZip64Entry::HEAD_ID);
	if (p == 0) {
		size_t size = ExtraData.size ();
		ExtraData.resize (size + 16 + 4); // sizeof (ubyte8) * 2
		p = &ExtraData[0] + size;
	}

	UByte2ToChars (p, entry.Tag);
	p += 2;
	UByte2ToChars (p, (ubyte2)16);
	p += 2;
	UByte8ToChars (p, entry.FileSize);
	p += 8;
	UByte8ToChars (p, entry.CompressSize);
	return;
}

void GmFileHeaderEntry::GetZip64Entry (GmZip64Entry & entry) const
{
	const char * p = 0;
	if (ExtraData.size () < 20 || (p = GetHead (GmZip64Entry::HEAD_ID)) == 0)
		throw GmException (_("IDS_UNMATCH_ZIP64"));

	entry.Size = 16;
	entry.StartSeg = 0;
	entry.Offset = 0;
	if (p == 0) {
		entry.FileSize = FileSize;
		entry.CompressSize = CompressSize;
		return;
	}

	CharsToUByte2 (p, &entry.Tag);
	if (entry.Tag != GmZip64Entry::HEAD_ID) {
		throw GmException (_("IDS_ERROR_ZIP64_HEAD_ID"));
	}
	p += 2;
	CharsToUByte2 (p, &entry.Size);
	p += 2;
	CharsToUByte8 (p, &entry.FileSize);
	p += 8;
	CharsToUByte8 (p, &entry.CompressSize);
	return;
}

static char * GetExtendHeadById (int hid, const vector<char> & ExtraData)
{
	if (ExtraData.empty ()) return 0;
	const char * p = &ExtraData[0];
	while (1) {
		if (p > &ExtraData.back ()) break;
		ubyte2 id = 0;
		ubyte2 hsz = 0;
		CharsToUByte2 (p, &id);
		CharsToUByte2 (p + 2, &hsz);
		if (id == hid) return const_cast<char*> (p);
		p += 4;
		p += hsz;
	}

	return 0;
}

char * GmFileHeaderEntry::GetHead (int hid) const
{
	return GetExtendHeadById (hid, ExtraData);
}

void GmFileHeaderEntry::AddAESEntry (GmAESEntry & entry)
{
	char * p = GetHead (GmAESEntry::HEAD_ID);
	if (p == 0) {
		size_t size = ExtraData.size ();
		ExtraData.resize (entry.GetSize () + size);
		p = &ExtraData[0] + size;
	}

	entry.WriteEntry (p);
}
void GmFileHeaderEntry::GetAESEntry (GmAESEntry & entry) const
{
	if (!(ExtraData.size () >= entry.GetSize ()))
		throw GmException (_("IDS_UNMATCH_ENCRYPTION"));

	const char * p = GetHead (GmAESEntry::HEAD_ID);
	if (p == 0) return;
	entry.ReadEntry (p);
}
//
// Central Directory Structure.
//

ubyte4 GmEndCDSEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte2ToChars (pBuffer, SegsNum);
	pBuffer += UByte2ToChars (pBuffer, CDSStartSegs);
	pBuffer += UByte2ToChars (pBuffer, TotalThisCDSNum);
	pBuffer += UByte2ToChars (pBuffer, TotalCDSNum);
	pBuffer += UByte4ToChars (pBuffer, TotalCDSSize);
	pBuffer += UByte4ToChars (pBuffer, CDSOffsetOnSeg);
	
	const_cast<GmEndCDSEntry*> (this)->CommentLen = (ubyte2)Comment.length ();
	pBuffer += UByte2ToChars (pBuffer, CommentLen);

	if (CommentLen > 0)
		memcpy (pBuffer, Comment.c_str (), CommentLen);
	pBuffer += CommentLen;

	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmEndCDSEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte2 (pBuffer, &SegsNum);
	pBuffer += CharsToUByte2 (pBuffer, &CDSStartSegs);
	pBuffer += CharsToUByte2 (pBuffer, &TotalThisCDSNum);
	pBuffer += CharsToUByte2 (pBuffer, &TotalCDSNum);
	pBuffer += CharsToUByte4 (pBuffer, &TotalCDSSize);
	pBuffer += CharsToUByte4 (pBuffer, &CDSOffsetOnSeg);
	pBuffer += CharsToUByte2 (pBuffer, &CommentLen);

	if (CommentLen > 0)
		Comment.assign (pBuffer, CommentLen);
	pBuffer += CommentLen;

	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmCDSEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte2ToChars (pBuffer, VerMadeBy);
	pBuffer += UByte2ToChars (pBuffer, VerExtract);
	pBuffer += UByte2ToChars (pBuffer, GeneralFlags);
	pBuffer += UByte2ToChars (pBuffer, CompressMethod);
	pBuffer += UByte2ToChars (pBuffer, ModifyTime);
	pBuffer += UByte2ToChars (pBuffer, ModifyDate);
	pBuffer += UByte4ToChars (pBuffer, Crc32);
	pBuffer += UByte4ToChars (pBuffer, CompressSize);
	pBuffer += UByte4ToChars (pBuffer, FileSize);

	const_cast<GmCDSEntry*> (this)->NameLen = (ubyte2)FileName.length ();
	pBuffer += UByte2ToChars (pBuffer, NameLen);

	const_cast<GmCDSEntry*> (this)->ExtraLen = (ubyte2)ExtraData.size ();
	pBuffer += UByte2ToChars (pBuffer, ExtraLen);

	const_cast<GmCDSEntry*> (this)->CommentLen = (ubyte2)Comment.length ();
	pBuffer += UByte2ToChars (pBuffer, CommentLen);
	pBuffer += UByte2ToChars (pBuffer, StartSeg);
	pBuffer += UByte2ToChars (pBuffer, IntAttr);
	pBuffer += UByte4ToChars (pBuffer, ExtAttr);
	pBuffer += UByte4ToChars (pBuffer, RelaOffset);

	if (NameLen)
		memcpy (pBuffer, FileName.c_str (), NameLen);
	pBuffer += NameLen;

	if (ExtraLen > 0)
		memcpy (pBuffer, &ExtraData[0], ExtraLen);
	pBuffer += ExtraLen;

	if (CommentLen > 0)
		memcpy (pBuffer, Comment.c_str (), CommentLen);
	pBuffer += CommentLen;

	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmCDSEntry::ReadHead (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte2 (pBuffer, &VerMadeBy);
	pBuffer += CharsToUByte2 (pBuffer, &VerExtract);
	pBuffer += CharsToUByte2 (pBuffer, &GeneralFlags);
	pBuffer += CharsToUByte2 (pBuffer, &CompressMethod);
	pBuffer += CharsToUByte2 (pBuffer, &ModifyTime);
	pBuffer += CharsToUByte2 (pBuffer, &ModifyDate);
	pBuffer += CharsToUByte4 (pBuffer, &Crc32);
	pBuffer += CharsToUByte4 (pBuffer, &CompressSize);
	pBuffer += CharsToUByte4 (pBuffer, &FileSize);
	pBuffer += CharsToUByte2 (pBuffer, &NameLen);
	pBuffer += CharsToUByte2 (pBuffer, &ExtraLen);
	pBuffer += CharsToUByte2 (pBuffer, &CommentLen);
	pBuffer += CharsToUByte2 (pBuffer, &StartSeg);
	pBuffer += CharsToUByte2 (pBuffer, &IntAttr);
	pBuffer += CharsToUByte4 (pBuffer, &ExtAttr);
	pBuffer += CharsToUByte4 (pBuffer, &RelaOffset);
	assert (GmCDSEntry::FIX_LEN == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmCDSEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += ReadHead (pBuffer);

	if (NameLen)
		FileName.assign (pBuffer, NameLen);
	pBuffer += NameLen;

	if (ExtraLen > 0)
		ExtraData.assign (pBuffer, pBuffer + ExtraLen);
	pBuffer += ExtraLen;

	if (CommentLen > 0)
		Comment.assign (pBuffer, CommentLen);
	pBuffer += CommentLen;

	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

void GmCDSEntry::AddZip64Entry (const GmZip64Entry &entry)
{
	GmZip64Entry item = entry; 

	bool bNeedZip64 = false;
	if (FileSize == -1) {
		item.Size = 16;
		CompressSize = -1;
		bNeedZip64 = true;
	}

	if (RelaOffset == -1) {
		item.Size += 8;
		bNeedZip64 = true;
	}
	if (StartSeg == -1) {
		bNeedZip64 = true;
		item.Size += 4;
	}

	if (!bNeedZip64) return;

	char * p = GetHead (GmZip64Entry::HEAD_ID);
	if (p == 0) {
		size_t size = ExtraData.size ();
		ExtraData.resize (size + item.Size + 4); //
		p = &ExtraData[0] + size;
	}

	UByte2ToChars (p, item.Tag);
	p += 2;
	UByte2ToChars (p, item.Size);
	p += 2;

	if (FileSize == -1) {
		UByte8ToChars (p, entry.FileSize);
		p += 8;
		UByte8ToChars (p, entry.CompressSize);
		p += 8;
	}

	if (RelaOffset == -1) {
		UByte8ToChars (p, entry.Offset);
		p += 8;
	}

	if (StartSeg == -1)
		UByte8ToChars (p, entry.StartSeg);

	return;
}

void GmCDSEntry::AddAESEntry (GmAESEntry & entry)
{
	char * p = GetHead (GmAESEntry::HEAD_ID);
	if (p == 0) {
		size_t size = ExtraData.size ();
		ExtraData.resize (entry.GetSize () + size);
		p = &ExtraData[0] + size;
	}

	entry.WriteEntry (p);
}
void GmCDSEntry::GetAESEntry (GmAESEntry & entry) const
{
	const char * p = 0;
	if (ExtraData.size () < entry.GetSize () || (p = GetHead (GmAESEntry::HEAD_ID)) == 0)
		throw GmException (_("IDS_UNMATCH_ENCRYPTION"));

	entry.ReadEntry (p);
}

void GmCDSEntry::GetZip64Entry (GmZip64Entry & entry) const
{
	const char * p = 0;
	if (ExtraData.empty () || (p = GetHead (GmZip64Entry::HEAD_ID)) == 0)
		throw GmException (_("IDS_UNMATCH_ZIP64"));

	entry.Size = 16;
	entry.StartSeg = 0;
	entry.Offset = 0;

	CharsToUByte2 (p, &entry.Tag);
	if (entry.Tag != GmZip64Entry::HEAD_ID) {
		throw GmException (_("IDS_ERROR_ZIP64_HEAD_ID"));
	}

	p += 2;
	CharsToUByte2 (p, &entry.Size);
	p += 2;

	if (FileSize == -1) {
		CharsToUByte8 (p, &entry.FileSize);
		p += 8;
		CharsToUByte8 (p, &entry.CompressSize);
		p += 8;
	}
	else {
		entry.FileSize = FileSize;
		entry.CompressSize = CompressSize;
	}

	if (entry.Size > 16 && RelaOffset == -1) {
		CharsToUByte8 (p, &entry.Offset);
		p += 8;
	}
	else
		entry.Offset = RelaOffset;

	if (entry.Size > 24 && StartSeg == -1)
		CharsToUByte4 (p, &entry.StartSeg);
	else
		entry.StartSeg = StartSeg;

	return;
}

char * GmCDSEntry::GetHead (int hid) const
{
	return GetExtendHeadById (hid, ExtraData);
}

ubyte4 GmAESEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte2 (pBuffer, &AESHeader);
	pBuffer += CharsToUByte2 (pBuffer, &HeaderLen);
	pBuffer += CharsToUByte2 (pBuffer, &AESVersion);
	memcpy (&AESVenID, pBuffer, 2);
	pBuffer += 2;
	AESStrength = *pBuffer;
	++pBuffer;
	pBuffer += CharsToUByte2 (pBuffer, &CompressMethod);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmAESEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte2ToChars (pBuffer, AESHeader);
	pBuffer += UByte2ToChars (pBuffer, HeaderLen);
	pBuffer += UByte2ToChars (pBuffer, AESVersion);
	memcpy (pBuffer, AESVenID, 2);
	pBuffer += 2;
	*pBuffer = AESStrength;
	++pBuffer;
	pBuffer += UByte2ToChars (pBuffer, CompressMethod);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64CDSLoc::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte4ToChars (pBuffer, CDSStartSegs);
	pBuffer += UByte8ToChars (pBuffer, Zip64EndCDSOffset);
	pBuffer += UByte4ToChars (pBuffer, TotalSegs);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64CDSLoc::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte4 (pBuffer, &CDSStartSegs);
	pBuffer += CharsToUByte8 (pBuffer, &Zip64EndCDSOffset);
	pBuffer += CharsToUByte4 (pBuffer, &TotalSegs);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64EndCDSEntry::ReadHead (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte8 (pBuffer, &SizeOfThis);
	pBuffer += CharsToUByte2 (pBuffer, &VerMadeBy);
	pBuffer += CharsToUByte2 (pBuffer, &VerExtract);
	pBuffer += CharsToUByte4 (pBuffer, &SegsNum);
	pBuffer += CharsToUByte4 (pBuffer, &CDSStartSegs);
	pBuffer += CharsToUByte8 (pBuffer, &TotalThisCDSNum);
	pBuffer += CharsToUByte8 (pBuffer, &TotalCDSNum);
	pBuffer += CharsToUByte8 (pBuffer, &TotalCDSSize);
	pBuffer += CharsToUByte8 (pBuffer, &CDSOffsetOnSeg);
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64EndCDSEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte8ToChars (pBuffer, SizeOfThis);
	pBuffer += UByte2ToChars (pBuffer, VerMadeBy);
	pBuffer += UByte2ToChars (pBuffer, VerExtract);
	pBuffer += UByte4ToChars (pBuffer, SegsNum);
	pBuffer += UByte4ToChars (pBuffer, CDSStartSegs);
	pBuffer += UByte8ToChars (pBuffer, TotalThisCDSNum);
	pBuffer += UByte8ToChars (pBuffer, TotalCDSNum);
	pBuffer += UByte8ToChars (pBuffer, TotalCDSSize);
	pBuffer += UByte8ToChars (pBuffer, CDSOffsetOnSeg);
	//
	// 写Zip64扩展数据
	//
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64EndCDSEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += ReadHead (pBuffer);
	//
	// 读Zip64扩展数据
	//
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

static void CopyZipEntryToLeafInfo (GmLeafInfo & leaf, const GmCDSEntry & entry)
{
	ubyte8 time;
	DosDateToUbyte8 (time, entry.ModifyTime, entry.ModifyDate);
	leaf.AccessTime = time;
	leaf.ModifyTime = time;
	leaf.CreateTime = time;
	leaf.CRC		= entry.Crc32;
	leaf.Name		= ToWxString (entry.FileName);
	leaf.Attribute	= entry.ExtAttr;
	leaf.CompressSize = entry.CompressSize;
	leaf.FileSize	= entry.FileSize;
}

GmLeafInfo::GmLeafInfo (const GmCDSEntry & entry)
{
	CopyZipEntryToLeafInfo (*this, entry);
}

GmLeafInfo & GmLeafInfo::operator = (const GmCDSEntry & entry)
{
	CopyZipEntryToLeafInfo (*this, entry);
	return *this;
}

ubyte4 GmZip64CDLEntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte4ToChars (pBuffer, SegOfCDR);
	pBuffer += UByte8ToChars (pBuffer, OffsetCDR);
	pBuffer += UByte4ToChars (pBuffer, TotalSegs);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64CDLEntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte4 (pBuffer, &SegOfCDR);
	pBuffer += CharsToUByte8 (pBuffer, &OffsetCDR);
	pBuffer += CharsToUByte4 (pBuffer, &TotalSegs);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64CDREntry::WriteEntry (char * pBuffer) const
{
	char * pBase = pBuffer;
	pBuffer += UByte4ToChars (pBuffer, Signature);
	pBuffer += UByte8ToChars (pBuffer, SizeOfCDR);
	pBuffer += UByte2ToChars (pBuffer, VerMadeBy);
	pBuffer += UByte2ToChars (pBuffer, VerExtract);
	pBuffer += UByte4ToChars (pBuffer, SegsNumOfThis);
	pBuffer += UByte4ToChars (pBuffer, StartSegs);
	pBuffer += UByte8ToChars (pBuffer, TotalThisCDSNum);
	pBuffer += UByte8ToChars (pBuffer, TotalCDSNum);
	pBuffer += UByte8ToChars (pBuffer, SizeOfCDS);
	pBuffer += UByte8ToChars (pBuffer, Offset);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}

ubyte4 GmZip64CDREntry::ReadEntry (const char * pBuffer)
{
	const char * pBase = pBuffer;
	pBuffer += CharsToUByte4 (pBuffer, &Signature);
	pBuffer += CharsToUByte8 (pBuffer, &SizeOfCDR);
	pBuffer += CharsToUByte2 (pBuffer, &VerMadeBy);
	pBuffer += CharsToUByte2 (pBuffer, &VerExtract);
	pBuffer += CharsToUByte4 (pBuffer, &SegsNumOfThis);
	pBuffer += CharsToUByte4 (pBuffer, &StartSegs);
	pBuffer += CharsToUByte8 (pBuffer, &TotalThisCDSNum);
	pBuffer += CharsToUByte8 (pBuffer, &TotalCDSNum);
	pBuffer += CharsToUByte8 (pBuffer, &SizeOfCDS);
	pBuffer += CharsToUByte8 (pBuffer, &Offset);
	assert (GetSize () == (ubyte4)(pBuffer - pBase));
	return (ubyte4)(pBuffer - pBase);
}
