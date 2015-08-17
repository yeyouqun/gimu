//
// job statistics's implementation.
// Author:yeyouqun@163.com
// 2010-2-8
//

#ifndef __GM_JOB_STATISTICS_H__
#define __GM_JOB_STATISTICS_H__

class GmStatisticsWindow : public wxPanel
{
public:
	GmStatisticsWindow (GmAppFrame * m_pMainFrame, wxSplitterWindow * pParent);
	wxTextCtrl * GetLogPage () const { return m_pLog; }
	wxTextCtrl * GetReportPage () const { return m_pReport; }
	wxTextCtrl * GetCurrentPage () const;
	void ChangeToPage (size_t page);
protected:
	GmAppFrame * m_pMainFrame;
	wxBookCtrl * m_pBook;
	wxTextCtrl * m_pLog;
	wxTextCtrl * m_pReport;
	DECLARE_EVENT_TABLE ()
};

#define LOG_PAGE		0
#define REPORT_PAGE		1

#endif //__GM_JOB_STATISTICS_H__