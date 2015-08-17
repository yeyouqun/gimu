//
// Zip restore node definition
// Author:yeyouqun@163.com
// 2009-11-13
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/writer.h>
#include <engine/execitem.h>
#include <engine/uifblock.h>
#include <engine/execunit.h>
#include <engine/uifalgo.h>
#include <engine/exception.h>
#include <engine/encdeflate.h>
#include <engine/zipblock.h>
#include <engine/uniindex.h>
#include <engine/uifblock.h>
#include <engine/event.h>
#include <engine/zipcrc.h>
#include <engine/reader.h>
#include <engine/restorenode.h>
#include <engine/zipfile.h>
#include <engine/ziprestorenode.h>

static inline wxString getSourcePath (GmWriter * pWriter, GmUifSourceEntry * pEntry, const wxString & szDestPath)
{
	wxString path = ToWxString (pEntry->absolutePath);
	return path;
}

static inline wxString getAbsolutePath (GmWriter * pWriter
									, GmUifSourceEntry * pEntry
									, const wxString & szDestPath)
{
	return ToWxString (pEntry->absolutePath);
}

static GmWriter * createWriter (GmZipFileReader * pSetReader
									, GmUifSourceEntry * pEntry
									, GmRestOp option
									, GmExecUnitBase * pExecUnit
									, GmTempEvent * pEvent)
{
	GmWriter * pWriter = 0;
	wxString destPath;

	//pWriter = createWriter (FST_DIR_SOURCE, pExecUnit, option);
	destPath = ToWxString (pEntry->absolutePath);
	destPath =GetDirPath (destPath);

	assert (pWriter != 0);
	pWriter->SetDestPath (destPath);

	return pWriter;	
}

GmFileEntryNode::GmFileEntryNode (GmLeafNode * pFileNode
								, GmUifSourceEntry * pSource
								, GmRestoreNode * pParent)
								: _pParent (pParent)
								, _pSource (pSource)
								, _pFileNode (pFileNode)
{
	assert (pFileNode);
	assert (pParent);
	assert (pSource);
	_prefixPath = _pParent->getName ();
}

int GmFileEntryNode::getType (void)
{
	return GNT_FILE;
}

GmDirInfo GmFileEntryNode::getDirectoryInfo () const
{
	GmDirInfo subInfo;
	memset (&subInfo, 0, sizeof (GmDirInfo));
	subInfo.totalFiles = 1;
	subInfo.totalSize += _pFileNode->originalSize;
	return subInfo;
}

GmDirEntryNode::GmDirEntryNode (GmDirectoryNode<GmLeafNode> * pDirNode
								, GmUifSourceEntry * pSource
								, GmRestoreNode * pParent)
								: _pParent (pParent)
								, _pSource (pSource)
								, _pDirNode (pDirNode)
{
	assert (pDirNode);
	assert (pParent);
	assert (pSource);

	int iNodeType = _pParent->getType ();

	if (iNodeType == GNT_DIR)
		_prefixPath = Combine (((GmDirEntryNode*)_pParent)->_prefixPath, _pParent->getName ());
}

int GmDirEntryNode::getType (void)
{
	return GNT_DIR;
}

GmDirInfo GmDirEntryNode::getDirectoryInfo () const
{
	GmDirInfo subInfo;
	memset (&subInfo, 0, sizeof (GmDirInfo));

	getZipSubInfo (_pDirNode, subInfo);

	return subInfo;
}

GmDirEntryNode::~GmDirEntryNode ()
{
	for_each (_vSubNodes.begin (), _vSubNodes.end (), ReleaseMemory<GmRestoreNode> ());
	_vSubNodes.clear ();
}
                                    
void GmDirEntryNode::expand (vector<GmRestoreNode*> * & subNodes)
{
	if (_vSubNodes.size () == 0)
		getSubNode (_vSubNodes, _pSource, this, _pDirNode);

	subNodes = &_vSubNodes;
}

void GmFileEntryNode::restore (GmZipFileReader * pSetReader
								, GmWriter * pWriter
								, GmRestOp option
								, GmExecUnitBase * pExecUnit) const
{
	GmRestoreNode * pParent = const_cast<GmFileEntryNode*> (this);

	while (pParent->getParent () != 0)
		pParent = pParent->getParent ();

	GmRootEntryNode * pEntryNode = (GmRootEntryNode*)pParent;
	pSetReader->setFile (pEntryNode->_sZipName);

	pSetReader->restoreFile (pEntryNode->_pEntryNode->totalSets, _pFileNode, pWriter, _prefixPath, 0);
}

GmDirInfo GmSrcEntryNode::getDirectoryInfo () const
{
	GmDirInfo subInfo;
	memset (&subInfo, 0, sizeof (GmDirInfo));

	getZipSubInfo (_pEntryNode, subInfo);

	return subInfo;
}

GmSrcEntryNode::~GmSrcEntryNode ()
{
	for_each (_vSubNodes.begin (), _vSubNodes.end (), ReleaseMemory<GmRestoreNode > ());
	_vSubNodes.clear ();
}

void GmSrcEntryNode::expand (vector<GmRestoreNode*> * & subNodes)
{
	if (_vSubNodes.size () == 0)
		getSubNode (_vSubNodes, _pSource, this, _pEntryNode);

	subNodes = &_vSubNodes;
}

GmRootEntryNode::GmRootEntryNode (GmUifRootEntry * pNode, GmUnitedIndexFile * pCatalog)
									: _pEntryNode (pNode)
									, _pCatalog (pCatalog)
{
	assert (pNode);
	assert (pCatalog);

	_pCatalog->getTailEntry (*_pEntryNode, _tail);

	vector<GmEntryBase *> & DataSets = _tail.DataSets;
	assert (DataSets.size () == 1);

	_sZipName = ToWxString (((GmDataSetEntry *)DataSets[0])->fileName);
}

wxString GmRootEntryNode::getName (void)
{
	wxDateTime date ((time_t)_pEntryNode->backupTime);
	return date.FormatISODate () + _("-") + date.FormatISOTime ();
}

wxString GmRestFileNode::getName (void)
{
	if (_pParent->getType () != GNT_ROOT) {
		wxString  fileName = ToWxString (_pFileNode->fileName);
		fileName = GetFileName (fileName);
		return fileName;
	}

	return ToWxString (_pFileNode->fileName);
}

GmRestZipDirNode::~GmRestZipDirNode ()
{
	for_each (_vSubNodes.begin (), _vSubNodes.end (), ReleaseMemory<GmRestoreNode> ());
	_vSubNodes.clear ();
}

wxString GmRestZipDirNode::getName (void)
{
	string fileName = GetFileName (_pDirectory->dirName);
	if (fileName[fileName.length () - 1] == '/')
		fileName[fileName.length () - 1] = 0;

	return ToWxString (fileName);
}

GmDirInfo GmRestZipDirNode::getDirectoryInfo () const
{
	GmDirInfo subInfo;
	getZipSubInfo (_pDirectory, subInfo);
	return subInfo;
}


GmDirInfo GmRestFileNode::getDirectoryInfo () const
{
	GmDirInfo subInfo;

	++subInfo.totalFiles;
	subInfo.totalSize = getOriginalSize (_pFileNode);
	return subInfo;
}


void restoreDirectory (GmZipFileReader * pSetReader
							, const wxString & szDestPath
							, GmWriter * pWriter
							, GmRestOp option
							, GmExecUnitBase * pExecUnit
							, GmTempEvent * pEvent
							, GmUifSourceEntry * pEntry
							, GmDirectoryNode<GmLeafNode> * pDirectory
							, ubyte4 uiTotalSets
							, const wxString & prefixPath)
{
	if ((option & GRP_NEW_PLACE) == GRP_NEW_PLACE) {
		assert (pWriter);
		wxString path = getSourcePath (pWriter, pEntry, szDestPath);
		pWriter->SetDestPath (path);

		pSetReader->restoreFile (pWriter, pDirectory, prefixPath, uiTotalSets, pEvent);
	}
	else {  // !GRP_NEW_PLACE
		assert (pWriter == 0);
		GmWriter * pWriter2 = createWriter (pSetReader, pEntry, option, pExecUnit, 0);

		if (pWriter2 == 0) {
			wxString msg;
			throw GmException (msg);
		}

		pSetReader->restoreFile (pWriter2, pDirectory, prefixPath, uiTotalSets, 0);
		delete pWriter2;
	}

	return;
}

void GmDirEntryNode::restore (GmZipFileReader * pSetReader
								, GmWriter * pWriter
								, GmRestOp option
								, GmExecUnitBase * pExecUnit) const
{
	GmUifSourceEntry * pEntry = _pSource;
	setZipName (pSetReader, const_cast<GmDirEntryNode*> (this));
	GmRootEntryNode * pCatalogEntry = getRoot (this);
	wxString szDestPath;
	restoreDirectory (pSetReader, szDestPath, pWriter, option, pExecUnit, 0
					, pEntry, _pDirNode, pCatalogEntry->_pEntryNode->totalSets, _prefixPath);

	return;
}

void GmSrcEntryNode::restore (GmZipFileReader * pSetReader
								, GmWriter * pWriter
								, GmRestOp option
								, GmExecUnitBase * pExecUnit) const
{
	GmUifSourceEntry * pEntry = _pSource;
	setZipName (pSetReader, const_cast<GmSrcEntryNode*> (this));
	GmRootEntryNode * pCatalogEntry = (GmRootEntryNode*)getParent ();
	wxString szDestPath;
	restoreDirectory (pSetReader
					, szDestPath
					, pWriter
					, option
					, pExecUnit
					, 0
					, pEntry
					, _pEntryNode
					, pCatalogEntry->_pEntryNode->totalSets
					, _prefixPath);
}

wxString GmSrcEntryNode::getName (void)
{
	ubyte2 iSourceType = _pSource->sourceType;
	wxString szName;

	if (iSourceType == (ubyte2)FST_DIR_SOURCE)
		szName = ToWxString (_pSource->absolutePath);
	else if (iSourceType == (ubyte2)FST_FILE_SOURCE) {
		wxString path = ToWxString (_pSource->absolutePath);
		wxString file = ToWxString (_pSource->sourceFileName);
		szName = Combine (path, file);
	}
	else
		assert (false);

	return szName;
}
	
int GmSrcEntryNode::getType (void)
{
	ubyte2 iSourceType = _pSource->sourceType;
	int iType = 0;
	if (iSourceType == (ubyte2)FST_DIR_SOURCE)
		iType = GNT_SOURCE_DIR;
	else if (iSourceType == (ubyte2)FST_FILE_SOURCE)
		iType = GNT_SOURCE_FILE;
	else
		assert (false);

	return iType;
}

void getSubNode (vector<GmRestoreNode* > & vpSubNodes
						, GmUifSourceEntry * pSource
						, GmRestoreNode * pParent
						, GmDirectoryNode<GmLeafNode> * pDirectory)
{
	typedef vector<GmDirectoryNode<GmLeafNode> *>::iterator diterator;
	for (diterator begin = pDirectory->vpSubDirs.begin (); begin != pDirectory->vpSubDirs.end (); ++begin)
	{
		GmDirEntryNode * pNode = new GmDirEntryNode (*begin, pSource, pParent);
		vpSubNodes.push_back (pNode);
	}

	typedef vector<GmLeafNode *>::iterator iterator;
	for (iterator begin = pDirectory->vpLeaviesNode.begin (); begin != pDirectory->vpLeaviesNode.end (); ++begin)
	{
		GmFileEntryNode * pNode = new GmFileEntryNode (*begin, pSource, pParent);
		vpSubNodes.push_back (pNode);
	}

	return;
}

void GmRootEntryNode::expand (vector<GmRestoreNode*> * & subNodes)
{
	if (_vpRestoreNodes.size () == 0) {
		subNodes = 0;
		assert (_vEntries.size () == 0);
		_pCatalog->getRestoreTree (*_pEntryNode, _vEntries);

		vector<GmEntryBase *> & sources = _tail.sources;

		typedef vector<GmDirectoryNode<GmLeafNode> *>::iterator iterator;
		int i = 0;
		for (iterator begin = _vEntries.begin (); begin != _vEntries.end (); ++begin, ++i)
		{
			GmUifSourceEntry * pSource = (GmUifSourceEntry*)sources[i];
			GmRestoreNode * pSourceNode 
				= new GmSrcEntryNode (*begin, pSource, this);				
			assert (pSourceNode != 0);
			_vpRestoreNodes.push_back (pSourceNode);
		}
	}

	subNodes = &_vpRestoreNodes;
}

GmDirInfo GmRootEntryNode::getDirectoryInfo () const
{
	assert (_vEntries.size () != 0);

	GmDirInfo subInfo;
	memset (&subInfo, 0, sizeof (GmDirInfo));

	typedef vector<GmDirectoryNode<GmLeafNode> *>::const_iterator iterator;
	for (iterator begin = _vEntries.begin (); begin != _vEntries.end (); ++begin)
	{
		getZipSubInfo (*begin, subInfo);
	}

	return subInfo;
}

GmRootEntryNode::~GmRootEntryNode ()
{
	for_each (_vEntries.begin (), _vEntries.end (), ReleaseMemory<GmDirectoryNode<GmLeafNode> > ());
	_vEntries.clear ();
	for_each (_vpRestoreNodes.begin (), _vpRestoreNodes.end (), ReleaseMemory<GmRestoreNode > ());
	_vpRestoreNodes.clear ();
}

void GmRootEntryNode::restore (GmZipFileReader * pSetReader
								, GmWriter * pWriter
								, GmRestOp option
								, GmExecUnitBase * pExecUnit) const
{
	pSetReader->setFile (_sZipName);
	const vector<GmEntryBase *> & sources = _tail.sources;
	if (_vEntries.size () == 0)
		_pCatalog->getRestoreTree (*_pEntryNode, *const_cast<vector<GmDirectoryNode<GmLeafNode> *> *> (&_vEntries));

#ifdef _DEBUG
	unsigned int size = (unsigned int) sources.size ();
#endif

	typedef vector<GmDirectoryNode<GmLeafNode> *>::const_iterator diterator;
	
	vector<GmEntryBase *>::const_iterator first, end = sources.end ();
	assert (_vEntries.size () == sources.size ());

	diterator begin = _vEntries.begin ();

	for (first = sources.begin (); first != end; ++first, ++begin) {
		GmUifSourceEntry * pEntry = (GmUifSourceEntry *)(*first);
		wxString szDestPath;
		restoreDirectory (pSetReader, szDestPath, pWriter, option, pExecUnit, 0
						, pEntry, *begin, _pEntryNode->totalSets, wxString ());
	}

	return;
}

void setZipName (GmZipFileReader * pZipReader, GmRestoreNode * pNode)
{
	assert (pNode != 0);

	while (pNode->getParent () != 0)
		pNode = pNode->getParent ();

	GmRootEntryNode * pEntry = (GmRootEntryNode *)pNode;
	pZipReader->setFile (pEntry->_sZipName);
	return;
}

void GmRestZipDirNode::expand (vector<GmRestoreNode*> * & subNodes)
{
	if (_vSubNodes.size () == 0) {
		typedef vector<GmZipDirectoryNode*>::const_iterator iterator;
		GmZipDirectoryNode & directory = *_pDirectory;

		for (iterator begin = directory.vSubDirs.begin (); begin != directory.vSubDirs.end (); ++begin)
		{
			GmRestZipDirNode * pNode = new GmRestZipDirNode (*begin, this);
			_vSubNodes.push_back (pNode);
		}

		typedef vector<GmCDSFileHdr *>::const_iterator fiterator;
		for (fiterator begin = directory.vSubFiles.begin (); begin != directory.vSubFiles.end (); ++begin)
		{
			GmRestFileNode * pNode = new GmRestFileNode (*begin, this);
			_vSubNodes.push_back (pNode);
		}
	}

	subNodes = &_vSubNodes;
}

void GmZipRootNode::expand (vector<GmRestoreNode*> * & subNodes)
{
	if (_vSubNodes.empty ()) {
		assert (_pTree);
		typedef ZipTreeType::iterator iterator;
		for (iterator begin = _pTree->begin (); begin != _pTree->end (); ++begin)
		{
			GmRestoreNode * pNode = 0;
			if ((*begin)->bIsFile) {
				GmRestFileNode * pFileNode = new GmRestFileNode ((*begin)->pFileNode, this);
				pNode = pFileNode;
			}
			else {
				GmRestZipDirNode * pDirectoryNode = new GmRestZipDirNode ((GmZipDirectoryNode*)(*begin), this);
				pNode = pDirectoryNode;
			}

			_vSubNodes.push_back (pNode);
		}
	}

	subNodes = &_vSubNodes;
}

void GmZipRootNode::restore (GmZipFileReader * pSetReader
							, GmWriter * pWriter
							, GmRestOp option
							, GmExecUnitBase * pExecUnit) const
{
	vector<GmRestoreNode*> * vSubNodes = 0;
	if (_vSubNodes.empty ())
		const_cast<GmZipRootNode*> (this)->expand (vSubNodes);

	for (size_t i = 0; i < _vSubNodes.size (); ++i)
		_vSubNodes[i]->restore (pSetReader, pWriter, option, pExecUnit);
}


GmDirInfo GmZipRootNode::getDirectoryInfo () const
{
	GmDirInfo subInfo;
	vector<GmRestoreNode*> * vSubNodes = 0;
	if (_vSubNodes.empty ())
		const_cast<GmZipRootNode*> (this)->expand (vSubNodes);

	for (size_t i = 0; i < _vSubNodes.size (); ++i)
		subInfo += _vSubNodes[i]->getDirectoryInfo ();

	return subInfo;
}

GmReader * createReader (GmRootEntryNode * pEntryNode
									 , GmExecUnitBase * pExecUnit
									 , GmTempEvent * pEvent)
{
	GmReader * pReader = 0;
	const vector<GmEntryBase *> & backupsets = pEntryNode->_tail.DataSets;
	assert (backupsets.size () == 1);

	GmDataSetEntry & setEntry = * (GmDataSetEntry*)backupsets[0];

	ubyte2 uiDestType = setEntry.destType;
	if (uiDestType == 	(ubyte2)DST_LOCAL) {
		pReader = new GmLocalReader (ToWxString (setEntry.absPath));
	}
	else
		assert (false);

	return pReader;
}
