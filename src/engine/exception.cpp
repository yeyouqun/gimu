//
//
// Gimu defs
// Authour:yeyouqun@163.com
// 2009-11-8
//
//
#include <gmpre.h>
#include <engine/exception.h>

GmException::GmException (const wxString & szMessage)
			: m_szMessage (szMessage)
{
}

GmException::~GmException ()
{
}

wxString GmException::GetMessage ()
{
	return m_szMessage;
}

void GmException::SetMessage (const wxString & szMessage)
{
	m_szMessage = szMessage;
}