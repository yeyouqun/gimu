//
// io stream
// Author:yeyouqun@163.com
// 2009-12-6
//
#include <wx/wfstream.h>
#ifndef __GM_IO_STREAM_H__
#define __GM_IO_STREAM_H__

class GmOutputStream
{
public:
	GmOutputStream () {}
	bool Open (const wxString & szOStream, bool bCreate = false);
	ubyte8 Seek (ubyte8 pos, wxSeekMode From);
	bool IsOpened ();
	ubyte4 Write (const char * pData, ubyte4 Size);
	void Flush ();
	ubyte8 SeekEnd () { return Seek (0, wxFromEnd); }
	void Close ();
	ubyte8 Tell ();
	wxString GetFileName () { return m_szFileName; }
private:
	wxString m_szFileName;
	wxFile m_Out; //read_write
};

class GmInputStream
{
public:
	GmInputStream () {}
	bool Open (const wxString & szIStream);
	ubyte8 Seek (ubyte8 pos, wxSeekMode From);
	bool IsOpened ();
	ubyte4 Read (char * pData, ubyte4 Size);
	ubyte8 SeekEnd () { return Seek (0, wxFromEnd); }
	void Close ();
	ubyte8 Tell ();
	wxString GetFileName () { return m_szFileName; }
private:
	wxString m_szFileName;
	wxFile m_In;
};

class GmMemoryOutStream
{
public:
	GmMemoryOutStream (size_t size = (10 * 1024))
		: m_block (size)
	{
	}
	int AddString (const wxString & str);
	int AddByte4 (const byte4 & value);
	int AddByte8 (const byte8 & value);
	int AddByte2 (const byte2 & value);
	int AddChar (char chr);

	int AddUByte4 (const ubyte4 & value) { return AddByte4 ((byte4)value); }
	int AddUByte8 (const ubyte8 & value) { return AddByte8 ((byte8)value); }
	int AddUByte2 (const ubyte2 & value) { return AddByte2 ((byte2)value); }
	int AddUChar (unsigned char chr) { return AddChar ((char)chr); }
	const ACE_Message_Block & GetBlock () { return m_block; }
private:
	ACE_Message_Block m_block;
};

class GmMemoryInStream
{
public:
	GmMemoryInStream (const char * pData, size_t size)
		: m_block (pData, size)
	{
		m_block.wr_ptr (size);
	}

	wxString GetString ();
	byte4 GetByte4 ();
	byte8 GetByte8 ();
	byte2 GetByte2 ();
	char GetChar ();

	ubyte4 GetUByte4 () { return (ubyte4)GetByte4 (); }
	ubyte8 GetUByte8 () { return (ubyte8)GetByte8 (); }
	ubyte2 GetUByte2 () { return (ubyte2)GetByte2 (); }
	unsigned char GetUChar () { return (unsigned char)GetChar (); }
private:
	ACE_Message_Block m_block;
};

#endif //