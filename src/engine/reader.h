//
// Zip Reader Calculator
// Author:yeyouqun@163.com
// 2009-11-10
//

#ifndef __GM_ZIP_READER_H__
#define __GM_ZIP_READER_H__

class GmReader
{
public:
	virtual unsigned Read (char *pData, unsigned Length) = 0;
	virtual bool Rebind (const wxString & fileName, ubyte8 & FileSize) = 0;
	virtual ubyte8 Seek (ubyte8 pos, wxSeekMode where) = 0;
	virtual void SetDestPath (const wxString & path) = 0;
	virtual void Close () = 0;
	virtual ~GmReader () {}
};

class GmLocalReader : public GmReader
{
	wxString		m_szPath;
	GmInputStream	m_In;
	GmMonitor *		m_pMonitor;
	virtual unsigned Read (char *pData, unsigned Length);
	virtual bool Rebind (const wxString & fileName, ubyte8 & FileSize);
	virtual ubyte8 Seek (ubyte8 pos, wxSeekMode where);
	virtual void SetDestPath (const wxString & path);
	virtual void Close ();
public:
	GmLocalReader (GmMonitor * pMonitor);
	~GmLocalReader ();
};

#endif //__GM_ZIP_READER_H__