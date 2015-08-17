//
// Monitor's implementation.
// Author:yeyouqun@163.com
// 2010-3-16
//
#include <gmpre.h>
#include <engine/monitor.h>

void GmMonitor::HandleNewFile (const wxString & file)
{
	m_szFiles.push_back (file);
}

void GmMonitor::GetAllNewFile (vector<wxString> & vszFiles)
{
	vszFiles = m_szFiles;
}

void GmMonitor::HandleDirectoryInfo (const GmDirectoryInfo & Info)
{
	m_Info = Info;
	DoHandleDirectoryInfo (m_Info);
}

const GmDirectoryInfo & GmMonitor::GetDirectoryInfo () const
{
	return m_Info;
}
