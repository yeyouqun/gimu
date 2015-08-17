//
// ÊµÏÖÈí¼þµÇÂ¼ÃÜÂë¡£
// Author:yeyouqun@163.com
// 2010-7-3
//

#ifndef __GIMU_PASSWORD_H__
#define __GIMU_PASSWORD_H__
#include <engine/password.h>

class GmPassword
{
	GmPassword () {}
	~GmPassword () {}
public:
	bool HasPassword ();
	bool CheckPassword (const wxString & password);
	void WritePassword (const wxString & password);
	wxString GetPassword ();
private:
	SINGLETON_DECL(GmPassword);
};

#endif //__GIMU_PASSWORD_H__