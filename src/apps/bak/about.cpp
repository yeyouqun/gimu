//
// about dialog's definition.
// Author:yeyouqun@163.com
// 2010-5-25
//
#include <gmpre.h>
#include <wx/statline.h>
#include "about.h"
#include <engine/defs.h>
#include <engine/hardware.h>
#include "res/dlg_res/about.xpm"

IMPLEMENT_DYNAMIC_CLASS (GmAboutDialog, wxDialog)
BEGIN_EVENT_TABLE(GmAboutDialog, wxDialog)
END_EVENT_TABLE()

GmAboutDialog::GmAboutDialog (wxWindow * parent, const wxString & title)
				: wxDialog (parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	wxBitmap xpm (about_xpm);
	wxStaticBitmap * bp = new wxStaticBitmap (this, wxID_ANY, xpm);
	pBoxSizer->Add (bp);
	wxStaticBoxSizer * pStaticSizer = new wxStaticBoxSizer (wxVERTICAL, this);
	pBoxSizer->Add (pStaticSizer, 1, wxEXPAND | wxALL, 15);

	long AddFlags = wxEXPAND | wxALIGN_CENTER | wxBOTTOM;

#if defined(ENTERPRISE_EDITION)
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_ENTERPRISE_EDITION")), 0, wxEXPAND);
#elif defined(PRO_EDITION)
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_PRO_EDITION")), 0, wxEXPAND);
#else
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, _("IDS_FREE_EDITION")), 0, AddFlags, 5);
#endif
	wxDateTime now;
	now.SetToCurrent ();
	//
	// Copyright descrition.
	//
	wxString message = wxString::Format (_("IDS_COPYRIGHT_DESC"), now.GetYear ());
	wxStaticText * text = new wxStaticText (this, wxID_ANY, message);
	pStaticSizer->Add (text, 1, AddFlags);
	//
	// Build description.
	//
	message = wxString::Format (_("IDS_BUILD_DESC")
								, MAJOR_VERSION
								, MINOR_VERSION
								, REVISION_VERSION
								, wxT ( BUILDDATE ));
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, message), 0, AddFlags, 5);

	//
	// Operating system
	//
	wxPlatformInfo info;
	message = wxString::Format (_("IDS_OS_DESC")
								, info.GetOperatingSystemFamilyName ().c_str ()
								, info.GetArchName ().c_str ());
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, message), 0, AddFlags, 5);

	//
	// CPU
	//
	GmHardwareInfo * pHardware = GmHardwareInfo::GetInstance ();
	message = wxString::Format (_("IDS_CPU_DESC"), pHardware->GetCPUSerial ().c_str (), pHardware->GetCPUNumber ());
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, message), 1, AddFlags);

	//
	// Memory
	//
	message = wxString::Format (_("IDS_MEMORY_DESC"), pHardware->GetTotalMemory (), pHardware->GetFreeMemory ());
	pStaticSizer->Add (new wxStaticText (this, wxID_ANY, message), 0, AddFlags, 5);

	//
	// bottom buttons.
	//
	pBoxSizer->Add (new wxStaticLine (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL)
						, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 15);
	wxBoxSizer * pBoxSizer2 = new wxBoxSizer (wxHORIZONTAL);
	pBoxSizer2->Add (new wxBoxSizer (wxHORIZONTAL), 1, wxEXPAND | wxRIGHT);
	pBoxSizer2->Add (new wxButton (this, wxID_OK, _("IDS_OK")), 0, wxEXPAND | wxALIGN_RIGHT | wxRIGHT, 20);
	pBoxSizer->Add (pBoxSizer2, 0, wxEXPAND | wxBOTTOM, 10);

	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);

	//text->Wrap (pStaticSizer->GetSize ().GetWidth ());
	wxSize size = pBoxSizer->CalcMin ();
	size.x = xpm.GetWidth ();
	size.y = 350;
	SetSize (size);

	CentreOnScreen ();
}