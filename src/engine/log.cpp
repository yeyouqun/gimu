//
//
// Gimu log definition, use uif as its implementation.
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
#include <engine/log.h>
#include <engine/writer.h>
#include <engine/zipfile.h>
#include <engine/execunit.h>
#include <engine/reader.h>

GmLogger::GmLogger (const wxString & szLogFile) : GmUifApp (szLogFile)
{
}

void GmLogger::AddNewLog (ubyte8 time)
{
	wxDateTime now (TimeTToWxDateTime (time));
	AddNewRootSource (now.Format (), time);
}

static inline void LogItemToLeafInfo (GmLeafInfo & node, const GmLogItem & item)
{
	node.Name = item.szJobName;
	node.CreateTime = item.LogTime;
}

void GmLogger::AddNewLogItem (const GmLogItem & item)
{
	GmLeafInfo info;
	LogItemToLeafInfo (info, item);
	vector<char> extras;
	GmMemoryOutStream out;
	item.ToBuffer (out);
	extras.assign (out.GetBlock ().rd_ptr (), out.GetBlock ().wr_ptr ());
	AddNewLeafItem (info, 0, extras);
}

void GmLogItem::ToBuffer (GmMemoryOutStream & out) const
{
	out.AddString (szJobName);
	out.AddString (szLogItem);
	out.AddUByte8 (LogTime);
}

void GmLogItem::FromBuffer (GmMemoryInStream & in)
{
	szJobName = in.GetString ();
	szLogItem = in.GetString ();
	LogTime = in.GetUByte8 ();
}

struct GmGetLogHandler : public GmNodeHandler
{
	vector<GmLogItem*> & logs;
	GmGetLogHandler (vector<GmLogItem*> & logs) : logs (logs) {}
	virtual bool HandleNode (GmSnapNode * pNode, const string & UNUSED (path))
	{
		GmLogItem * pLogItem = new GmLogItem;
		const vector<char> & extras = pNode->pLeafNode->vcExtendData;
		GmMemoryInStream in (&extras[0], extras.size ());
		pLogItem->FromBuffer (in);
		logs.push_back (pLogItem);
		return true;
	}
};

void GmLogger::GetAllLogs (vector<GmLogItem*> & logs)
{
	GmGetLogHandler handler (logs);
	GetAllItems (handler);
}

void GmLogger::EndAddLog ()
{
	EndAddRoot ();
}

GmLogger::~GmLogger ()
{
	EndAddLog ();
}

void GetLogsOfJob (const wxString & Job, const wxString & log, vector<GmLogItem*> & logs)
{
	ACE_Lock_Adapter<ACE_Process_Mutex> locker (GmHandleDataThread::m_mutex);
	GmLogger loger (log);

	vector<GmLogItem*> alllogs;
	GmAutoClearVector<GmLogItem> acreports (alllogs);
	loger.GetAllLogs (alllogs);
	for (size_t index = 0; index < alllogs.size (); ++index) {
		if (Job.IsEmpty () || alllogs[index]->szJobName == Job) {
			logs.push_back (alllogs[index]);
			alllogs[index] = 0;
		}
	}

	vector<GmLogItem*>::iterator pos = std::remove (alllogs.begin ()
														, alllogs.end ()
														, static_cast<GmLogItem*> (0));
	alllogs.erase (pos, alllogs.end ());
}

void ExportLogsToTXT (const wxString & Job, const wxString & logs, const wxString & txt)
{
	try {
		if (!wxFileName::DirName (GetPathName (txt)).IsAbsolute ()) return;
		if (!wxFile::Exists (logs)) return;
		if (!wxFile ().Open (txt, wxFile::write)) return;

		wxTextFile txtfile (txt);
		if (!txtfile.Open (wxConvLibc)) return;

		vector<GmLogItem*> logitems;
		GmAutoClearVector<GmLogItem> aclogs (logitems);
		GetLogsOfJob (Job, logs, logitems);

		for (size_t index = 0; index < logitems.size (); ++index) {
			wxString message = wxString::Format (_("IDS_JOB_LOG")
									, wxDateTime (TimeTToWxDateTime (logitems[index]->LogTime)).Format ()
									, logitems[index]->szJobName.c_str ()
									, logitems[index]->szLogItem.c_str ());
			txtfile.AddLine (message);
		}

		txtfile.Write (wxTextFileType_Dos);
	}
	catch (...) {
	}

	return;
}


wxString ENGINE_EXPORT GetLogString (const GmLogItem & log)
{
	wxString message = wxString::Format (_("IDS_JOB_LOG")
									, wxDateTime (TimeTToWxDateTime (log.LogTime)).Format ()
									, log.szJobName.c_str (), log.szLogItem.c_str ());
	message += wxT ("\n");
	return message;
}