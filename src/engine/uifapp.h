//
//
// Gimu uif application base definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2010-1-10
//
//
#include <engine/uiffile.h>

#ifndef __GM_UIF_APP_H__
#define __GM_UIF_APP_H__

#define REMOVED_ITEM -1

class GmUifApp
{
public:
	void TidyByNumber (size_t number);
protected:
	GmUnitedIndexFile m_app;
	void GetAllItems (GmNodeHandler & handler);
	void AddNewRootSource (const wxString &, ubyte8);
	void AddSubDirectory (const wxString &, ubyte8);
	void EndSubDirectory ();
	void AddNewLeafItem (const GmLeafInfo &, int flags = 0, const vector<char> & extra = vector<char> ());
	void EndAddRoot ();
	GmUifApp (const wxString &);
	void RemoveById (ubyte4 LogId);
	void RemoveBySize (ubyte4 SizeBound);
	wxString GetAppName () { return m_app.GetIndexFileName (); }
	~GmUifApp ();
};

#endif //__GM_UIF_APP_H__