//
// Utililty
// Author:yeyouqun@163.com
// 2009-11-15
//

#include <engine/defs.h>
#include <crypto++/include/integer.h>
#include <crypto++/include/osrng.h>
#include <crypto++/include/nbtheory.h>
#pragma warning(disable:4996)
#include <ace/File_Lock.h>
#include <engine/exception.h>

#ifndef __GM_UTIL_H__
#define __GM_UTIL_H__

class ENGINE_EXPORT GmAppInfo
{
	GmAppInfo ();
	GmAppInfo & operator = (const GmAppInfo &);
public:
	static wxString GetAppPath ();
	static wxString GetAppName ();
	static wxString GetFullAppName ();
};

wxString ENGINE_EXPORT GetFileName (const wxString & FileName);
wxString ENGINE_EXPORT AssemblePath (const wxString & Path, const wxString & FileName);
string ENGINE_EXPORT AssemblePath (const string & Path, const string & FileName);
byte4 ENGINE_EXPORT SplitPath (const wxString & source, vector<wxString> & paths);
wxString ENGINE_EXPORT GetSystemError ();

wxString ENGINE_EXPORT GetDesktopFolder ();
wxString ENGINE_EXPORT GetMyDocFolder ();
wxString ENGINE_EXPORT GetUserHome ();

string ENGINE_EXPORT GetFileName (const string & source);
string ENGINE_EXPORT GetPathName (const string & source);
wxString ENGINE_EXPORT GetPathName (const wxString & source);
string ENGINE_EXPORT GetFullPath (const string & source);
byte4 ENGINE_EXPORT SplitPath (const string & source, vector<string> & paths);
bool ENGINE_EXPORT IsValidName (const wxString &);

////////////////////////////////////////////////////////////////////////

wxString GetTmpFile (const wxString &);

inline wxString ToWxString (const string & str)
{
	return wxString (str.c_str (), wxConvLibc);
}

inline string ToStlString (const wxString & str)
{
	return string (str.mb_str (wxConvLibc));
}

inline FILETIME UByte8ToFileTime (ubyte8 fileTime)
{
	FILETIME time;
	time.dwHighDateTime = (unsigned int) (fileTime >> 32);
	time.dwLowDateTime = (unsigned int) (fileTime);
	return time;
}

inline ubyte8 FileTimeToUByte8 (FILETIME fileTime)
{
	ubyte8 time = fileTime.dwHighDateTime;
	time <<= 32;
	time += fileTime.dwLowDateTime;
	return time;
}

inline bool IsFile (const wxString & szFullName)
{
	return wxFile::Exists (szFullName);
}

inline wxString GetZipTypeStr ()
{
	return (wxT (".ZIP"));
}

inline wxString Get7zTypeStr ()
{
	return (wxT (".7Z"));
}

inline wxString GetSyncTypeStr ()
{
	return _("IDS_SYNC");
}

inline wxString GetDupTypeStr ()
{
	return _("IDS_COPY");
}

inline void GetSupportFormat (wxArrayString & items)
{
	items.Add (GetDupTypeStr ());
	items.Add (GetSyncTypeStr ());
	items.Add (GetZipTypeStr ());
	//items.Add (Get7zTypeStr ());
	//items.Add (wxT (".RAR"));
}

template <typename PointerType
		, template <typename T, typename A = std::allocator<T> > 
		class ContType = std::vector>
struct GmAutoClearVector
{
	ContType<PointerType*> & vpTypePointers;
	GmAutoClearVector (ContType<PointerType*> & vpTypePointers) : vpTypePointers (vpTypePointers) {}
	void Clear ()
	{
		//
		// 调用此接口不需要清理。
		//
		vpTypePointers.clear ();
	}
	void ReleaseNow ()
	{
		for_each (vpTypePointers.begin (), vpTypePointers.end (), ReleaseMemory ());
		vpTypePointers.clear ();
	}

	~GmAutoClearVector ()
	{
		ReleaseNow ();
	}
};

void RemoveDirTree (const wxString &);
bool IsNtfsPartition (const wxString &);

wxString GetTimeFolderName (const wxDateTime & now);

//
// 可以用于单实例或者同一进程中的文件锁。
//
class ENGINE_EXPORT GmFileLocker
{
public:
	GmFileLocker (const wxString & file);
	bool Lock ();
	void Unlock ();
private:
	//FILE *				m_File;
	auto_ptr<ACE_File_Lock> m_locker;
};

inline void ReplaceAll (string & s, const string & old, const string & dest)
{
	string::size_type pos = 0;
	while ((pos = s.find (old, pos)) != string::npos)
		s.replace (pos, old.size (), dest);
}

inline static wxString RemoveLastSep (const wxString & dir)
{
	if (dir.IsEmpty ()) return dir;
	wxString d1 = dir;
	wxChar lc1 = *d1.rbegin ();
	if (lc1 == '\\' || lc1 == '/') d1.RemoveLast ();
	return d1;
}

bool ENGINE_EXPORT IsSameDirName (const wxString & DirName1, const wxString & DirName2);
inline bool IsSameFile (const wxString & FileName1, const wxString & FileName2)
{
	return IsSameDirName (FileName1, FileName2);
}

ENGINE_EXPORT wxString MakeCapacityString (ubyte8 size);
inline wxString EnclosureWithQuotMark (const wxString & str)
{
	wxString mark = wxT("\"");
	return (mark + str + mark);
}

inline bool IsVolume (const wxString & path)
{
#ifdef _WIN32
	wxString volume, szPath;
	wxFileName::SplitVolume (path, &volume, &szPath);
	if (!volume.IsEmpty () && szPath.IsEmpty ())
		return true;
#endif
	return false;
}

#endif //