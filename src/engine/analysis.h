//
// executors implementation.
// Author:yeyouqun@163.com
// 2009-11-29
//
#include <engine/filter.h>
#ifndef __GM_MAIN_EXEC_H__
#define __GM_MAIN_EXEC_H__

struct GmCollector;

template <typename NodeType>
inline void GetFileTimesAndAttributes (NodeType * pNode, const wxString & szFullName)
{
	wxFileName FileName (szFullName);
#ifdef _WIN32
	pNode->Attribute = GetFileAttributes (szFullName.c_str ());
#else
	pNode->Attribute = 0;
#endif //
	wxDateTime AccessTime, ModifyTime, CreationTime;
	FileName.GetTimes (&AccessTime, &ModifyTime, &CreationTime);
	pNode->AccessTime = AccessTime.GetValue ().GetValue ();
	pNode->ModifyTime = ModifyTime.GetValue ().GetValue ();
	pNode->CreateTime = CreationTime.GetValue ().GetValue ();
	return;
}

inline bool GetDirectoryNode (GmUifDirEntry & Node, const wxString & szFullName, const wxString & Name)
{
	if (!wxDir ().Open (szFullName))
		return false;

	Node.DirName = ToStlString (Name);
	GetFileTimesAndAttributes (&Node, szFullName + wxT("/"));
	return true;
}

inline bool GetFileNode (GmLeafEntry & Node, const wxString & szFullName, const wxString & Name)
{
	if (!wxFile::Access (szFullName, wxFile::read))
		return false;

	wxFileName FileName (szFullName);
	Node.FileSize = FileName.GetSize ().GetValue ();
	GetFileTimesAndAttributes (&Node, szFullName);
	Node.LeafName = ToStlString (Name);
	return true;
}


enum FileDirFlags {
	FDF_FILE,
	FDF_DIR,
};

/**
 * 分析文件树，及对文件进行操作，
 *
 * @param DirName			需要分析的目录或者文件的全名
 * @param pTree				分析生成树的类型，可以为空，即不需要生成分析树。
 * @param pFilter			文件及目录过滤器，可以为空。
 * @param method			遍历目录的方法，只能是 TraversalMtd 类型。
 * @throw					No Throw;
 */
void ENGINE_EXPORT TraverseTheDirForAnylasis (const wxString & DirName
									 , GmUifSourcePairT * pTree
									 , GmFiltersCollector * pFilter // 过滤器
									 , GmMonitor * m_pMonitor = 0
									 , const TraversalMtd method = TM_DEPTH_FIRST);
/**
 * 处理文件树，及对文件进行操作。具体操作放在 pCollector 中进行操作。
 *
 * @param tree				分析生成树的类型
 * @param collector			文件及目录收集器，同时处理错误及警告。
 * @param method			遍历目录的方法，只能是 TraversalMtd 类型。
 * @throw					No Throw, 执行完成的结点都在 Flags 里添加了 NODE_COLLECTED_FLAG
 *							，在添加到记录文件时，这个标求需要清除。
 */
void ENGINE_EXPORT TraverseTheDirForCollector (GmUifSourcePairT & tree
									 , GmCollector & collector
									 , const TraversalMtd method = TM_DEPTH_FIRST);
void ENGINE_EXPORT TraverseTheDirForCollector (GmUifSourceVectorT & tree
									 , GmCollector & collector
									 , const TraversalMtd method = TM_DEPTH_FIRST);

/**
 * 删除没有完成的文件及目录结点，并恢复结点的原有的标志设置，应该与
	 void ENGINE_EXPORT TraverseTheDirForCollector (GmUifSourcePairT & tree
										 , GmCollector & collector
										 , const TraversalMtd method = TM_DEPTH_FIRST);
   接口一起使用。
 *
 * @param tree				分析生成树的类型
 */
void RemoveNodeNotFinished (GmUifSourcePairT & tree);
void RemoveNodeNotFinished (GmUifSourceVectorT & trees);

/**
 * 备份分析，将两个树进行分析，保留时间有变动及新添加的结点。
 *
 * @param OriTree			根据文件系统目录分析生成树的类型
 * @param RefTree			参照树，根据最新时间点生成的参考树。
 * @throw					No Throw;
 */
void AnalysisLatestTree (GmUifSourceVectorT & OriTree, const GmUifSourceVectorT & RefTree);

#endif //