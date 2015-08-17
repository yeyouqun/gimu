//
// Uif algo header
// Author:yeyouqun@163.com
// 2009-11-8
//

#ifndef __GM_UIF_ALGO_H__
#define __GM_UIF_ALGO_H__

struct GmSnapNode
{
	GmSnapNode () : Time (0)
					, pLeafNode (0)
	{
	}
	ubyte8					Time;
	GmLeafEntry *			pLeafNode;
	~GmSnapNode () { if (pLeafNode) delete pLeafNode; }
};

struct GmUifDirectory
{
	GmUifDirectory () : pMySelf (0), bOnlyClearDir (false) {}
	virtual ~GmUifDirectory ()
	{
		for_each (vpSubDirs.begin (), vpSubDirs.end (), ReleaseMemory ());
		if (!bOnlyClearDir) {
			//
			// 在只清楚目录的情况下，目录的 pMySelf 就其他树的对象，因此不需要删除。
			//
			if (pMySelf != 0) delete pMySelf;
			for_each (vpSubFiles.begin (), vpSubFiles.end (), ReleaseMemory ());
		}
	}
	void AddLeafEntry (GmLeafEntry * pNode, ubyte8 EntryTime)
	{
		GmSnapNode * pLeafNode = new GmSnapNode;
		pLeafNode->Time = EntryTime;
		pLeafNode->pLeafNode = pNode;
		vpSubFiles.push_back (pLeafNode);
	}
	void AddDirNode (GmUifDirectory * pNode)
	{
		vpSubDirs.push_back (pNode);
	}

	GmUifDirEntry *					pMySelf;
	vector<GmUifDirectory *>		vpSubDirs;
	vector<GmSnapNode *>			vpSubFiles;
	bool							bOnlyClearDir;
};

typedef std::pair<GmUifSourceEntry*, GmUifDirectory*>		GmUifSourcePairT;
typedef std::vector<GmUifSourcePairT*>						GmUifSourceVectorT;
typedef std::pair<GmUifRootEntry *, GmUifSourceVectorT*>	GmUifRootPairT;

void ENGINE_EXPORT ClearUifSourceTree (GmUifSourcePairT * pSourceTree);
void ENGINE_EXPORT ClearUifTree (GmUifSourceVectorT * pUifTree);
void ENGINE_EXPORT ClearRootTree (GmUifRootPairT * pTree);

struct GmAutoClearSourceVectorTree
{
	GmUifSourceVectorT & tree;
	GmAutoClearSourceVectorTree (GmUifSourceVectorT & tree) : tree (tree) {}
	~GmAutoClearSourceVectorTree ()
	{
		ClearUifTree (&tree);
	}
};


struct GmAutoClearRootPairTree
{
	GmUifRootPairT & tree;
	GmAutoClearRootPairTree (GmUifRootPairT & tree) : tree (tree) {}
	~GmAutoClearRootPairTree ()
	{
		ClearRootTree (&tree);
	}
};

struct GmNodeHandler
{
	virtual bool HandleNode (GmUifDirectory *, const string & = string ()) { return true; }
	virtual bool HandleNode (GmSnapNode *, const string & = string ()) { return true; }
	virtual ~GmNodeHandler () {}
};

struct GmCommHandleFileType {};

//
//  所有的物化 TraverseType 类型都使用这个实现，理论上可以是任何类型，但为了防止
//  这例化不必要的版本而增加代码量，因此限于使用预定义的几种类型，对于特化版本，则用
//  户直接实现相应的版本即可。
//

inline void TraverseFileNode (vector<GmSnapNode*> & vpSubFiles
					   , GmNodeHandler * pHandler
					   , const string & path
					   , GmCommHandleFileType chft)
{
	for (size_t index = 0; index < vpSubFiles.size (); ++index) {
		if (pHandler != 0)
			pHandler->HandleNode (vpSubFiles[index], path);
	}
}

template <typename HandleFileT>
void TraverseTree (GmUifDirectory * pNode, GmNodeHandler * pHandler, const string & path, HandleFileT hft)
{
	if (pHandler != 0 && !(pHandler->HandleNode (pNode, path))) return;

	vector<GmUifDirectory*> & vpSubDirs = pNode->vpSubDirs;
	for (size_t index = 0; index < vpSubDirs.size (); ++index) {
		string NextPath;
		if (vpSubDirs[index]->pMySelf != 0) NextPath = AssemblePath (path, vpSubDirs[index]->pMySelf->DirName);
		else NextPath = path;
		TraverseTree (vpSubDirs[index], pHandler, NextPath, hft);
	}

	TraverseFileNode (pNode->vpSubFiles, pHandler, path, hft);
}

#endif //__GM_UIF_ALGO_H__