
//
// app xml config file, 
// Author:yeyouqun@163.com
// 2010-5-16
//

//
// <config>
// <language>1</language>
// <directories>
//		<directory>c:\xxxxx\yyyyy1</directory>
//		<directory>c:\xxxxx\yyyyy2</directory>
//		<directory>c:\xxxxx\yyyyy3</directory>
//		at most ten.
// </directories>
// <archive_type>.zip</archive_type>
// <snap_mode>1</snap_mode>
// <serial_number>GMjry3zOXhG9YouFLjmSLWuzoeiqsLr/hUl2s/HpMMrGKH1TBRcqPrceb+tPFUwvXHZ+1BvVfWA=</serial_number>
// </config>
//


#include <gmpre.h>
#include "engineinc.h"
#include <engine/register.h>
#include <engine/hardware.h>
#include <wx/xml/xml.h>
#include "appconfig.h"

#define ROOT				wxT("config")
#define USED_DIRECTORIES	wxT("directories")
#define USED_DIRECTORY_NODE wxT("directory")
#define LANGUAGE			wxT("language")
#define ARCHIVE_TYPE		wxT("archive_type")
#define SERIAL_NO			wxT("serial_number")
#define SERIAL_NO_TYPE		wxT("type")
#define SNAP_MODE			wxT("snap_mode")
#define MINIMIZE_TO_BAR		wxT("mini_to_bar")
#define START_WITH_SYSTEM	wxT("start_with_system")
#define LOG_REPORT_LIMIT	wxT("log_report_limit")

void GmAppConfig::Save ()
{
	if (m_ConfigDoc.GetRoot () == NULL)
		return;

	m_ConfigDoc.Save (m_szCfgFile);
}

static wxXmlNode * GetSpecifiedChild (const wxXmlNode & parent, const wxString & name)
{
	wxXmlNode *child = parent.GetChildren();
	while (child) {
		if (child->GetName() == name) return child;
		child = child->GetNext();
	}

	return NULL;
}

static unsigned GetChildrenNumber (const wxXmlNode & parent)
{
	unsigned number = 0;
	wxXmlNode *child = parent.GetChildren();
	while (child) {
		number++;
		child = child->GetNext();
	}

	return number;
}

static wxXmlNode * GetLastChild (const wxXmlNode & parent)
{
	wxXmlNode *child = parent.GetChildren();
	while (child) {
		if (child->GetNext () == NULL) return child;
		child = child->GetNext();
	}

	return NULL;
}


void GmAppConfig::OpenConfig (const wxString & file)
{
	if (!wxFile::Exists (file)) {
		wxFile (file, wxFile::write);
	}

	m_szCfgFile = file;
	m_ConfigDoc.Load (m_szCfgFile);
	m_pRoot = m_ConfigDoc.GetRoot ();
	if (m_pRoot == NULL) {
		//
		// empty xml.
		//
		m_pRoot = new wxXmlNode (NULL, wxXML_ELEMENT_NODE, ROOT);
		m_ConfigDoc.SetRoot (m_pRoot);
	}
}

bool GmAppConfig::AddUsedDirectory (const wxString & dir)
{
	if (m_pRoot == NULL) return false;
	if (dir.IsEmpty ()) return false;

	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, USED_DIRECTORIES);
	if (node == NULL) {
		node = new wxXmlNode (m_pRoot, wxXML_ELEMENT_NODE, USED_DIRECTORIES);
	}

	//
	// 如果已经存在这个结点，不添加。
	//
	wxXmlNode *child = node->GetChildren();
	while (child) {
		if (child->GetNodeContent () == dir) return false;
		child = child->GetNext();
	}

	wxXmlNode * entry = new wxXmlNode (node, wxXML_ELEMENT_NODE, USED_DIRECTORY_NODE);
	wxXmlNode * content = new wxXmlNode(entry, wxXML_TEXT_NODE, wxEmptyString, dir);
	unsigned number = GetChildrenNumber (*node);

	while (number > 10) {
		wxXmlNode * last = GetLastChild (*node);
		if (last && node->RemoveChild (last))
			delete last;

		--number;
	}

	Save ();
	return true;
}

void GmAppConfig::GetUsedDirectory (wxArrayString & dirs)
{
	if (m_pRoot == NULL) return;

	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, USED_DIRECTORIES);
	if (node != NULL) {
		wxXmlNode *child = node->GetChildren();
		while (child) {
			if (child->GetName () == USED_DIRECTORY_NODE) {
				wxString content = child->GetNodeContent ();
				if (!content.IsEmpty ()) dirs.Insert (content, 0);
			}

			child = child->GetNext();
		}
	}
}


wxString GmAppConfig::GetDefaultType ()
{
	if (m_pRoot == NULL) return GetDupTypeStr ();
	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, ARCHIVE_TYPE);
	if (node == NULL)  return GetDupTypeStr ();
	return node->GetNodeContent ();
}

wxXmlNode * GmAppConfig::AddOrSetXmlNode (wxXmlNode & parent, const wxString & content, const wxString & type)
{
	if (m_pRoot == NULL) return 0;

	wxXmlNode * node = GetSpecifiedChild (parent, type);
	if (node == NULL)  {
		node = new wxXmlNode (&parent, wxXML_ELEMENT_NODE, type);
		wxXmlNode * xmlcontent = new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString, content);
		node->SetChildren (xmlcontent);
	}
	else {
		wxXmlNode * child = node->GetChildren ();
		if (child != 0 && child->GetType () == wxXML_TEXT_NODE)
			child->SetContent (content);
	}

	Save ();
	return node;
}

void GmAppConfig::SetDefaultType (const wxString & type)
{
	if (m_pRoot == NULL) return;
	AddOrSetXmlNode (*m_pRoot, type, ARCHIVE_TYPE);
	return;
}


GmLangID GmAppConfig::GetLangID ()
{
	GmLangID deftid = LANGUAGE_ENGLISH;
	long id = wxLocale::GetSystemLanguage ();
	if (id == wxLANGUAGE_CHINESE_SIMPLIFIED)
		deftid = LANGUAGE_SIMPLIFIED_CHINESE;
	else if (id == wxLANGUAGE_CHINESE_TRADITIONAL ||
			id == wxLANGUAGE_CHINESE_TAIWAN  ||
			id == wxLANGUAGE_CHINESE_HONGKONG)
		deftid = LANGUAGE_TRADITIONAL_CHINESE;
	else
		deftid = LANGUAGE_ENGLISH;

	if (m_pRoot == NULL) return deftid;
	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, LANGUAGE);
	if (node == NULL)  return deftid;

	wxString langid = node->GetNodeContent ();
	langid.ToLong (&id);
	if (id < 0 || id > LANGUAGE_ENGLISH) return deftid;
	return (GmLangID)id;
}

void GmAppConfig::SetLangID (GmLangID id)
{
	if (m_pRoot == NULL) return;
	AddOrSetXmlNode (*m_pRoot, wxString::Format (wxT ("%d"), id), LANGUAGE);
	return;
}

void GmAppConfig::SetSerialNo (const wxString & serial)
{
	if (m_pRoot == NULL) return;
	wxXmlNode * node = AddOrSetXmlNode (*m_pRoot, serial, SERIAL_NO);
	if (node == NULL) return;
	Save ();
}

#ifdef ENTERPRISE_EDITION
wxString GmAppConfig::GetHardwareUser ()
{
	GmHardwareInfo * hardware = GmHardwareInfo::GetInstance ();
	return hardware->GetHardDiskSerial (wxT("first disk"));
}
#endif //

bool GmAppConfig::CheckSerialNo (const wxString & serial)
{
#if defined (ENTERPRISE_EDITION)
	return GmRegister::GetInstance ()->CheckMachEigenCodeSN (GetHardwareUser (), serial);
#elif defined (PRO_EDITION)
	return GmRegister::GetInstance ()->CheckRandomSN (serial);
#else
	return true;
#endif //
}

bool GmAppConfig::CheckSerialNo ()
{
	if (m_pRoot == NULL) return false;
	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, SERIAL_NO);
	if (node == NULL)  return false;

	wxString serial = node->GetNodeContent ();
	return 	CheckSerialNo (serial);
}

GmAppConfig::~GmAppConfig ()
{
	Save ();
	GmRegister::DeleteInstance ();
	GmHardwareInfo::DeleteInstance ();
}

void GmAppConfig::SetBoolValue (bool value, const wxString & node)
{
	if (m_pRoot == NULL) return;
	wxString szMode = wxString::Format (wxT ("%d"), static_cast<int> (value));
	AddOrSetXmlNode (*m_pRoot, szMode, node);
	return;
}

bool GmAppConfig::GetBoolValue (const wxString & NodeStr)
{
	if (m_pRoot == NULL) return false;
	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, NodeStr);
	if (node == NULL)  return false;
	long value;
	bool result = node->GetNodeContent ().ToLong (&value);
	return result ? value > 0 ? true : false : false;
}

void GmAppConfig::SetSnapMode (bool mode)
{
	SetBoolValue (mode, SNAP_MODE);
}

bool GmAppConfig::GetSnapMode ()
{
	return GetBoolValue (SNAP_MODE);
}

void GmAppConfig::SetMinimizeToTaskBar (bool value)
{
	SetBoolValue (value, MINIMIZE_TO_BAR);
}

bool GmAppConfig::GetMinimizeToTaskBar ()
{
	return GetBoolValue (MINIMIZE_TO_BAR);
}

//
// 随系统一起起动。
//
void GmAppConfig::SetStartWithSystem (bool value)
{
	SetBoolValue (value, START_WITH_SYSTEM);
}

bool GmAppConfig::GetStartWithSystem ()
{
	return GetBoolValue (START_WITH_SYSTEM);
}

int GmAppConfig::GetLogReportLimit ()
{
	int DefValue = 10;
	if (m_pRoot == NULL) return DefValue;
	wxXmlNode * node = GetSpecifiedChild (*m_pRoot, LOG_REPORT_LIMIT);
	if (node == NULL) return DefValue;
	long value;
	bool result = node->GetNodeContent ().ToLong (&value);
	return result ? (int)value : DefValue;
}

void GmAppConfig::SetLogReportLimit (int value)
{
	if (m_pRoot == NULL) return;
	wxString v = wxString::Format (wxT ("%d"), value);
	wxXmlNode * node = AddOrSetXmlNode (*m_pRoot, v, LOG_REPORT_LIMIT);
	if (node == NULL) return;
	Save ();
}

SIGNLETON_IMPL (GmAppConfig);

