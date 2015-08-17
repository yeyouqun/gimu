//
// hardware information getter implementation.
// Author:yeyouqun@163.com
// 2010-5-2
//
#include <engine/classbase.h>

#ifndef __GM_HARDWARE_H__
#define __GM_HARDWARE_H__

class ENGINE_EXPORT GmHardwareInfo
{
	GmHardwareInfo ();
	~GmHardwareInfo ();
public:
	wxString GetHardDiskSerial (const wxString & disk);
	wxString GetCPUSerial ();
	int GetTotalMemory ();
	int GetFreeMemory ();
	int GetCPUNumber ();
	//wxString GetBiosSerial ();
	void GetAdaptersAddresses (vector<wxString> & address);
	SINGLETON_DECL (GmHardwareInfo);
};

#endif //__GM_HARDWARE_H__