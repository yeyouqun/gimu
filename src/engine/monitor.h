//
// Monitor's implementation.
// Author:yeyouqun@163.com
// 2009-11-10
//
#include <engine/defs.h>

#ifndef __GM_MONITOR_H__
#define __GM_MONITOR_H__

struct GmLeafInfo;
struct ENGINE_EXPORT GmMonitor
{
	GmMonitor () : m_bStopped (false) {}
	virtual ~GmMonitor () {}
	virtual void HandleFile (const wxString &) = 0;
	virtual void HandleDir (const wxString &) = 0;
	virtual void HandleSize (ubyte4) = 0;
	virtual void HandleMessage (const wxString &) = 0;
	virtual void BeginAnalysis (const wxString & = wxString ()) = 0;
	virtual void EndAnalysis (const wxString & = wxString ()) = 0;
	virtual void HandleError (const wxString &) = 0;
	virtual void HandleWarn (const wxString &) = 0;
	virtual void HandleManualStop (const wxString &) = 0;
	//
	// no event created in these interface.
	//
	virtual void Begin (const wxString & = wxString ()) = 0;
	virtual void End (const wxString & = wxString ()) = 0;
	//
	// If return is empty, means not select the file.
	//
	virtual wxString SelectFile (const wxString &) = 0;
	virtual wxString HandlePassword () = 0;
	virtual bool OverwriteFile (const wxString &, ubyte8 ModifyTime, ubyte8 Size) = 0;
	virtual bool OverwriteFile (const wxString &, const GmLeafInfo & New, const GmLeafInfo & Old) = 0;
	virtual wxString HandleSelectDir (const wxString &) = 0;

	//
	// Get report or log file name.
	//
	virtual wxString GetReportFile () const = 0;
	virtual wxString GetLogFile () const = 0;
	virtual int GetLogReportLimit () const = 0;
	//
	// not virtual
	//
	void HandleDirectoryInfo (const GmDirectoryInfo &);
	const GmDirectoryInfo & GetDirectoryInfo () const;
	void HandleNewFile (const wxString & file);
	void ClearAllFile () { m_szFiles.clear (); }
	void GetAllNewFile (vector<wxString> &);
	bool IsStopped () { return m_bStopped; }
	void SetStop () { m_bStopped = true; }
protected:
	virtual void DoHandleDirectoryInfo (const GmDirectoryInfo &) = 0;
	GmDirectoryInfo m_Info;
	GmDirectoryInfo m_InfoLast;
private:
	bool m_bStopped;
	vector<wxString>	m_szFiles;
};

#endif //__GM_MONITOR_H__