//
// monitor implemetation definition.
// Author:yeyouqun@163.com
// 2010-3-13
//
#include <gmpre.h>
#include <engine/monitor.h>
#include <wx/window.h>
#include <wx/gauge.h>

#ifndef __MONITOR_IMPL_H__
#define __MONITOR_IMPL_H__

DECLARE_GM_EVENT_TYPE(GmEventHandleFile)
DECLARE_GM_EVENT_TYPE(GmEventHandleDir)
DECLARE_GM_EVENT_TYPE(GmEventHandleSize)
DECLARE_GM_EVENT_TYPE(GmEventHandleMessage)
DECLARE_GM_EVENT_TYPE(GmEventHandleBegin)
DECLARE_GM_EVENT_TYPE(GmEventHandleEnd)
DECLARE_GM_EVENT_TYPE(GmEventHandleBeginAnalysis)
DECLARE_GM_EVENT_TYPE(GmEventHandleEndAnalysis)
DECLARE_GM_EVENT_TYPE(GmEventHandleError)
DECLARE_GM_EVENT_TYPE(GmEventHandleWarn)
DECLARE_GM_EVENT_TYPE(GmEventHandleOverwrite1)
DECLARE_GM_EVENT_TYPE(GmEventHandleOverwrite2)
DECLARE_GM_EVENT_TYPE(GmEventHandleManualStop)
DECLARE_GM_EVENT_TYPE(GmEventHandleDirectoryInfo)

class GmJobItem;
class GmRootCreator;

struct JobListItemData
{
	JobListItemData () : pGauge (0)
						, pJob (0)
						, pThread (0)
						, pMonitor (0)
						, pRootCreator (0)
						, bEndAnalysis (false) {}
	wxGauge *		pGauge;
	wxStaticText *	pPercent;
	GmJobItem *		pJob;
	wxThread *		pThread;
	GmMonitor *		pMonitor;
	GmRootCreator * pRootCreator;
	wxString		szDestPath;
	bool			bEndAnalysis;
};

struct GmJobEvent : public wxEvent
{
	GmJobEvent (wxEventType eventType) : wxEvent (wxID_ANY, eventType), m_EventData (0) {}
	GmJobEvent (const GmJobEvent & clone) : wxEvent (clone)
	{
		SetValue (clone.message, clone.m_Info, clone.m_InfoLast, clone.m_pParent, clone.m_pItemData);
		m_EventData = clone.m_EventData;
	}
	wxEvent * Clone () const { return new GmJobEvent (*this); }
	void SetValue (const wxString Message
				, const GmDirectoryInfo & Info
				, const GmDirectoryInfo & InfoLast
				, const wxWindow * pParent
				, const JobListItemData * pItemData);

	wxString message;
	GmDirectoryInfo m_Info;
	GmDirectoryInfo m_InfoLast;
	wxWindow * m_pParent;
	JobListItemData * m_pItemData;
	void *			m_EventData;
};

class wxGauge;

class GmJobMonitor : public GmMonitor
{
public:
	GmJobMonitor (wxWindow * parent, JobListItemData * pData);
private:
	void AddPendingEvent (wxEventType type, const wxString & Message);
	virtual ~GmJobMonitor ();
	virtual void HandleFile (const wxString &);
	virtual void HandleDir (const wxString &);
	virtual void HandleSize (ubyte4);
	virtual void HandleMessage (const wxString &);
	virtual void BeginAnalysis (const wxString &);
	virtual void EndAnalysis (const wxString &);
	virtual void HandleError (const wxString &);
	virtual void HandleWarn (const wxString &);
	virtual void HandleManualStop (const wxString &);
	virtual wxString HandleSelectDir (const wxString & path);

	//
	// no event created in these interface.
	//
	virtual void Begin (const wxString &);
	virtual void End (const wxString &);
	virtual wxString SelectFile (const wxString &);
	virtual wxString HandlePassword ();
	virtual bool OverwriteFile (const wxString &, ubyte8 ModifyTime, ubyte8 Size);
	virtual bool OverwriteFile (const wxString &, const GmLeafInfo & New, const GmLeafInfo & Old);
	//
	// Get report or log file name.
	//
	virtual wxString GetReportFile () const;
	virtual wxString GetLogFile () const;
	virtual int GetLogReportLimit () const;
	//
	// 其他的虚拟函数。
	//
	virtual void DoHandleDirectoryInfo (const GmDirectoryInfo &);
private:
	wxWindow *		m_pParent;
	JobListItemData * m_pItemData;
	bool			m_bReplaceAll;
};

class GmOverwriteDlg : public wxDialog
{
public:
	struct OverwriteBlob
	{
		OverwriteBlob (const wxString & FileName
					, const GmLeafInfo & New
					, const GmLeafInfo & Old) : 
					m_FileName (FileName), m_New (New), m_Old (Old), m_IsOK (false), m_RetCode (-1) {}
		wxString		m_FileName;
		GmLeafInfo		m_New;
		GmLeafInfo		m_Old;
		bool			m_IsOK;
		int				m_RetCode;
	};

	enum { IDC_REPLACE_ALL = 10000, };
	GmOverwriteDlg (wxWindow * parent
					, const wxString & FileName
					, const GmLeafInfo & New
					, const GmLeafInfo & Old
					, const wxString & title);

private:
	void OnReplaceAll (wxCommandEvent &event);
	DECLARE_EVENT_TABLE ()
    DECLARE_NO_COPY_CLASS(GmOverwriteDlg)
};

#endif //__MONITOR_IMPL_H__
