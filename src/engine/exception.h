//
//
// Gimu defs
// Authour:yeyouqun@163.com
// 2009-11-8
//
//
#ifndef __GM_EXCEPTION_H__
#define __GM_EXCEPTION_H__
class ENGINE_EXPORT GmException
{
public:
	GmException (const wxString & szMessage);
	virtual ~GmException ();
	wxString GetMessage ();
	void SetMessage (const wxString & szMessage);
protected:
	wxString m_szMessage;
};

class GmStopProcedureException : public GmException
{
public:
	GmStopProcedureException () : GmException (wxString ()) {}
};

class GmPasswordNotCorrectException : public GmException
{
public:
	GmPasswordNotCorrectException (const wxString & str) : GmException (str) {}
};

#endif //