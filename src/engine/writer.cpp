//
// Gimu writer definition.
// Author:yeyuqun@163.com
// 2009-11-8
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/exception.h>
#include <engine/uifblock.h>
#include <engine/gmiostream.h>
#include <engine/writer.h>
#include <engine/util.h>
#include <engine/job.h>
#include <engine/uifalgo.h>
#include <engine/uifblock.h>
#include <engine/uiffile.h>
#include <engine/analysis.h>

static void ConvertWxToFileTime(FILETIME *ft, const wxDateTime& dt)
{
    SYSTEMTIME st;
    st.wDay = dt.GetDay();
    st.wMonth = (WORD)(dt.GetMonth() + 1);
    st.wYear = (WORD)dt.GetYear();
    st.wHour = dt.GetHour();
    st.wMinute = dt.GetMinute();
    st.wSecond = dt.GetSecond();
    st.wMilliseconds = dt.GetMillisecond();

    FILETIME ftLocal;
	if (::SystemTimeToFileTime(&st, &ftLocal))
		LocalFileTimeToFileTime(&ftLocal, ft);
	return;
}

bool SetDirectoryTimes (const wxString & dir
						, const wxDateTime *dtAccess
                        , const wxDateTime *dtMod
                        , const wxDateTime *dtCreate)
{
#if defined(_WIN32)
	HANDLE handle = ::CreateFile(dir.c_str (), // name
								 GENERIC_READ | GENERIC_WRITE,
								 FILE_SHARE_READ | FILE_SHARE_WRITE, // (allow everything)
								 NULL, // no secutity attr
								 OPEN_EXISTING, // creation disposition
								 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // no flags
								 NULL);
	if (handle == INVALID_HANDLE_VALUE) return false;
    FILETIME ftAccess, ftCreate, ftWrite;
    if (dtCreate) ConvertWxToFileTime(&ftCreate, *dtCreate);
    if (dtAccess) ConvertWxToFileTime(&ftAccess, *dtAccess);
    if (dtMod) ConvertWxToFileTime(&ftWrite, *dtMod);
	if (::SetFileTime(handle,dtCreate ? &ftCreate : NULL,dtAccess ? &ftAccess : NULL,dtMod ? &ftWrite : NULL)) {
		CloseHandle (handle);
        return true;
	}

	CloseHandle (handle);
#else // other platform
    wxUnusedVar(dtCreate);

    if ( !dtAccess && !dtMod )
    {
        // can't modify the creation time anyhow, don't try
        return true;
    }

    // if dtAccess or dtMod is not specified, use the other one (which must be
    // non NULL because of the test above) for both times
    utimbuf utm;
    utm.actime = dtAccess ? dtAccess->GetTicks() : dtMod->GetTicks();
    utm.modtime = dtMod ? dtMod->GetTicks() : dtAccess->GetTicks();
    if ( utime(GetFullPath().fn_str(), &utm) == 0 )
    {
        return true;
    }
#endif // platforms
	return false;
}

unsigned GmLocalWriter::Write (const char * pData, unsigned Length)
{
	if (!m_out.IsOpened ()) return 0;
	if (m_pMonitor != 0) m_pMonitor->HandleSize (Length);
	return m_out.Write (pData, Length);
}

ubyte8 GmLocalWriter::Seek (ubyte8 pos, wxSeekMode from)
{
	return m_out.Seek (pos, from);
}

void GmLocalWriter::Close ()
{
	m_out.Close ();
}

bool GmLocalWriter::Rebind (const wxString & FileName, const GmLeafInfo * pInfo)
{
	m_szFileOrDir = AssemblePath (m_path, FileName);
	wxString path = GetPathName (m_szFileOrDir);
	if (!wxDir::Exists (path) && !wxMkdir (path)) return false;

	if (wxFile::Exists (m_szFileOrDir) && m_pMonitor != 0) {
		bool bGoOn = false;
		GmLeafEntry entry;
		if (!GetFileNode (entry, m_szFileOrDir, GetFileName (FileName)))
			return false;

		if (m_Option & RO_DECIDE_BY_USER) {
			assert (pInfo);
			GmLeafInfo info (entry);
			bGoOn = m_pMonitor->OverwriteFile (m_szFileOrDir, *pInfo, info);
		}
		else if (m_Option & RO_REPLACE_OLD) {
			if (entry.ModifyTime < pInfo->ModifyTime)
				bGoOn = true;
		}
		else if (m_Option & RO_SKIP) {
			bGoOn = false;
		}
		else if (m_Option & RO_OVERWRITE) {
			bGoOn = true;
		}

		if (!bGoOn) return false;
	}

	if (!m_out.Open (m_szFileOrDir, true)) return false;
	m_bIsFile = true;
	if (m_pMonitor != 0) m_pMonitor->HandleFile (m_szFileOrDir);
	return true;
}

bool GmLocalWriter::CreateDir (const wxString & name)
{
	m_szFileOrDir = AssemblePath (m_path, name);
	if (m_out.IsOpened ()) m_out.Close ();
	m_bIsFile = false;
	if (wxDir::Exists (m_szFileOrDir)) return true;
	if (!wxFileName::Mkdir (m_szFileOrDir, 0777, wxPATH_MKDIR_FULL)) return false;
	if (m_pMonitor != 0) m_pMonitor->HandleDir (m_szFileOrDir);
	return true;
}

void GmLocalWriter::SetFileAttr (const ubyte8 & ct, const ubyte8 & mt, const ubyte8 & at, ubyte4 attribute)
{
	if (m_out.IsOpened ()) m_out.Close ();
	if (m_szFileOrDir.IsEmpty ()) return;

	wxDateTime ctt = TimeTToWxDateTime(ct);
	wxDateTime mtt = TimeTToWxDateTime(mt);
	wxDateTime att = TimeTToWxDateTime(at);

	if (!m_bIsFile) {
		SetDirectoryTimes (m_szFileOrDir, &att, &mtt, &ctt);
	}
	else {
		wxFileName::FileName (m_szFileOrDir).SetTimes (&att, &mtt, &ctt);
	}
}

bool GmLocalWriter::SetDestPath (const wxString & path)
{
	m_path = path;
	if (wxDir::Exists (m_path)) return true;
	return wxFileName::Mkdir (m_path, 0777, wxPATH_MKDIR_FULL);
}

GmLocalWriter::GmLocalWriter (GmMonitor * pMonitor/* = 0*/, int Option/* = 0*/)
				: m_out ()
				, m_pMonitor (pMonitor)
				, m_Option (Option)
{
}

GmLocalWriter::~GmLocalWriter ()
{
	m_out.Close ();
}

bool GmLocalWriter::Exists (const wxString & FileName) const
{
	wxString pathfile = AssemblePath (m_path, FileName);
	return wxFile::Exists (pathfile);
}

bool GmLocalWriter::RemoveFile (const wxString & FileName) const
{
	wxString path = AssemblePath (m_path, FileName);
	return wxRemoveFile (path);
}

bool GmLocalWriter::RemoveDir (const wxString & DirName) const
{
	wxString path = AssemblePath (m_path, DirName);
	RemoveDirTree (path);
	return true;
}

bool GmLocalWriter::MoveFileTo (const wxString & NewFile, const wxString & OldFile)
{
	//
	// 文件在同一上目的地，因此移动文件是一个非常快的过程。
	//
	wxString NewFileName = AssemblePath (m_path, NewFile);
	wxString OldFileName = AssemblePath (m_path, OldFile);
	if (MoveFile (OldFileName, NewFileName)) return true;
	return false;
}
