//
// executors implementation.
// Author:yeyouqun@163.com
// 2009-11-29
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/util.h>
#include <engine/uifalgo.h>
#include <engine/analysis.h>
#include <engine/zipfile.h>
#include <engine/uiffile.h>
#include <engine/exception.h>
#include <engine/collector.h>

static inline GmUifDirectory * GetNextDirectory (GmUifDirectory * pDirectory
															 , const wxString & FullPath
															 , const wxString & DirName)
{
	auto_ptr<GmUifDirectory > NextDirectory;
	if (pDirectory != 0) {
		NextDirectory.reset (new GmUifDirectory);
		NextDirectory->pMySelf = new GmUifDirEntry;
		if (GetDirectoryNode (*(NextDirectory->pMySelf), FullPath, DirName))
			pDirectory->AddDirNode (NextDirectory.get ());
		else
			return (GmUifDirectory*)0;
	}

	return NextDirectory.release ();
}
//
// 会可能有异常出来吗？
//
void TraverseTheDirForAnylasis (const wxString & DirName
								 , const wxString & szRelaPath
								 , GmUifDirectory * pDirectory
								 , GmFiltersCollector * pFilter
								 , GmMonitor * pMonitor
								 , ubyte8 TimeNow
								 , const TraversalMtd method)
{
	wxDir dir (DirName);
	vector<wxString> vSubDirs;

	wxString DirOrFile;
	bool bEnumerateOK = dir.GetFirst (&DirOrFile);
	while (bEnumerateOK) {
		wxString szFullPath = AssemblePath (DirName, DirOrFile);
		if (IsFile (szFullPath)) {
			if (pFilter == 0 || pFilter->ReserveFile (DirName, DirOrFile)) {
				if (pDirectory != 0) {
					auto_ptr<GmLeafEntry> node (new GmLeafEntry);
					if (GetFileNode (*node.get (), szFullPath, GetFileName (szFullPath)))
						pDirectory->AddLeafEntry (node.release (), TimeNow);
				}
			}

			if (pMonitor != 0 && pMonitor->IsStopped ()) throw GmStopProcedureException ();
		}
		else {
			if (pFilter == 0 || pFilter->ReserveDir (DirName, DirOrFile)) {
				GmUifDirectory * pNextDirectory = GetNextDirectory (pDirectory, szFullPath, DirOrFile);
				if (pNextDirectory != 0) {
					if (method == TM_DEPTH_FIRST) {
						wxString szNextPath = AssemblePath (szRelaPath, DirOrFile);
						TraverseTheDirForAnylasis (szFullPath
													, szNextPath
													, pNextDirectory
													, pFilter
													, pMonitor
													, TimeNow
													, method);
					}
					else if (method == TM_WIDTH_FIRST) {
						vSubDirs.push_back (DirOrFile);
					}
				}
			}
		}

		bEnumerateOK = dir.GetNext (&DirOrFile);
	};

	if (method == TM_WIDTH_FIRST) {
		for (size_t dindex = 0; dindex < vSubDirs.size (); ++dindex) {
			const wxString & SubDirName = vSubDirs[dindex];
			wxString szFullPath = AssemblePath (DirName, SubDirName);
			GmUifDirectory * pNextDirectory = GetNextDirectory (pDirectory, szFullPath, SubDirName);

			if (pNextDirectory != 0) {
				wxString szNextPath = AssemblePath (szRelaPath, SubDirName);
				TraverseTheDirForAnylasis (szFullPath
											, szNextPath
											, pNextDirectory
											, pFilter
											, pMonitor
											, TimeNow
											, method);
			}
		}
	}
}

void TraverseTheDirForAnylasis (const wxString & DirName
								 , GmUifSourcePairT * pTree
								 , GmFiltersCollector * pFilter // 过滤器
								 , GmMonitor * pMonitor
								 , const TraversalMtd method/* = TM_DEPTH_FIRST*/)
{
	if (!wxIsAbsolutePath (DirName))
		return;

	bool bNeedCreateTree = pTree != 0 ? true : false;

	auto_ptr<GmUifSourceEntry> source;
	auto_ptr<GmUifDirectory > dir;

	wxDateTime now;
	now.SetToCurrent ();
	ubyte8 TimeNow = now.GetValue ().GetValue ();

	if (wxDir::Exists (DirName)) {
		if (bNeedCreateTree) {
			source.reset (new GmUifSourceEntry);
			source->SourceName = ToStlString (DirName);
			source->SourceType = GST_DIRECTORY;
			dir.reset (new GmUifDirectory);
		}

		wxString FileName = GetFileName (DirName);
		TraverseTheDirForAnylasis (DirName, FileName, dir.get (), pFilter, pMonitor, TimeNow, method);
	}
	else if (wxFile::Exists (DirName)) {
		if (bNeedCreateTree) {
			source.reset (new GmUifSourceEntry);
			source->SourceName = ToStlString (DirName);
			source->SourceType = GST_FILE;
			dir.reset (new GmUifDirectory);
			auto_ptr<GmLeafEntry> node (new GmLeafEntry);
			if (GetFileNode (*node.get (), DirName, GetFileName (DirName)))
				dir->AddLeafEntry (node.release (), TimeNow);
		}
	}

	if (bNeedCreateTree) {
		pTree->first = source.release ();
		pTree->second = dir.release ();
	}

	return;
}

///////////////////////////////////////////////////////////////
//
// 会可能有异常出来吗？
//
void TraverseTheDirForCollector (const wxString & DirName
								 , const wxString & szRelaPath
								 , GmUifDirectory * pDirectory
								 , GmCollector & collector
								 , const TraversalMtd method)
{
	//
	//  凡是处理成功的都在结点的 Flags 中设置 NODE_COLLECTED_FLAG 位，然后再试用相应结点处理相应在结点。
	//
	vector<GmUifDirectory*> vpSubDirs;
	for (size_t dindex = 0; dindex < pDirectory->vpSubDirs.size (); ++dindex) {
		GmUifDirectory * pNextDirectory = pDirectory->vpSubDirs[dindex];
		wxString szNextPath = AssemblePath (szRelaPath, ToWxString (pNextDirectory->pMySelf->DirName));
		wxString szFullPath = AssemblePath (DirName, ToWxString (pNextDirectory->pMySelf->DirName));

		if (method == TM_DEPTH_FIRST) {
			if (collector.OnDir (szFullPath, szNextPath, pNextDirectory)) {
				pNextDirectory->pMySelf->Flags |= NODE_COLLECTED_FLAG;
				TraverseTheDirForCollector (szFullPath, szNextPath, pNextDirectory, collector, method);
			}
			else {
				//
				// 如果没有成功创建最上层目录，后面的操作都将失败。
				//
				wxString message = wxString::Format (_("IDS_CANT_CREATE_DEST_FILE")
													, szFullPath.c_str ()
													, GetSystemError ().c_str ());
				collector.OnMessage (message);
			}
		}
		else {
			vpSubDirs.push_back (pNextDirectory);
		}
	}

	for (size_t findex = 0; findex < pDirectory->vpSubFiles.size (); ++findex) {
		GmSnapNode * pLeafNode = pDirectory->vpSubFiles[findex];
		wxString szFullPath = AssemblePath (DirName, ToWxString (pLeafNode->pLeafNode->LeafName));
		if (collector.OnFile (szFullPath, szRelaPath, pLeafNode)) {
			pLeafNode->pLeafNode->Flags |= NODE_COLLECTED_FLAG;
		}
		else {
			//
			// 如果没有成功创建最上层目录，后面的操作都将失败。
			//
			wxString message = wxString::Format (_("IDS_CANT_CREATE_DEST_FILE")
												, szFullPath.c_str ()
												, GetSystemError ().c_str ());
			collector.OnMessage (message);
		}
	}

	if (method != TM_WIDTH_FIRST)
		return;

	for (size_t subindex = 0; subindex < vpSubDirs.size (); ++subindex) {
		GmUifDirectory * pNextDirectory = pDirectory->vpSubDirs[subindex];
		wxString szNextPath = AssemblePath (szRelaPath, ToWxString (pNextDirectory->pMySelf->DirName));
		wxString szFullPath = AssemblePath (DirName, ToWxString (pNextDirectory->pMySelf->DirName));

		if (collector.OnDir (szFullPath, szNextPath, pNextDirectory)) {
			pNextDirectory->pMySelf->Flags |= NODE_COLLECTED_FLAG;
			TraverseTheDirForCollector (szFullPath, szNextPath, pNextDirectory, collector, method);
		}
		else {
			wxString message = wxString::Format (_("IDS_CANT_CREATE_DEST_FILE")
												, szFullPath.c_str ()
												, GetSystemError ().c_str ());
			collector.OnMessage (message);
		}
	}
}


void TraverseTheDirForCollector (GmUifSourcePairT & tree
											 , GmCollector & collector
											 , const TraversalMtd method/* = TM_DEPTH_FIRST*/)
{
	const GmUifSourceEntry & source = *tree.first;
	GmUifDirectory * pDirectory = tree.second;
	wxString szSourcePath = ToWxString (source.SourceName);

	if (source.SourceType != GST_FILE) {
		if (!wxDir::Exists (szSourcePath))
			return;

		TraverseTheDirForCollector (szSourcePath, wxT (""), pDirectory, collector, method);
	}
	else {
		if (!wxFile::Exists (szSourcePath)) return;
		assert (pDirectory->vpSubFiles.size () == 1);
		if (collector.OnFile (szSourcePath, wxT (""), pDirectory->vpSubFiles.at (0)))
			pDirectory->vpSubFiles.at (0)->pLeafNode->Flags |= NODE_COLLECTED_FLAG;
	}
}

void TraverseTheDirForCollector (GmUifSourceVectorT & tree
											 , GmCollector & collector
											 , const TraversalMtd method /*= TM_DEPTH_FIRST*/)
{
	for (size_t sindex = 0; sindex < tree.size (); ++sindex) {
		GmUifSourcePairT & sp = *tree[sindex];
		collector.OnSource (*sp.first);
		TraverseTheDirForCollector (sp, collector, method);
	}
}

////////////////////////////////////////////////////////////////////////////////
void RemoveNodeNotFinished (GmUifDirectory * pDirectory)
{
	//
	//  凡是处理成功的都在结点的 Flags 中设置 NODE_COLLECTED_FLAG 位，然后再试用相应结点处理相应在结点。
	//
	for (size_t dindex = 0; dindex < pDirectory->vpSubDirs.size (); ++dindex) {
		GmUifDirectory *& pNextDirectory = pDirectory->vpSubDirs[dindex];
		if ((pNextDirectory->pMySelf->Flags & NODE_COLLECTED_FLAG) == 0) {
			//
			// 没有完成的结点，需要删除。
			//
			delete pNextDirectory; // 会递归删除子目录。
			pNextDirectory = 0;
		}
		else {
			RemoveNodeNotFinished (pNextDirectory);
		}
	}

	vector<GmUifDirectory*>::iterator dpos = std::remove (pDirectory->vpSubDirs.begin ()
																	, pDirectory->vpSubDirs.end ()
																	, (GmUifDirectory*)0);

	pDirectory->vpSubDirs.erase (dpos, pDirectory->vpSubDirs.end ());

	for (size_t findex = 0; findex < pDirectory->vpSubFiles.size (); ++findex) {
		GmSnapNode *& pSnapNode = pDirectory->vpSubFiles[findex];
		if ((pSnapNode->pLeafNode->Flags & NODE_COLLECTED_FLAG) == 0) {
			delete pSnapNode;
			pSnapNode = 0;
		}
	}

	vector<GmSnapNode*>::iterator fpos = std::remove (pDirectory->vpSubFiles.begin ()
													, pDirectory->vpSubFiles.end ()
													, (GmSnapNode*)0);

	pDirectory->vpSubFiles.erase (fpos, pDirectory->vpSubFiles.end ());
}

void RemoveNodeNotFinished (GmUifSourcePairT & tree)
{
	RemoveNodeNotFinished (tree.second);
	ClearEmptyUifTreeDirectory (tree.second->vpSubDirs);
}


void RemoveNodeNotFinished (GmUifSourceVectorT & trees)
{
	for (size_t sindex = 0; sindex < trees.size (); ++sindex) {
		RemoveNodeNotFinished (*trees[sindex]);
	}

	ClearEmtpySourceTree (trees);
}
////////////////////////////////////////////////////////////////////////////////
