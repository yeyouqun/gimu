//
// app xml config file, 
// Author:yeyouqun@163.com
// 2010-5-16
// Format:
// <entry>value</entry>
// <serial_no>xxxxxx</serial_no>
// <language>xxxxxx</language>
// ....
//

#ifndef __GM_APP_CONFIG_H__
#define __GM_APP_CONFIG_H__

enum GmLangID
{
	LANGUAGE_SIMPLIFIED_CHINESE,
	LANGUAGE_TRADITIONAL_CHINESE,
	LANGUAGE_ENGLISH,
};

class GmAppConfig
{
	wxString		m_szCfgFile;
	wxXmlDocument	m_ConfigDoc;
	wxXmlNode *		m_pRoot;
	void Save ();
	wxXmlNode * AddOrSetXmlNode (wxXmlNode & parent, const wxString & content, const wxString & type);
	~GmAppConfig ();
#ifdef ENTERPRISE_EDITION
	wxString GetHardwareUser ();
#endif
	void SetBoolValue (bool, const wxString &);
	bool GetBoolValue (const wxString &);
public:
	//
	// 打开系统配置文件。
	//
	void OpenConfig (const wxString & file);
	wxString GetConfigFile () { return m_szCfgFile; }
	//
	// 取得使用过的目标目录
	//
	bool AddUsedDirectory (const wxString & dir);
	void GetUsedDirectory (wxArrayString & dirs);

	//
	// 默认文件类型。
	//
	wxString GetDefaultType ();
	void SetDefaultType (const wxString & type);

	//
	// 语言选项，
	//
	GmLangID GetLangID ();
	void SetLangID (GmLangID id);
	//
	// 取得应用程序的序列号
	//
	void SetSerialNo (const wxString & serial);
	bool CheckSerialNo ();
	bool CheckSerialNo (const wxString &);

	//
	// 设置数据取回模式。
	//
	void SetSnapMode (bool);
	bool GetSnapMode ();
	//
	// 最小化到任务栏
	//
	void SetMinimizeToTaskBar (bool);
	bool GetMinimizeToTaskBar ();

	//
	// 随系统一起起动。
	//
	void SetStartWithSystem (bool);
	bool GetStartWithSystem ();
	//
	// 日志报表限额。
	//
	int GetLogReportLimit ();
	void SetLogReportLimit (int value);
	SINGLETON_DECL(GmAppConfig);
};

#endif //__GM_APP_CONFIG_H__