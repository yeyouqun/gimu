//
// Gimu writer definition.
// Author:yeyuqun@163.com
// 2009-11-8
//
#ifndef __GM_WRITER_H__
#define __GM_WRITER_H__

class ENGINE_EXPORT GmWriter
{
protected:
	wxString		m_path;
public:
	virtual unsigned Write (const char * pData, unsigned Length) = 0;
	virtual ubyte8 Seek (ubyte8 pos, wxSeekMode from) = 0;
	virtual void Close () = 0;
	virtual bool Rebind (const wxString & fileName, const GmLeafInfo * pInfo = 0) = 0;
	virtual bool CreateDir (const wxString & name) = 0;
	virtual void SetFileAttr (const ubyte8 & ct, const ubyte8 & mt, const ubyte8 & at, ubyte4 attribute) = 0;
	virtual bool SetDestPath (const wxString & path) = 0;
	virtual bool Exists (const wxString & FileName) const = 0;
	virtual bool RemoveFile (const wxString & FileName) const = 0;
	virtual bool RemoveDir (const wxString & DirName) const = 0;
	virtual bool MoveFileTo (const wxString & NewFile, const wxString & OldFile) { return false; }
	virtual ~GmWriter () {}
	wxString GetDestPath () const { return m_path; }
};

class ENGINE_EXPORT GmLocalWriter : public GmWriter
{
	GmOutputStream	m_out;
	wxString		m_szFileOrDir;
	bool			m_bIsFile;
	GmMonitor *		m_pMonitor;
	int				m_Option;
public:
	GmLocalWriter (GmMonitor * pMonitor = 0, int Option = 0);
	virtual unsigned Write (const char * pData, unsigned Length);
	virtual ubyte8 Seek (ubyte8 pos, wxSeekMode from);
	virtual void Close ();
	virtual bool Rebind (const wxString & fileName, const GmLeafInfo * pInfo);
	virtual bool CreateDir (const wxString & name);
	virtual void SetFileAttr (const ubyte8 & ct, const ubyte8 & mt, const ubyte8 & at, ubyte4 attribute);
	virtual bool SetDestPath (const wxString & path);
	virtual bool Exists (const wxString & FileName) const;
	virtual bool RemoveFile (const wxString & FileName) const;
	virtual bool RemoveDir (const wxString & DirName) const;
	virtual bool MoveFileTo (const wxString & NewFile, const wxString & OldFile);
	~GmLocalWriter ();
};

bool SetDirectoryTimes (const wxString & dir
						, const wxDateTime *dtAccess
                        , const wxDateTime *dtMod
                        , const wxDateTime *dtCreate);

#endif //__GM_WRITER_H__