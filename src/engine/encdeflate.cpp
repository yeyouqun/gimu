//
// zip writer and its block definition.
// Author:yeyouqun@163.com
// 2009-11-9
//
#include <gmpre.h>
#include <engine/encdeflate.h>
#include <engine/defs.h>
#include <engine/exception.h>
#include <engine/util.h>

GmZippor::GmZippor (int level/* = Z_DEFAULT_COMPRESSION*/)
					: m_ZipLevel (level)
{
	m_ZipStream.next_in = 0;
	m_ZipStream.next_out = 0;

	m_ZipStream.avail_in = 0;
	m_ZipStream.avail_out = 0;

	m_ZipStream.zalloc = Z_NULL;
	m_ZipStream.zfree = Z_NULL;
	m_ZipStream.opaque = Z_NULL;
	Begin ();
}

GmZippor::~GmZippor ()
{
	End ();
}

void GmZippor::Begin ()
{
	int result = deflateInit2(&m_ZipStream
							, m_ZipLevel
							, Z_DEFLATED
							, -MAX_WBITS
							, MAX_MEM_LEVEL
							, Z_DEFAULT_STRATEGY);

	if (result != Z_OK) {
		deflateEnd (&m_ZipStream);
		throw GmException (ToWxString (m_ZipStream.msg));
	}
}

void GmZippor::End ()
{
	deflateEnd (&m_ZipStream);
}

void GmUnzippor::Begin ()
{
	int result = inflateInit2(&m_ZipStream, -MAX_WBITS);

	if (result != Z_OK) {
		inflateEnd (&m_ZipStream);					//结束！		
		throw GmException (ToWxString (m_ZipStream.msg));
	}
}


void GmUnzippor::End ()
{
	inflateEnd (&m_ZipStream);
}

bool GmZippor::compress (char * pData, unsigned uiInLen, char * pOut, unsigned & OutSize, bool bIsEnd)
{
	unsigned OriSize = OutSize;

	if (m_ZipStream.avail_in == 0) {
		//
		// 上次的数据没有全部输出。这次是再次用原来的缓冲调用，因此不需要再次设定输入参数。
		//
		m_ZipStream.next_in	= (Bytef*)pData;
		m_ZipStream.avail_in	= uiInLen;
	}
	m_ZipStream.next_out	= (Bytef*)pOut;
	m_ZipStream.avail_out	= OutSize;

	int Result = deflate (&m_ZipStream, bIsEnd ? Z_FINISH : Z_NO_FLUSH);
	bool bNeedNextCall = false;
	if (Result == Z_OK) {
		if (m_ZipStream.avail_out == 0 && m_ZipStream.avail_in > 0) bNeedNextCall = true;
	}
	else if (Result == Z_STREAM_END) deflateReset (&m_ZipStream);
	else throw GmException (wxT ("zip error"));
	
	OutSize = OriSize - m_ZipStream.avail_out;
	return bNeedNextCall;
}

GmUnzippor::GmUnzippor ()
{
	m_ZipStream.next_in	= 0;
	m_ZipStream.next_out	= 0;
	m_ZipStream.avail_in	= 0;
	m_ZipStream.avail_out = 0;
	m_ZipStream.zalloc	= Z_NULL;
	m_ZipStream.zfree		= Z_NULL;
	m_ZipStream.opaque	= Z_NULL;
	Begin ();
}

GmUnzippor::~GmUnzippor ()
{
	End ();
}

bool GmUnzippor::decompress (char * pData, unsigned uiInLen, char * pOut, unsigned & OutSize, bool &bIsEnd)
{
	unsigned int OriSize = OutSize;

	if (m_ZipStream.avail_in == 0) {
		//
		// 上次的数据没有全部输出。这次是再次用原来的缓冲调用，因此不需要再次设定输入参数。
		//
		m_ZipStream.next_in	= (Bytef*)pData;
		m_ZipStream.avail_in	= uiInLen;
	}

	m_ZipStream.next_out	= (Bytef*)pOut;
	m_ZipStream.avail_out	= OutSize;

	int Result = inflate (&m_ZipStream, Z_SYNC_FLUSH);
	bool bNeedNextCall = false;
	if (Result == Z_OK) {
		if (m_ZipStream.avail_out == 0 && m_ZipStream.avail_in > 0) bNeedNextCall = true;
	}
	else if (Result == Z_STREAM_END) inflateReset (&m_ZipStream);
	else throw GmException (wxT ("unzip error"));
	OutSize = OriSize - m_ZipStream.avail_out;
	return bNeedNextCall;
}

GmWizipAesEnc::GmWizipAesEnc (GmEncDir method, GmZipKeySize KeySize, const string& password)
				: m_dir (method)
				, m_password (password)
{
#ifndef FIXED_TABLES
	gen_tabs ();
#endif
	m_Mode = KeySize == ZKS128 ? 1 : KeySize == ZKS192 ? 2 : 3;
}

void GmWizipAesEnc::BeginFile (char * pSalt, char * pPasVeri)
{
 	const unsigned char * passwd = (unsigned char *)m_password.c_str ();
 	size_t size = (size_t) m_password.length ();
 	if (size > MAX_PWD_LENGTH)
 		throw GmException (wxT ("The length of password is over MAX_PWD_LENGTH"));
 		
 	int result = fcrypt_init (m_Mode, passwd, (unsigned) size, (unsigned char *)pSalt
			 				#ifdef PASSWORD_VERIFIER
							, (unsigned char *)pPasVeri
			 				#endif
			 				, &m_ctx);
 	if (result != GOOD_RETURN)
 		throw GmException (wxT ("fcrypt_init failed."));
}

unsigned GmWizipAesEnc::CaculateData (char * pData, unsigned uiSize)
{
	if (m_dir == ENCRYPT)
		fcrypt_encrypt ((unsigned char *)pData, uiSize, &m_ctx);
	else
		fcrypt_decrypt ((unsigned char *)pData, uiSize, &m_ctx);

	return uiSize;
}

unsigned GmWizipAesEnc::EndFile (char * pData, unsigned Length)
{
	Length = fcrypt_end((unsigned char *)pData, &m_ctx);
	return Length;
}

GmWizipAesEnc::~GmWizipAesEnc ()
{
}

GmWinzipRand * GmWinzipRand::m_pIntance = 0;

GmWinzipRand * GmWinzipRand::GetInstance ()
{
	return m_pIntance == 0 ? m_pIntance = new GmWinzipRand () : m_pIntance;
}

void GmWinzipRand::DeleteInstance ()
{
	if (m_pIntance != 0) 	delete m_pIntance;
	m_pIntance = 0;
}


int GenerateRand (unsigned char buf[], unsigned int len)
{
	ubyte8 timenow = rand ();
	for (unsigned i = 0;i < len; ++i) {
		srand (rand ());
		timenow *= rand ();
		srand (rand ());
		timenow = rand ();
		srand (timenow);
		buf[i] = (rand () << i) & 0xFF;
	}
	return len;
}

GmWinzipRand::GmWinzipRand ()
{
	prng_init (&::GenerateRand, &_ctx);
}

GmWinzipRand::~GmWinzipRand ()
{
	prng_end (&_ctx);
}

void GmWinzipRand::GenerateRand (char * pData, unsigned Size)
{
	prng_rand ((unsigned char*)pData, Size, &_ctx);
	return;
}
