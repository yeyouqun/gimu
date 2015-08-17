//
// collector's base class.
// Author:yeyouqun@163.com
// 2010-4-14
//
#ifndef __GM_COLLECTOR_H__
#define __GM_COLLECTOR_H__

struct GmMonitor;
struct GmUifSourceEntry;
struct GmUifDirectory;
struct GmSnapNode;

struct ENGINE_EXPORT GmCollector
{
	GmCollector (GmMonitor * pMonitor, bool bNeedTellSize = false);
	virtual ~GmCollector () = 0;
	//
	// 处理关于数据源绝对路径，防止出现多个数据源同目录同文件的错误。这里需要好好处理。
	//
	virtual void OnSource (const GmUifSourceEntry & entry) = 0;
	/**
	 * 收集目录的信息
	 * @param FullName				文件的全路径
	 * @param szRelaPath			处理文件时所使用的相对路径，不包含文件名。
	 * @return						返回空。
	 */
	virtual bool OnDir (const wxString & FullName, const wxString & szRelaPath, const GmUifDirectory * pDir) = 0;
	/**
	 * 收集文件的信息
	 * @param path					文件的全路径
	 * @param szRelaPath			处理文件时所使用的相对路径，不包含文件名。
	 * @return						返回空。
	 */
	virtual bool OnFile (const wxString & FullName, const wxString & szRelaPath, const GmSnapNode * pLeafNode) = 0;
	/**
	 * 收集信息的过程序中，出现了错误。
	 * @param Message				错误消息。
	 * @return						返回空。
	 */
	void OnError (const wxString & Message);
	/**
	 * 收集信息的过程序中，出现了警告。
	 * @param Warn					警告消息。
	 * @return						返回空。
	 */
	void OnWarn (const wxString & Warn);
	/**
	 * 收集信息的过程序中，出现了警告。
	 * @param message				消息字串。
	 * @return						返回空。
	 */
	void OnMessage (const wxString & message);
protected:
	/**
	 * open the szFileName file and handle the data by HanldeThisBlock;
	 */
	void HandleFile (const wxString & szFileName);
	bool NeedTellSize () const { return m_bTellSize; }
private:
	virtual void HanldeThisBlock (const char * pData, ubyte4 Bytes, bool bEndFile) = 0;
	GmMonitor * m_pMonitor;
	ACE_Message_Block m_block;
	bool		m_bTellSize;// 是否在处理文件时，上报处理数据块的长度，在 ZIP 备份时，与 Copy 或者 Sync 时由于
							// 在 Writer 中也上报了一次，导致会错误的完成率。
};

//
// 这个接口以数据源完整路径为参数来计算 CRC32 校验码，并以此校验码为目录名来存存放这个数据源
// 下的文件。该方法用在复制类型的作业中。
//
wxString GetCrc32String (const char *pData, unsigned length);

#endif //__GM_COLLECTOR_H__