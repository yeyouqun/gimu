//
// 7z collector implementation.
// Author:yeyouqun@163.com
// 2010-4-16
//
#include <gmpre.h>
#include <engine/defs.h>
#include <engine/uifblock.h>
#include <engine/uifalgo.h>
#include <engine/util.h>
#include <engine/analysis.h>
#include <engine/uiffile.h>
#include <engine/exception.h>
#include <engine/7zcollector.h>

Gm7zCollector::Gm7zCollector (GmMonitor * pMonitor)
					: GmCollector (pMonitor)
					, block (READ_BLOCK_LEN)
{
}

Gm7zCollector::~Gm7zCollector ()
{
}

bool Gm7zCollector::OnDir (const wxString & FullName, const wxString & szRelaPath)
{
	return true;
}

bool Gm7zCollector::OnFile (const wxString & FullName, const wxString & szRelaPath)
{
	return true;
}

void Gm7zCollector::OnError (const wxString & Message)
{
}

void Gm7zCollector::OnWarn (const wxString & Warn)
{
}
