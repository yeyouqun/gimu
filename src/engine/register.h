//
// register implementation.
// Author:yeyouqun@163.com
// 2010-5-2
//
#include <engine/classbase.h>

#ifndef __GM_REGISTER_H__
#define __GM_REGISTER_H__

#define MAX_MACH_EIGEN_CODE 48
#define RAND_REG_EIGEN_CODE_LEN 10

#define MODULO_N1 wxT ("404BCCAF515735775161B95150C8667A99BD128E773F031A845ABA8B67F02B544350B9EB29F0B835F9B279DD29A82475B006E9C0D8C04A15h")
#define MODULO_N2 wxT ("4D4C732B16572EA35701D7F11E59D8EB29F478F83DE9BF8516558BA0E31FBA1A45B9FC4C9B3852C99CF5512650E107275BE637317727C67Dh")


class ENGINE_EXPORT GmRegister
{
	GmRegister ();
	~GmRegister ();
public:
	//
	// 这里的用户名可能是一个硬件唯一标识，如硬盘序列号，CPU序列号，或者网卡地址。
	//
	bool CheckMachEigenCodeSN (const wxString & szUser, const wxString & szSerial);
	bool CheckRandomSN (const wxString & szSerial);
	wxString GetMachEigenCode (const wxString & szUser);
	bool GenerateRSAKeyPairModuleValue (wxString & DKey, wxString & EKey, wxString & NValue);
	SINGLETON_DECL (GmRegister);
};

ENGINE_EXPORT wxString EncodeByRSA (const wxString & szCode // sha1 and base64 encoded.
											  , const wxString & ds
											  , const wxString & ns);
ENGINE_EXPORT wxString DecodeByRSA (const wxString & szCode
										   , const wxString & es
										   , const wxString & ns);
#endif //__GM_REGISTER_H__