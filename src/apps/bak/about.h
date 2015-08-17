//
// about dialog's definition.
// Author:yeyouqun@163.com
// 2010-5-25
//

#ifndef __GM_ABOUT_H__
#define __GM_ABOUT_H__

class GmAboutDialog : public wxDialog
{
public:
	GmAboutDialog (wxWindow * parent, const wxString & title);
	GmAboutDialog () : wxDialog () {}

private:
	DECLARE_DYNAMIC_CLASS (GmAboutDialog)
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmAboutDialog)
};

#endif //