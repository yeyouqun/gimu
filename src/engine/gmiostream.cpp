//
// io stream
// Author:yeyouqun@163.com
// 2009-12-6
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/gmiostream.h>
#include <engine/util.h>
#include <engine/exception.h>

bool GmOutputStream::Open (const wxString & szOStream, bool bCreate/* = false*/)
{
	Close ();
	m_szFileName = szOStream;

	wxFile::OpenMode mode = wxFile::read_write;
	if (bCreate && !wxFile::Exists (m_szFileName))
		mode = wxFile::write ;

	return m_Out.Open (szOStream, mode);
}

ubyte8 GmOutputStream::Seek (ubyte8 pos, wxSeekMode From)
{
	if (!m_Out.IsOpened ()) return -1;
	return m_Out.Seek (pos, From);
}

bool GmOutputStream::IsOpened ()
{
	return m_Out.IsOpened ();
}

ubyte4 GmOutputStream::Write (const char * pData, ubyte4 Size)
{
	if (!m_Out.IsOpened ()) return -1;
	return static_cast<ubyte4> (m_Out.Write (pData, Size));
}

ubyte8 GmOutputStream::Tell ()
{
	if (!m_Out.IsOpened ()) return -1;
	return m_Out.Tell ();
}


void GmOutputStream::Flush ()
{
	if (!m_Out.IsOpened ()) return;
	m_Out.Flush ();
}

void GmOutputStream::Close ()
{
	m_Out.Close ();
}

/////////////////////

bool GmInputStream::Open (const wxString & szIStream)
{
	Close ();
	m_szFileName = szIStream;
	return m_In.Open (szIStream);
}

ubyte8 GmInputStream::Seek (ubyte8 pos, wxSeekMode From)
{
	if (!m_In.IsOpened ()) return -1;
	return m_In.Seek (pos, From);
}
bool GmInputStream::IsOpened ()
{
	return m_In.IsOpened ();
}

ubyte4 GmInputStream::Read (char * pData, ubyte4 Size)
{
	if (!m_In.IsOpened ()) return -1;
	return m_In.Read (pData, Size);
}

ubyte8 GmInputStream::Tell ()
{
	if (!m_In.IsOpened ()) return -1;
	return m_In.Tell ();
}

void GmInputStream::Close ()
{
	m_In.Close ();
}

int GmMemoryOutStream::AddString (const wxString & str)
{
	string str2 = ToStlString (str);
	ubyte4 size = static_cast<ubyte4> (str2.length ());
	if (size + sizeof (ubyte4) > m_block.space ()) {
		m_block.size (m_block.size () * 2);
	}

	AddUByte4 (size);
	memcpy (m_block.wr_ptr (), str2.c_str (), size);
	m_block.wr_ptr (size);
	return size + sizeof (ubyte4);
}

#define ADDVALUE(value_type,value,tochars)				\
	do {												\
		if (sizeof (value_type) > m_block.space ())		\
			m_block.size (m_block.size () * 2);			\
														\
		tochars (m_block.wr_ptr (), value);				\
		m_block.wr_ptr (sizeof (value_type));			\
		return sizeof (value_type);						\
	}while (0)

int GmMemoryOutStream::AddByte4 (const byte4 & value)
{
	ADDVALUE (byte4, value, Byte4ToChars);
}

int GmMemoryOutStream::AddByte8 (const byte8 & value)
{
	ADDVALUE (byte8, value, Byte8ToChars);
}

int GmMemoryOutStream::AddByte2 (const byte2 & value)
{
	ADDVALUE (byte2, value, Byte2ToChars);
}

int GmMemoryOutStream::AddChar (char chr)
{
	if (sizeof (chr) > m_block.space ())
		m_block.size (m_block.size () * 2);

	*m_block.wr_ptr () = static_cast<unsigned char> (chr);
	m_block.wr_ptr (sizeof (chr));
	return sizeof (chr);
}

wxString GmMemoryInStream::GetString ()
{
	string str;
	ubyte4 size = GetUByte4 ();
	if (size > m_block.length ())
		throw GmException (_("IDS_BUFFER_ERROR"));

	str.assign (m_block.rd_ptr (), size);
	m_block.rd_ptr (size);
	return ToWxString (str);
}

#define GETVALUE(value_type,tovalue)							\
	do {														\
		if (sizeof (value_type) > m_block.length ())			\
			throw GmException (_("IDS_BUFFER_ERROR"));			\
																\
		value_type value;										\
		tovalue (m_block.rd_ptr (), &value);					\
		m_block.rd_ptr (sizeof (value_type));					\
		return value;											\
	}while (0)

byte4 GmMemoryInStream::GetByte4 ()
{
	GETVALUE (byte4, CharsToByte4);
}

byte8 GmMemoryInStream::GetByte8 ()
{
	GETVALUE (byte8, CharsToByte8);
}

byte2 GmMemoryInStream::GetByte2 ()
{
	GETVALUE (byte2, CharsToByte2);
}

char GmMemoryInStream::GetChar ()
{
	if (sizeof (char) > m_block.length ())
		throw GmException (_("IDS_BUFFER_ERROR"));

	char chr = *m_block.rd_ptr ();
	m_block.rd_ptr (sizeof (char));
	return chr;
}
