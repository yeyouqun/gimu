//
// filter implementation.
// Author:yeyouqun@163.com
// 2010-3-14
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/util.h>
#include <engine/gmiostream.h>
#include <engine/uifblock.h>
#include <engine/job.h>
#include <engine/filter.h>


GmFilter::GmFilter (const wxString & path, const wxString & FileName)
				: m_FileName (FileName)
{
	m_DirName = RemoveLastSep (path);
}

bool GmFilter::Satisfy (const wxString & path, const wxString & FileName)
{
	if (!m_DirName.IsEmpty ())
		if (!IsSameDirName (path, m_DirName))
			 return false;

	return FileName.Matches (m_FileName);
}

GmFiltersCollector::GmFiltersCollector (const GmJobItem * pJob)
				: bExclude (true)
{
	if (pJob == 0)
		return;

	bExclude = pJob->GetFilter ().bExclude;
	const vector<GmJobFilterItem*> & filters = pJob->GetFilter ().vpFilters;
	for (size_t index = 0; index < filters.size (); ++index) {
		const GmJobFilterItem & item = *(filters[index]);
		if (item.IsDir) {
			m_vpDirs.push_back (new GmFilter (item.DirName, item.FileName));
		}
		else {
			m_vpFiles.push_back (new GmFilter (item.DirName, item.FileName));
		}
	}
}

GmFiltersCollector::~GmFiltersCollector ()
{
	GmAutoClearVector<GmFilter> acd (m_vpFiles);
	GmAutoClearVector<GmFilter> acf (m_vpDirs);
}

bool GmFiltersCollector::ReserveFile (const wxString & path, const wxString & FileName)
{
	if (m_vpFiles.empty ()) return true;
	bool bSatisfied = true;
	for (size_t index = 0; index < m_vpFiles.size (); ++index)
		if (bSatisfied = m_vpFiles[index]->Satisfy (path, FileName)) // 表示符合过滤条件，具体情况视 bExclude 而定
			break;

	return bSatisfied ? !bExclude : bExclude;
}

bool GmFiltersCollector::ReserveDir (const wxString & path, const wxString & FileName)
{
	if (m_vpDirs.empty ()) return true;
	bool bSatisfied = true;
	for (size_t index = 0; index < m_vpDirs.size (); ++index)
		if (bSatisfied = m_vpDirs[index]->Satisfy (path, FileName))  // 表示符合过滤条件，具体情况视 bExclude 而定
			break;

	return bSatisfied ? !bExclude : bExclude;
}