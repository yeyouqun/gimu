//
// Zip file's implementation.
// Author:yeyouqun@163.com
// 2009-11-10
//
#include <engine/zipblock.h>
#include <engine/gmiostream.h>
#include <engine/encdeflate.h>
#include <engine/zipcrc.h>
#ifndef __GM_ZIPFILE_H__
#define __GM_ZIPFILE_H__
class GmWriter;
struct ENGINE_EXPORT GmZipDirectory
{
	GmZipDirectory () : pMySelf (0) {}
	GmCDSEntry *		pMySelf;
	string				PathName;
	vector<GmCDSEntry*> vpSubFiles;
	vector<GmZipDirectory*> vpSubDirs;
	~GmZipDirectory ();
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZipDirectory)
};

typedef GmZipDirectory ZipTreeT;

ENGINE_EXPORT void GetZipDirectoryInfo (const GmZipDirectory * pDirNode, GmDirectoryInfo & infos);

#define MAX_SALT_LEN (256/8/2)

struct ENGINE_EXPORT GmZipParams
{
	GmZipParams ();
	GmZipParams (const GmZipParams &);
	GmZipParams & operator = (const GmZipParams &);
	string	szPassword;
	ubyte4	Flags;
	GmZipKeySize	KeySize;
	int		ZipLevel;
	ubyte8	SplitSize;
	bool	bEncode;
	bool	bCompress;
};

//
// Zip 文件生成器。
//
class ENGINE_EXPORT GmZipCreator
{
public:
	GmZipCreator (const wxString &, const GmZipParams &, GmMonitor * = 0);
	void BeginCreate ();
	void AddDirectory (const wxString &, const GmLeafInfo &);
	void BeginAddFile (const wxString &, const GmLeafInfo &);
	void AddFileBlock (const char *, ubyte4, bool = false);
	void EndAddFile (const vector<char> & = vector<char> ());
	void EndCreate (const wxString &);
	~GmZipCreator ();
private:
	ubyte8 WriteLocalFile (GmFileHeaderEntry &);
	void CreateOneFileEntry (const wxString &, const GmLeafInfo &, bool);
	void CreateNextZipFile (ubyte4);
	void AddNextNBytes (char * pDataBase, ubyte4, GmFileHeaderEntry &, GmCDSEntry &);
private:
	//
	// members.
	//
	GmOutputStream		m_Out;
	list<GmCDSEntry*>	m_pCDSEntries;
	wxString			m_ZipName;
	GmZipParams			m_Params;
	GmMonitor *			m_pMonitor;
	//
	// For encryption.
	//
	char				m_SaltValue[MAX_SALT_LEN];
	int					m_SaltLen;
	char				m_Strength;
	char				m_AuthCode[AUTHEN_LEN];
	char				m_PassVeri[PASSVERIFY_LEN];
	vector<char>		m_CharsRemained;

	//
	// For data handler.
	//
	auto_ptr<GmZippor>			m_Zippor;
	auto_ptr<GmWizipAesEnc>		m_Encoder;
	ubyte4						m_CurrSegs;
	GmZipCRCor					m_Crcor;
	std::pair<ubyte8, auto_ptr<GmFileHeaderEntry> > m_CurFile;
	ACE_Message_Block			m_DataBlock;
	GmZip64Entry				m_Zip64Entry;
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZipCreator)
};

//
// Zip 文件恢复器。
//
class ENGINE_EXPORT GmZipDistiller
{
public:
	GmZipDistiller (const wxString & szFileName, GmMonitor * pMonitor = 0);
	void GetFileTree (ZipTreeT &);
	void RestoreFile (GmCDSEntry *, GmWriter &);
	void RestoreDirectory (const GmZipDirectory *, GmWriter &);
	~GmZipDistiller ();
private:
	void GetEndEntry (GmEndCDSEntry &);
	void RearrangeToTree (const GmEndCDSEntry &, ZipTreeT &);
	void ReadCDSEntry (GmCDSEntry &, ubyte4 &);
	void OpenNextSeg (ubyte4 &);
	ubyte2 InitDataHandler (GmCDSEntry *);
	void ReadLocalFileHead (GmFileHeaderEntry &, ubyte4 &);
	void ReadNextNBytes (char *, ubyte4, ubyte4 &);
private:
	wxString			m_ZipName;
	wxString			m_CurSegFile;
	GmInputStream		m_In;
	auto_ptr<GmUnzippor>		m_Unzippor;
	auto_ptr<GmWizipAesEnc>		m_Decoder;
	GmZipCRCor			m_Crcor;
	GmMonitor *			m_Monitor;
	GmEndCDSEntry		m_EndEntry;
	//
	// For decryption.
	//
	int					m_SaltLen;
	char				m_Strength;
	//
	// Decompress, or Decrytion buffer.
	//
	ACE_Message_Block	m_DataBlock;
	ACE_Message_Block	m_OutBlock;
	string				m_password;
	//
	// If zip64 End used.
	//
	bool				m_IsZip64;
	GmZip64CDREntry		m_cdr;
	GmZip64CDLEntry		m_cdl;
	DECL_NO_COPYASSIGN_CONSTRUCTOR (GmZipDistiller)
};

inline bool IsThisTime (const GmSnapNode * pSnapNode, const GmUifRootEntry & entry)
{
	return pSnapNode->Time == entry.EntryTime ? true : false;
}

//
// 用在恢复时，缓存 ZipDistiller 对象，以防止重新生成ZipDistiller对象，导致性能下降。
//
class GmZipDistillerBuffer
{
	GmZipDistillerBuffer () {}
	~GmZipDistillerBuffer ();
	struct Distiller
	{
		ZipTreeT *			pTree;
		GmZipDistiller *	pDistiller;
		wxString			ZipName;
	};
	vector<Distiller*>		m_distillers;
	Distiller * Find (const wxString &) const;
	Distiller * Add (const wxString & ZipFile, GmMonitor * pMonitor);
public:
	const ZipTreeT & GetZipTree (const wxString & ZipFile, GmMonitor * pMonitor);
	GmZipDistiller & GetZipDistiller (const wxString & ZipFile, GmMonitor * pMonitor);
	SINGLETON_DECL (GmZipDistillerBuffer);
};

class GmUnitedIndexFile;
void GetDataBackFromZipArchive (GmUnitedIndexFile * pUifFile
								 , GmUifRootEntry * root
								 , GmSnapNode * pFile
								 , const wxString & path
								 , int Option
								 , GmWriter * pWriter
								 , GmMonitor * pMonitor);
#endif // __GM_ZIPFILE_H__