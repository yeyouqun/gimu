//
// register gimu dialog.
// Author:yeyouqun@163.com
// 2010-5-23
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/register.h>
#include <engine/hardware.h>
#include "regsoft.h"

BEGIN_EVENT_TABLE(GmRegSoftDialog, wxDialog)
END_EVENT_TABLE()

GmRegSoftDialog::GmRegSoftDialog (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title
						, wxDefaultPosition, wxDefaultSize, wxNO_3D | wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxStaticText * text = new wxStaticText (this, wxID_ANY, _("IDS_REGISTER_PROMPT"));

	pBoxSizer->Add (text, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 15);
	wxStaticBoxSizer * pStaticBoxSizer = new wxStaticBoxSizer (wxVERTICAL, this);
	pBoxSizer->Add (pStaticBoxSizer, 1, wxEXPAND | wxALL, 15);
#ifdef ENTERPRISE_EDITION
	pStaticBoxSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_EIGEN_CODE")), 0, wxALL, 10);
	wxString eigen = GmHardwareInfo::GetInstance ()->GetHardDiskSerial (wxEmptyString);
	eigen = GmRegister::GetInstance ()->GetMachEigenCode (eigen);
	wxTextCtrl * code = new wxTextCtrl (this, wxID_ANY, wxT (""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	code->SetValue (eigen);
	pStaticBoxSizer->Add (code, 1, wxEXPAND | wxBOTTOM, 10);
#endif //
	m_szSerialNo = new wxTextCtrl (this, wxID_ANY);
	pStaticBoxSizer->Add (m_szSerialNo, 1, wxEXPAND);
	pBoxSizer->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 15);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
	Centre ();
}

wxString GmRegSoftDialog::GetSerialNo () const
{
	return m_szSerialNo->GetValue ();
}