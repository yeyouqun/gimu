//
// register implementation.
// Author:yeyouqun@163.com
// 2010-5-2
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/register.h>
#include <crypto++/include/integer.h>
#include <crypto++/include/osrng.h>
#include <crypto++/include/nbtheory.h>
#include <engine/util.h>
#include <crypto++/include/base64.h>
#include <crypto++/include/adler32.h>
#include <crypto++/include/sha.h>

#define PUBLIC_KEY_E1 wxT ("353BAE00758F042E88E5C6CB9EF102D578AEE0B6B780790DDC59DA7E0FB0E6278E18C45E7006EDA9D14F4486BA617B104EF05AF5F4148295h")
#define PUBLIC_KEY_E2 wxT ("3225C838AC241E6CCB2B011927FB462FFA981A3584B7FD64365983619C8ED08493A2BE667EA59732B0B78223BD3098DD79FCA2D884F89233h")

using CryptoPP::Integer;

GmRegister::GmRegister ()
{
}

GmRegister::~GmRegister ()
{
}

static wxString GetDecoderUser (const Integer & ui)
{
	string user;
	vector<unsigned char> vi (ui.ByteCount (), 0);
	ui.Encode (static_cast<unsigned char*> (&vi[0]), vi.size ());
	user.assign (reinterpret_cast<char*> (&vi[0]), vi.size ());
	return ToWxString (user);
}

Integer GetDecodeInteger (const wxString & szSerial
										   , const wxString & es
										   , const wxString & ns)
{
	//
	// 解码 Base64 编码。
	//
	CryptoPP::Base64Decoder decoder;
	string serial = ToStlString (szSerial);
	decoder.PutMessageEnd ((unsigned char*)serial.c_str (), serial.length ());
	unsigned char buffer[1024];
	size_t length = decoder.Get (buffer, 1024);
	buffer[length] = 0;
	
	//
	// 解密原来的 user 名。
	//
	Integer sn;
	sn.Decode (buffer, length);
	const Integer e (es.c_str ());
	const Integer n (ns.c_str ());

	return CryptoPP::a_exp_b_mod_c (sn, e, n);
}

bool GmRegister::CheckMachEigenCodeSN (const wxString & szUser, const wxString & szSerial)
{
	if (szSerial.IsEmpty () || szUser.IsEmpty ()) return false;
	wxString user = GetMachEigenCode (szUser);
	wxString DecoderUser = DecodeByRSA (szSerial, PUBLIC_KEY_E1, MODULO_N1);
	return user == DecoderUser;
}

ENGINE_EXPORT wxString DecodeByRSA (const wxString & szCode
										   , const wxString & es
										   , const wxString & ns)
{
	const Integer ui = GetDecodeInteger (szCode, es, ns);
	wxString DecoderUser = GetDecoderUser (ui);
	return DecoderUser;
}
//
// 从机器码中取得用户特征码。
//
wxString GmRegister::GetMachEigenCode (const wxString & szUser)
{
	if (szUser.IsEmpty ())
		return wxEmptyString;

	string user = ToStlString (szUser);
	//
	// 求取SHA1值
	//
	CryptoPP::SHA1 sha1;
	sha1.Update ((unsigned char*)user.c_str (), user.length ());
	vector<unsigned char> vs (sha1.DigestSize ());
	sha1.Final (&vs[0]);

	CryptoPP::Base64Encoder encoder (0, false);
	encoder.PutMessageEnd (&vs[0], vs.size ());
	unsigned char buffer[512];//no more than 512
	size_t length = encoder.Get (buffer, 512);
	buffer[length] = 0;

	user = reinterpret_cast<char*> (buffer);

	if (user.length () > MAX_MACH_EIGEN_CODE) {
		return ToWxString (user.substr (0, MAX_MACH_EIGEN_CODE));
	}

	return ToWxString (user);
}

bool GmRegister::CheckRandomSN (const wxString & szSerial)
{
	if (szSerial.IsEmpty ()) return false;

	string sn = ToStlString (szSerial);
	string rand = sn.substr (0, RAND_REG_EIGEN_CODE_LEN);
	sn = sn.substr (RAND_REG_EIGEN_CODE_LEN);

	const Integer ui = GetDecodeInteger (ToWxString (sn), PUBLIC_KEY_E2, MODULO_N2);
	wxString DecoderUser = GetDecoderUser (ui);

	return ToWxString (rand) == DecoderUser;
}

static wxString GetHexRepresentationOfInteger (const Integer & value)
{
	wxString HexStr;
	vector<unsigned char> v (value.ByteCount ());
	value.Encode (&v[0], v.size ());

	for (size_t i = 0; i < v.size (); ++i) {
		char_t buffer[5];
		wsprintf (buffer, wxT ("%.2X"), v[i] & 0xff);
		HexStr += buffer;
	}

	return HexStr;
}

bool GmRegister::GenerateRSAKeyPairModuleValue (wxString & DKey, wxString & EKey, wxString & NValue)
{
	//
	// 计算公钥
	//
	CryptoPP::AutoSeededRandomPool pool (false, 128);
	Integer P;
	P.Randomize (pool, Integer::Power2 (192), Integer::Power2 (224), Integer::PRIME);
	Integer Q;
	Q.Randomize (pool, Integer::Power2 (192), Integer::Power2 (224), Integer::PRIME);

	if (!CryptoPP::IsPrime (P) || !CryptoPP::IsPrime (Q)) return false;

	Integer N = P * Q;
	NValue = GetHexRepresentationOfInteger (N);

	Integer Fn = (P -1) * (Q - 1);
	Integer E;
	for (;;) {
		E.Randomize (pool, (Fn - 1)/2, (Fn - 1));
		if (!E.GetBit (0)) continue;
		if (CryptoPP::GCD (Fn, E) == Integer::One ())
			break;
	}

	EKey = GetHexRepresentationOfInteger (E);
	Integer D = CryptoPP::EuclideanMultiplicativeInverse (E, Fn);
	DKey = GetHexRepresentationOfInteger (D);
	return true;
}

wxString EncodeByRSA (const wxString & szCode // sha1 and base64 encoded.
									  , const wxString & ds
									  , const wxString & ns)
{
	//
	// 本软件采用192位的PQ，所以n为2^192*2^192 ＝ 2^384
	// 加密信息 m（二进制表示）时，首先把m分成等长数据块 m1 ,m2,..., mi ，块长s，
	// 其中 2^s <= n (2^384), s 尽可能的大(这里 S 应该小于 384 位，也就是小于 384/8 = 48 字节）。
	//
	string ec = ToStlString (szCode);
	if (ec.length () == 0) return wxEmptyString;

	if (ec.length () > MAX_MACH_EIGEN_CODE) {
		return wxEmptyString;
	}

	using CryptoPP::Integer;
	Integer input;
	//
	// 将字串当成是编码的整数值。
	//
	input.Decode (reinterpret_cast<const unsigned char*> (ec.c_str ()), ec.length ());
	Integer D (ds.c_str ());
	Integer N (ns.c_str ());

	//
	// 计算加密值。
	//
	Integer EncodeMessage = CryptoPP::a_exp_b_mod_c (input, D, N);
	vector<unsigned char> em (EncodeMessage.ByteCount ());
	EncodeMessage.Encode (&em[0], em.size ());

	//
	// 计算 Base64 编码。
	//
	CryptoPP::Base64Encoder encoder (0, false);
	encoder.PutMessageEnd (&em[0], em.size ());
	unsigned char buffer[1024];
	size_t length = encoder.Get (buffer, 1024);
	buffer[length] = 0;

	return ToWxString (string (reinterpret_cast<char*> (buffer)));
}


SIGNLETON_IMPL(GmRegister);