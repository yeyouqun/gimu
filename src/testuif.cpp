
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/uifalgo.h>
#include <engine/uiffile.h>
#include <engine/util.h>
#include <engine/analysis.h>
#include <engine/zipfile.h>
#include <engine/writer.h>
#include <engine/zipporcollector.h>
#include <crypto++/include/integer.h>
#include <crypto++/include/osrng.h>
#include <crypto++/include/nbtheory.h>
#include <crypto++/include/base64.h>
#include <iostream>
#include <iomanip>

void test_uif (int argn, char ** argc)
{
	if (argn < 2) {
		return;
	}

	GmUifSourceVectorT vps;
	for (int i = 1; i < argn; ++i) {
		wxString dir = ToWxString (argc[i]);
		GmUifSourcePairT * pTree = new GmUifSourcePairT;
		vps.push_back (pTree);
		TraverseTheDirForAnylasis (dir, pTree, 0, 0);
	}

	GmUnitedIndexFile uif (wxT ("e:\\test.uif"), 0);
	for (int index = 0; index < 5; ++index) {
		AddTheseTreeToUifFile (vps, uif, GRET_TYPE_FULL, ST_TYPE_NONE, TM_DEPTH_FIRST, wxDateTime::GetTimeNow ());
		AddTheseTreeToUifFile (vps, uif, GRET_TYPE_FULL, ST_TYPE_NONE, TM_WIDTH_FIRST, wxDateTime::GetTimeNow ());
		AddTheseTreeToUifFile (vps, uif, GRET_TYPE_FULL, ST_TYPE_NONE, TM_WIDTH_FIRST, wxDateTime::GetTimeNow ());
	}

	ClearUifTree (&vps);
	vector<GmUifRootEntry*> vpRoots = uif.GetAllRootEntries ();

	vector<GmUifSourceEntry*> vpSources;
	GmAutoClearVector<GmUifSourceEntry> acvs (vpSources);

	vector<GmDataSetEntry*> vpDataSet;
	GmAutoClearVector<GmDataSetEntry> acvd (vpDataSet);

	vector<char> vpExtendData;
	uif.GetAllUifSource (*vpRoots[2], vpSources);
	uif.GetAllUifDataSet (*vpRoots[8], vpDataSet);
	uif.GetAllUifExtendData (*vpRoots[14], vpExtendData);

	vector<GmDataSetEntry*> vpDataSet2;
	GmAutoClearVector<GmDataSetEntry> acvd2 (vpDataSet2);

	GmUifRootPairT vpst1;
	uif.GetUifRootTree (*vpRoots[0], vpst1);

	GmUifRootPairT vpst2;
	uif.GetUifRootTree (*vpRoots[14], vpst2);

	GmUifRootPairT vpst3;
	uif.GetUifRootTree (*vpRoots[3], vpst3);

	ClearRootTree (&vpst1);
	ClearRootTree (&vpst2);
	ClearRootTree (&vpst3);

	uif.RemoveUifRootEntry (2, vpDataSet2);
	uif.RemoveUifRootEntry (2, vpDataSet2);
	uif.RemoveUifRootEntry (4, vpDataSet2);
	uif.RemoveUifRootEntry (6, vpDataSet2);
	uif.RemoveUifRootEntry (6, vpDataSet2);
	uif.RemoveUifRootEntry (10, vpDataSet2);
	uif.RemoveUifRootEntry (10, vpDataSet2);
	uif.Tidy ();

	vpRoots = uif.GetAllRootEntries ();

	uif.GetUifRootTree (*vpRoots[2], vpst1);
	uif.GetUifRootTree (*vpRoots[10], vpst2);
	uif.GetUifRootTree (*vpRoots[4], vpst3);


	ClearRootTree (&vpst1);
	ClearRootTree (&vpst2);
	ClearRootTree (&vpst3);
}

struct GmTestMonitor : public GmMonitor
{
	vector<wxString> zips;
	virtual void HandleFile (const wxString &) {}
	virtual void HandleDir (const wxString &) {}
	virtual wxString HandlePassword () { return wxString (wxT ("xxxx")); }
	virtual bool OverwriteFile (const wxString &) { return false;}
	virtual void HandleSize (ubyte4) {}
	virtual void HandleMessage (const wxString &) {}
	virtual wxString SelectFile (const wxString &) { return wxEmptyString; }
	virtual void HandleNewFile (const wxString & ZipFile)
	{
		zips.push_back (ZipFile);
	}
	virtual wxString HandleSelectDir (const wxString &) { return wxEmptyString; }
	virtual void BeginAnalysis (const wxString & = wxString ()) { return; }
	virtual void EndAnalysis (const wxString & = wxString ()) { return; }
	virtual void HandleError (const wxString &) {}
	virtual void HandleWarn (const wxString &) {}
	virtual void Begin (const wxString & = wxString ()) {}
	virtual void End (const wxString & = wxString ()){}
	virtual void HandleDirectoryInfo (const GmDirectoryInfo &) {}
	virtual bool OverwriteFile (const wxString &, ubyte8 ModifyTime, ubyte8 Size) { return false; }
	void HandleManualStop(const wxString &) {}
	bool OverwriteFile(const wxString &,const GmLeafInfo &,const GmLeafInfo &) { return false; }
	wxString GetReportFile(void) const { return wxEmptyString; }
	wxString GetLogFile(void) const { return wxEmptyString; }
	int GetLogReportLimit(void) const { return 0; }
	void DoHandleDirectoryInfo(const GmDirectoryInfo &) { return; }
};

void test_zip_with_params (const GmZipParams & params
						   , const wxString & ZipFile
						   , int argn
						   , char ** argc)
{
	GmTestMonitor monitor;

	GmZipCollector collector (ZipFile, params, &monitor);
	GmZipCreator & ac = collector.GetZippor ();
	ac.BeginCreate ();
	for (int i = 1; i < argn; ++i) {
		wxString dir = ToWxString (argc[i]);
		TraverseTheDirForAnylasis (dir, 0, 0);
	}

	ac.EndCreate (wxString ());
	ZipTreeT ziptrees;

	{
		GmZipDistiller distiller (ZipFile, &monitor);
		distiller.GetFileTree (ziptrees);
		GmLocalWriter writer;
		distiller.RestoreDirectory (&ziptrees, writer);
	}

	for (size_t index = 0; index < monitor.zips.size (); ++index)
		wxRemoveFile (monitor.zips[index]);
}

void test_zip (int argn, char ** argc)
{
	if (argn < 2) {
		return;
	}

	GmZipParams params;
	params.bCompress = true;
	params.KeySize = ZKS128;
	params.SplitSize = -1;
	params.szPassword = "xxxx";
	params.ZipLevel = -1;

	// compress but not encode.
	params.bEncode = false;
	test_zip_with_params (params, wxT ("e:\\test.zip"), argn, argc);

	// no compress but encode.
	params.bCompress = false;
	test_zip_with_params (params, wxT ("e:\\test.zip"), argn, argc);

	// split
	params.SplitSize = (1 * 1024 * 1024);
	test_zip_with_params (params, wxT ("e:\\test.zip"), argn, argc);

	// no compress, encode, split
	params.bEncode = true;
	test_zip_with_params (params, wxT ("e:\\test.zip"), argn, argc);

	// compress, encode, split
	params.bCompress = true;
	test_zip_with_params (params, wxT ("e:\\test.zip"), argn, argc);

	// no compress, no encode, split
	params.bCompress = false;
	params.bEncode = false;
	test_zip_with_params (params, wxT ("e:\\test.zip"), argn, argc);
}

using CryptoPP::Integer;

void print_integer (const Integer & value)
{
	vector<unsigned char> v (value.ByteCount ());
	value.Encode (&v[0], v.size ());

	for (size_t i = 0; i < v.size (); ++i) {
		char buffer[5];
		sprintf (buffer, "%.2X", v[i] & 0xff);
		std::cout << buffer;
		if ((i + 1) % 48 == 0) std::cout << std::endl;
	}

	std::cout << std::endl;
	std::cout << "---------------------------------------------------" << std::endl;
}

void test_rsa_key_pair (const Integer & e, const Integer &d, const Integer & n)
{
}

void get_rsa_key_pair (const wchar_t * p, const wchar_t * q)
{
	//
	// ¼ÆËã¹«Ô¿
	//
	CryptoPP::AutoSeededRandomPool pool (false, 128);
	Integer P;
	P.Randomize (pool, Integer::Power2 (192), Integer::Power2 (224), CryptoPP::Integer::PRIME);
	std::cout << "P:\n";
	print_integer (P);

	Integer Q;
	Q.Randomize (pool, Integer::Power2 (192), Integer::Power2 (224), CryptoPP::Integer::PRIME);
	std::cout << "Q:\n";
	print_integer (Q);

	assert (CryptoPP::IsPrime (P));
	assert (CryptoPP::IsPrime (Q));

	Integer N = P * Q;
	std::cout << "P * Q:\n";
	print_integer (N);

	Integer Fn = (P -1) * (Q - 1);
	std::cout << "(P -1) * (Q - 1):\n";
	print_integer (Fn);

	Integer E;
	for (;;) {
		E.Randomize (pool, (Fn - 1)/2, (Fn - 1));
		if (!E.GetBit (0)) continue;
		if (CryptoPP::GCD (Fn, E) == Integer::One ())
			break;
	}

	std::cout << "E:\n";
	print_integer (E);

	Integer D = CryptoPP::EuclideanMultiplicativeInverse (E, Fn);
	std::cout << "D:\n";
	print_integer (D);

	///
	// test.
	//
	Integer test (pool, 255);
	print_integer (test);
	Integer encode_message = CryptoPP::a_exp_b_mod_c (test, E, N);
	print_integer (encode_message);
	vector<unsigned char> em (encode_message.ByteCount ());
	encode_message.Encode (&em[0], em.size ());
	CryptoPP::Base64Encoder encoder (0, false);
	encoder.PutMessageEnd (&em[0], em.size ());
	unsigned char buffer[1024];
	//encoder.Flush (true);
	size_t length = encoder.Get (buffer, 1024);
	buffer[length] = 0;
	std::cout << "BASE64:\n" << buffer << std::endl;

	assert (!encoder.AnyRetrievable ());
	Integer decode_message = CryptoPP::a_exp_b_mod_c (encode_message, D, N);
	print_integer (decode_message);
	assert ((decode_message == test));
}

#define PRIME_P1 wxT ("95F40104F2B21A42340919684443F305F051654429D407D50DA41F5467B6B02EA9050B97C52A75003756474E65B01F52B2E193A6D1F6DF9D69E12A9A29CA396B6745B6BA3C1502AB34F062BDA631A5AD1A836A59D3F52C0658D64642C6290674E0AAFA41DEC40B3F2ACC1EDC2AD2CB5B5B9FA8DF1DC04563E5C1F20F6F434123h")
#define PRIME_Q1 wxT ("CDA312CC01B4A71D7180655B2C17F7A2C37ADCBCC819F87C5E6B520A7D1A5DC06F075E889E4C75CE3875B82D36D9D05D942BC85EC978561E45EE195C4BDAFA7B8385DFDAADDE2D352A5B8475387FC3ACCA1AAD5F5BBCE588C2B8C53C2A5133B375074F8221398FCED73D73E26A57EAA336E3CA7CC3F93ADC36910CEC8D2F4BA7h")
int main (int argn, char ** argc)
{
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//test_uif (argn, argc);
	//test_zip (argn, argc);
	get_rsa_key_pair (PRIME_P1, PRIME_Q1);
	return 0;
}