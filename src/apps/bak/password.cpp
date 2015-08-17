//
// ÊµÏÖÈí¼þµÇÂ¼ÃÜÂë¡£
// Author:yeyouqun@163.com
// 2010-7-3
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/password.h>
#include "password.h"

#define PASSWORD_FILE wxT ("gimu.ace")
#define PASSWORD_BAK_FILE wxT ("unipro.pri")

bool GmPassword::HasPassword ()
{
	const wxString path = GmAppInfo::GetAppPath ();
	const wxString ps1 = AssemblePath (path, PASSWORD_FILE);
	const wxString ps2 = AssemblePath (path, PASSWORD_BAK_FILE);
	return ::HasPassword (ps1, ps2);
}

bool GmPassword::CheckPassword (const wxString & password)
{
	const wxString path = GmAppInfo::GetAppPath ();
	const wxString ps1 = AssemblePath (path, PASSWORD_FILE);
	const wxString ps2 = AssemblePath (path, PASSWORD_BAK_FILE);
	return ::CheckPassword (password, ps1, ps2);
}

void GmPassword::WritePassword (const wxString & password)
{
	const wxString path = GmAppInfo::GetAppPath ();
	const wxString ps1 = AssemblePath (path, PASSWORD_FILE);
	const wxString ps2 = AssemblePath (path, PASSWORD_BAK_FILE);
	return ::WritePassword (password, ps1, ps2);
}

wxString GmPassword::GetPassword ()
{
	const wxString path = GmAppInfo::GetAppPath ();
	const wxString ps1 = AssemblePath (path, PASSWORD_FILE);
	const wxString ps2 = AssemblePath (path, PASSWORD_BAK_FILE);
	return ::GetPassword (ps1, ps2);
}

SIGNLETON_IMPL(GmPassword);
