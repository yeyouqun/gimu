//
// zip header and its block definition.
// Author:yeyouqun@163.com
// 2009-11-8
//
#include <engine/monitor.h>
#ifndef __GM_ZIP_BLOCK_H__
#define __GM_ZIP_BLOCK_H__

enum GmZipAESVersion
{
	WZV_AES_1 = 0x0001,
	WZV_AES_2 = 0x0002,
};

enum {AUTHEN_LEN = 10, PASSVERIFY_LEN = 2, AES_BLOCK_LEN = 16};

enum GmAESZipStrenth
{
	AZS_AES128 = 0x01,
	AZS_AES192 = 0x02,
	AZS_AES256 = 0x03,
};

//
// Local file header.
//
struct GmAESEntry : public GmEntryBase
{
	enum { HEAD_ID = 0x990 };
	GmAESEntry () : AESHeader (HEAD_ID)
					, HeaderLen (7)
	{
		AESVenID[0] = 'A';
		AESVenID[1] = 'E';
	}
	ubyte2		AESHeader;
	ubyte2		HeaderLen;
	ubyte2		AESVersion;
	char		AESVenID[2];
	char		AESStrength;
	ubyte2		CompressMethod;
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const { return 11; }
	virtual ubyte4 ReadEntry (const char * pBuffer);
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmAESEntry)
};

struct GmZip64Entry
{
	enum { HEAD_ID = 0x0001 };
	GmZip64Entry () : Tag (HEAD_ID) {}
	ubyte2 Tag; //Tag for this "extra" block type
	ubyte2 Size; //Size of this "extra" block
	ubyte8 FileSize; //Original uncompressed file size
	ubyte8 CompressSize; //Size of compressed data
	ubyte8 Offset; //Offset of local header record(for cds entry)
	ubyte4 StartSeg; //Number of the disk on which  this file starts(for cds entry)
	void Init ()
	{
		Size = 0;
		FileSize = 0;
		CompressSize = 0;
		Offset = 0;
		StartSeg = 0;
	}
};

struct GmFileHeaderEntry : public GmEntryBase
{
	enum { SIGNATURE = 0x04034b50, FIX_LEN = 30 };
	GmFileHeaderEntry () : Signature (GmFileHeaderEntry::SIGNATURE) {}
	ubyte4			Signature;
	ubyte2			VerExtract;
	ubyte2			GeneralFlags;
	ubyte2			CompressMethod;
	ubyte2			ModifyTime;
	ubyte2			ModifyDate;
	ubyte4			Crc32;
	ubyte4			CompressSize;
	ubyte4			FileSize;
	ubyte2			NameLength;
	ubyte2			ExtraLen;
	string			FileName;
	vector<char>	ExtraData;
	ubyte4 ReadHead (const char * Buffer);
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const { return SIZET_TO_UBYTE4 (GmFileHeaderEntry::FIX_LEN + FileName.length () + ExtraData.size ()); }
	virtual ubyte4 ReadEntry (const char * pBuffer);
	void AddAESEntry (GmAESEntry & entry);
	void GetAESEntry (GmAESEntry & entry) const;
	void AddZip64Entry (const GmZip64Entry &entry);
	void GetZip64Entry (GmZip64Entry & entry) const;
	bool HasZip64 () const { return GetHead (GmZip64Entry::HEAD_ID) != 0 ? true : false; }
private:
	char * GetHead (int hid) const;
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmFileHeaderEntry)
};

//
// Central Directory Structure.
//

struct GmCDSEntry : public GmEntryBase
{
	enum { SIGNATURE = 0x02014b50, FIX_LEN = 46};
	GmCDSEntry () : Signature (GmCDSEntry::SIGNATURE) {}
    ubyte4			Signature;
	ubyte2			VerMadeBy;
	ubyte2			VerExtract;
	ubyte2			GeneralFlags;
	ubyte2			CompressMethod;
	ubyte2			ModifyTime;
	ubyte2			ModifyDate;
	ubyte4			Crc32;
	ubyte4			CompressSize;
	ubyte4			FileSize;
	ubyte2			NameLen;
	ubyte2			ExtraLen;
	ubyte2			CommentLen;
	ubyte2			StartSeg;
	ubyte2			IntAttr;
	ubyte4			ExtAttr;
	ubyte4			RelaOffset;
	string			FileName;
	vector<char>	ExtraData;
	string			Comment;
	ubyte4 ReadHead (const char * Buffer);
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return SIZET_TO_UBYTE4 (GmCDSEntry::FIX_LEN + FileName.length () + ExtraData.size () + Comment.length ());
	}
	virtual ubyte4 ReadEntry (const char * pBuffer);
	void AddAESEntry (GmAESEntry & entry);
	void GetAESEntry (GmAESEntry & entry) const;
	void AddZip64Entry (const GmZip64Entry &entry);
	void GetZip64Entry (GmZip64Entry & entry) const;
	bool HasZip64 () const { return GetHead (GmZip64Entry::HEAD_ID) != 0 ? true : false; }
private:
	char * GetHead (int hid) const;
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmCDSEntry)
};

//G.  Zip64 end of central directory structure
struct GmZip64EndCDSEntry : public GmEntryBase
{
	enum { SIGNATURE = 0x06064b50, FIX_LEN = 56};
	GmZip64EndCDSEntry () : Signature (GmZip64EndCDSEntry::SIGNATURE) {}
	ubyte4			Signature;
	ubyte8			SizeOfThis;
	ubyte2			VerMadeBy;
	ubyte2			VerExtract;
	ubyte4			SegsNum;
	ubyte4			CDSStartSegs;
	ubyte8			TotalThisCDSNum;
	ubyte8			TotalCDSNum;
	ubyte8			TotalCDSSize;
	ubyte8			CDSOffsetOnSeg;
	//////
	ubyte4 ReadHead (const char * Buffer);
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return GmZip64EndCDSEntry::FIX_LEN;
	}
	virtual ubyte4 ReadEntry (const char * pBuffer);
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZip64EndCDSEntry)
};

struct GmZip64CDSLoc : public GmEntryBase
{
	enum { SIGNATURE = 0x07064b50, FIX_LEN = 20};
	GmZip64CDSLoc () : Signature (GmZip64CDSLoc::SIGNATURE) {}
	ubyte4			Signature;
	ubyte4			CDSStartSegs;
	ubyte8			Zip64EndCDSOffset;
	ubyte4			TotalSegs;
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const
	{
		return GmZip64CDSLoc::FIX_LEN;
	}
	virtual ubyte4 ReadEntry (const char * pBuffer);
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZip64CDSLoc)
};

struct GmEndCDSEntry : public GmEntryBase
{
	enum { SIGNATURE = 0x06054b50,  FIX_LEN = 22};
	GmEndCDSEntry () : Signature (GmEndCDSEntry::SIGNATURE) {}
	ubyte4				Signature;
	ubyte2				SegsNum;
	ubyte2				CDSStartSegs;
	ubyte2				TotalThisCDSNum;
	ubyte2				TotalCDSNum;
	ubyte4				TotalCDSSize;
	ubyte4				CDSOffsetOnSeg;
	ubyte2				CommentLen;
	string				Comment;
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const { return SIZET_TO_UBYTE4 (GmEndCDSEntry::FIX_LEN + Comment.length ()); }
	virtual ubyte4 ReadEntry (const char * pBuffer);
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmEndCDSEntry)
};

struct GmZip64CDREntry : public GmEntryBase //Zip64 end of central directory record
{
	GmZip64CDREntry () : Signature (0x06064b50) {}
	enum { SIGNATURE = 0x06064b50,  FIX_LEN = 56};
	ubyte4 Signature; //0x06064b50)
	ubyte8 SizeOfCDR; // size of zip64 end of central directory record
	ubyte2 VerMadeBy; //version made by                 2 bytes
	ubyte2 VerExtract; //version needed to extract       2 bytes
	ubyte4 SegsNumOfThis; //number of this disk             4 bytes
	ubyte4 StartSegs; // number of the disk with the start of the central directory  4 bytes
	ubyte8 TotalThisCDSNum; // total number of entries in the central directory on this disk  8 bytes
	ubyte8 TotalCDSNum; // total number of entries in the central directory               8 bytes
	ubyte8 SizeOfCDS; // size of the central directory   8 bytes
	ubyte8 Offset; // offset of start of central directory with respect to the starting disk number        8 bytes
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const { return FIX_LEN; }
	virtual ubyte4 ReadEntry (const char * pBuffer);
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZip64CDREntry)
};


struct GmZip64CDLEntry : public GmEntryBase //Zip64 end of central directory locator
{
	GmZip64CDLEntry () : Signature (0x07064b50) {}
	enum { SIGNATURE = 0x07064b50,  FIX_LEN = 20};
	ubyte4 Signature; // (0x07064b50)
	ubyte4 SegOfCDR; // number of the disk with the start of the zip64 end of central directory
	ubyte8 OffsetCDR; // relative offset of the zip64 end of central directory record
	ubyte4 TotalSegs; // total number of disks
	virtual ubyte4 WriteEntry (char * pBuffer) const;
	virtual ubyte4 GetSize () const { return FIX_LEN; }
	virtual ubyte4 ReadEntry (const char * pBuffer);
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZip64CDLEntry)
};


enum GmVersionMadeBy
{
	VMB_UPPER_MSDOS			= 0x0000,
	VMB_UPPER_UNIX			= 0x0300,
	VMB_UPPER_MAC			= 0x0700,
	VMB_UPPER_NTFS			= 0x0a00,
	VMB_UPPER_VFAT			= 0x0e00,
	/*
		The value/10 indicates the major 
				  version number, and the value mod 10 is the minor version 
				  number.  
	*/
	VMB_LOWER_ZIP64			= 0x002d,
	VMB_LOWER_VESION		= 0x0033,//6.3
};

enum GmVersionToExtract
{
	VTE_DEFAULT_VERSION		= 0x000a, // Default value
	VTE_DEFLATE_VERSIOIN	= 0x0014, // 2.0 - File is compressed using Deflate compression
	VTE_ZIP64_VERSION		= 0x002D,
	VTE_AES_VERSION			= 0x0033, // File is encrypted using AES encryption
};

enum GmGeneralFlagBits
{
	GFB_FILE_ENCRYPTED		= 0x0001,
	GFB_DEFLATE89_NORMAL	= 0x0000,
	GFB_DEFLATE89_MEDIUM	= 0x0002,
	GFB_DEFLATE89_FAST		= 0x0004,
	GFB_DEFLATE89_SUPFAST	= 0x0006,
	GFB_CRC_AT_FILE_HEADER	= 0x0008,
	GFB_STRONG_ENCRYPTED	= 0x0041,
};

enum GmCompressMethod
{
	CM_FILE_STORED		= 0x0000,
	CM_COMPRESS_ZLIB	= 0x0008,
	CM_COMPRESS_ZLIB64	= 0x0009,
	CM_COMPRESS_BZIP2	= 0x000c,
	CM_COMPRESS_LZMA	= 0x000d,
	CM_COMPRESS_AES_METHOD = 99,
};

enum GmExtraType
{
	ZIP64_EXTRA = 0x0001,
};

void DosTimeToSystemTime (SYSTEMTIME &, ubyte2, ubyte2);
void DosTimeToWxDateTime (wxDateTime &, ubyte2, ubyte2);
FILETIME DosDateToFileTime (ubyte2, ubyte2);
wxString DosDateToString (ubyte2, ubyte2);
void Ubyte8ToDosDate (ubyte8, ubyte2 &, ubyte2 &);
void DosDateToUbyte8 (ubyte8&, ubyte2, ubyte2);

#define CRC_MAGIC		(0xdebb20e3)
/*
When a decryption header is present it will
          be placed in front of the file data and the value of the
          compressed file size will include the bytes of the decryption
          header. 
*/


#endif //__GM_ZIP_BLOCK_H__