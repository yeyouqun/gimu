//
// Zip Reader Calculator
// Author:yeyouqun@163.com
// 2009-11-10
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/exception.h>
#include <engine/uifblock.h>
#include <engine/gmiostream.h>
#include <engine/reader.h>
#include <engine/util.h>

unsigned GmLocalReader::Read (char *pData, unsigned Length)
{
	if (!m_In.IsOpened ()) return 0;
	return m_In.Read (pData, Length);
}

bool GmLocalReader::Rebind (const wxString & FileName, ubyte8 & FileSize)
{
	FileSize = 0;
	wxString szFileName = AssemblePath (m_szPath, FileName);

	if (!wxFile::Exists (szFileName)) {
		if (m_pMonitor != 0)
			szFileName = m_pMonitor->SelectFile (szFileName);
		else return false;
	}

	if (m_In.IsOpened ()) m_In.Close ();
	FileSize = wxFile (szFileName).Length ();
	if (!m_In.Open (szFileName)) return false;
	return true;
}

ubyte8 GmLocalReader::Seek (ubyte8 pos, wxSeekMode where)
{
	if (!m_In.IsOpened ()) return -1;
	return m_In.Seek (pos, where);
}

GmLocalReader::GmLocalReader (GmMonitor * pMonitor)
				: m_In ()
				, m_pMonitor (pMonitor)
{
}

GmLocalReader::~GmLocalReader ()
{
	assert (!m_In.IsOpened ());
}

void GmLocalReader::SetDestPath (const wxString & path)
{
	m_szPath = path;
}

void GmLocalReader::Close ()
{
	if (m_In.IsOpened ()) m_In.Close ();
}
