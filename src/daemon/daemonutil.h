//
// daemon's implementation, daemon server as a service.
// Author:yeyouqun@163.com
// 2010-7-19
//

#ifndef __GM_DAEMON_UTIL_H__
#define __GM_DAEMON_UTIL_H__
bool IsGimuAppStart ();

class GmJobItem;
struct DaemonContext
{
	DaemonContext ();
	~DaemonContext ();
	vector<GmJobItem*>	Jobs;
	bool				bNotStop;
};

void ReadJobs ();

#endif //__GM_DAEMON_UTIL_H__