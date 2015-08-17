//
//
// Gimu job definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2009-12-12
//
//
#include <gmpre.h>
#include <engine/defs.h>

#ifndef __GM_JOB_H__
#define __GM_JOB_H__

struct ENGINE_EXPORT GmJobFilterItem
{
	GmJobFilterItem (const wxString & FileName, wxString & DirName, bool IsDir);
	GmJobFilterItem ();
	void ToBuffer (GmMemoryOutStream &) const;
	void FromBuffer (GmMemoryInStream &);
	bool IsDir;
	wxString	FileName;  //
	wxString	DirName;
private:
	friend bool operator == (const GmJobFilterItem &left, const GmJobFilterItem &right)
	{
		return (left.IsDir == right.IsDir && left.FileName == right.FileName && left.DirName == right.DirName);
	}
};

class GmMemoryOutStream;
class GmMemoryInStream;

struct GmJobFilter
{
	GmJobFilter (bool bExclude = false);
	void AddFilter (const wxString & FileName, wxString & DirName, bool IsDir);
	void ToBuffer (GmMemoryOutStream &) const;
	void FromBuffer (GmMemoryInStream &);
	void SetExclude (bool bExclude = true);
	void ClearFilters () {	GmAutoClearVector<GmJobFilterItem> filters (vpFilters); }
	~GmJobFilter ();
	bool						bExclude;
	vector<GmJobFilterItem*>	vpFilters;
private:
	friend class GmJobItem;
	void CopyItem (const GmJobFilter & item);
	GmJobFilter (const GmJobFilter & item) { CopyItem (item); }
	GmJobFilter & operator = (const GmJobFilter & item) { CopyItem (item);  return *this; }
};

#define DAY_INTERVAL 0x0
#define WEEK_INTERVAL 0x1

enum MyWeekDay {
	SUNDAY		= 1,
	MONDAY		= 1 << 1,
	TUESDAY		= 1 << 2,
	WENDESDAY	= 1 << 3,
	THURSDAY	= 1 << 4,
	FRIDAY		= 1 << 5,
	SATURDAY	= 1 << 6,
};

struct ENGINE_EXPORT GmJobSource
{
	GmJobSource (const wxString & path, int Options = 0) : szPath (path), Options (0) {}
	GmJobSource () : Options (0) {}
	void ToBuffer (GmMemoryOutStream &out) const
	{
		out.AddString (szPath);
		out.AddByte4 (Options);
	}
	void FromBuffer (GmMemoryInStream &in)
	{
		szPath = in.GetString ();
		Options = in.GetByte4 ();
	}
	wxString	szPath;
	int			Options;
private:
	friend bool operator == (const GmJobSource &left, const GmJobSource & right)
	{
		return IsSameDirName (left.szPath, right.szPath);
	}
	friend class GmJobItem;
	void CopyItem (const GmJobSource & item)
	{
		szPath = item.szPath;
		Options = item.Options;
	}
	GmJobSource (const GmJobSource & item) { CopyItem (item); }
	GmJobSource & operator = (const GmJobSource & item) { CopyItem (item);  return *this; }
};

struct ENGINE_EXPORT GmJobPlan
{
	ubyte8 GetNextTime () const;
	GmJobPlan (ubyte8 Start, ubyte8 End, char Intver, ubyte2 AtHour, ubyte2 Type, ubyte4 Day);
	GmJobPlan ();
	void ToBuffer (GmMemoryOutStream &) const;
	void FromBuffer (GmMemoryInStream &);

	ubyte8	StartTime;
	ubyte8	EndTime;
	ubyte8	NextTime;
	char	IntervalType; // 0 stands for day, 1 stands for week;
	ubyte4	WeekOrDay; // for day, is interval days, for week, from the 
						// least significant bit 0-6,stands for sunday to monday
	ubyte2	AtHour;
	ubyte2	BakType; // Full or inc,only for internal use.
private:
	friend bool operator == (const GmJobPlan &left, const GmJobPlan & right)
	{
		return memcmp (&left, &right, sizeof (GmJobPlan)) == 0 ? true : false;
	}
};

enum GmOptions {
	GO_OPTIONS_MASK			= 0x0000FFFF,
	GO_NO_RUN_PLAN			= 0x00000000,
	GO_RUN_ATSTART			= 0x00000001,
	GO_RUN_ATSHUTDOWN		= 0x00000002,//1 << 1,
	GO_RMFILE_AFTER_MERGE	= 0x00000004, //1 << 2,
	GO_IS_TEMP_JOB			= 0x00000008,
	GO_SYNC_REMOVED_FILE	= 0x00000010,
	GO_TYPE_MASK		= 0x000F0000,
	GO_TYPE_NONE		= 0x00000000,
	GO_TYPE_SYNC		= 0x00010000, // Sync
	GO_TYPE_DUP			= 0x00020000, // copy
	GO_TYPE_ZIP			= 0x00030000, // Zip
	GO_TYPE_7Z			= 0x00040000, // 7z
};

enum GmRestoreOption
{
	RO_NEW_PLACE		= 1,
	RO_ORIGINAL_PLACE	= 0,
	RO_DECIDE_BY_USER	= 1 << 2,
	RO_REPLACE_OLD		= 1 << 3,
	RO_SKIP				= 1 << 4,
	RO_OVERWRITE		= 1 << 5,
};

enum GmDeflateType
{
	DEF_BEST_SPEED		= 1,
	DEF_BEST_DEFLAT		= 9,
	DEF_DEFAULT			= -1,
	DEF_NO_DEFLATE		= 0,
};

#define DEFAULT_MERGE_TIMES 5
#define	NOT_SPLIT			-1

enum GmEncryptType {
	ENC_NO_ENC,
	ENC_AES128,
	ENC_AES192,
	ENC_AES256,
};


class GmMemoryOutStream;
class GmMemoryInStream;
class ENGINE_EXPORT GmJobItem
{
public:
	GmJobItem (const wxString & Name
				, const wxString & Dest
				, GmOptions Options
				, ubyte4 Time
				, const wxString & = wxString ());
	GmJobItem ();
	~GmJobItem ();
	GmJobItem (const GmJobItem & item) { CopyItem (item); }
	GmJobItem & operator = (const GmJobItem & item) { CopyItem (item);  return *this; }
	void ToBuffer (GmMemoryOutStream &) const;
	void FromBuffer (GmMemoryInStream &);
public:
	// plan
	void AddPlan (ubyte8 Start, ubyte8 End, char Intver, ubyte2 AtHour, ubyte2 Type, ubyte4 Day);
	void AddPlan (const GmJobPlan & plan) { vpPlans.push_back (new GmJobPlan (plan)); }
	const vector<GmJobPlan*> & GetPlans () const { return vpPlans; }
	vector<GmJobPlan*> & GetPlans () { return vpPlans; }
	void ClearPlan () {	GmAutoClearVector<GmJobPlan> plan (vpPlans); }
	// fiters
	void AddFilter (const wxString & FileName, wxString & DirName, bool IsDir);
	const GmJobFilter & GetFilter () const { return Filter; }
	GmJobFilter & GetFilter () { return Filter; }
	void ClearFilter () { Filter.ClearFilters (); }
	void SetExclude (bool Exclude = true);
	// password
	void SetPass (const wxString & pass) { szPass = pass; }
	wxString GetPass () const { return szPass; }
	// source
	void AddSource (const wxString & path, int Options = 0);
	void RemoveSource (const wxString & path);
	void ClearSources () {	GmAutoClearVector<GmJobSource> sources (vpSources); }
	const vector<GmJobSource*> & GetSources () const { return vpSources; }
	vector<GmJobSource*> & GetSources () { return vpSources; }
	// other members
	wxString DestPath () const { return szDest; }
	void DestPath (const wxString & path) { szDest = path; }
	void Name (const wxString & name) { szName = name; }
	wxString Name () const { return szName; }
	wxString GetUifName () const;
	void SetJobType (int type);
	ubyte4 GetJobType () const { return (Options & GO_TYPE_MASK); }
	wxString GetJobString () const;
	///------
	void DescStr (const wxString & desc) { szDesc = desc; }
	wxString DescStr () const { return szDesc; }
	void SetTime (ubyte8 time) { Time = time; }
	ubyte8 GetTime () const { return Time; }
	void SetSplitSize (ubyte8 Size) { SplitSize = Size; }
	ubyte8 GetSplitSize () const { return SplitSize; }
	void Times (ubyte2 times) { MergeTimes = times; }
	ubyte2 Times () const { return MergeTimes; }
	void SetDeflateType (GmDeflateType type) { DeflateType = type; }
	GmDeflateType GetDeflateType () const { return DeflateType; }
	void SetEncryptType (GmEncryptType type) { EncryptType = type; }
	GmEncryptType GetEncryptType () const { return EncryptType; }
	void SetOption (ubyte4 option) { Options |= option; }
	void ClearOption (ubyte4 option) { Options &= ~option; }
	ubyte4 GetOptions () const { return Options; }
	bool RunStart () const { return Options & GO_RUN_ATSTART ? true : false; }
	bool RunShutdown () const { return Options & GO_RUN_ATSHUTDOWN ? true : false; }
	void SetTempJob (bool temp)
	{ 
		if (temp) Options |= GO_IS_TEMP_JOB;
		else Options &= ~GO_IS_TEMP_JOB;
	}
	bool IsTempJob () const { return Options & GO_IS_TEMP_JOB ? true : false; }
private:
	wxString				szName;
	wxString				szDest;
	wxString				szDesc;
	wxString				szPass;

	ubyte8					Time;
	ubyte8					SplitSize;
	ubyte2					MergeTimes;

	GmDeflateType			DeflateType;
	GmEncryptType			EncryptType;

	vector<GmJobSource*>	vpSources;
	GmJobFilter				Filter;
	vector<GmJobPlan*>		vpPlans;
	ubyte4					Options;
private:
	//
	// copy and assign.
	//
	void CopyItem (const GmJobItem & item);
};

inline void JobItemToInfo (const GmJobItem & item, GmLeafInfo & info)
{
	info.CreateTime = item.GetTime ();
	info.Name = item.Name ();
}

wxString ENGINE_EXPORT GetJobAttribStr (GmJobItem*);
void ENGINE_EXPORT AddJobsToJobFile (const wxString &, vector<GmJobItem*> &);
void ENGINE_EXPORT GetJobsFromJobFile (const wxString &, vector<GmJobItem*> &);
ENGINE_EXPORT GmJobItem * CreateDefaultJobItem (const wxString &name, const wxString &dest);
ENGINE_EXPORT ubyte8 GetNextExecTime (const GmJobItem*);
void ENGINE_EXPORT UniqueSource (GmJobItem * pJobItem);
int ENGINE_EXPORT GetArchiveType (const wxString & filename);
wxString ENGINE_EXPORT GetJobTypeString (int);
ubyte4 ENGINE_EXPORT GetJobTypeValue (const wxString &);

ENGINE_EXPORT void GetJobsRunWhileStart (const wxString & jobcontainer, vector<wxString> & jobs);
ENGINE_EXPORT void GetJobsRunWhileShutdown (const wxString & jobcontainer, vector<wxString> & jobs);
ENGINE_EXPORT void GetJobsScheduledNow (const wxString & jobcontainer, vector<wxString> & jobs);
ENGINE_EXPORT bool IsThisJobAtScheduleTime (const GmJobItem * pJobItem);

int RootArchieveTypeToJobArchieveType (int type);
int JobArchieveTypeToRootArchieveType (int type);

inline wxString GetDestPath (const GmJobItem & item)
{
	return AssemblePath (item.DestPath (), item.Name ());
}

#endif //__GM_JOB_H__