
#include <gmpre.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include "regapp.h"
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/register.h>
#include <crypto++/include/integer.h>
#include <crypto++/include/osrng.h>
#include <crypto++/include/nbtheory.h>
#include <crypto++/include/zdeflate.h>
#include <crypto++/include/base64.h>
#include <crypto++/include/adler32.h>
#include <crypto++/include/sha.h>

#define PRVIATE_KEY_D1 wxT ("0911251A86F0AADE8DC2749B1C19C0C1D0C982CEF648F129003A73C8D7E92451EEA339735A88960855696D260B8BDF94F6001B93C5EC9EBDh")
#define PRVIATE_KEY_D2 wxT ("0D3AE98854B4FE238F26E481CE0DE618600D9CD132FD99D798EBD1786677A5FC9B5252F0EBB31FE1C57FDB0076F00CBC2D0E839F310F4FFBh")
#define GIMU_REG_LANG_MO _("regpo.mo")

IMPLEMENT_APP(GmRegisterApp)

GmRegisterApp::GmRegisterApp ()
{
}

bool GmRegisterApp::OnInit (void)
{
	if (!wxApp::OnInit())
        return false;
	
	m_pLocale = new wxLocale ();
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	m_pLocale->Init();
	wxString szExePath = GmAppInfo::GetAppPath ();
	m_pLocale->AddCatalogLookupPathPrefix (szExePath);
	m_pLocale->AddCatalog (GIMU_REG_LANG_MO);

	GmRegisterFrame * pFrame = new GmRegisterFrame (_("IDS_REGISTER_APP"));
	SetTopWindow (pFrame);
	pFrame->Show ();
	return true;
}

int GmRegisterApp::OnExit (void)
{
	return 0;
}

GmRegisterApp::~GmRegisterApp ()
{
	if (m_pLocale) delete m_pLocale;
}

BEGIN_EVENT_TABLE(GmRegisterFrame, wxFrame)
	EVT_PAINT  (GmRegisterFrame::OnPaint)
	EVT_BUTTON(IDC_CALC_SN, GmRegisterFrame::OnCalcSN)
	EVT_BUTTON(IDC_CALC_RAND_SN, GmRegisterFrame::OnCalcRandSN)
	EVT_BUTTON(wxID_CANCEL, GmRegisterFrame::OnCloseRegApp)
	EVT_BUTTON(IDC_TEST_SN, GmRegisterFrame::OnTestSN)
	EVT_BUTTON(IDC_TEST_RAND_SN, GmRegisterFrame::OnTestRandSN)
	EVT_BUTTON(IDC_CALC_RSA_KEYS, GmRegisterFrame::OnCalcRSA)
	EVT_SIZE(GmRegisterFrame::OnChangeSize)
END_EVENT_TABLE()

GmRegisterFrame::GmRegisterFrame (const wxString& title)
			: wxFrame ((wxFrame *) NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)
{
	SetIcon (wxICON (gimu));
	m_pPanel = new wxPanel (this, wxID_ANY);

	wxBoxSizer * pPanelSizer = new wxBoxSizer (wxVERTICAL);

	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, m_pPanel, _("IDS_REGISTER"));
	pPanelSizer->Add (pStaticSizer, 0, wxEXPAND | wxALL, 10);

	wxFlexGridSizer * pGridSizer = new wxFlexGridSizer (2, 2, 5, 5);
	pStaticSizer->Add (pGridSizer, 1, wxEXPAND);
	pGridSizer->AddGrowableCol (1);

	pGridSizer->Add (new wxStaticText (m_pPanel, wxID_ANY, _("IDS_MACH_FINGER_PRINT")), 0, wxALIGN_RIGHT);
	pGridSizer->Add (m_pFingerPrt = new wxTextCtrl (m_pPanel, IDC_MACH_FINGER_PRINT, _("")), 1, wxEXPAND);

	pGridSizer->Add (new wxStaticText (m_pPanel, wxID_ANY, _("IDS_SOFTWARE_SN")), 0, wxALIGN_RIGHT);
	pGridSizer->Add (m_SerialNo = new wxTextCtrl (m_pPanel, IDC_SOFTWARE_SN, _("")), 1, wxEXPAND);
	pGridSizer->Add (new wxStaticText (m_pPanel, wxID_ANY, _("IDS_SOFTWARE_RAND_SN")), 0, wxALIGN_RIGHT);
	pGridSizer->Add (m_RandSerialNo = new wxTextCtrl (m_pPanel, IDC_SOFTWARE_RAND_SN, _("")), 1, wxEXPAND);

	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
#ifdef _DEBUG
	pBoxSizer2->Add (new wxButton (m_pPanel, IDC_TEST_SN, _("IDS_TEST_SN")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (m_pPanel, IDC_TEST_RAND_SN, _("IDS_TEST_RAND_SN")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
#endif //
	pBoxSizer2->Add (new wxButton (m_pPanel, IDC_CALC_RAND_SN, _("IDS_CALC_RAND_SN")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (m_pPanel, IDC_CALC_SN, _("IDS_CALC_SN")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pPanelSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	pPanelSizer->Add (new wxStaticLine (m_pPanel
										, wxID_ANY
										, wxDefaultPosition
										, wxDefaultSize
										, wxLI_HORIZONTAL)
						, 0
						, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM
						, 10);
	//
	// Éú³É RSA key pair and n_value
	//

	wxStaticBoxSizer * pStaticSizer2 = new wxStaticBoxSizer (wxVERTICAL, m_pPanel, _("IDS_RSA_KEYS"));
	pPanelSizer->Add (pStaticSizer2, 0, wxEXPAND | wxALL, 10);

	wxFlexGridSizer * pGridSizer2 = new wxFlexGridSizer (2, 2, 5, 5);
	pStaticSizer2->Add (pGridSizer2, 1, wxEXPAND);
	pGridSizer2->AddGrowableCol (1);

	//
	// rsa keys.
	//
	long style = wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP;
	pGridSizer2->Add (new wxStaticText (m_pPanel, wxID_ANY, _("IDS_PRIVATE_DKEY")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	pGridSizer2->Add (m_pDKeys = new wxTextCtrl (m_pPanel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, style), 1, wxEXPAND);
	pGridSizer2->Add (new wxStaticText (m_pPanel, wxID_ANY, _("IDS_PUBLIC_EKEY")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	pGridSizer2->Add (m_pEKeys = new wxTextCtrl (m_pPanel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, style), 1, wxEXPAND);
	pGridSizer2->Add (new wxStaticText (m_pPanel, wxID_ANY, _("IDS_RSA_NVALUE")), 0, wxALIGN_RIGHT | wxALIGN_CENTER);
	pGridSizer2->Add (m_pNValue = new wxTextCtrl (m_pPanel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, style), 1, wxEXPAND);

	pPanelSizer->Add (new wxStaticLine (m_pPanel
										, wxID_ANY
										, wxDefaultPosition
										, wxDefaultSize
										, wxLI_HORIZONTAL)
						, 0
						, wxEXPAND | wxRIGHT | wxLEFT | wxBOTTOM
						, 10);

	pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND);
	pBoxSizer2->Add (new wxButton (m_pPanel, IDC_CALC_RSA_KEYS, _("IDS_CALC_RSA_KEYS")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer2->Add (new wxButton (m_pPanel, wxID_CANCEL, _("&Cancel")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 15);
	pPanelSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	m_pPanel->SetSizer (pPanelSizer);
	pPanelSizer->SetSizeHints (m_pPanel);
	m_pPanel->SetAutoLayout (true);
	wxSize size = GetBestSize ();
#ifdef _DEBUG
	size.x *= 1.8;
#else
	size.x *= 2.5;
#endif
	SetSize (size);
	
	CenterOnScreen ();
}

static wxString GenerateRandomSN ()
{
	using CryptoPP::Integer;
	CryptoPP::AutoSeededRandomPool pool (false, 128);
	Integer r;
	r.Randomize (pool, Integer::Power2 (192), Integer::Power2 (224), CryptoPP::Integer::PRIME);

	vector<unsigned char> vs (r.ByteCount ());
	r.Encode (&vs[0], vs.size ());

	CryptoPP::SHA1 sha1;
	sha1.Update (&vs[0], vs.size ());
	vector<unsigned char> va (sha1.DigestSize ()); //160bit=20Bytes
	sha1.Final (&va[0]);

	CryptoPP::Base64Encoder encoder (0, false);
	encoder.PutMessageEnd (&va[0], va.size ());
	unsigned char buffer[512];//no more than 512
	size_t length = encoder.Get (buffer, 512);
	buffer[length] = 0;

	string RandCode = reinterpret_cast<char*> (buffer); // must greater than 20bytes.
	RandCode = RandCode.substr (0, RAND_REG_EIGEN_CODE_LEN);
	wxString sn = EncodeByRSA (ToWxString (RandCode), PRVIATE_KEY_D2, MODULO_N2);
	RandCode += ToStlString (sn);

	return ToWxString (RandCode);
}

void GmRegisterFrame::OnCloseRegApp (wxCommandEvent &event)
{
	Destroy ();
}

void GmRegisterFrame::OnTestSN (wxCommandEvent &event)
{
	GmRegister::GetInstance ()->CheckMachEigenCodeSN (m_pFingerPrt->GetLabel (), m_SerialNo->GetLabel ());
}

void GmRegisterFrame::OnTestRandSN (wxCommandEvent &event)
{
	GmRegister::GetInstance ()->CheckRandomSN (m_RandSerialNo->GetLabel ());
}

void GmRegisterFrame::OnCalcRSA (wxCommandEvent &event)
{
	wxString D, E, N;
	while (!GmRegister::GetInstance ()->GenerateRSAKeyPairModuleValue (D, E, N));
	m_pDKeys->SetValue (D);
	m_pEKeys->SetValue (E);
	m_pNValue->SetValue (N);
}

void GmRegisterFrame::OnCalcSN (wxCommandEvent &event)
{
	wxString fp = m_pFingerPrt->GetLabel ();
	m_SerialNo->SetLabel (EncodeByRSA (fp, PRVIATE_KEY_D1, MODULO_N1));
}

void GmRegisterFrame::OnCalcRandSN (wxCommandEvent &event)
{
	wxString fp = m_pFingerPrt->GetLabel ();
	if (!fp.IsEmpty ()) {
		return;
	}

	m_RandSerialNo->SetLabel (GenerateRandomSN ());
}

GmRegisterFrame::~GmRegisterFrame (void)
{
	GmRegister::DeleteInstance ();
}

void GmRegisterFrame::OnPaint(wxPaintEvent & event)
{
	wxPaintDC dc(this);
	PrepareDC (dc);
	event.Skip ();
}

void GmRegisterFrame::OnChangeSize (wxSizeEvent &event)
{
	wxSizer * pSizer = m_pPanel->GetSizer ();
	if (pSizer) {
		//pSizer->SetMinSize (event.GetSize ());
		//pSizer->RecalcSizes ();
		//m_pPanel->Fit ();
		//pSizer->Layout ();
		////pSizer->Fit (m_pPanel);
	}

	event.Skip ();
}