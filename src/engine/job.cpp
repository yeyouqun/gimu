//
//
// Gimu job definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2009-12-12
//
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/uiffile.h>
#include <engine/uifapp.h>
#include <engine/job.h>
#include <engine/exception.h>
#include <engine/gmiostream.h>
#include <engine/reader.h>

#define JOB_SOURCE_NAME wxT("GM_JOB_LIST")

class GmJobsContainer : public GmUifApp
{
	friend void AddJobsToJobFile (const wxString & , vector<GmJobItem*> &);
	friend void GetJobsFromJobFile (const wxString &, vector<GmJobItem*> &);
public:
	GmJobsContainer (const wxString & szJobFile) : GmUifApp (szJobFile) {}
	void AddNewGroup (const wxString & szGroup, ubyte8 time)
	{
		AddNewRootSource (szGroup, time);
	}

	void AddNewJobItem (const GmJobItem & item)
	{
		GmLeafInfo info;
		JobItemToInfo (item, info);
		vector<char> extras;
		GmMemoryOutStream out;
		item.ToBuffer (out);
		extras.assign (out.GetBlock ().rd_ptr (), out.GetBlock ().wr_ptr ());
		AddNewLeafItem (info, 0, extras);
	}
	void GetAllJobs (vector<GmJobItem*> & items);
	void EndSubGroup ()
	{
		EndSubDirectory ();
	}
	wxString GetLogName () { return GetAppName (); }
	~GmJobsContainer ()
	{
	}
};

struct GmGetJobsHandler : public GmNodeHandler
{
	vector<GmJobItem*> & items;
	GmGetJobsHandler (vector<GmJobItem*> & items) : items (items) {}
	virtual bool HandleNode (GmSnapNode * pNode, const string & UNUSED (path))
	{
		GmJobItem * pJobItem = new GmJobItem;
		const vector<char> & extras = pNode->pLeafNode->vcExtendData;
		GmMemoryInStream in (&extras[0], extras.size ());
		pJobItem->FromBuffer (in);
		items.push_back (pJobItem);
		return true;
	}
};

void GmJobsContainer::GetAllJobs (vector<GmJobItem*> & items)
{
	GmGetJobsHandler handler (items);
	GetAllItems (handler);
}

void AddJobsToJobFile  (const wxString &szJobFile, vector<GmJobItem*> & items)
{
	if (wxFileName::FileExists (szJobFile))
		if (!wxRemoveFile (szJobFile)) {
			throw GmException (GetSystemError ());
		}

	if (items.empty ()) {
		return;
	}

	GmJobsContainer cont (szJobFile);
	cont.AddNewGroup (JOB_SOURCE_NAME, wxDateTime::GetTimeNow ());
	for (size_t index = 0; index < items.size (); ++index) {
		cont.AddNewJobItem (*items[index]);
	}

	cont.EndAddRoot ();
}

void GetJobsFromJobFile  (const wxString &szJobFile, vector<GmJobItem*> & items)
{
	GmJobsContainer cont (szJobFile);
	cont.GetAllJobs (items);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GmJobItem::GmJobItem (const wxString & Name, const wxString & Dest
					  , GmOptions Options, ubyte4 Time, const wxString & desc)
					: szName (Name)
					, szDest (Dest)
					, szDesc (desc)
					, Time (Time)
					, Options (Options)
{
}

void GmJobItem::AddPlan (ubyte8 Start, ubyte8 End, char Intver, ubyte2 AtHour, ubyte2 Type, ubyte4 Day)
{
	vpPlans.push_back (new GmJobPlan (Start, End, Intver, AtHour, Type, Day));
}

void GmJobItem::AddFilter (const wxString & FileName, wxString & DirName, bool IsDir)
{
	Filter.AddFilter (FileName, DirName, IsDir);
}

void GmJobItem::SetExclude (bool Exclude)
{
	Filter.SetExclude (Exclude);
}

GmJobItem::GmJobItem ()
			: Time (wxDateTime::GetTimeNow ())
			, SplitSize (-1)
			, MergeTimes (5)
			, DeflateType (DEF_NO_DEFLATE)
			, EncryptType (ENC_NO_ENC)
			, Options (GO_NO_RUN_PLAN)
{
}

void GmJobItem::ToBuffer (GmMemoryOutStream & out)  const
{
	out.AddString (szName);
	out.AddString (szDest);
	out.AddString (szDesc);
	out.AddString (szPass);

	out.AddUByte8 (Time);

	out.AddUByte8 (SplitSize);
	out.AddUByte2 (MergeTimes);
	out.AddByte4 (DeflateType);
	out.AddByte4 (EncryptType);

	ubyte4 size = (ubyte4)vpSources.size ();
	out.AddUByte4 (size);
	for (ubyte4 index = 0; index < size; ++index) {
		vpSources[index]->ToBuffer (out);
	}

	Filter.ToBuffer (out);

	size = (ubyte4)vpPlans.size ();
	out.AddUByte4 (size);
	for (size_t index = 0; index < vpPlans.size (); ++index)
		vpPlans[index]->ToBuffer (out);
	
	out.AddUByte4 ((ubyte4)Options);
}

void GmJobItem::FromBuffer (GmMemoryInStream &in)
{
	szName = in.GetString ();
	szDest = in.GetString ();
	szDesc = in.GetString ();
	szPass = in.GetString ();

	Time = in.GetUByte8 ();
	SplitSize = in.GetUByte8 ();
	MergeTimes = in.GetUByte2 ();
	DeflateType = static_cast<GmDeflateType> (in.GetByte4 ());
	EncryptType = static_cast<GmEncryptType> (in.GetByte4 ());

	ubyte4 size = in.GetUByte4 ();
	for (ubyte4 index = 0; index < size; ++index) {
		GmJobSource * source = new GmJobSource;
		source->FromBuffer (in);
		vpSources.push_back (source);
	}

	Filter.FromBuffer (in);

	size = in.GetUByte4 ();
	for (ubyte4 index = 0; index < size; ++index) {
		GmJobPlan * plan = new GmJobPlan;
		plan->FromBuffer (in);
		vpPlans.push_back (plan);
	}
	
	Options = (GmOptions)in.GetUByte4 ();
}

GmJobItem::~GmJobItem ()
{
	ClearFilter ();
	ClearSources ();
	ClearPlan ();
}

void GmJobItem::SetJobType (int type)
{
	Options &= ~GO_TYPE_MASK;// 清除原来的类型。
	type &= GO_TYPE_MASK;
	Options |= type;
}

wxString ENGINE_EXPORT GetJobTypeString (int type)
{
	type &= GO_TYPE_MASK;
	if (type == GO_TYPE_SYNC) return GetSyncTypeStr ();
	else if (type == GO_TYPE_DUP) return GetDupTypeStr ();
	else if (type == GO_TYPE_ZIP) return GetZipTypeStr ();
	return Get7zTypeStr ();
}

ubyte4 ENGINE_EXPORT GetJobTypeValue (const wxString & str)
{
	if (str == GetSyncTypeStr ()) return GO_TYPE_SYNC;
	else if (str == GetDupTypeStr ()) return GO_TYPE_DUP;
	else if (str == GetZipTypeStr ()) return GO_TYPE_ZIP;
	return GO_TYPE_7Z;
}

wxString GmJobItem::GetJobString () const
{
	return GetJobTypeString (Options);
}

void GmJobItem::AddSource (const wxString & path, int Options/* = 0*/)
{
	vpSources.push_back (new GmJobSource (path, Options));
}

void GmJobItem::RemoveSource (const wxString & path)
{
	size_t index = 0;
	GmJobSource * pSource = 0;
	for (size_t index = 0; index < vpSources.size (); ++index) {
		if (vpSources[index]->szPath == path) {
			pSource = vpSources[index];
			break;
		}
	}

	if (pSource != 0) {
		vector<GmJobSource*>::iterator begin = vpSources.begin ();
		begin += index;
		vpSources.erase (begin);
		delete pSource;
	}
}

void GmJobItem::CopyItem (const GmJobItem & item)
{
	szName		= item.szName;
	szDest		= item.szDest;
	szDesc		= item.szDesc;
	szPass		= item.szPass;
	Time		= item.Time;
	SplitSize	= item.SplitSize;
	MergeTimes	= item.MergeTimes;

	DeflateType = item.DeflateType;
	EncryptType = item.EncryptType;

	for_each (vpSources.begin (), vpSources.end (), ReleaseMemory ());
	vpSources.clear ();
	for (size_t index = 0; index < item.vpSources.size (); ++index)
		AddSource (item.vpSources[index]->szPath, item.vpSources[index]->Options);

	Filter = item.Filter;
	for_each (vpPlans.begin (), vpPlans.end (), ReleaseMemory ());
	vpPlans.clear ();
	for (size_t index = 0; index < item.vpPlans.size (); ++index)
		AddPlan (*item.vpPlans[index]);

	Options = item.Options;
}

wxString GmJobItem::GetUifName () const
{
	wxString path = AssemblePath (szDest, szName);
	path = AssemblePath (path, szName);
	path += wxT (".uif");
	return 	path;
}

////////////////

void GmJobPlan::ToBuffer (GmMemoryOutStream &out)  const
{
	out.AddUByte8 (StartTime);
	out.AddUByte8 (EndTime);
	out.AddUByte8 (NextTime);
	out.AddChar (IntervalType);
	out.AddUByte4 (WeekOrDay);
	out.AddUByte2 (AtHour);
	out.AddUByte2 (BakType);
}

void GmJobPlan::FromBuffer (GmMemoryInStream &in)
{
	StartTime = in.GetByte8 ();
	EndTime = in.GetUByte8 ();
	NextTime = in.GetUByte8 ();
	IntervalType = in.GetChar ();
	WeekOrDay = in.GetUByte4 ();
	AtHour = in.GetUByte2 ();
	BakType = in.GetUByte2 ();
}

static MyWeekDay GetMyWeekDayFromWxDateTime (const wxDateTime::WeekDay day)
{
	switch (day)
	{
	case wxDateTime::Sun:
		return SUNDAY;
	case wxDateTime::Mon:
		return MONDAY;
	case wxDateTime::Tue:
		return TUESDAY;
	case wxDateTime::Wed:
		return WENDESDAY;
	case wxDateTime::Thu:
		return THURSDAY;
	case wxDateTime::Fri:
		return FRIDAY;
	case wxDateTime::Sat:
		return SATURDAY;
	}

	return SUNDAY;
}

static bool TodayIsSelected (const wxDateTime::WeekDay day, ubyte4 WeekOrDay)
{
	bool yes = false;
	if (((WeekOrDay & SUNDAY) && (day == wxDateTime::Sun)) ||
		((WeekOrDay & MONDAY) && (day == wxDateTime::Mon)) ||
		((WeekOrDay & TUESDAY) && (day == wxDateTime::Tue)) ||
		((WeekOrDay & WENDESDAY) && (day == wxDateTime::Wed)) ||
		((WeekOrDay & THURSDAY) && (day == wxDateTime::Thu)) ||
		((WeekOrDay & FRIDAY) && (day == wxDateTime::Fri)) ||
		((WeekOrDay & SATURDAY) && (day == wxDateTime::Sat)))
		yes = true;

	return yes;
}

class WeekDayLinkInitiator
{
	struct WeekDayLink
	{
		WeekDayLink * next;
		wxDateTime::WeekDay day;
	}sun, mon, tue, wed, thu, fri, sat;
public:
	WeekDayLinkInitiator ()
	{
		sun.day = wxDateTime::Sun;
		sun.next = &mon;

		mon.day = wxDateTime::Mon;
		mon.next = &tue;

		tue.day = wxDateTime::Tue;
		tue.next = &wed;

		wed.day = wxDateTime::Wed;
		wed.next = &thu;

		thu.day = wxDateTime::Thu;
		thu.next = &fri;

		fri.day = wxDateTime::Fri;
		fri.next = &sat;

		sat.day = wxDateTime::Sat;
		sat.next = &sun;
	}

	int GetNextWeekDayInterval (wxDateTime::WeekDay now, int WeekDay)
	{
		if (WeekDay == 0) return 0;
		WeekDayLink * p = 0;
		int i = 0;
		for (p = &sun;; p = p->next) {
			if (p->day == now) break;
			if (++i >= 7) break; // 防止进入死循环。
		}

		i = 0; 

		while (true) {
			if (GetMyWeekDayFromWxDateTime (p->day) & WeekDay) // 一定有一个条件为真。
				break;
			++i;
			p = p->next;
		}

		return i;
	}
}wdli;



ubyte8 GmJobPlan::GetNextTime () const
{
	ubyte8 next = 0;
	wxDateTime now;
	now.SetToCurrent ();
	ubyte8 tn = now.GetValue ().GetValue ();

	if (tn >= StartTime && tn <= EndTime) {
		if (IntervalType == DAY_INTERVAL) {
			wxDateTime start (TimeTToWxDateTime (StartTime));
			wxTimeSpan span = now - start;
			//
			// 计算当前时间已经过去了几个间隔天数。
			//
			ubyte4 daysinterval = span.GetDays () / WeekOrDay;
			ubyte4 daysremain = span.GetDays () % WeekOrDay;

			if (daysremain == 0) {
				//
				//  如果时间间隔刚好过去整个间隔天数，并且当前小时已经大于设定小时，应该要下一个间隔才执行。
				//
				if (now.GetHour () > AtHour) // beyond the time now.
					daysinterval++;
			}
			else {
				//
				// 如果时间间隔还还没到，则执行应该在下一个间隔时。
				//
				daysinterval ++;
			}

			daysinterval *= WeekOrDay;
			wxTimeSpan span2 = wxTimeSpan::Days (daysinterval);
			span2 += wxTimeSpan::Hours (AtHour);
			start += span2;
			next = start.GetValue ().GetValue ();
		}
		else if (IntervalType == WEEK_INTERVAL) {
			wxDateTime::WeekDay day = now.GetWeekDay ();
			bool selected = TodayIsSelected (day, WeekOrDay);
			ubyte4 daysinterval = 0;
			if (!selected || AtHour > now.GetHour ()) {
				daysinterval = wdli.GetNextWeekDayInterval (day, WeekOrDay);
				assert (daysinterval <= 7);
			}
			//now.GetWeekDay ()
		}
	}

	return next;
}

GmJobPlan::GmJobPlan (ubyte8 Start, ubyte8 End, char Intver, ubyte2 AtHour, ubyte2 Type, ubyte4 Day)
{
	StartTime = Start;
	EndTime = End;
	NextTime = Start;
	IntervalType = Intver;
	WeekOrDay = Day;
	AtHour = AtHour;
	BakType = GRET_TYPE_INC;
}

GmJobPlan::GmJobPlan () 
		: StartTime (0)
		, EndTime (0)
		, NextTime (0)
		, IntervalType (0)
		, WeekOrDay (0)
		, AtHour (0)
		, BakType (GRET_TYPE_INC)
{
}

//////////////////
void GmJobFilterItem::ToBuffer (GmMemoryOutStream &out) const
{
	out.AddChar ((char) IsDir);
	out.AddString (FileName);
	out.AddString (DirName);
}

void GmJobFilterItem::FromBuffer (GmMemoryInStream &in)
{
	IsDir = (bool)in.GetChar ();
	FileName = in.GetString ();
	DirName = in.GetString ();
}

GmJobFilterItem::GmJobFilterItem (const wxString & FileName, wxString & DirName, bool IsDir)
					: IsDir (IsDir)
					, FileName (FileName)
					, DirName (DirName)
{
}

GmJobFilterItem::GmJobFilterItem ()
{
}

////////////////////
GmJobFilter::GmJobFilter (bool bExclude)
					: bExclude (bExclude )
{
}

void GmJobFilter::AddFilter (const wxString & FileName, wxString & DirName, bool IsDir)
{
	vpFilters.push_back (new GmJobFilterItem (FileName, DirName, IsDir));
}

void GmJobFilter::CopyItem (const GmJobFilter & item)
{
	for_each (vpFilters.begin (), vpFilters.end (), ReleaseMemory ());
	vpFilters.clear ();
	bExclude = item.bExclude;
	for (size_t index = 0; index < item.vpFilters.size (); ++index)
		AddFilter (item.vpFilters[index]->FileName, item.vpFilters[index]->DirName, item.vpFilters[index]->IsDir);
}

GmJobFilter::~GmJobFilter ()
{
	ClearFilters ();
}

void GmJobFilter::ToBuffer (GmMemoryOutStream &out) const
{
	out.AddChar ((char)bExclude);
	ubyte4 size = (ubyte4)vpFilters.size ();
	out.AddUByte4 (size);
	for (size_t index = 0; index < vpFilters.size (); ++index)
		vpFilters[index]->ToBuffer (out);
}
void GmJobFilter::FromBuffer (GmMemoryInStream &in)
{
	bExclude = (bool)in.GetChar ();
	ubyte4 size = in.GetUByte4 ();
	for (ubyte4 index = 0; index < size; ++index) {
		GmJobFilterItem * filter = new GmJobFilterItem;
		filter->FromBuffer (in);
		vpFilters.push_back (filter);
	}
}

void GmJobFilter::SetExclude (bool bExclude)
{
	bExclude = bExclude;
}

////////////////////


GmJobItem * CreateDefaultJobItem (const wxString &name, const wxString &dest)
{
	GmJobItem * pJob = new GmJobItem;
	pJob->Name (name);
	pJob->DestPath (dest);
	pJob->SetTime (wxDateTime ().GetTimeNow ());
	pJob->SetSplitSize (NOT_SPLIT);
	pJob->Times (DEFAULT_MERGE_TIMES);

	pJob->SetDeflateType (DEF_DEFAULT);
	pJob->SetEncryptType (ENC_NO_ENC);

	pJob->SetOption (GO_NO_RUN_PLAN);
	return pJob;
}

ubyte8 ENGINE_EXPORT GetNextExecTime (const GmJobItem* pJob)
{
	if (pJob->GetPlans ().size () == 0) {
		return 0;
	}

	const vector<GmJobPlan*> & plans = pJob->GetPlans ();
	ubyte8 next = 0;
	for (size_t index = 0; index < plans.size (); ++index) {
		const GmJobPlan & plan = *plans[index];
		ubyte8 next2 = plan.GetNextTime ();
		next = next > next2 ? next : next2;
	}

	return next;
}

void ENGINE_EXPORT UniqueSource (GmJobItem * pJobItem)
{
	vector<GmJobSource*> & sources = pJobItem->GetSources ();
	std::sort (sources.begin (), sources.end ());
	vector<GmJobSource*>::iterator pos = std::unique (sources.begin (), sources.end ());
	for (; pos != sources.end (); ++pos) delete *pos;
	sources.erase (pos, sources.end ());
}

int ENGINE_EXPORT GetArchiveType (const wxString & filename)
{
	wxFileName fn (filename);
	if (!fn.IsAbsolute ())
		return GO_TYPE_NONE;

	if (fn.GetExt ().CmpNoCase (wxT ("uif")) == 0) {
		GmUnitedIndexFile uif (filename, 0);
		const vector<GmUifRootEntry*> items = uif.GetAllRootEntries ();
		if (items.empty ()) return GO_TYPE_ZIP;
		const GmUifRootEntry & root = *items.at (0);
		return RootArchieveTypeToJobArchieveType (root.EntryDataType);
	}

	if (fn.GetExt ().CmpNoCase (wxT ("zip")) == 0) {
		return GO_TYPE_ZIP;
	}
	else if (fn.GetExt ().CmpNoCase (wxT ("7z")) == 0) {
		return GO_TYPE_7Z;
	}

	throw GmException (_(""));
}

int RootArchieveTypeToJobArchieveType (int type)
{
	if (type == ST_TYPE_SYNC) return GO_TYPE_SYNC;
	else if (type == ST_TYPE_DUP) return GO_TYPE_DUP;
	else if (type == ST_TYPE_ZIP) return GO_TYPE_ZIP;
	return GO_TYPE_7Z;
}

int JobArchieveTypeToRootArchieveType (int type)
{
	if (type == GO_TYPE_SYNC) return ST_TYPE_SYNC;
	else if (type == GO_TYPE_DUP) return ST_TYPE_DUP;
	else if (type == GO_TYPE_ZIP) return ST_TYPE_ZIP;
	return GO_TYPE_7Z;
}

ENGINE_EXPORT void GetJobsRunWhileStart (const wxString & jobcontainer, vector<wxString> & jobs)
{
	jobs.clear ();
	vector<GmJobItem *>	items;
	GmAutoClearVector<GmJobItem> acvj (items);
	try {
		GetJobsFromJobFile (jobcontainer, items);
	}
	catch (...) {
	}

	for (size_t index = 0; index < items.size (); ++index) {
		GmJobItem * pItem = items[index];
		if (pItem->RunStart ()) jobs.push_back (pItem->Name ());
	}
}

ENGINE_EXPORT void GetJobsRunWhileShutdown (const wxString & jobcontainer, vector<wxString> & jobs)
{
	vector<GmJobItem *>	items;
	GmAutoClearVector<GmJobItem> acvj (items);
	try {
		GetJobsFromJobFile (jobcontainer, items);
	}
	catch (...) {
	}

	for (size_t index = 0; index < items.size (); ++index) {
		GmJobItem * pItem = items[index];
		if (pItem->RunShutdown ()) jobs.push_back (pItem->Name ());
	}
}

static bool InTimePlusSeconds (const wxDateTime & time, ubyte8 next, int DeltaSeconds)
{
	if (next >= time.GetValue ().GetValue ()) {
		wxDateTime schedule (TimeTToWxDateTime (next));
		wxTimeSpan span = schedule - time;
		if (span.GetSeconds () <= DeltaSeconds) return true;
	}

	return false;
}

ENGINE_EXPORT void GetJobsScheduledNow (const wxString & jobcontainer, vector<wxString> & jobs)
{
	vector<GmJobItem *>	items;
	GmAutoClearVector<GmJobItem> acvj (items);
	try {
		GetJobsFromJobFile (jobcontainer, items);
	}
	catch (...) {
	}

	wxDateTime now;
	now.SetToCurrent ();
	ubyte8 time = now.GetValue ().GetValue ();

	for (size_t index = 0; index < items.size (); ++index) {
		GmJobItem * pItem = items[index];
		ubyte8 next = GetNextExecTime (pItem);
		if (next == 0) continue;
		//
		// Daemon 每 CHECK_INTERVAL 检查一次，因此，如果执行时间在两倍 CHECK_INTERVAL 时间之内，
		// 还是可以执行的。
		//
		if (InTimePlusSeconds (now, next, GimuJobScheduleDispersion))
			jobs.push_back (pItem->Name ());
	}
}


ENGINE_EXPORT bool IsThisJobAtScheduleTime (const GmJobItem * pJobItem)
{
	wxDateTime now;
	now.SetToCurrent ();
	ubyte8 next = GetNextExecTime (pJobItem);
	if (next == 0) return false;
	if (InTimePlusSeconds (now, next, GimuJobScheduleDispersion)) return true;
	return false;
}