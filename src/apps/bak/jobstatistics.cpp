//
// job statistics's implementation.
// Author:yeyouqun@163.com
// 2010-2-8
//
#include <gmpre.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/image.h>
#include "gimuframe.h"
#include "jobstatistics.h"
#include "res/report.xpm"
#include "res/log.xpm"

BEGIN_EVENT_TABLE(GmStatisticsWindow, wxPanel)
END_EVENT_TABLE()

GmStatisticsWindow::GmStatisticsWindow (GmAppFrame * m_pMainFrame, wxSplitterWindow * pParent)
							: wxPanel (pParent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
							, m_pMainFrame (m_pMainFrame)
{
	wxBoxSizer * pBoxSizer = new wxBoxSizer (wxVERTICAL);
	m_pBook = new wxBookCtrl (this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_BOTTOM | wxNB_FIXEDWIDTH | wxNB_FLAT);
	long flags = wxTE_NOHIDESEL | wxBORDER_THEME | wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL;
	m_pLog = new wxTextCtrl (m_pBook, wxID_ANY, wxT (""), wxDefaultPosition, wxDefaultSize, flags);
	m_pReport = new wxTextCtrl (m_pBook, wxID_ANY, wxT (""), wxDefaultPosition, wxDefaultSize, flags);
	
	wxImageList * pimglst = new wxImageList (32, 32);
	pimglst->Add (wxBitmap (report_xpm), wxColor (0, 0, 0));
	pimglst->Add (wxBitmap (log_xpm), wxColor (0, 0, 0));

	m_pBook->AssignImageList (pimglst);
	m_pBook->AddPage (m_pLog, _("IDS_LOG_DATA"), false, 1);
	m_pBook->AddPage (m_pReport, _("IDS_REPORT_DATA"), false, 0);

	pBoxSizer->Add (m_pBook, 1, wxEXPAND);
	SetSizer (pBoxSizer);
	pBoxSizer->SetSizeHints (this);
	SetAutoLayout (true);
}


void GmStatisticsWindow::ChangeToPage (size_t page)
{
	if (page != LOG_PAGE && page != REPORT_PAGE) return;
	m_pBook->ChangeSelection (page);
}

wxTextCtrl * GmStatisticsWindow::GetCurrentPage () const
{
	int page = m_pBook->GetSelection ();
	if (page == LOG_PAGE) return m_pLog;
	else if (page == REPORT_PAGE) return m_pReport;
	return 0;
}