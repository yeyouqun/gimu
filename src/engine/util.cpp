//
// Utililty
// Author:yeyouqun@163.com
// 2009-11-15
//

#include <gmpre.h>
#include <wx/stdpaths.h>
#include <engine/util.h>
#include <engine/defs.h>
#include <shlobj.h>
#include <crypto++/include/integer.h>
#include <crypto++/include/osrng.h>
#include <crypto++/include/nbtheory.h>
#include <stdlib.h>
#include <ace/File_Lock.h>

wxString GmAppInfo::GetAppPath ()
{
	return GetPathName (GmAppInfo::GetFullAppName ());
}

wxString GmAppInfo::GetAppName ()
{
	return GetFileName (GmAppInfo::GetFullAppName ());
}

wxString GmAppInfo::GetFullAppName ()
{
	return wxStandardPaths ().GetExecutablePath ();
}


string::size_type FindAnyCharFirst (const string & source, const string & chars, string::size_type from = string::npos)
{
	typedef string::const_iterator iterator;
	string::size_type pos = string::npos;
	if ((pos = source.find_first_of (chars, from)) != string::npos)
		return pos;

	return string::npos;
}

string::size_type FindAnyCharLast (const string & source, const string & chars, string::size_type from = string::npos)
{
	typedef string::const_iterator iterator;
	string::size_type pos = string::npos;
	if ((pos = source.find_last_of (chars, from)) != string::npos)
		return pos;

	return string::npos;
}

string GetFileName (const string & source)
{
	if (source.empty ()) return string ();
	string::size_type pos;
	string::value_type lc = *source.rbegin ();
	if (lc == '\\' || lc == '/') pos = source.size () - 2;
	else pos = string::npos;

#ifdef _WIN32
	//
	// 如果是根目录
	//
	if (pos != string::npos && source.at (pos) == ':')
		return string ();
#endif//

	pos = FindAnyCharLast (source, "/\\", pos);
	if (pos == string::npos) return source;
	else return source.substr (pos + 1);
}

wxString GetFileName (const wxString & FileName)
{
	wxString szFileName, szExt;
	bool bHasExt;
	wxFileName::SplitPath (FileName, 0, 0, &szFileName, &szExt, &bHasExt);
	if (!bHasExt) return szFileName;

	return szFileName + wxT(".") + szExt;
}

wxString GetPathName (const wxString & source)
{
	wxString szPath;
	wxString szVol;
	wxFileName::SplitPath (source, &szVol, &szPath, 0, 0);
#ifdef _WIN32
	if (!szVol.IsEmpty ()) {
		szVol += wxT(":");
		szVol += szPath;
		return szVol;
	}
#else
#endif//
	return szPath;
}

string GetPathName (const string & source)
{
	return ToStlString (GetPathName (ToWxString (source)));
}

string GetFullPath (const string & source)
{
	if (source.size () == 0)
		return source;

	string::size_type pos = 0;
	if (source[source.size () - 1] == '\\') {
		return source;
	}
	else {
		pos = source.find_last_of ('\\', source.size ());
		return source.substr (0, pos + 1);
	}
}

byte4 SplitPath (const string & source, vector<string> & pathes)
{
	if (source.size () == 0)
		return 0;

	string::size_type from = 0;
	while (true) {
		string::size_type pos = FindAnyCharFirst (source, "/\\", from);
		string sub = source.substr (from, pos - from);
		if (!sub.empty ()) pathes.push_back (sub);
		if (pos == string::npos) break;
		from = pos + 1;
	}

	return (ubyte4)pathes.size ();
}

wxString AssemblePath (const wxString & Path, const wxString & FileName)
{
	if (Path.IsEmpty ())
		return FileName;

	wxString::value_type lc = *Path.rbegin ();
	if (lc == '\\' || lc == '/')
		return Path + FileName;

	return Path + wxT ("\\") + FileName;
}

string ENGINE_EXPORT AssemblePath (const string & Path, const string & FileName)
{
	if (Path.empty ())
		return FileName;

	string::value_type lc = *Path.rbegin ();
	if (lc == '\\' || lc == '/')
		return Path + FileName;

	return Path +  string ("\\") + FileName;
}

byte4 SplitPath (const wxString & source, vector<wxString> & paths)
{
	vector<string> stlpathes;
	ubyte4 segs = SplitPath (ToStlString (source), stlpathes);
	for (size_t index = 0; index < stlpathes.size (); ++index)
		paths.push_back (ToWxString (stlpathes[index]));

	return segs;
}

wxString GetTmpFile (const wxString & source)
{
	wxString path = GetPathName (source);
	ubyte8 time = wxDateTime::GetTimeNow ();
	return AssemblePath (path, wxULongLong (time).ToString ());
}

wxString GetSystemError ()
{
	return wxSysErrorMsg (wxSysErrorCode ());
}


wxString ENGINE_EXPORT GetDesktopFolder ()
{
	char_t chars[MAX_PATH];
	BOOL result = SHGetSpecialFolderPath (NULL, chars, CSIDL_DESKTOP, 0);
	if (result == TRUE)
		return wxString (chars);

	return wxEmptyString;
}

wxString ENGINE_EXPORT GetMyDocFolder ()
{
	char_t chars[MAX_PATH];
	BOOL result = SHGetSpecialFolderPath (NULL, chars, CSIDL_PERSONAL, 0);
	if (result == TRUE)
		return wxString (chars);

	return wxEmptyString;
}

wxString ENGINE_EXPORT GetUserHome ()
{
	return ::wxGetHomeDir ();
}

bool ENGINE_EXPORT IsValidName (const wxString &name)
{
	wxString forbiden = wxFileName::GetForbiddenChars ();
	string::size_type pos = FindAnyCharFirst (ToStlString (name), ToStlString (forbiden), 0);
	if (pos != string::npos)
		return false;

	return true;
}

void TraverseDeleteTheDir (const wxString & DirName)
{
	wxDir dir (DirName);
	wxString DirOrFile;
	bool bEnumerateOK = dir.GetFirst (&DirOrFile);
	while (bEnumerateOK) {
		wxString szFullPath = AssemblePath (DirName, DirOrFile);
		if (IsFile (szFullPath)) {
			if (!wxRemoveFile (szFullPath)) throw GmException (GetSystemError ());
		}else {
			TraverseDeleteTheDir (szFullPath);
			if (!wxRmdir (szFullPath)) throw GmException (GetSystemError ());
		}
		bEnumerateOK = dir.GetNext (&DirOrFile);
	}
}

void RemoveDirTree (const wxString & dir)
{
	if (!wxDir::Exists (dir) && !wxIsAbsolutePath (dir))
		return;

	TraverseDeleteTheDir (dir);
	if (!wxRmdir (dir)) throw GmException (GetSystemError ());
}

bool IsNtfsPartition (const wxString & directory)
{
	wxString volume;
	wxFileName::SplitPath (directory, &volume, 0, 0, 0);
	if (volume.IsEmpty ()) return false;
	if (volume.Last () != '\\' && volume.Last () != '/')
		volume += wxT (":\\");

	char_t szFsType [MAX_PATH+1];
	DWORD CompLen;
	DWORD Flags;
	BOOL result = GetVolumeInformation (volume.c_str ()
										, NULL
										, NULL
										, NULL
										, &CompLen
										, &Flags
										, szFsType
										, MAX_PATH+1);
	if (!result) return false;
	if (_wcsicmp (szFsType, wxT ("NTFS")) == 0) return true;
	return false;
}

bool CheckSoftwareSN (const wxString & serial)
{
	return true;
}

wxString CalcSoftwareSN (const wxString & user)
{
	return user;
}

void GetAdaptersAddresses (const vector<wxString> & address)
{
}

wxString GetTimeFolderName (const wxDateTime & now)
{
	wxDateTime::Tm tmdata = now.GetTm ();
	wxString szNameDateStr = wxString::Format (wxT ("%d-%d-%d_%d-%d-%d")
										, tmdata.year
										, tmdata.mon + 1
										, tmdata.mday
										, tmdata.hour
										, tmdata.min
										, tmdata.sec);
	return szNameDateStr;
}


GmFileLocker::GmFileLocker (const wxString & file)
{
	m_locker.reset (new ACE_File_Lock (ACE_TEXT_WCHAR_TO_TCHAR (file.c_str ()), O_RDWR | O_CREAT, 0));
}

bool GmFileLocker::Lock ()
{
	if (m_locker->tryacquire () < 0) return false;
	return true;
}

void GmFileLocker::Unlock ()
{
	m_locker->release ();
}

bool IsSameDirName (const wxString & DirName1, const wxString & DirName2)
{
#ifdef _WIN32
	if (DirName1.CmpNoCase (DirName2)) return true;
	if (DirName1.IsEmpty () || DirName2.IsEmpty ()) return false;
	if (RemoveLastSep (DirName1).CmpNoCase (RemoveLastSep (DirName2))) return true;
#else
	if (DirName1 == DirName2) return true;
	if (DirName1.IsEmpty () || DirName2.IsEmpty ()) return false;
	if (RemoveLastSep (DirName1) == RemoveLastSep (DirName2)) return true;
#endif
	return false;
}

ENGINE_EXPORT wxString MakeCapacityString (ubyte8 size)
{
	double capacity = size;
	wxString str;
	if (size > (1024 * 1024 * 1024 * 1024ull)) {
		capacity /= (1024 * 1024 * 1024 * 1024ull);
		str = wxString::Format (wxT ("%.3 T"), capacity);
	}
	else if (size > (1024 * 1024 * 1024ull)) {
		capacity /= (1024 * 1024 * 1024ull);
		str = wxString::Format (wxT ("%.3 G"), capacity);
	}
	else if (size > (1024 * 1024ull)) {
		capacity /= (1024 * 1024ull);
		str = wxString::Format (wxT ("%.3f M"), capacity);
	}
	else if (size > 1024ull) {
		capacity /= 1024ull;
		str = wxString::Format (wxT ("%.2f K"), capacity);
	}
	else {
		str = wxString::Format (wxT ("%d"), (int)size);
	}

	return str;
}