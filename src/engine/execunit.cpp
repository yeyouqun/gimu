//
// exec unit of job
// Author:yeyouqun@163.com
// 2009-11-12
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/gmiostream.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/job.h>
#include <engine/uifalgo.h>
#include <engine/uifblock.h>
#include <engine/uiffile.h>
#include <engine/analysis.h>
#include <engine/zipfile.h>
#include <engine/exception.h>
#include <engine/storeentry.h>
#include <engine/execunit.h>
#include <engine/zipporcollector.h>
#include <engine/copycollector.h>
#include <engine/writer.h>
#include <engine/reader.h>
#include <engine/report.h>
#include <engine/log.h>

ACE_Process_Mutex GmHandleDataThread::m_mutex;

GmFiltersCollector * GetFilter (const GmJobItem * pJob)
{
	if (pJob->GetFilter ().vpFilters.empty ())
		return 0;

	//
	// 其他的代码。
	//
	return new GmFiltersCollector (pJob);
}

GmUnitedIndexFile * GetFormerTree (const GmJobItem * pJob, GmUifSourceVectorT & vspformer, GmMonitor * pMonitor)
{
	wxString UifName;
	UifName = wxString::Format (wxT ("%s\\%s\\%s.uif"), pJob->DestPath ().c_str ()
													, pJob->Name ().c_str ()
													, pJob->Name ().c_str ());
	auto_ptr<GmUnitedIndexFile> Uif (new GmUnitedIndexFile (UifName, pMonitor));
	const vector<GmUifRootEntry*> & roots = Uif->GetAllRootEntries ();

	typedef vector<GmUifRootEntry*>::const_reverse_iterator crit;
	for (crit index = roots.rbegin (); index != roots.rend (); ++index) {
		GmUifRootPairT rt;
		GmAutoClearRootPairTree acrt (rt);
		Uif->GetUifRootTree (**index, rt);
		MergeToLatestTree (vspformer, *rt.second);
	}
	
	return Uif.release ();
}

void * GmHandleDataThread::Entry ()
{
	try {
		GmZipDistillerBuffer::GetInstance ();
		m_pMonitor->Begin ();
		if (!m_bGetDataBack) {
			StoreDataTo ();
		}
		else {
			GetDataBack ();
		}
		EndJob ();
		m_pMonitor->End ();
	}
	catch (GmStopProcedureException &) {
		if (!m_pJob->IsTempJob ()) {
			wxDateTime now;
			now.SetToCurrent ();
			ubyte8 time = now.GetValue ().GetValue ();
			wxString log = wxString::Format (_("IDS_LOG_STOP_MESSAGE"), m_pJob->Name ().c_str ());
			AddLog (time, log);
		}
		m_pMonitor->HandleManualStop (wxT (""));
	}
	catch (GmException &ex) {
		if (!m_pJob->IsTempJob ()) {
			wxDateTime now;
			now.SetToCurrent ();
			ubyte8 time = now.GetValue ().GetValue ();
			wxString log = wxString::Format (_("IDS_LOG_ERROR_MESSAGE")
											, m_pJob->Name ().c_str ()
											, ex.GetMessage ().c_str ());
			AddLog (time, log);
		}

		wxString message;
		message.Format (_("IDS_JOB_HAS_EXCEPTION"), ex.GetMessage ().c_str ());
		m_pMonitor->HandleError (message);
	}
	catch (...) {
		wxString message (_("IDS_JOB_HAS_UNKNOW_EXCEPTION"));
		m_pMonitor->HandleError (message);
	}

	GmZipDistillerBuffer::DeleteInstance ();
	return 0;
}

void GmHandleDataThread::EndJob ()
{
	DoEndJob ();

	const GmDirectoryInfo & info = m_pMonitor->GetDirectoryInfo ();
	//
	// 没有产生数据，不需要记录。
	//
	if (info.Dirs == 0 && info.Files == 0)
		return;

	wxDateTime now;
	now.SetToCurrent ();
	ubyte8 time = now.GetValue ().GetValue ();
	wxString log = wxString::Format (_("IDS_LOG_MESSAGE"), m_pJob->Name ().c_str ());
	AddLog (time, log);
	AddReport (time);
}

//
// 由于只有一个数据文件，因此在添加报表或者日志时，必须顺序化进行。
//
void GmHandleDataThread::AddReport (ubyte8 time)
{
	ACE_Lock_Adapter<ACE_Process_Mutex> locker (m_mutex);
	GmReport report (m_pMonitor->GetReportFile ());

	GmReportItem item;
	item.Info = m_pMonitor->GetDirectoryInfo ();
	item.JobName = m_pJob->Name ();
	item.JobDest = m_pJob->DestPath ();
	item.JobType = m_bGetDataBack ? GET_DATA_BACK : STORE_DATA_TO;
	item.ReportTime = time;
	report.AddNewReport (time);
	report.AddReportItem (item);
	report.EndAddReport ();
	report.TidyByNumber (m_pMonitor->GetLogReportLimit ());
}

void GmHandleDataThread::AddLog (ubyte8 time, const wxString & message)
{
	ACE_Lock_Adapter<ACE_Process_Mutex> locker (m_mutex);
	GmLogger loger (m_pMonitor->GetLogFile ());
	GmLogItem li;
	li.LogTime = time;
	li.szJobName = m_pJob->Name ();
	li.szLogItem = message;
	loger.AddNewLog (time);
	loger.AddNewLogItem (li);
	loger.EndAddLog ();
	loger.TidyByNumber (m_pMonitor->GetLogReportLimit ());
}

void GmHandleDataThread::StoreDataTo ()
{
	wxDateTime now;
	now.SetToCurrent ();

	GmUifSourceVectorT vps;
	GmUifSourceVectorT vspformer;
	GmAutoClearSourceVectorTree acsvt_vps (vps);
	GmAutoClearSourceVectorTree acsvt_vspformer (vspformer);

	auto_ptr<GmFiltersCollector> filter (GetFilter (m_pJob));
	//
	// analysis
	//
	m_pMonitor->BeginAnalysis ();
	const vector<GmJobSource*> & sources = m_pJob->GetSources ();
	for (size_t index = 0; index < sources.size (); ++index) {
		GmUifSourcePairT * pTree = new GmUifSourcePairT;
		vps.push_back (pTree);
		TraverseTheDirForAnylasis (sources[index]->szPath, pTree, filter.get (), m_pMonitor);
	}

	GetJobWriter (m_pJob);

	auto_ptr<GmUnitedIndexFile> uif (GetFormerTree (m_pJob, vspformer, m_pMonitor));
	AnalysisLatestTree (vps, vspformer);
	ClearEmtpySourceTree (vps);

	GmUifRootPairT rp;
	rp.first = 0;
	rp.second = &vps;
	GmDirectoryInfo info = GetUifRootTreeDirectoryInfo (rp);
	m_pMonitor->EndAnalysis ();
	m_pMonitor->HandleDirectoryInfo (info);

	if (!vps.empty ()) {
		//
		// handle file data.
		//
		DoHandleData (vps, uif.get ());
		//
		// 先删除没成功处理的结点。
		//
		RemoveNodeNotFinished (vps);
		//
		// recored file node to uif file.
		//
		AddTreeToUif (now, vps, *uif);
	}
	//
	// merge data, not finished.
	//
	MergeDataToOne (*uif);
	DoHandleDataTrees (vps, vspformer);
}

void GmHandleDataThread::GetDataBack ()
{
	//
	// analysis
	//
	m_pMonitor->BeginAnalysis ();

	GmDirectoryInfo info;
	for (size_t index = 0; index < m_vpStoreNodes.size (); ++index)
		info += m_vpStoreNodes[index]->GetDirectoryInfo ();

	m_pMonitor->EndAnalysis ();
	m_pMonitor->HandleDirectoryInfo (info);
	if (RO_NEW_PLACE & m_Options) {
		GmLocalWriter * p = new GmLocalWriter (m_pMonitor, m_Options);
		p->SetDestPath (m_szDestPath);
		m_writer.reset (p);
	}
	//
	// get file data back.
	//
	for (size_t index = 0; index < m_vpStoreNodes.size (); ++index)
		m_vpStoreNodes[index]->GetDataBack (this);

	return;
}

GmWriter * GmHandleDataThread::GetDataBackWriter (const GmStoreEntry * pEntry, const GmUifSourceEntry * entry)
{
	if (RO_NEW_PLACE & m_Options) {
		assert (m_writer.get ());
	}
	else {
		assert (entry);
		//
		// 每个结点都是不一样的，
		//
		if (m_writer.get () != 0) delete m_writer.release ();

		m_writer.reset (new GmLocalWriter (m_pMonitor, m_Options));
		wxString szFullPath;
		if (entry->SourceType == GST_DIRECTORY)
			szFullPath = ToWxString (entry->SourceName);
		else
			szFullPath = ToWxString (GetPathName (entry->SourceName));

		m_writer->SetDestPath (szFullPath);
	}

	return m_writer.get ();
}
wxString GetJobDestPath (GmUnitedIndexFile * pUifFile
							   , const GmUifRootEntry & root
							   , GmMonitor * pMonitor);

GmReader * GmHandleDataThread::GetJobReader (GmUnitedIndexFile * pUifFile
											 , GmUifRootEntry * pUifRoot
											 , const GmUifSourceEntry * entry)
{
	//
	// 暂时支持本地的 Reader。
	//
	if (m_reader.get () == 0) m_reader.reset (new GmLocalReader (m_pMonitor));

	wxString szDestPath = GetJobDestPath (pUifFile, *pUifRoot, m_pMonitor);
	string srcpath = entry->SourceName;
	wxString crcpath = GetCrc32String (srcpath.c_str (), srcpath.length ());

	m_reader->SetDestPath (AssemblePath (szDestPath, crcpath));
	return m_reader.get ();
}

GmWriter * GmHandleDataThread::GetJobWriter (const GmJobItem * pItem)
{
	assert (!m_bGetDataBack);
	if (m_writer.get () == 0)
		m_writer.reset (new GmLocalWriter (m_pMonitor));
	//
	// 暂时只支持本地。
	//
	wxString dir = GetDestPath (*pItem);

	if (!m_writer->SetDestPath (dir)) {
		wxString message = wxString::Format (_("IDS_CANT_CREATE_DIR"), dir.c_str (), GetSystemError ().c_str ());
		throw GmException (message);
	}

	return m_writer.get ();
}

void GmHandleDataThread::AddTreeToUif (const wxDateTime & now
									   , GmUifSourceVectorT & vps
									   , GmUnitedIndexFile & uif)
{
	if (vps.empty ()) return;
	ubyte8 timenow = now.GetValue ().GetValue ();
	GmRootEntryType Type = uif.GetAllRootEntries ().empty () ? GRET_TYPE_FULL : GRET_TYPE_INC;
	ubyte2 type = (ubyte2)JobArchieveTypeToRootArchieveType (m_pJob->GetOptions () & GO_TYPE_MASK);
	vector<GmDataSetEntry*> vpsets;
	GmAutoClearVector<GmDataSetEntry> acvpsets (vpsets);
	vector<wxString> vszsets;
	m_pMonitor->GetAllNewFile (vszsets);

	for (size_t index = 0; index < vszsets.size (); ++index) {
		GmDataSetEntry * p = new GmDataSetEntry;
		p->SetName = ToStlString (vszsets[index]);
		vpsets.push_back (p);
	}

	AddTheseTreeToUifFile (vps, uif, Type, type, TM_DEPTH_FIRST, timenow, vpsets, vector<char> ());
	return;
}

void GmHandleDataThread::MergeTree (GmUifRootPairT & ResultRootPair, GmUnitedIndexFile & uif)
{
	//
	// 合并成最新的树。
	//
	assert (m_pMonitor);
	m_pMonitor->HandleMessage (_("IDS_BEGIN_MERGE"));
	m_pMonitor->BeginAnalysis ();

	const vector<GmUifRootEntry*> & roots = uif.GetAllRootEntries ();
	typedef vector<GmUifRootEntry*>::const_reverse_iterator crit;

	ResultRootPair.first = 0;
	for (crit rbegin = roots.rbegin (); rbegin != roots.rend (); ++rbegin) {
		if (ResultRootPair.first == 0 && ResultRootPair.first == 0) {
			uif.GetUifRootTree (**rbegin, ResultRootPair);
		}
		else {
			GmUifRootPairT source;
			GmAutoClearRootPairTree acut (source);
			uif.GetUifRootTree (**rbegin, source);
			MergeToLatestTree (*ResultRootPair.second, *source.second);
		}
	}

	m_pMonitor->EndAnalysis ();
	GmDirectoryInfo info = GetUifRootTreeDirectoryInfo (ResultRootPair);
	m_pMonitor->HandleDirectoryInfo (info);
}

GmHandleDataThread::GmHandleDataThread (const GmJobItem * pJob, GmMonitor * pMonitor)
										: m_pJob (pJob)
										, m_pMonitor (pMonitor)
										, m_vpStoreNodes ()
										, m_szDestPath ()
										, m_Options (0)
										, m_bGetDataBack (false)
{
	assert (pJob);
	assert (pMonitor);
}

GmHandleDataThread::GmHandleDataThread (const vector<GmStoreEntry*> & vpStoreNodes
										, const wxString & szDestPath
										, int Option
										, const GmJobItem * pJob
										, GmMonitor * pMonitor)
										: m_pJob (pJob)
										, m_pMonitor (pMonitor)
										, m_vpStoreNodes (vpStoreNodes) // don't release these nodes.
										, m_szDestPath (szDestPath)
										, m_Options (Option)
										, m_bGetDataBack (true)
{
	assert (pJob);
	assert (pMonitor);
}
