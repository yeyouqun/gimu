#include <gmpre.h>
#include <engine/util.h>
#include "gmutil.h"
#include <engine/encdeflate.h>
#include <engine/register.h>
#include <crypto++/include/blowfish.h>
#ifdef _WIN32
#include <wx/msw/registry.h>
#else
#endif //

bool IsValidPath (const wxString & path)
{
	if (path.IsEmpty ()) {
		wxMessageBox (_("IDS_PATH_IS_EMPTY"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	wxFileName dirname (path, wxEmptyString);
	if (!dirname.IsAbsolute ()) {
		wxMessageBox (_("IDS_NOT_ABSOLUPATH"), _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	wxArrayString dirs = dirname.GetDirs ();
	if (dirs.IsEmpty ())
		return true;

	for (size_t index = 0; index < dirs.GetCount (); ++index) {
		if (!IsValidName (dirs.Item (index))) {
			wxString message = wxString::Format (_("IDS_INVALID_PATH"), wxFileName::GetForbiddenChars ().c_str ());
			wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
			return false;
		}
	}

	return true;
}

bool IsThisTypeSupported (const wxString & type)
{
#ifdef ENTERPRISE_EDITION
	//
	// 支持所有的类型。
	//
#elif PRO_EDITION
	//
	// 专业版本不支持 7Z 文件格式。
	//
	if (type == Get7zTypeStr ()) {
		wxString message = wxString::Format (_("IDS_NO_7Z_SUPPORTED"), type.c_str (), type.c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}
#else
	//
	// 免费版本不支持7Z，并且不支持同步操作。
	//
	if (type == Get7zTypeStr () || type == GetSyncTypeStr ()) {
		wxString message = wxString::Format (_("IDS_NO_7Z_SYNC_SUPPORTED")
											, Get7zTypeStr ().c_str ()
											, GetSyncTypeStr ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}
#endif//

	return true;
}
#define GIMU_STARUP wxT("gimu")
#define GIMU_RUN_REG_KEY wxT ("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
bool AddSystemStartItem (const wxString & app)
{
#ifdef _WIN32
	wxRegKey RegKey (GIMU_RUN_REG_KEY);
	if (!RegKey.Exists ())
		RegKey.Create ();
	if (!RegKey.Open (wxRegKey::Write)) return false;
	bool bResult = RegKey.SetValue (GIMU_STARUP, app);
	RegKey.Close ();
	return bResult;
#else
#endif //
}

bool RemoveSystemStartItem (const wxString & app)
{
#ifdef _WIN32
	wxRegKey RegKey (GIMU_RUN_REG_KEY);
	if (!RegKey.Exists ()) return false;

	if (!RegKey.Open (wxRegKey::Read)) return false;

	wxString Value;
	long index;
	bool bResult = RegKey.GetFirstValue (Value, index);
	while (bResult) {
		wxString data;
		if (RegKey.QueryValue (Value, data) && data.CmpNoCase (app) == 0) {
			if (!RegKey.DeleteValue (Value))
				return false;
		}

		bResult = RegKey.GetNextValue(Value, index);
	}
#else
#endif //
	return true;
}

void UninstallDaemon (const wxString & daemon)
{
	SC_HANDLE scm = OpenSCManager (0, 0, GENERIC_ALL);
	if(!scm) {
		wxString message = wxString::Format (_("IDS_CANT_OPEN_SCM"), GetSystemError ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	SC_HANDLE dh = OpenService (scm
								, GimuDaemonService
								, GENERIC_EXECUTE | GENERIC_READ | DELETE);
	if(!dh) {
		wxString message = wxString::Format (_("IDS_CANT_OPEN_SERVICE")
											, GimuDaemonService
											, GetSystemError ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		CloseServiceHandle(scm);
		return;
	}

	SERVICE_STATUS status;
	ControlService (dh, SERVICE_CONTROL_STOP, &status);
	if (!DeleteService (dh)) {
		wxString message = wxString::Format (_("IDS_CANT_DELETE_SERVICE")
											, GimuDaemonService
											, GetSystemError ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
	}

	CloseServiceHandle(dh);
	CloseServiceHandle(scm);
	return;
}

void InstallDaemon (const wxString & daemon)
{
	SC_HANDLE scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if(!scm) {
		wxString message = wxString::Format (_("IDS_CANT_OPEN_SCM"), GetSystemError ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return;
	}

	wxString desc (_("IDS_DAEMON_SERVICE_DESC"));
	SC_HANDLE dh = CreateService (scm
								,  GimuDaemonService
								, GimuDaemonService
								, SERVICE_ALL_ACCESS
								, SERVICE_WIN32_OWN_PROCESS
								, SERVICE_AUTO_START
								, SERVICE_ERROR_NORMAL
								, daemon.c_str (), 0, 0, 0, 0, 0);
	if(!dh) {
		wxString message = wxString::Format (_("IDS_CANT_CREATE_SERVICE")
											, GimuDaemonService
											, GetSystemError ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		CloseServiceHandle(scm);
		return;
	}

	CloseServiceHandle(dh);
	CloseServiceHandle(scm);
	return;
}

bool IsDaemonInstalled (const wxString & daemon)
{
	SC_HANDLE scm = OpenSCManager(0, 0, GENERIC_ALL);
	if(!scm) {
		wxString message = wxString::Format (_("IDS_CANT_OPEN_SCM"), GetSystemError ().c_str ());
		wxMessageBox (message, _("IDS_ERROR_INFO"), wxOK | wxICON_ERROR);
		return false;
	}

	SC_HANDLE dh = OpenService (scm
								, GimuDaemonService
								, GENERIC_EXECUTE | GENERIC_READ);
	if(!dh) {
		bool exists = (GetLastError () == ERROR_SERVICE_DOES_NOT_EXIST) ? false : true;
		CloseServiceHandle(scm);
		return exists;
	}

	CloseServiceHandle(dh);
	CloseServiceHandle(scm);
	return true;
}
