//
//
// Gimu report definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2009-12-12
//
//
#include <gmpre.h>
#include <engine/uifapp.h>

#ifndef __GM_REPORT_H__
#define __GM_REPORT_H__

struct GmReportItem
{
	wxString		JobName;
	wxString		JobDest;
	GmDirectoryInfo	Info;
	ubyte8			ReportTime;
	char			JobType; //STORE_DATA_TO or GET_DATA_BACK
private:
	friend struct GmGetReportHandler;
	friend class GmReport;
	void ToBuffer (GmMemoryOutStream &) const;
	void FromBuffer (GmMemoryInStream &);
};

class ENGINE_EXPORT GmReport : public GmUifApp
{
public:
	//
	// 报表文件名。
	//
	GmReport (const wxString & ReportName);
	//
	// 添加报表的存储数据源。
	//
	void AddNewReport (ubyte8 Time);
	void AddReportItem (const GmReportItem &);
	//
	// 取得所有报表项
	//
	void GetAllReport (vector<GmReportItem*> & Reports);
	//
	// 添加完成后，一定需要调用这个接口，否则将可能出现错误。
	//
	void EndAddReport ();
	wxString GetReportName () { return GetAppName (); }
	~GmReport ();
};

//
// @Job		任务名，如果任务名为空，则取得所有的报表。
// @report	对应的报表数据文件。
// @rpts	对应任务的所有报表数据。
//
void ENGINE_EXPORT GetReportsOfJob (const wxString & Job, const wxString & report, vector<GmReportItem*> & rpts);
//
// @Job		任务名，如果任务名为空，则输出所有的报表。
// @report	对应的报表数据文件。
// @xml		txt 文件名
//
void ENGINE_EXPORT ExportReportsToTXT (const wxString & Job, const wxString & report, const wxString & txt);

//
// 取得报表的字串表示
// @report 报表对象。
//
//
wxString ENGINE_EXPORT GetReportString (const GmReportItem & report);

#endif //__GM_REPORT_H__
