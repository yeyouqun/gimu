//
// 实现软件登录密码。
// Author:yeyouqun@163.com
// 2010-7-3
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/encdeflate.h>
#include <engine/register.h>
#include <crypto++/include/blowfish.h>
#include <engine/util.h>
#include <engine/password.h>

#define DKEY wxT ("085A94578FB4A204244AC938AFA35FAFE50B433CFC30462AF558FAF284B305249E1F69B09DA80BB78C1703DC1FBCCA76129A6021D5BE226Dh")
#define EKEY wxT ("0B80553DD4A1B98825263358BCF24F39537D4A14A906084BF3861BDEEB4F14B6BEC1403DA7FD6125BE79EB3E387C6281C3CE816C68ADEE65h")
#define NMODULE wxT ("15EEF5C9F17A5AA161EB1A4EA06939DD1580A05D75D90CC79829517F5F44322BEC03A8B533152DC14A73E7066C31BFEF28151D24C2EE0FDDh")

static unsigned char blowfish_key [] = {0x0E, 0x09, 0x00, 0xE9, 0xA8, 0xE8, 0x10, 0x00
							 , 0xE9, 0xA3, 0x1F, 0x0A, 0x00, 0xE9, 0x8E, 0x7B};
static unsigned char blowfish_iv [] = {0x0E, 0x00, 0xE9, 0xD9, 0xA3, 0x0C, 0x00, 0xE9};

static inline void SwapBytes (char * buffer, size_t length)
{
	for (size_t i = 0, j = length - 1; i < j; ++i, --j) {
		char tmp = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = tmp;
	}
}

void ClearPassword (const wxString & ps1, const wxString & ps2)
{
	wxFile fps1 (ps1, wxFile::write);
	wxFile fps2 (ps2, wxFile::write);
	string buffer (1024, (char)0);
	GmWinzipRand::GetInstance ()->GenerateRand ((char*)buffer.c_str (), 1024);
	GmWinzipRand::DeleteInstance ();

	buffer.insert (1, 1, (char)0);
	buffer.insert (9, 1, (char)0);
	buffer.insert (17, 1, (char)0);
	buffer.insert (23, 1, (char)0);
	buffer.insert (52, 1, (char)0);

	fps1.Write (buffer.c_str (), buffer.size ());
	//
	// 将缓冲交换
	//
	SwapBytes ((char*)buffer.c_str (),  buffer.size ());
	fps2.Write (buffer.c_str (), buffer.size ());
	return;
}

static bool ReadAndWrite (const wxString & from, const wxString & to, vector<char> & bytes)
{
	bytes.clear ();
	wxFile fps1 (from, wxFile::read);
	if (!fps1.IsOpened ()) return false;
	bytes.insert (bytes.begin (), fps1.Length (), (char)0);
	if (bytes.empty ()) return false;
	size_t res = fps1.Read ((void*)&bytes[0], fps1.Length ());
	if (res == 0 || res == -1) return false;

	vector<char> swpstr = bytes;
	SwapBytes ((char*)&swpstr[0], swpstr.size ());
	wxFile fps2 (to, wxFile::write);
	fps2.Write ((void*)&swpstr[0], swpstr.size ());

	return true;
}

static string GetPasswordChars (const wxString & ps1, const wxString & ps2)
{
	vector<char> bytes;
	string pass;
	bool ok = false;
	if (wxFile::Exists (ps1)) {
		ok = ReadAndWrite (ps1, ps2, bytes);
	}
	else if (wxFile::Exists (ps2)) {
		ok = ReadAndWrite (ps2, ps1, bytes);
		if (ok) SwapBytes ((char*)&bytes[0], bytes.size ());
	}

	if (!ok || bytes.size () < 52) {
		ClearPassword (ps1, ps2);
		return pass;
	}

	pass.assign (&bytes[0], bytes.size ());
	return pass;
}

static string GetPassword2 (const wxString & ps1, const wxString & ps2)
{
	string pass = GetPasswordChars (ps1, ps2);
	if (pass.empty ()) return string ();

	pass.erase (52, 1);
	pass.erase (23, 1);
	pass.erase (17, 1);
	pass.erase (9, 1);
	pass.erase (1, 1);

	CryptoPP::BlowfishDecryption blowfish (blowfish_key);
	size_t blksize = blowfish.BlockSize ();

	ubyte4 passlen = 0;
	CharsToUByte4 (pass.c_str (), &passlen);
	pass.erase (0, 4);
	if (passlen > pass.length ()) return string ();
	if (pass.length () % blksize != 0) return string ();

	vector<unsigned char> buffer (pass.length ());
	//blowfish.SetKeyWithIV (blowfish_key, 16, blowfish_iv, 8);
	for (size_t pos = 0, outpos = 0; pos < pass.length (); pos += blksize, outpos += blksize) {
		blowfish.ProcessBlock ((unsigned char*)(pass.c_str () + pos), &buffer[outpos]);
	}

	pass.clear ();
	pass.assign ((char*)&buffer[0], passlen);

	return pass;
}

bool HasPassword (const wxString & ps1, const wxString & ps2)
{
	if (!wxFile::Exists (ps1) && !wxFile::Exists (ps2)) {
		//
		// 生成默认文件，说明如何加密解密。
		//
		ClearPassword (ps1, ps2);
		return false;
	}

	//
	// 只要有一个文件存在说明密码未清空。
	//
	string bytes = GetPasswordChars (ps1, ps2);
	if (bytes.empty ()) return false;
	if (bytes.at (52) == (char)0 && bytes.at (23) == (char)0 && 
		bytes.at (17) == (char)0 && bytes.at (9) == (char)0 && bytes.at (1) == (char)0)
		return false;

	return true;
}

bool CheckPassword (const wxString & password, const wxString & ps1, const wxString & ps2)
{
	assert (!(!wxFile::Exists (ps1) && !wxFile::Exists (ps2)));
	string bytes = GetPassword2 (ps1, ps2);
	if (bytes.empty ()) return true;
	wxString passwd = DecodeByRSA (ToWxString (bytes), EKEY, NMODULE);
	if (password == passwd) return true;
	return false;
}

wxString ENGINE_EXPORT GetPassword (const wxString & ps1, const wxString & ps2)
{
	if (!HasPassword (ps1, ps2)) wxEmptyString;
	string bytes = GetPassword2 (ps1, ps2);
	if (bytes.empty ()) return wxEmptyString;
	wxString passwd = DecodeByRSA (ToWxString (bytes), EKEY, NMODULE);
	return passwd;
}

void WritePassword (const wxString & password, const wxString & ps1, const wxString & ps2)
{
	if (password.IsEmpty ()) {
		ClearPassword (ps1, ps2);
		return;
	}

	try {
		string enpass = ToStlString (EncodeByRSA (password, DKEY, NMODULE));
		CryptoPP::BlowfishEncryption blowfish (blowfish_key);
		
		size_t blksize = blowfish.BlockSize ();
		size_t padsize = enpass.length () % blksize;
		size_t bufferlen = (padsize == 0 ? 0 : blksize - padsize);
		//
		// 填够blksize整数倍。
		//
		if (bufferlen != 0) enpass.insert (enpass.end (), bufferlen, (char)0);
		bufferlen = (enpass.length () + 4); //actual length of password.

		vector<unsigned char> buffer (bufferlen);
		//blowfish.SetKeyWithIV (blowfish_key, 16, blowfish_iv, 8);
		for (size_t pos = 0, outpos = 4; pos < enpass.length (); pos += blksize, outpos += blksize) {
			blowfish.ProcessBlock ((unsigned char*)enpass.c_str () + pos, &buffer[outpos]);
		}

		UByte4ToChars ((char*)&buffer[0], (ubyte4)enpass.length ());

		//
		// 第1,9,17,23,52个位置的字节全部为零，则表示无密码,当有一个不为零是表示有密码。
		//
		enpass.clear ();
		enpass.assign ((char*)&buffer[0], buffer.size ());
		enpass.insert (1, 1, (char)1);
		enpass.insert (9, 1, (char)1);
		enpass.insert (17, 1, (char)1);
		enpass.insert (23, 1, (char)1);
		enpass.insert (52, 1, (char)1);

		wxFile fps1 (ps1, wxFile::write);
		fps1.Write (enpass.c_str (), enpass.length ());

		wxFile fps2 (ps2, wxFile::write);
		SwapBytes ((char*)enpass.c_str (), enpass.length ());
		fps2.Write (enpass.c_str (), enpass.length ());
	}
	catch (CryptoPP::Exception &e) {
		;
	}
	return;
}
