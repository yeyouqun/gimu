//
//
// Gimu log definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2009-12-12
//
//

#include <engine/uiffile.h>
#include <engine/uifapp.h>

#ifndef __GM_LOG_H__
#define __GM_LOG_H__

//
//  日志 ID 从零开始。条数小于 4G。
//
struct GmLogItem
{
	wxString	szJobName;
	wxString	szLogItem;
	ubyte8		LogTime;
private:
	friend class GmLogger;
	friend struct GmGetLogHandler;
	void ToBuffer (GmMemoryOutStream &) const;
	void FromBuffer (GmMemoryInStream &);
};

class ENGINE_EXPORT GmLogger : public GmUifApp
{
public:
	GmLogger (const wxString & szLogFile);
	void AddNewLog (ubyte8 time);
	void AddNewLogItem (const GmLogItem & item);
	void GetAllLogs (vector<GmLogItem*> & logs);
	void EndAddLog ();
	wxString GetLogName () { return GetAppName (); }
	~GmLogger ();
};

//
// @Job		任务名，如果任务名为空，则取得所有的日志。
// @report	对应的报表数据文件。
// @rpts	对应任务的所有日志数据。
//
void ENGINE_EXPORT GetLogsOfJob (const wxString & Job, const wxString & log, vector<GmLogItem*> & logs);
//
// @Job		任务名，如果任务名为空，则输出所有的日志。
// @report	对应的报表数据文件。
// @xml		txt 文件名
//
void ENGINE_EXPORT ExportLogsToTXT (const wxString & Job, const wxString & logs, const wxString & txt);

wxString ENGINE_EXPORT GetLogString (const GmLogItem & log);
#endif //__GM_LOG_H__