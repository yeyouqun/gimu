//
// util for apps definition.
// Author:yeyouqun@163.com
// 2010-3-8
//

#ifndef __GM_APP_UTIL_H__
#define __GM_APP_UTIL_H__

bool IsValidPath (const wxString & path);
bool IsThisTypeSupported (const wxString &);
bool IsThisTypeSuppportedNow (const wxString &);
bool AddSystemStartItem (const wxString & app);
bool RemoveSystemStartItem (const wxString & app);
void InstallDaemon (const wxString & daemon);
void UninstallDaemon (const wxString & daemon);
bool IsDaemonInstalled (const wxString & daemon);
#endif //__GM_APP_UTIL_H__