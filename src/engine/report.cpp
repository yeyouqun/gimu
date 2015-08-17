//
//
// Gimu report definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2009-12-12
//
//

#include <gmpre.h>
#include <wx/textfile.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/report.h>
#include <engine/writer.h>
#include <engine/zipfile.h>
#include <engine/execunit.h>
#include <engine/reader.h>

GmReport::GmReport (const wxString & szReportName) : GmUifApp (szReportName)
{
}

void GmReport::AddNewReport (ubyte8 time)
{
	wxDateTime now (TimeTToWxDateTime (time));
	AddNewRootSource (now.Format (), time);
}

void GmReportItem::ToBuffer (GmMemoryOutStream & out) const
{
	out.AddString (JobName);
	out.AddString (JobDest);
	out.AddUByte4 (Info.Files);
	out.AddUByte4 (Info.Dirs);
	out.AddUByte8 (Info.Size);
	out.AddUByte8 (Info.CompressSize);
	out.AddUByte8 (ReportTime);
	out.AddUChar (JobType); //STORE_DATA_TO or GET_DATA_BACK
}

void GmReportItem::FromBuffer (GmMemoryInStream & in)
{
	JobName		= in.GetString ();
	JobDest		= in.GetString ();
	Info.Files	= in.GetUByte4 ();
	Info.Dirs	= in.GetUByte4 ();
	Info.Size	= in.GetUByte8 ();
	Info.CompressSize = in.GetUByte8 ();
	ReportTime	= in.GetUByte8 ();
	JobType		= in.GetUChar (); //STORE_DATA_TO or GET_DATA_BACK
}

static inline void ReportItemToLeafInfo (GmLeafInfo & node, const GmReportItem & item)
{
	//
	// 名字用来记录日志内容，
	//
	node.Name = item.JobName;
	node.CreateTime = item.ReportTime;
}

void GmReport::AddReportItem (const GmReportItem & item)
{
	GmLeafInfo info;
	ReportItemToLeafInfo (info, item);

	vector<char> extras;
	GmMemoryOutStream out;
	item.ToBuffer (out);
	extras.assign (out.GetBlock ().rd_ptr (), out.GetBlock ().wr_ptr ());
	AddNewLeafItem (info, 0, extras);
}


struct GmGetReportHandler : public GmNodeHandler
{
	vector<GmReportItem*> & reports;
	GmGetReportHandler (vector<GmReportItem*> & reports) : reports (reports) {}
	virtual bool HandleNode (GmSnapNode * pNode, const string & UNUSED (path))
	{
		GmReportItem * pReportItem = new GmReportItem;
		const vector<char> & extras = pNode->pLeafNode->vcExtendData;
		GmMemoryInStream in (&extras[0], extras.size ());
		pReportItem->FromBuffer (in);

		reports.push_back (pReportItem);
		return true;
	}
};

void GmReport::GetAllReport (vector<GmReportItem*> & reports)
{
	GmGetReportHandler handler (reports);
	GetAllItems (handler);
}

void GmReport::EndAddReport ()
{
	EndAddRoot ();
}

GmReport::~GmReport ()
{
	EndAddReport ();
}

void GetReportsOfJob (const wxString & Job, const wxString & report, vector<GmReportItem*> & rpts)
{
	ACE_Lock_Adapter<ACE_Process_Mutex> locker (GmHandleDataThread::m_mutex);
	vector<GmReportItem*> rptsdata;
	GmAutoClearVector<GmReportItem> acreports (rptsdata);

	GmReport rpt (report);
	rpt.GetAllReport (rptsdata);
	for (size_t index = 0; index < rptsdata.size (); ++index) {
		if (Job.IsEmpty () || rptsdata[index]->JobName == Job) {
			rpts.push_back (rptsdata[index]);
			rptsdata[index] = 0;
		}
	}

	vector<GmReportItem*>::iterator pos = std::remove (rptsdata.begin ()
														, rptsdata.end ()
														, static_cast<GmReportItem*> (0));
	rptsdata.erase (pos, rptsdata.end ());
}

void ENGINE_EXPORT ExportReportsToTXT (const wxString & Job, const wxString & report, const wxString & txt)
{
	try {
		if (!wxFileName::DirName (GetPathName (txt)).IsAbsolute ()) return;
		if (!wxFile::Exists (report)) return;
		if (!wxFile ().Open (txt, wxFile::write)) return;

		wxTextFile txtfile (txt);
		if (!txtfile.Open (wxConvLibc)) return;

		vector<GmReportItem*> reports;
		GmAutoClearVector<GmReportItem> acreports (reports);
		GetReportsOfJob (Job, report, reports);

		for (size_t index = 0; index < reports.size (); ++index) {
			GmReportItem * p = reports[index];
			wxString message = wxT ("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
			if (!Job.IsEmpty ()) {
				message += wxString::Format (_("IDS_JOB_REPORT_MESSAGE"), p->JobName.c_str (), (int)index);
				message += wxT ("\n");
			}
			message += GetReportString (*p);
			txtfile.AddLine (message);
		}

		txtfile.Write (wxTextFileType_Dos);
	}
	catch (...) {
	}

	return;
}

wxString ENGINE_EXPORT GetReportString (const GmReportItem & report)
{
	wxString message;
	message += wxString::Format (_("IDS_JOB_TIME_REPORT"), wxDateTime (TimeTToWxDateTime (report.ReportTime)).Format ());
	message += wxT ("\n");

	message += wxString::Format (_("IDS_JOB_NAME_REPORT"), report.JobName.c_str ());
	message += wxT ("\n");

	wxString type = report.JobType == STORE_DATA_TO ? _("IDS_STORE_DATA_TO_TYPE") : _("IDS_GET_DATA_BACK_TYPE");
	message += wxString::Format (_("IDS_JOB_RUN_TYPE"), type.c_str ());
	message += wxT ("\n");

	message += wxString::Format (_("IDS_TOTAL_DIR"), report.Info.Dirs);
	message += wxT ("\n");

	message += wxString::Format (_("IDS_TOTAL_FILE"), report.Info.Files);
	message += wxT ("\n");

	message += wxString::Format (_("IDS_TOTAL_SIZE"), report.Info.Size);
	message += wxT ("\n");

	return message;
}
