//
//
// Gimu uif application base definition, use uif as its implementation.
// Authour:yeyouqun@163.com
// 2010-1-10
//
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/log.h>

GmUifApp::GmUifApp (const wxString & szUifAppFile) : m_app (szUifAppFile, 0)
{
}

void GmUifApp::AddNewRootSource (const wxString & szItem, ubyte8 time)
{
	m_app.CreateRootEntry (GRET_TYPE_FULL, ST_TYPE_NONE, TM_DEPTH_FIRST, time);
	m_app.AddUifSource (szItem, GST_DIRECTORY);
}

void GmUifApp::AddSubDirectory (const wxString & SubName, ubyte8 time)
{
	GmLeafInfo info;
	info.Name = SubName;
	info.CreateTime = time;
	m_app.AddUifDir (info);
}

void GmUifApp::EndSubDirectory ()
{
	m_app.EndUifDir ();
}

void GmUifApp::AddNewLeafItem (const GmLeafInfo & item, int flags, const vector<char> & extra)
{
	m_app.AddUifLeaf (item, 0, extra);
}

void GmUifApp::EndAddRoot ()
{
	m_app.EndUifDir ();
	m_app.EndRootEntry ();
}

struct GmSetDeleteFlagHandler : public GmNodeHandler
{
	ubyte4 LogId;
	GmSetDeleteFlagHandler (ubyte4 LogId) : LogId (LogId) {}
	virtual bool HandleNode (GmSnapNode * pNode, const string & UNUSED (path))
	{
		if (pNode->pLeafNode->Attribute == LogId)
			pNode->pLeafNode->Attribute = REMOVED_ITEM;
		return true;
	}
};

static bool IsRemovedID (GmSnapNode * pNode)
{
	if (pNode->pLeafNode->Attribute == REMOVED_ITEM) {
		delete pNode;
		return true;
	}

	return false;
}

struct GmUifAppHanldeFileType {};

void TraverseFileNode (vector<GmSnapNode*> & vpSubFiles
					   , GmNodeHandler * pHandler
					   , const string & path
					   , GmUifAppHanldeFileType hft)
{
	for (size_t index = 0; index < vpSubFiles.size (); ++index) {
		if (pHandler != 0)
			pHandler->HandleNode (vpSubFiles[index], path);
	}

	vector<GmSnapNode*>::iterator pos = std::remove_if (vpSubFiles.begin (), vpSubFiles.end (), IsRemovedID);
	vpSubFiles.erase (pos, vpSubFiles.end ());
	return;
}

void GmUifApp::RemoveById (ubyte4 LogId)
{
	wxString tmpfile = GetTmpFile (GetAppName ());
	try {
		GmUnitedIndexFile uif (tmpfile, 0);
		const vector<GmUifRootEntry*> & roots = m_app.GetAllRootEntries ();
		GmSetDeleteFlagHandler handler (LogId);
		for (size_t index = 0; index < roots.size (); ++index) {
			GmUifRootPairT tree;
			GmAutoClearRootPairTree act (tree);
			m_app.GetUifRootTree (*roots[index], tree);
			for (size_t tindex = 0; tindex < tree.second->size (); ++tindex) {
				GmUifSourcePairT * pNode = (*tree.second)[tindex];
				TraverseTree (pNode->second, &handler, pNode->first->SourceName, GmUifAppHanldeFileType ());
			}

			ClearEmptyRootTreePair (tree);
			const GmUifRootEntry & entry = *tree.first;
			AddTheseTreeToUifFile (*tree.second, uif, (GmRootEntryType)entry.EntryType
									, entry.EntryDataType, entry.TraverseMtd, entry.EntryTime);
		}

		uif.Close ();
		m_app.Close ();
		if (wxRemoveFile (GetAppName ()))
			wxRenameFile (tmpfile, GetAppName ());
	}
	catch (...) {
		wxRemoveFile (tmpfile);
		throw;
	}
}

void GmUifApp::RemoveBySize (ubyte4 SizeBound)
{
	const vector<GmUifRootEntry*> & roots = m_app.GetAllRootEntries ();
	if (roots.empty ())
		return;

	ubyte4 TotalSize = 0;
	size_t index = index = roots.size ();
	for (; index > 0; --index) {
		vector<GmUifSourceEntry*> vpSources;
		GmAutoClearVector<GmUifSourceEntry> acvs (vpSources);
		m_app.GetAllUifSource (*roots[index - 1], vpSources);
		for (size_t sindex = 0; sindex < vpSources.size (); ++sindex)
			for (size_t bindex = 0; bindex < vpSources[sindex]->SourceBlocks.size (); ++bindex)
				TotalSize += vpSources[sindex]->SourceBlocks[bindex];

		if (TotalSize >= SizeBound)
			break;
	}

	for (size_t rindex = 0; rindex < index; ++rindex) {
		vector<GmDataSetEntry*> vpDataSets;
		GmAutoClearVector<GmDataSetEntry> acvs (vpDataSets);
		m_app.RemoveUifRootEntry (rindex, vpDataSets);
	}

	m_app.Tidy ();
}

GmUifApp::~GmUifApp ()
{
	EndAddRoot ();
}

void GmUifApp::GetAllItems (GmNodeHandler & handler)
{
	const vector<GmUifRootEntry*> & roots = m_app.GetAllRootEntries ();
	for (size_t index = 0; index < roots.size (); ++index) {
		GmUifRootPairT tree;
		GmAutoClearRootPairTree act (tree);
		m_app.GetUifRootTree (*roots[index], tree);
		for (size_t tindex = 0; tindex < tree.second->size (); ++tindex) {
			GmUifSourcePairT * pNode = (*tree.second)[tindex];
			TraverseTree (pNode->second, &handler, pNode->first->SourceName, GmCommHandleFileType ());
		}
	}
}

void GmUifApp::TidyByNumber (size_t number)
{
	const vector<GmUifRootEntry*> & roots = m_app.GetAllRootEntries ();
	if (roots.size () <= number) return;
	int ToDelete = (int)(roots.size ()) - number;

	for (int i = 0; i < ToDelete; ++i) {
		vector<GmDataSetEntry*> vpDataSets;
		GmAutoClearVector<GmDataSetEntry> acvs (vpDataSets);
		m_app.RemoveUifRootEntry (i, vpDataSets);
	}

	m_app.Tidy ();
	return;
}