//
// collector's base class.
// Author:yeyouqun@163.com
// 2010-4-14
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/monitor.h>
#include <engine/collector.h>
#include <engine/exception.h>
#include <crypto++/include/crc.h> 

GmCollector::GmCollector (GmMonitor * pMonitor
						, bool bNeedTellSize/* = false*/) : m_pMonitor (pMonitor)
												, m_block (READ_BLOCK_LEN)
												, m_bTellSize (bNeedTellSize)
{
}


GmCollector::~GmCollector ()
{
}

void GmCollector::HandleFile (const wxString & szFileName)
{
	wxFile file (szFileName);
	ubyte8 TotalSize = file.Length ();
	ubyte8 HandleSize = 0;

	while (true) {
		if (m_pMonitor != 0 && m_pMonitor->IsStopped ())
			throw GmStopProcedureException ();

		ubyte4 BytesRead = file.Read (m_block.base (), m_block.size ());
		bool bEndFile = false;
		HandleSize += BytesRead;
		if (HandleSize >= TotalSize) bEndFile = true;

		HanldeThisBlock (m_block.base (), BytesRead, bEndFile);
		if (NeedTellSize () && m_pMonitor != 0) m_pMonitor->HandleSize (BytesRead);
		if (bEndFile)
			break;
	}
}

void GmCollector::OnError (const wxString & Message)
{
	if (m_pMonitor != 0) m_pMonitor->HandleError (Message);
}

void GmCollector::OnWarn (const wxString & Warn)
{
	if (m_pMonitor != 0) m_pMonitor->HandleWarn (Warn);
}


void GmCollector::OnMessage (const wxString & message)
{
	if (m_pMonitor != 0) m_pMonitor->HandleMessage (message);
}

wxString GetCrc32String (const char *pData, unsigned length)
{
	CryptoPP::CRC32 crc32;
	vector<unsigned char> digest (crc32.DigestSize ());
	crc32.CalculateDigest (&digest[0], (unsigned char*)pData, length);
	wxString crcstr;
	for (size_t i = 0; i < digest.size (); ++i)
		crcstr = wxString::Format (wxT ("%s%.2X"), crcstr.c_str (), digest[i]);
	return crcstr;
}
