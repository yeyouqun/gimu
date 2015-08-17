//
// zip writer and its block definition.
// Author:yeyouqun@163.com
// 2009-11-9
//

#include <engine/winzipaes/prng.h>
#include <engine/winzipaes/fileenc.h>
#include "zlib/zlib.h"

#ifndef __GM_ENC_DEFLATE_H__
#define __GM_ENC_DEFLATE_H__

enum GmEncDir
{
	ENCRYPT,
	DECRYPT
};

class GmZippor
{
	z_stream_s		m_ZipStream;
	int				m_ZipLevel;
	void Begin ();
	void End ();
public:
	explicit GmZippor (int level = Z_DEFAULT_COMPRESSION);
	~GmZippor ();
	bool compress (char * pData, unsigned uiInLen, char * pOut, unsigned & outSize, bool bIsEnd);
};

class GmUnzippor
{
	z_stream_s		m_ZipStream;
	void Begin ();
	void End ();
public:
	GmUnzippor ();
	~GmUnzippor ();
	bool decompress (char * pData, unsigned uiInLen, char * pOut, unsigned & outSize, bool & bIsEnd);
};

class GmEncrytor
{
public:
	virtual unsigned CaculateData (char * pData, unsigned uiSize) = 0;
	virtual void BeginFile (char * pData, char * pData2) = 0;
	virtual unsigned EndFile (char * pData, unsigned Length) = 0;
};

enum GmZipKeySize
{
	ZKS128 = 128,
	ZKS192 = 192,
	ZKS256 = 256,
};

class GmWizipAesEnc : public GmEncrytor
{
	int m_Mode;
	string m_password;
	GmEncDir m_dir;
	fcrypt_ctx	m_ctx;
public:
	GmWizipAesEnc (GmEncDir method, GmZipKeySize keySize, const string & password);
	~GmWizipAesEnc ();
	unsigned CaculateData (char * pData, unsigned uiSize);
	void BeginFile (char * pSalt, char * pPasVeri);
	unsigned EndFile (char * pData, unsigned Length);
};

class GmWinzipRand
{
	prng_ctx				_ctx;
	static GmWinzipRand * m_pIntance;
	GmWinzipRand ();
	~GmWinzipRand ();
public:
	static GmWinzipRand * GetInstance ();
	static void DeleteInstance ();
	void GenerateRand (char *, unsigned);
};

#endif //__GM_ENC_DEFLATE_H__