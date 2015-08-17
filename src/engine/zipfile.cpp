//
// Zip file's implementation.
// Author:yeyouqun@163.com
// 2009-11-10
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/zipcrc.h>
#include <engine/encdeflate.h>
#include <engine/exception.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/uiffile.h>
#include <engine/zipblock.h>
#include <engine/gmiostream.h>
#include <engine/zipfile.h>
#include <engine/writer.h>

#define FIX_BLOCK_LEN 2048
#define MAX_ENTRY_BYTES 1024

GmZipParams::GmZipParams ()
				: Flags (0)
				, KeySize (ZKS128)
				, ZipLevel (-1)
				, SplitSize (-1)
				, bEncode (false)
				, bCompress (false)
{
}

GmZipParams::GmZipParams (const GmZipParams &params)
				: szPassword (params.szPassword)
				, Flags (params.Flags)
				, KeySize (params.KeySize)
				, ZipLevel (params.ZipLevel)
				, SplitSize (params.SplitSize)
				, bEncode (params.bEncode)
				, bCompress (params.bCompress)
{
}

GmZipParams & GmZipParams::operator = (const GmZipParams &params)
{
	szPassword = params.szPassword;
	Flags = params.Flags;
	KeySize = params.KeySize;
	ZipLevel = params.ZipLevel;
	SplitSize = params.SplitSize;
	bEncode = params.bEncode;
	bCompress = params.bCompress;
	return *this;
}

GmZipDirectory::~GmZipDirectory ()
{
	GmAutoClearVector<GmCDSEntry> acvf (vpSubFiles);
	GmAutoClearVector<GmZipDirectory> acvd (vpSubDirs);
	if (pMySelf != 0)
		delete pMySelf;
}

static inline wxString GetSplitZipName (const wxString & szZipName, ubyte4 nSeg, ubyte4 total = -1)
{
	wxString szName;
	wxString szPath = GetPathName (szZipName);
	if (total == -1 || nSeg < total) {
		//
		// 当在生成 Zip 文件时，是没有 total 参数的，因此直接转换成相应的 Zip 分段文件名。
		//
		wxFileName::SplitPath (szZipName, 0, 0, &szName, 0);
		if (nSeg < 100)
			return AssemblePath (szPath, wxString::Format (wxT ("%s.z%.2d") , szName.c_str (), nSeg + 1));
		return 	AssemblePath (szPath, wxString::Format (wxT ("%s.z%d"), szName.c_str (), nSeg + 1));
	}

	return szZipName;
}

//
// 大缓冲，一次性给压缩加密完。
//
#define INTERAL_BLOCK_SIZE (5*1024*1024)

GmZipCreator::GmZipCreator (const wxString & szFileName, const GmZipParams & params, GmMonitor * pMonitor)
				: m_ZipName (szFileName)
				, m_Params (params)
				, m_pMonitor (pMonitor)
				, m_SaltLen (0)
				, m_Strength (0)
				, m_CurrSegs (0)
				, m_DataBlock (INTERAL_BLOCK_SIZE)
{
	m_CurFile.first = 0;
	memset (m_SaltValue, 0, MAX_SALT_LEN);
	//
	// 如果同时有加密与压缩，则先压缩，再加密。
	//
	if (m_Params.bCompress) m_Zippor.reset (new GmZippor (m_Params.ZipLevel));
	if (m_Params.bEncode) {
		m_Encoder.reset (new GmWizipAesEnc (ENCRYPT, m_Params.KeySize, m_Params.szPassword));
		m_SaltLen = m_Params.KeySize / 16;
		m_Strength = m_Params.KeySize == ZKS128 ? AZS_AES128 :
					m_Params.KeySize == ZKS192 ? AZS_AES192 : AZS_AES256;
	}
}

GmZipCreator::~GmZipCreator ()
{
	m_Out.Close ();
	GmAutoClearVector<GmCDSEntry, list> acvc (m_pCDSEntries);
	GmWinzipRand::DeleteInstance ();
}

void GmZipCreator::BeginCreate ()
{
	//
	// 必须不存在
	//
	if (wxFile::Exists (m_ZipName)) {
		wxString message (_("IDS_FILE_EXISTS"));
		message = wxString::Format (message.c_str (), m_ZipName.c_str ());
		throw GmException (message);
	}

	wxFile (m_ZipName, wxFile::write);
	if (!m_Out.Open (m_ZipName)) {
		wxString message = wxString::Format (_("IDS_OPEN_FILE_FAILED"), m_ZipName.c_str (), GetSystemError ().c_str ());
		throw GmException (message);
	}

	if (m_pMonitor != 0) m_pMonitor->HandleNewFile (m_ZipName);
}

static inline void SetCompressMethod (ubyte2 & CompressMethod
									, bool bEncode
									, bool bCompressed)
{
	if (bEncode) CompressMethod = CM_COMPRESS_AES_METHOD;
	else CompressMethod = bCompressed ? CM_COMPRESS_ZLIB : CM_FILE_STORED;
	return;
}

static inline void SetBitsFlags (ubyte2 & VerExtract, ubyte2 & GerneralFlags, bool bEncode)
{
	if (bEncode) {
		VerExtract = VTE_AES_VERSION;
		GerneralFlags |= GFB_STRONG_ENCRYPTED;
		return;
	}

	VerExtract = VTE_DEFLATE_VERSIOIN;
	GerneralFlags |= GFB_DEFLATE89_MEDIUM;
	return;
}

template <typename EntryType>
static void inline InitEntry (EntryType & header
							, const GmLeafInfo & Info
							, const wxString & szName
							, bool bEncode
							, bool bCompress)
{
	header.Signature = EntryType::SIGNATURE;
	header.GeneralFlags = 0;
	SetBitsFlags (header.VerExtract, header.GeneralFlags, bEncode);
	SetCompressMethod (header.CompressMethod, bEncode, bCompress);

	if (Info.FileSize == 0) header.CompressMethod = CM_FILE_STORED;
	header.CompressSize = 0;//set latter.
	header.FileSize	= Info.FileSize;
	Ubyte8ToDosDate (Info.ModifyTime, header.ModifyTime, header.ModifyDate);
	header.Crc32 = 0;
	header.FileName	= ToStlString (szName);

	if (Info.Attribute & FILE_ATTRIBUTE_DIRECTORY) {
		header.VerExtract		= VTE_DEFAULT_VERSION;
		header.GeneralFlags		= GFB_DEFLATE89_NORMAL;
		header.CompressMethod	= CM_FILE_STORED;
	}
	return;
}

static void InitLocalFile(GmFileHeaderEntry & header
						, const GmLeafInfo & Info
						, const wxString & szName
						, bool bEncode
						, bool bCompress)
{
	InitEntry (header, Info, szName, bEncode, bCompress);
}

static inline void InitCDSEntry (GmCDSEntry & header
								, const GmLeafInfo & Info
								, const wxString szName
								, bool bEncode
								, bool bCompress)
{
	InitEntry (header, Info, szName, bEncode, bCompress);
	header.IntAttr	= 0;
	header.ExtAttr	= Info.Attribute;
	header.RelaOffset = 0;
#ifdef _WIN32
	header.VerMadeBy = VMB_UPPER_MSDOS;
#elif defined (MAC)
	header.VerMadeBy = VMB_UPPER_MAC;
#endif //
	if (bEncode)
		header.VerMadeBy |= VMB_LOWER_VESION;
	return;
}

ubyte8 GmZipCreator::WriteLocalFile (GmFileHeaderEntry & HeadEntry)
{
	//
	// 将本地文件头写入Zip文件，并且更改中心文件结点的参数，如起始段及起始位置等。
	//
	assert (m_Out.IsOpened ());
	ubyte8 pos = m_Out.SeekEnd ();
	ubyte4 Size = HeadEntry.WriteEntry (m_DataBlock.base ());
	if (pos + Size > m_Params.SplitSize) {
		CreateNextZipFile (pos);
		pos = 0;
	}

	m_Out.Write (m_DataBlock.base (), Size);
	return pos;
}

void GmZipCreator::CreateOneFileEntry (const wxString & szRelaPath, const GmLeafInfo & Info, bool bIsDir)
{
	assert (m_Out.IsOpened ());
	m_Out.SeekEnd ();
	m_CurFile.first = m_Out.Tell ();
	if (m_CurFile.second.get () != 0) {
		delete m_CurFile.second.release ();
	}

	m_CurFile.second.reset (new GmFileHeaderEntry);
	m_pCDSEntries.push_back (new GmCDSEntry);

	wxString PathFile = szRelaPath;
	if (bIsDir) PathFile += wxT ("/");
	else PathFile = AssemblePath (PathFile, Info.Name);
	PathFile.Replace (wxT ("\\"), wxT ("/"));
	InitLocalFile (*m_CurFile.second.get (), Info, PathFile, m_Params.bEncode, m_Params.bCompress);
	InitCDSEntry (*m_pCDSEntries.back (), Info, PathFile, m_Params.bEncode, m_Params.bCompress);

	//
	// 不加密目录。
	//
	if (m_Params.bEncode && !bIsDir) {
		GmAESEntry entry;
		entry.AESStrength = m_Strength;
		entry.AESVersion = WZV_AES_1;//support currently, but can open WZV_AES_2
		entry.CompressMethod = m_Params.bCompress ? CM_COMPRESS_ZLIB : CM_FILE_STORED;
		//
		// 将扩展头加入到结点中。
		//
		m_CurFile.second.get ()->AddAESEntry (entry);
		m_pCDSEntries.back ()->AddAESEntry (entry);
	}



	GmFileHeaderEntry & HeadEntry = *m_CurFile.second.get ();
	GmCDSEntry & CDSEntry = *m_pCDSEntries.back ();

	m_Zip64Entry.Init ();
	m_Zip64Entry.FileSize = Info.FileSize;
	if (!bIsDir) {
		//
		// 只有文件才有可能出现文件大小及压缩大小超过限制的情况，并且如果一个文件大小超 4G
		// ，那么他的压缩大小才有可能超过4G（这个假设不安全，但是大多数情况下如此）我们不能等
		// 文件数据压缩完成后再去扩展Local文件头的大小，这样会导致后面所有的数据需要变动。
		//
		if (Info.FileSize & 0xffffffff00000000ull) {
			HeadEntry.FileSize = -1;
			HeadEntry.CompressSize = -1;
			HeadEntry.AddZip64Entry (m_Zip64Entry);
		}
	}

	m_Zip64Entry.Offset = m_CurFile.first = WriteLocalFile (HeadEntry);
	m_Zip64Entry.StartSeg = m_CurrSegs;

	return;
}

void GmZipCreator::AddDirectory (const wxString & szRelaPath, const GmLeafInfo & Info)
{
	CreateOneFileEntry (szRelaPath, Info, true);
}

void GmZipCreator::BeginAddFile (const wxString & szRelaPath, const GmLeafInfo & Info)
{
	CreateOneFileEntry (szRelaPath, Info, false);
	assert (m_CurFile.second.get () != 0);
	assert (!m_pCDSEntries.empty ());
	GmFileHeaderEntry & entry = *m_CurFile.second.get ();
	GmCDSEntry & cds = *m_pCDSEntries.back ();

	m_Crcor.Init ();
	if (m_Params.bEncode) {
		char buffer[MAX_SALT_LEN + PASSVERIFY_LEN];
		assert (m_Encoder.get ());
		memset (m_AuthCode, 0, AUTHEN_LEN);
		memset (m_PassVeri, 0, PASSVERIFY_LEN);
		memset (m_SaltValue, 0, MAX_SALT_LEN);

		GmWinzipRand * pRandGen = GmWinzipRand::GetInstance ();
		pRandGen->GenerateRand (m_SaltValue, m_SaltLen);
		m_Encoder->BeginFile (m_SaltValue, m_PassVeri);
		ubyte2 TotalSize = m_SaltLen + PASSVERIFY_LEN;
		memcpy (buffer, m_SaltValue, m_SaltLen);
		memcpy (buffer + m_SaltLen, m_PassVeri, PASSVERIFY_LEN);
		AddNextNBytes (buffer, TotalSize, entry, cds);
	}
}

void GmZipCreator::AddNextNBytes (char * pDataBase, ubyte4 OutSize, GmFileHeaderEntry & entry, GmCDSEntry & cds)
{
	entry.CompressSize += OutSize;
	cds.CompressSize += OutSize;
	m_Zip64Entry.CompressSize += OutSize;
	//
	// 有可能一次写操作跨越几个文件。
	//
	//
	ubyte4 pos = (ubyte4)m_Out.Tell ();
	while (true) {
		if ((pos + OutSize) > m_Params.SplitSize) {
			ubyte4 PadSize  = m_Params.SplitSize - pos;
			m_Out.Write (pDataBase, PadSize);
			pDataBase += PadSize;
			OutSize -= PadSize;
			pos += PadSize;
			CreateNextZipFile (pos);
			pos = 0;
		}
		else break;
	}
	
	m_Out.Write (pDataBase, OutSize);
}

void GmZipCreator::AddFileBlock (const char * pData, ubyte4 Bytes, bool bEndFile/* = false*/)
{
	assert (m_Out.IsOpened ());
	assert (m_CurFile.second.get () != 0);
	assert (!m_pCDSEntries.empty ());
	GmFileHeaderEntry & entry = *m_CurFile.second.get ();
	GmCDSEntry & cds = *m_pCDSEntries.back ();
	//
	// CRC 计算是以原始数据做为输入的。
	//
	m_Crcor.Update (pData, Bytes);

	bool bInflateAgain = false;
	do {
		char * pDataBase = const_cast<char*> (pData);
		ubyte4 OutSize = Bytes;
		if (m_Zippor.get ()) {
			OutSize = (ubyte4) m_DataBlock.size ();
			pDataBase = m_DataBlock.base ();
			bInflateAgain = m_Zippor->compress (const_cast<char*> (pData), Bytes, pDataBase, OutSize, bEndFile);
		}

		if (m_Encoder.get ()) {
			if (!m_CharsRemained.empty ()) {
				//
				// 如果上次有剩余，凑够 AES_BLOCK_LEN 做为一次计数。
				//
				assert (AES_BLOCK_LEN > m_CharsRemained.size ());

				char intbuf[AES_BLOCK_LEN];
				ubyte4 PadBytes = SIZET_TO_UBYTE4 (AES_BLOCK_LEN - m_CharsRemained.size ());
				//
				// 有可能输出加上上一次的剩余都不足 AES_BLOCK_LEN，则取最小字节数。
				//
				PadBytes = PadBytes > OutSize ? OutSize : PadBytes;

				if (PadBytes + m_CharsRemained.size () < AES_BLOCK_LEN && (!bEndFile || bInflateAgain)) {
					m_CharsRemained.insert (m_CharsRemained.end (), pDataBase, pDataBase + PadBytes);
					pDataBase += PadBytes;
					OutSize -= PadBytes; // OutSize == 0,也就是不会再向 m_CharsRemained 加入数据
				}
				else {
					//
					// 如果没有后续数据填充，或者本身够 AES_BLOCK_LEN 字节，则写入一块。
					//
					memcpy (intbuf, &m_CharsRemained[0], m_CharsRemained.size ());
					memcpy (intbuf + m_CharsRemained.size (), pDataBase, PadBytes);
					m_Encoder->CaculateData (intbuf, AES_BLOCK_LEN);
					AddNextNBytes (intbuf, AES_BLOCK_LEN, entry, cds);
					m_CharsRemained.clear ();
					pDataBase += PadBytes;
					OutSize -= PadBytes; // OutSize == 0,也就是不会再向 m_CharsRemained 加入数据
				}
			}

			if (OutSize % AES_BLOCK_LEN != 0 && (!bEndFile || bInflateAgain)) {
				assert (m_CharsRemained.empty ());
				//
				// 数据块不与 AES_BLOCK_LEN 对齐，并且不是文件结尾，或者还有后续数据，则将不足 AES_BLOCK_LEN
				// 的最后一块数据，放到下一次处理。运行到这里，m_CharsRemained 为空
				//
				ubyte4 OldSize = OutSize;
				OutSize = ((OutSize / AES_BLOCK_LEN) * AES_BLOCK_LEN); // AES_BLOCK_LEN bytes align.
				m_CharsRemained.insert (m_CharsRemained.end (), pDataBase + OutSize, pDataBase + OldSize);
			}

			if (OutSize > 0) m_Encoder->CaculateData (pDataBase, OutSize);
		}

		if (OutSize > 0) AddNextNBytes (pDataBase, OutSize, entry, cds);
	} while (bInflateAgain);

	return;
}

void GmZipCreator::EndAddFile (const vector<char> & ExtendData /*= vector<char> ()*/)
{
	assert (m_Out.IsOpened ());
	assert (m_CurFile.second.get () != 0);
	assert (!m_pCDSEntries.empty ());
	m_DataBlock.reset ();

	GmFileHeaderEntry & fhe = *m_CurFile.second.get ();
	GmCDSEntry & cds = *m_pCDSEntries.back ();
	assert (fhe.CompressSize == cds.CompressSize);

	fhe.Crc32 = m_Crcor.GetDigest ();
	cds.Crc32 = fhe.Crc32;

	if (m_Params.bEncode) {
		unsigned AuthSize = 0;
		assert (m_Encoder.get ());
		AuthSize = m_Encoder->EndFile (m_AuthCode, AuthSize);
		assert (AUTHEN_LEN == AuthSize);
		ubyte4 pos = (ubyte4)m_Out.Tell ();

		char * pAuthCode = (char*)m_AuthCode;
		AddNextNBytes (pAuthCode, AuthSize, fhe, cds);
	}
	//
	// 写入本地头。
	//
	if (fhe.FileSize != -1)
		fhe.AddZip64Entry (m_Zip64Entry);

	if (m_Zip64Entry.Offset & 0xffffffff00000000ull)
		cds.RelaOffset = -1;
	else
		cds.RelaOffset = (ubyte4)m_Zip64Entry.Offset;

	if (m_Zip64Entry.StartSeg & 0xffff0000)
		cds.StartSeg = -1;
	else
		cds.StartSeg = (ubyte2)m_Zip64Entry.StartSeg;

	cds.AddZip64Entry (m_Zip64Entry);
	m_Zip64Entry.Init ();
	ubyte4 Size = fhe.WriteEntry (m_DataBlock.base ());
	if (m_Zip64Entry.StartSeg == m_CurrSegs) {
		m_Out.Seek (m_Zip64Entry.Offset, wxFromStart);
		m_Out.Write (m_DataBlock.base (), Size);
		m_Out.SeekEnd ();
	}
	else {
		GmOutputStream out;
		wxString szSplitFile = GetSplitZipName (m_ZipName, m_Zip64Entry.StartSeg);
		if (!wxFile::Exists (szSplitFile)) {
			wxString message (_("IDS_NO_SUCH_FILE"));
			message = wxString::Format (message.c_str (), szSplitFile.c_str ());
			throw GmException (message);
		}

		out.Open (szSplitFile);
		out.Seek (m_Zip64Entry.Offset, wxFromStart);
		out.Write (m_DataBlock.base (), Size);
	}
	//
	// 等待下一次的写
	//
	m_Out.SeekEnd ();
	delete m_CurFile.second.release ();
}

void GmZipCreator::CreateNextZipFile (ubyte4 OldPos)
{
	//
	// 填充字节到 m_Params.SplitSize; 不会超过一个CDS的大小。
	//
	char buffer[MAX_ENTRY_BYTES]; // 本程序生成的项目结点不超MAX_ENTRY_BYTES个字节。
	ubyte4 PadSize = m_Params.SplitSize - OldPos;
	memset (buffer, 0, PadSize);
	if (PadSize > MAX_ENTRY_BYTES) {
		throw GmException (_("IDS_ZIP_ENTRY_TWO_BIG"));
	}

	if (PadSize != 0) m_Out.Write ((char*)buffer, PadSize);

	m_Out.Close ();
	wxString NextFile = GetSplitZipName (m_ZipName, m_CurrSegs);
	bool bOverWrite = true;
	if (wxFile::Exists (NextFile)) {
		if (m_pMonitor != 0) {
			wxFileName file (NextFile);
			wxDateTime mdt;
			file.GetTimes (0, &mdt, 0);
			bOverWrite = m_pMonitor->OverwriteFile (NextFile
													, mdt.GetValue ().GetValue ()
													, file.GetSize ().GetValue ());
		}

		if (!bOverWrite) {
			wxString message (_("IDS_FILE_EXISTS"));
			message = wxString::Format (message.c_str (), m_ZipName.c_str ());
			throw GmException (message);
		}
	}

	if (m_pMonitor != 0) m_pMonitor->HandleNewFile (NextFile);
	if (!wxRenameFile (m_ZipName, NextFile)) {
		wxString message (_("IDS_RENAME_FILE_ERROR"));
		message = wxString::Format (message.c_str ()
									, m_ZipName.c_str ()
									, NextFile.c_str ()
									, GetSystemError ().c_str ());
		throw GmException (message);
	}
	
	// 生成这个文件。
	if (!m_Out.Open (m_ZipName, true)) {
		wxString message (_("IDS_OPEN_FILE_FAILED"));
		message = wxString::Format (message.c_str (), m_ZipName.c_str (), GetSystemError ().c_str ());
		throw GmException (message);
	}

	m_CurrSegs ++;
}

void GmZipCreator::EndCreate (const wxString & comment)
{
	assert (m_Out.IsOpened ());
	assert (!m_pCDSEntries.empty ());
	//
	// Save m_pCDSEntries to Zip File.
	//
	ubyte8 pos = m_Out.SeekEnd ();
	GmEndCDSEntry EndEntry;

	EndEntry.Signature = GmEndCDSEntry::SIGNATURE;
	EndEntry.CDSOffsetOnSeg = pos & 0xffffffff00000000 ? -1 : (ubyte4)pos;
	EndEntry.TotalCDSNum = (ubyte4)m_pCDSEntries.size ();
	EndEntry.CDSStartSegs = (m_CurrSegs & 0xffff0000) ? -1 : EndEntry.CDSStartSegs;
	EndEntry.TotalCDSSize = 0;
	EndEntry.TotalThisCDSNum = 0;
	EndEntry.Comment = ToStlString (comment);
	ubyte4 EndSeg = m_CurrSegs;
	ubyte4 StartSeg = EndSeg;
	ubyte4 CDSNoOfEndSeg = 0;
	ubyte8 CDSNoOfTotal = 0;
	ubyte8 CDSSize = 0;
	ubyte8 CDSOffset = pos;

	typedef list<GmCDSEntry*>::iterator iterator;
	for (iterator begin = m_pCDSEntries.begin (); begin != m_pCDSEntries.end (); ++begin) {
		ubyte4 Size = (*begin)->WriteEntry (m_DataBlock.base ());
		CDSSize += Size;
		++CDSNoOfEndSeg;

		if (pos + Size > m_Params.SplitSize) {
			CreateNextZipFile (pos);
			pos = 0;
			CDSNoOfEndSeg = 0;

			if (begin == m_pCDSEntries.begin ())
				StartSeg = m_CurrSegs; // cds 写在了下一个文件。
		}

		m_Out.Write (m_DataBlock.base (), Size);
		pos += Size;
	}

	bool bNeedZip64 = false;
	ubyte8 Zip64EndLen = 0;
	GmZip64CDREntry cdr;
	GmZip64CDLEntry cdl;

	if (StartSeg & 0xffff0000) {
		bNeedZip64 = true;
		EndEntry.CDSStartSegs = -1;
	}
	else
		EndEntry.CDSStartSegs = (ubyte2)StartSeg;

	if (CDSNoOfEndSeg & 0xffff0000) {
		bNeedZip64 = true;
		EndEntry.TotalThisCDSNum = -1;
	}
	else
		EndEntry.TotalThisCDSNum = CDSNoOfEndSeg;

	if (CDSNoOfTotal & 0xffffffffffff0000ull) {
		bNeedZip64 = true;
		EndEntry.TotalCDSNum = -1;
	}
	else
		EndEntry.TotalCDSNum = (ubyte2)CDSNoOfTotal;

	if (CDSSize & 0xffffffff00000000ull) {
		bNeedZip64 = true;
		EndEntry.TotalCDSSize = -1;
	}
	else
		EndEntry.TotalCDSSize = (ubyte4)CDSSize;

	if (CDSOffset & 0xffffffff00000000ull) {
		bNeedZip64 = true;
		EndEntry.CDSOffsetOnSeg = -1;
	}
	else
		EndEntry.CDSOffsetOnSeg = (ubyte4)CDSOffset;

	//
	// 创建 Zip64 结构，	GmZip64CDREntry，GmZip64CDLEntry, EndEntry不可以分面在不同的 Zip 文件中
	// 因此，在写入这两上结构时，看是否已经达到分割大小。
	//
	if (bNeedZip64) {
		cdr.Offset = CDSOffset;
		cdr.SizeOfCDR = 44;
		cdr.StartSegs = StartSeg;
		cdr.TotalCDSNum = m_pCDSEntries.size ();
		cdr.TotalThisCDSNum = CDSNoOfEndSeg;
		cdr.VerExtract = VMB_LOWER_ZIP64; //4.5
		cdr.VerMadeBy = VMB_LOWER_ZIP64; // 4.5
		cdr.SizeOfCDS = CDSSize;
		cdr.SegsNumOfThis = 0;

		Zip64EndLen += cdr.GetSize ();
		Zip64EndLen += cdl.GetSize ();

		if ((pos + Zip64EndLen + EndEntry.GetSize ()) > m_Params.SplitSize) {
			CreateNextZipFile (pos);
			pos = 0;
			if (EndEntry.TotalThisCDSNum != -1)
				EndEntry.TotalThisCDSNum = 0;
		}
		else
			cdr.SegsNumOfThis = CDSNoOfEndSeg;

		cdl.OffsetCDR = pos;
		cdl.SegOfCDR = m_CurrSegs;
		cdr.SegsNumOfThis = m_CurrSegs;
		cdl.TotalSegs = m_CurrSegs + 1;
		m_DataBlock.reset ();
		ubyte4 size = cdr.WriteEntry (m_DataBlock.wr_ptr ());
		m_DataBlock.wr_ptr (size);
		size = cdl.WriteEntry (m_DataBlock.wr_ptr ());
		m_DataBlock.wr_ptr (size);
		m_Out.Write (m_DataBlock.rd_ptr (), m_DataBlock.length ());
	}
	else {
		EndEntry.SegsNum = m_CurrSegs;
		if ((pos + EndEntry.GetSize ()) > m_Params.SplitSize) {
			CreateNextZipFile (pos);
			EndEntry.SegsNum = m_CurrSegs;
			EndEntry.TotalThisCDSNum = 0;
		}
	}

	m_DataBlock.reset ();
	ubyte4 EndSize = EndEntry.WriteEntry (m_DataBlock.base ());
	m_Out.Write (m_DataBlock.base (), EndSize);
	m_Out.Close ();
	return;
}

//// distiller

GmZipDistiller::GmZipDistiller (const wxString & szFileName, GmMonitor * pMonitor/* = 0*/)
				: m_ZipName (szFileName)
				, m_CurSegFile (szFileName)
				, m_Monitor (pMonitor)
				, m_DataBlock (INTERAL_BLOCK_SIZE)
				, m_OutBlock (INTERAL_BLOCK_SIZE)
				, m_IsZip64 (false)
{
	GetEndEntry (m_EndEntry);
}

void GmZipDistiller::GetEndEntry (GmEndCDSEntry & EndEntry)
{
	int EndPos = -FIX_BLOCK_LEN;
	m_In.Open (m_ZipName);
	m_In.Seek (EndPos, wxFromEnd);
	char buffer[FIX_BLOCK_LEN];
	ubyte4 Bytes = m_In.Read ((char*)buffer, FIX_BLOCK_LEN);
	if (Bytes < GmEndCDSEntry::FIX_LEN) {
		wxString message (_("IDS_ZIP_WRONG_ENDDATA"));
		throw GmException (message);
	}

	ubyte4 pos = Bytes - GmEndCDSEntry::FIX_LEN;
	for (;;) {
		char * pData = buffer;
		pData += pos;
		ubyte4 Signature;
		CharsToUByte4 (pData, &Signature);
		if (GmEndCDSEntry::SIGNATURE == Signature) {
			EndEntry.ReadEntry (pData);
			break;
		}

		pos--;
		//
		// 大小不超过一K
		//
		if (pos == 0) {
			wxString message (_("IDS_ZIP_WRONG_ENDDATA"));
			throw GmException (message);
		}
	}

	if (EndEntry.CDSOffsetOnSeg == -1 ||
		EndEntry.CDSStartSegs == -1 ||
		EndEntry.SegsNum == -1 ||
		EndEntry.TotalCDSNum == -1 ||
		EndEntry.TotalCDSSize == -1 ||
		EndEntry.TotalThisCDSNum == -1) {
		//
		// Zip64 End.
		//
		m_IsZip64 = true;
		ubyte4 Signature = 0;
		char * pData = buffer;
		pos -= m_cdl.GetSize ();
		pData += pos;
		CharsToUByte4 (pData, &Signature);
		
		if (Signature != GmZip64CDLEntry::SIGNATURE) {
			wxString message (_("IDS_ZIP64_WRONG_CDLDATA"));
			throw GmException (message);
		}

		m_cdl.ReadEntry (pData);
		pos -= m_cdr.GetSize ();
		pData += pos;
		CharsToUByte4 (pData, &Signature);

		if (Signature != GmZip64CDREntry::SIGNATURE) {
			wxString message (_("IDS_ZIP64_WRONG_CDRDATA"));
			throw GmException (message);
		}

		m_cdr.ReadEntry (pData);
	}

	return;
}

void GmZipDistiller::OpenNextSeg (ubyte4 & CurSegs)
{
	++CurSegs;
	m_CurSegFile = GetSplitZipName (m_ZipName, CurSegs, m_EndEntry.SegsNum);
	m_In.Close ();
	if (!wxFile::Exists (m_CurSegFile)) {
		if (m_Monitor != 0)
			m_CurSegFile = m_Monitor->SelectFile (m_CurSegFile);
		else {
			wxString message (_("IDS_NO_SUCH_FILE"));
			message = wxString::Format (message.c_str (), m_CurSegFile.c_str ());
			throw GmException (message);
		}
	}

	m_In.Open (m_CurSegFile);
}

void GmZipDistiller::ReadCDSEntry (GmCDSEntry & CDS, ubyte4 & CurSegs)
{
	assert (m_In.IsOpened ());
	char * pData = (char *)m_DataBlock.base ();

READ_AGAIN:
	ubyte4 BytesRead = m_In.Read (pData, GmCDSEntry::FIX_LEN);

	if (BytesRead < GmCDSEntry::FIX_LEN) {
		//
		// 由于CDS不允许被分在不同的文件中，出现这种情况，就是需要读取下一个文件了。
		//
		OpenNextSeg (CurSegs);
		goto READ_AGAIN;
	}

	//
	// 也有可能最后文件结束时可用字节大于 FIX_LEN,但是小于整个项目大小。
	//
	ubyte4 CDSSig;
	CharsToUByte4 (pData, &CDSSig);
	if (CDSSig != GmCDSEntry::SIGNATURE) {
		//
		// 由于CDS不允许被分在不同的文件中，出现这种情况，就是需要读取下一个文件了。
		//
		OpenNextSeg (CurSegs);
		goto READ_AGAIN;
	}

	ubyte4 HeadLen = CDS.ReadHead (pData);
	ubyte4 TailLen = CDS.CommentLen + CDS.ExtraLen + CDS.NameLen;
	if (m_DataBlock.size () < HeadLen + TailLen) {
		wxString message (_("IDS_ZIP_ENTRY_TWO_BIG"));
		throw GmException (message);
	}

	ubyte4 ByteRead = m_In.Read (pData + HeadLen, TailLen);
	if (ByteRead != TailLen) {
		ThrowReadError (m_CurSegFile, TailLen, ByteRead);
	}

	CDS.ReadEntry (pData);
	if ((CDS.VerExtract > VTE_AES_VERSION) || ((CDS.VerMadeBy & 0x00ff) > VMB_LOWER_VESION)) {
		throw GmException (wxString (_("IDS_NOT_SUPPORT_ZIP_FORMAT")));
	}

	if ((CDS.GeneralFlags & GFB_DEFLATE89_NORMAL) != GFB_DEFLATE89_NORMAL && 
		(CDS.GeneralFlags & GFB_DEFLATE89_MEDIUM) != GFB_DEFLATE89_MEDIUM &&
		(CDS.GeneralFlags & GFB_DEFLATE89_FAST) != GFB_DEFLATE89_FAST) {
		throw GmException (wxString (_("IDS_NOT_SUPPORT_ZIP_FORMAT")));
	}

	if ((CDS.GeneralFlags & GFB_FILE_ENCRYPTED) == GFB_FILE_ENCRYPTED && 
		(CDS.GeneralFlags & GFB_STRONG_ENCRYPTED) != GFB_STRONG_ENCRYPTED) {
			throw GmException (wxString (_("IDS_NOT_SUPPORT_ZIP_FORMAT")));
	}

	if ((CDS.GeneralFlags & GFB_CRC_AT_FILE_HEADER) == GFB_CRC_AT_FILE_HEADER) {
		throw GmException (wxString (_("IDS_NOT_SUPPORT_ZIP_FORMAT")));
	}

	return;
}

static inline GmZipDirectory * 
FindPath (const string & path, const vector<GmZipDirectory*> & vpSubDirs)
{
	for (size_t index = 0; index < vpSubDirs.size (); ++index) {
		if (path == vpSubDirs[index]->PathName)
			return vpSubDirs[index];
	}
	
	return 0;
}

static inline GmZipDirectory * 
CreateSubDir (GmZipDirectory * pTheNode, const string & path)
{
	GmZipDirectory * pSubDir = new GmZipDirectory;
	pSubDir->PathName = path;
	pTheNode->vpSubDirs.push_back (pSubDir);
	return pSubDir;
}

static inline bool 
InsertToDirectory (auto_ptr <GmCDSEntry> & header
				, const vector<string> & pathes
				, GmZipDirectory * pRoot)
{
	vector<string>::const_iterator begin = pathes.begin ();
	GmZipDirectory * pTheNode = pRoot;
	for (; begin != pathes.end (); ++begin) {
		GmZipDirectory * pSubDir = FindPath ((*begin), pTheNode->vpSubDirs);
		if (!pSubDir) break;
		pTheNode = pSubDir;
	}
	
	for (; begin != pathes.end (); ++begin)
		pTheNode = CreateSubDir (pTheNode, *begin);

	if (*(header->FileName.rbegin ()) != '/')
		pTheNode->vpSubFiles.push_back (header.release ());
	else
		pTheNode->pMySelf = header.release ();

	return true;
}

void InsertNode (ZipTreeT * trees, auto_ptr<GmCDSEntry> & CDSEntry, const string & path)
{
	vector<string> pathes;
	if (!path.empty ())
		SplitPath (path, pathes);

	InsertToDirectory (CDSEntry, pathes, trees);
}

void GmZipDistiller::RearrangeToTree (const GmEndCDSEntry & EndEntry, ZipTreeT & trees)
{
	if (EndEntry.CDSStartSegs != EndEntry.SegsNum) {
		m_In.Close ();
		wxString PrevName = GetSplitZipName (m_ZipName, EndEntry.CDSStartSegs, EndEntry.SegsNum);
		m_In.Open (PrevName);
	}

	ubyte4 TotalCDSNum = EndEntry.TotalCDSNum;
	TotalCDSNum &= 0x0000ffff;
	ubyte4 CurSegs = EndEntry.CDSStartSegs;
	CurSegs &= 0x0000ffff;
	m_In.Seek (EndEntry.CDSOffsetOnSeg, wxFromStart);

	for (ubyte4 CurCDSNo = 0; CurCDSNo < TotalCDSNum; ++CurCDSNo) {
		auto_ptr<GmCDSEntry> CDSEntry (new GmCDSEntry);
		ReadCDSEntry (*CDSEntry.get (), CurSegs);
		vector<string> vPaths;
		string path;
		if (*(CDSEntry->FileName.rbegin ()) == '/')
			path = CDSEntry->FileName;
		else
			path = GetPathName (CDSEntry->FileName);
	
		InsertNode (&trees, CDSEntry, path);
	}
}

void GmZipDistiller::GetFileTree (ZipTreeT & trees)
{
	trees.PathName.clear ();
	if (!m_In.IsOpened ()) {
		if (!wxFile::Exists (m_ZipName)) {
			if (m_Monitor != 0)
				m_ZipName = m_Monitor->SelectFile (m_ZipName);
			else {
				wxString message (_("IDS_NO_SUCH_FILE"));
				message = wxString::Format (message.c_str (), m_ZipName.c_str ());
				throw GmException (message);
			}
		}
	}

	RearrangeToTree (m_EndEntry, trees);
}

static void inline ConvertToLeafInfo (GmLeafInfo * pInfo, GmCDSEntry * pFileNode)
{
	pInfo->AccessTime;
	pInfo->Attribute = pFileNode->ExtAttr;
	DosDateToUbyte8 (pInfo->ModifyTime, pFileNode->ModifyTime, pFileNode->ModifyDate);
	pInfo->AccessTime = pInfo->CreateTime = pInfo->ModifyTime; // may be use ntfs extend data in extend data field.
	pInfo->Name = ToWxString (pFileNode->FileName);

	if (pFileNode->HasZip64 ()) {
		GmZip64Entry entry;
		pFileNode->GetZip64Entry (entry);
		pInfo->CompressSize = entry.CompressSize;
		pInfo->FileSize = entry.FileSize;
		pInfo->StartDataSet = entry.StartSeg;
		pInfo->StartOffset = entry.Offset;
	}
	else {
		pInfo->CompressSize = pFileNode->CompressSize;
		pInfo->FileSize = pFileNode->FileSize;
		pInfo->StartDataSet = pFileNode->StartSeg;
		pInfo->StartOffset = pFileNode->RelaOffset;
	}
	return;
}

ubyte2 GmZipDistiller::InitDataHandler (GmCDSEntry *pFileNode)
{
	m_SaltLen = 0;
	m_Strength = 0;
	ubyte2 CompressMethod = pFileNode->CompressMethod;
	bool bEncoded = (pFileNode->GeneralFlags & GFB_FILE_ENCRYPTED) != 0 ? true : false;
	if (bEncoded) {
		if (pFileNode->CompressMethod != CM_COMPRESS_AES_METHOD || 
			(pFileNode->GeneralFlags & GFB_STRONG_ENCRYPTED) != GFB_STRONG_ENCRYPTED) {
			wxString message (_("IDS_NOT_SUPPORT_ENCRYPT_METHOD"));
			message = wxString::Format (message.c_str (), pFileNode->CompressMethod);
			throw GmException (message);
		}

		GmAESEntry entry;
		pFileNode->GetAESEntry (entry);
		if (entry.AESVersion != WZV_AES_1 && entry.AESVersion != WZV_AES_2) {
			wxString message (_("IDS_NOT_SUPPORT_ENCRYPT_METHOD"));
			message = wxString::Format (message.c_str (), entry.AESVersion);
			throw GmException (message);
		}

		if (entry.AESStrength < AZS_AES128 || entry.AESStrength > AZS_AES256) {
			wxString message (_("IDS_NOT_SUPPORT_ENCRYPT_METHOD"));
			message = wxString::Format (message.c_str (), entry.AESStrength);
			throw GmException (message);
		}

		m_Strength = entry.AESStrength;
		GmZipKeySize KeySize = (m_Strength == AZS_AES128 ? ZKS128 :
							m_Strength == AZS_AES192 ? ZKS192 : ZKS256);

		m_SaltLen = KeySize / 16;
		if (m_Monitor != 0 && m_password.length () == 0)
			m_password = ToStlString (m_Monitor->HandlePassword ());
		
		if (m_Decoder.get () == 0)
			m_Decoder.reset (new GmWizipAesEnc (DECRYPT, KeySize, m_password));

		CompressMethod = entry.CompressMethod;
	}

	if (CompressMethod != CM_FILE_STORED && CompressMethod != CM_COMPRESS_ZLIB) {
		wxString message (_("IDS_NOT_SUPPORT_COMRESS_METHOD"));
		message = wxString::Format (message.c_str (), CompressMethod);
		throw GmException (message);
	}

	if (CompressMethod == CM_COMPRESS_ZLIB && m_Unzippor.get () == 0)
		m_Unzippor.reset (new GmUnzippor);

	m_Crcor.Init ();
	ubyte4 TotalSegs;
	if (m_IsZip64)
		TotalSegs = m_EndEntry.SegsNum;
	else
		TotalSegs = m_cdl.TotalSegs;

	ubyte4 StartSeg = pFileNode->StartSeg;
	ubyte4 Offset = pFileNode->RelaOffset;
	if (pFileNode->HasZip64 ()) {
		GmZip64Entry entry;
		pFileNode->GetZip64Entry (entry);
		StartSeg = entry.StartSeg;
		Offset = entry.Offset;
	}

	wxString SegName = GetSplitZipName (m_ZipName, StartSeg, TotalSegs);
	if (m_CurSegFile != SegName) {
		m_CurSegFile = SegName;
		m_In.Close ();
		m_In.Open (m_CurSegFile);
	}
	else if (!m_In.IsOpened ()){
		m_In.Open (m_CurSegFile);
	}
	
	m_In.Seek (Offset, wxFromStart);
	return CompressMethod;
}

void GmZipDistiller::ReadLocalFileHead (GmFileHeaderEntry & entry, ubyte4 & CurSeg)
{
	assert (m_In.IsOpened ());
	char * pData = m_DataBlock.base ();
	ubyte4 Bytes = m_In.Read (pData, GmFileHeaderEntry::FIX_LEN);
	if (Bytes < GmFileHeaderEntry::FIX_LEN) {
		throw GmException (_("IDS_UNEXPECTED_END_OF_FILE"));
	}

	entry.ReadHead (pData);
	ubyte4 HeadLen = GmFileHeaderEntry::FIX_LEN;
	ubyte4 TailLen = entry.ExtraLen + entry.NameLength;
	if (m_DataBlock.size () < HeadLen + TailLen) {
		wxString message (_("IDS_ZIP_ENTRY_TWO_BIG"));
		throw GmException (message);
	}

	//
	// 同一个文件项目不允分布在不同的文件中。
	//
	Bytes = m_In.Read (pData + HeadLen, TailLen);
	if (Bytes < TailLen) {
		wxString message (_("IDS_ZIP_ENTRY_TWO_BIG"));
		throw GmException (message);
	}
	return;
}

void GmZipDistiller::ReadNextNBytes (char * pData, ubyte4 BytesToRead, ubyte4 & CurSeg)
{
	assert (m_In.IsOpened ());
	ubyte4 LeaveSize = BytesToRead;
	ubyte4 Pos = 0;
	ubyte4 BytesRead = 0;
READ_AGAIN:
	BytesRead = m_In.Read (pData + Pos, LeaveSize);
	if (BytesRead < LeaveSize) {
		//
		// 如果数据跨文件，一直将需要读取的文件读完 ToReadBytes 字节。
		//
		OpenNextSeg (CurSeg);
		LeaveSize -= BytesRead;
		Pos += BytesRead;
		goto READ_AGAIN;
	}

	return;
}

void GmZipDistiller::RestoreFile (GmCDSEntry * pFileNode, GmWriter & Writer)
{
	wxString FileName = ToWxString (pFileNode->FileName);
	FileName.Replace (wxT ("/"), wxT ("\\"));
	bool bSuccess = true;
	GmLeafInfo Info;
	ConvertToLeafInfo (&Info, pFileNode);
	if (!Writer.Rebind (FileName, &Info))
		return;

	ubyte4 StartSeg = pFileNode->StartSeg;
	ubyte4 CompressSize = pFileNode->CompressSize;
	if (pFileNode->HasZip64 ()) {
		GmZip64Entry entry;
		pFileNode->GetZip64Entry (entry);
		StartSeg = entry.StartSeg;
		CompressSize = entry.CompressSize;
	}

	ubyte2 CompressMethod = InitDataHandler (pFileNode);
	bool bCompressed = CompressMethod == CM_FILE_STORED ? false : true;
	bool bEncoded = (pFileNode->GeneralFlags & GFB_FILE_ENCRYPTED) != 0 ? true : false;

	GmFileHeaderEntry entry;
	ReadLocalFileHead (entry, StartSeg);

	if (bEncoded) {
		assert (m_Decoder.get () != 0);
		char buffer[MAX_SALT_LEN + PASSVERIFY_LEN];
		char * pData = (char*)buffer;
		ubyte4 Size = m_SaltLen + PASSVERIFY_LEN;
		CompressSize -= (Size + AUTHEN_LEN);
		ReadNextNBytes (pData, Size, StartSeg);
		m_Decoder->BeginFile (pData, pData + m_SaltLen);
	}

	//
	// 下面是恢复数据,每次读取 AES_BLOCK_LEN 整数倍字节（最后一块除外），这是解压要求。INTERAL_BLOCK_SIZE
	// 是 AES_BLOCK_LEN 整数倍。也必须是 AES_BLOCK_LEN 的整数倍。
	//
	while (CompressSize > 0) {
		if (m_Monitor != 0 && m_Monitor->IsStopped ())
			throw GmStopProcedureException ();

		ubyte4 ToReadBytes = CompressSize > INTERAL_BLOCK_SIZE ? INTERAL_BLOCK_SIZE : CompressSize;
		ReadNextNBytes (m_DataBlock.base (), ToReadBytes, StartSeg);
		CompressSize -= ToReadBytes;

		if (bEncoded) {
			assert (m_Decoder.get () != 0);
			//
			// 输入与输出同用一个缓冲空间。
			//
			m_Decoder->CaculateData (m_DataBlock.base (), ToReadBytes);
		}

		if (bCompressed)  {
			assert (m_Unzippor.get () != 0);
			bool bEndFile = false;
			if (CompressSize == 0) bEndFile = true;
			bool bHasMore;
			do {
				//
				// 未解压完成，需要再执行一次。
				//
				ubyte4 OutSize = SIZET_TO_UBYTE4 (m_OutBlock.size ());
				bHasMore = m_Unzippor->decompress (m_DataBlock.base ()
												, ToReadBytes
												, m_OutBlock.base ()
												, OutSize
												, bEndFile);
				Writer.Write (m_OutBlock.base (), OutSize);
				m_Crcor.Update (m_OutBlock.base (), OutSize);
			}while (bHasMore);
		}
		else {
			//
			// 没有压缩。
			//
			Writer.Write (m_DataBlock.base (), ToReadBytes);
			m_Crcor.Update (m_DataBlock.base (), ToReadBytes);
		}
	}

	if (bEncoded) {
		//
		// 解压时，输入字节与输出字节相同。
		//
		assert (m_Decoder.get () != 0);
		char decode[AUTHEN_LEN];
		char encode[AUTHEN_LEN];
		char * pData = (char*)encode;
		ReadNextNBytes (pData, AUTHEN_LEN, StartSeg);
		unsigned AutLen = m_Decoder->EndFile (decode, AUTHEN_LEN);
		assert (AutLen == AUTHEN_LEN);
		if (memcmp (decode, encode, AUTHEN_LEN) != 0) {
			throw GmException (_("IDS_WRONG_PASSWORD"));
		}
	}

	if (m_Crcor.GetDigest () != pFileNode->Crc32) {
		throw GmException (_("IDS_CRC_CHECK_ERROR"));
	}

	Writer.Close ();
	GmLeafInfo info = *pFileNode;
	Writer.SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
	return;
}

void GmZipDistiller::RestoreDirectory (const GmZipDirectory * pDirNode, GmWriter & Writer)
{
	if (pDirNode->pMySelf != 0) {
		GmLeafInfo Info;
		ConvertToLeafInfo (&Info, pDirNode->pMySelf);
		Writer.CreateDir (ToWxString (pDirNode->PathName));
	}
	else {
		Writer.CreateDir (ToWxString (pDirNode->PathName));
	}

	for (size_t dindex = 0; dindex < pDirNode->vpSubDirs.size (); ++dindex) {
		RestoreDirectory (pDirNode->vpSubDirs[dindex], Writer);
	}

	for (size_t findex = 0; findex < pDirNode->vpSubFiles.size (); ++findex) {
		RestoreFile (pDirNode->vpSubFiles[findex], Writer);
	}

	if (pDirNode->pMySelf) {
		GmLeafInfo info = *pDirNode->pMySelf;
		Writer.SetFileAttr (info.CreateTime, info.ModifyTime, info.AccessTime, info.Attribute);
	}
}

GmZipDistiller::~GmZipDistiller ()
{
}

void GetZipDirectoryInfo (const GmZipDirectory * pDirNode, GmDirectoryInfo & infos)
{
	infos.Dirs += SIZET_TO_UBYTE4 (pDirNode->vpSubDirs.size ());
	for (size_t dindex = 0; dindex < pDirNode->vpSubDirs.size (); ++dindex) {
		GetZipDirectoryInfo (pDirNode->vpSubDirs[dindex], infos);
	}

	infos.Files += SIZET_TO_UBYTE4 (pDirNode->vpSubFiles.size ());
	for (size_t findex = 0; findex < pDirNode->vpSubFiles.size (); ++findex) {
		infos.CompressSize += pDirNode->vpSubFiles[findex]->CompressSize;
		infos.Size += pDirNode->vpSubFiles[findex]->FileSize;
	}
}

//////////////////////////////GetDataBackup///////////////////////////////////////

static GmCDSEntry * FindEntry (const string & dirpath
							   , const vector<GmZipDirectory *> & vpDirs
							   , const vector<GmCDSEntry*> &vpFiles
							   , const string & pathfile)
{
	if (pathfile.find (dirpath) == 0 && 
		pathfile.find ('/', dirpath.size () + 1) == string::npos) {
		//
		// 最后一级目录
		//
		for (size_t fi = 0; fi < vpFiles.size (); ++fi) {
			if (vpFiles[fi]->FileName == pathfile)
				return  vpFiles[fi];
		}

		return 0;
	}

	for (size_t index = 0; index <vpDirs.size (); ++index) {
		string pathnext = AssemblePath (dirpath, vpDirs[index]->PathName);
		ReplaceAll (pathnext, "\\", "/");
		if (pathfile.find (pathnext) == 0) {
			return FindEntry (pathnext, vpDirs[index]->vpSubDirs, vpDirs[index]->vpSubFiles, pathfile);
		}
	}

	return 0;
}

GmCDSEntry * FindEntry (const ZipTreeT & tree, const string & pathfile)
{
		return FindEntry (tree.PathName
									, tree.vpSubDirs
									, tree.vpSubFiles
									, pathfile);
}

class GmGetZipDataBackHandler : public GmNodeHandler
{
	GmUifRootEntry &	m_root;
	GmZipDistiller &	m_distiller;
	const ZipTreeT &	m_tree;
	GmWriter *			m_pWriter;
	GmMonitor *			m_pMonitor;

	bool HandleNode (GmUifDirectory * pSnapDir, const string & path)
	{
		string newpath = path;
		ReplaceAll (newpath, "\\", "/");
		GmLeafInfo * pInfo = 0;
		GmLeafInfo info;
		if (pSnapDir->pMySelf) {
			info = *(pSnapDir->pMySelf);
			pInfo = &info;
		}

		if (m_pWriter->CreateDir (ToWxString (newpath))) {
			if (pInfo)
				m_pWriter->SetFileAttr (pInfo->CreateTime
										, pInfo->ModifyTime
										, pInfo->AccessTime
										, pInfo->Attribute);
			return true;
		}

		return false;
	}
	bool HandleNode (GmSnapNode * pFile, const string & path)
	{
		if (!IsThisTime (pFile, m_root)) return false;
		string pathfile = AssemblePath (path, GetNodeName (pFile));
		ReplaceAll (pathfile, "\\", "/");
		GmCDSEntry * pCDS = FindEntry (m_tree, pathfile);
		if (pCDS == 0) {
			wxString message = wxString::Format (_("IDS_NOT_FOUND_ZIP_FILE"), ToWxString (pathfile).c_str ());
			if (m_pMonitor) m_pMonitor->HandleMessage (message);
			return false;
		}

		m_distiller.RestoreFile (pCDS, *m_pWriter);
		return true;
	}
public:
	GmGetZipDataBackHandler (GmUifRootEntry & root
							, GmZipDistiller & distiller
							, const ZipTreeT & tree
							, GmWriter * pWriter
							, GmMonitor * pMonitor)
		: m_root (root)
		, m_distiller (distiller)
		, m_tree (tree)
		, m_pWriter (pWriter) 
		, m_pMonitor (pMonitor) {}
};

bool GetTheseFileDataBack (GmUnitedIndexFile * pUifFile
								 , GmUifRootEntry * root
								 , GmUifDirectory * pSnapDir
								 , const wxString & path
								 , GmWriter * pWriter
								 , GmMonitor * pMonitor)
{
	vector<GmDataSetEntry*> vpDataSets;
	GmAutoClearVector<GmDataSetEntry> acvs (vpDataSets);
	pUifFile->GetAllUifDataSet (*root, vpDataSets);
	if (vpDataSets.empty ()) return false;

	wxString dir = GetPathName (pUifFile->GetIndexFileName ());
	wxString file = ToWxString (GetFileName (vpDataSets.back ()->SetName));
	wxString ZipFile = AssemblePath (dir, file);
	if (!wxFile::Exists (ZipFile))
		ZipFile = pMonitor->SelectFile (file);

	if (ZipFile.IsEmpty ()) {
		wxString message = wxString::Format (_("IDS_NO_NEEDED_FILE"), file.c_str ());
		throw GmException (message);
	}

	GmZipDistillerBuffer *pdb = GmZipDistillerBuffer::GetInstance ();
	GmZipDistiller & distiller = pdb->GetZipDistiller (ZipFile, pMonitor);
	const ZipTreeT & tree = pdb->GetZipTree (ZipFile, pMonitor);
	GmGetZipDataBackHandler handler (*root, distiller, tree, pWriter, pMonitor);
	string NextPath = ToStlString (path);
	if (pSnapDir->pMySelf != 0) NextPath = AssemblePath (NextPath, pSnapDir->pMySelf->DirName);
	TraverseTree (pSnapDir, &handler, NextPath, GmCommHandleFileType ());
	return true;
}


void GetDataBackFromZipArchive (GmUnitedIndexFile * pUifFile
								 , GmUifRootEntry * root
								 , GmUifDirectory * pSnapDir
								 , const wxString & path
								 , int Option
								 , GmWriter * pWriter
								 , GmMonitor * pMonitor)
{
	const vector<GmUifRootEntry*> & roots = pUifFile->GetAllRootEntries ();
	for (size_t index = 0; index < roots.size (); ++index) {
		if (GetTheseFileDataBack (pUifFile, roots[index], pSnapDir, path, pWriter, pMonitor))//??
			break;
	}
}

void GetDataBackFromZipArchive (GmUnitedIndexFile * pUifFile
								 , GmUifRootEntry * root
								 , GmSnapNode * pFile
								 , const wxString & path
								 , int Option
								 , GmWriter * pWriter
								 , GmMonitor * pMonitor)
{
	vector<GmDataSetEntry*> vpDataSets;
	GmAutoClearVector<GmDataSetEntry> acvs (vpDataSets);
	pUifFile->GetAllUifDataSet (*root, vpDataSets);
	if (vpDataSets.empty ()) return;

	wxString dir = GetPathName (pUifFile->GetIndexFileName ());
	wxString file = ToWxString (GetFileName (vpDataSets.back ()->SetName));
	wxString ZipFile = AssemblePath (dir, file);
	if (!wxFile::Exists (ZipFile))
		ZipFile = pMonitor->SelectFile (file);

	if (ZipFile.IsEmpty ()) {
		wxString message = wxString::Format (_("IDS_NO_NEEDED_FILE"), file.c_str ());
		throw GmException (message);
	}

	GmZipDistillerBuffer *pdb = GmZipDistillerBuffer::GetInstance ();
	GmZipDistiller & distiller = pdb->GetZipDistiller (ZipFile, pMonitor);
	const ZipTreeT & tree = pdb->GetZipTree (ZipFile, pMonitor);

	wxString pathfile = AssemblePath (path, ToWxString (GetNodeName (pFile)));
	pathfile.Replace (wxT ("\\"), wxT ("/"));
	GmCDSEntry * pCDS = FindEntry (tree, ToStlString (pathfile));
	if (pCDS == 0) return;
	distiller.RestoreFile (pCDS, *pWriter);
	return;
}


SIGNLETON_IMPL (GmZipDistillerBuffer);
GmZipDistillerBuffer::~GmZipDistillerBuffer ()
{
	for (size_t index = 0; index < m_distillers.size (); ++index) {
		Distiller & distiller = *m_distillers.at (index);
		if (distiller.pTree) {
			delete distiller.pTree;
		}
		if (distiller.pDistiller)
			delete distiller.pDistiller;
	}
}

GmZipDistillerBuffer::Distiller * GmZipDistillerBuffer::Find (const wxString & ZipFile) const
{
	for (size_t index = 0; index < m_distillers.size (); ++index) {
		Distiller & distiller = *m_distillers.at (index);
		assert (distiller.pTree);
		assert (distiller.pDistiller);
		if (distiller.ZipName.CmpNoCase (ZipFile) == 0)
			return &distiller;
	}

	return 0;
}

const ZipTreeT & GmZipDistillerBuffer::GetZipTree (const wxString & ZipFile, GmMonitor * pMonitor)
{
	Distiller * dist = Find (ZipFile);
	if (dist == 0)
		dist = Add (ZipFile, pMonitor);

	return *(dist->pTree);
}

GmZipDistillerBuffer::Distiller * GmZipDistillerBuffer::Add (const wxString & ZipFile, GmMonitor * pMonitor)
{
	auto_ptr<Distiller> ndist (new Distiller);
	ndist->pDistiller = new GmZipDistiller (ZipFile, pMonitor);
	ndist->ZipName = ZipFile;
	ndist->pTree = new ZipTreeT;
	ndist->pDistiller->GetFileTree (*(ndist->pTree));
	m_distillers.push_back (ndist.release ());
	return m_distillers.back ();
}

GmZipDistiller & GmZipDistillerBuffer::GetZipDistiller (const wxString & ZipFile, GmMonitor * pMonitor)
{
	Distiller * dist = Find (ZipFile);
	if (dist == 0)
		dist = Add (ZipFile, pMonitor);

	return *(dist->pDistiller);
}