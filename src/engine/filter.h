//
// filter implementation.
// Author:yeyouqun@163.com
// 2010-3-14
//
#ifndef __GM_FILTER_H__
#define __GM_FILTER_H__

struct ENGINE_EXPORT GmFilter
{
	GmFilter (const wxString & path, const wxString & FileName);
	~GmFilter () {}
	/**
	 * 过滤器是否保留下面的文件
	 * @param path					文件的父路径
	 * @param FileName				文件的名字，不包含路径。
	 * @return						如果保留（不过滤），则返回真，否则返回假）
	 */
	bool Satisfy (const wxString & path, const wxString & FileName);
private:
	wxString		m_DirName;
	wxString		m_FileName;
};

class GmJobItem;
struct GmFiltersCollector
{
	GmFiltersCollector (const GmJobItem * pJob);
	~GmFiltersCollector ();
	bool ReserveFile (const wxString & path, const wxString & FileName);
	bool ReserveDir (const wxString & path, const wxString & FileName);
private:
	vector<GmFilter*>	m_vpFiles;
	vector<GmFilter*>	m_vpDirs;
	bool				bExclude;
};

#endif //__GM_FILTER_H__