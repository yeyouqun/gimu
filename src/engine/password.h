//
// ÊµÏÖÈí¼þµÇÂ¼ÃÜÂë¡£
// Author:yeyouqun@163.com
// 2010-7-3
//

#ifndef __GM_PASSWORD_H__
#define __GM_PASSWORD_H__
bool ENGINE_EXPORT HasPassword (const wxString & ps1, const wxString & ps2);
bool ENGINE_EXPORT CheckPassword (const wxString & password, const wxString & ps1, const wxString & ps2);
void ENGINE_EXPORT WritePassword (const wxString & password, const wxString & ps1, const wxString & ps2);
wxString ENGINE_EXPORT GetPassword (const wxString & ps1, const wxString & ps2);
#endif //__GM_PASSWORD_H__