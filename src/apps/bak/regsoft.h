//
// register gimu dialog.
// Author:yeyouqun@163.com
// 2010-5-23
//

#ifndef __REGSOFT_H__
#define __REGSOFT_H__

class GmRegSoftDialog : public wxDialog
{
public:
	GmRegSoftDialog (wxWindow * parent, const wxString & title);
	~GmRegSoftDialog () {}
	wxString GetSerialNo () const;
private:
	wxTextCtrl *			m_szSerialNo;
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmRegSoftDialog)
};

#endif //__REGSOFT_H__