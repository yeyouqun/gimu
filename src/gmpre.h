//
// Pre-include header file.
// Author:yeyouqun@163.com
// 2009-11-8
//

#ifndef __GM_PRE_H__
#define __GM_PRE_H__

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#ifdef GM_USE_ACE
#include <ace/config.h>
#include <ace/ACE.h>
#include <ace/Message_Block.h>
#include <ace/Lock_Adapter_T.h>
#include <ace/Process_Mutex.h>
#endif //

#ifdef GM_USE_WXWIDGETS
#include <wx/wx.h>
#include <wx/strconv.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/event.h>
#define DECLARE_GM_EVENT_TYPE(name) extern const wxEventType name;
#endif //

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>

using std::auto_ptr;
using std::for_each;
using std::vector;
using std::string;
using std::pair;
using std::list;

#ifndef MAKE_ENGINE_DLL
#define ENGINE_EXPORT __declspec(dllimport)
#else
#define ENGINE_EXPORT __declspec(dllexport)
#endif//

#define MAJOR_VERSION 2
#define MINOR_VERSION 1
#define REVISION_VERSION 0

#define CHECK_INTERVAL				5
#define GimuDaemonService			wxT ("GimuDaemonService")
#define GimuDaemonExe				wxT ("daemon.exe")
#define GimuJobScheduleDispersion	2

#include <buildnum.h>
#include <Shellapi.h>



template <int val>
struct Int2Type
{
};

#endif //__GM_PRE_H__