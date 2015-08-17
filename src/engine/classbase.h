//
// common base class declaration and general base class.
// Author:yeyouqun@163.com
// 2010-5-2
//

#ifndef __GM_BASE_CLASS_H__
#define __GM_BASE_CLASS_H__

struct ENGINE_EXPORT GmEntryBase
{
	virtual ubyte4 WriteEntry (char * pBuffer) const = 0;
	virtual ubyte4 GetSize () const = 0;
	virtual ubyte4 ReadEntry (const char * pBuffer) = 0;
	virtual ~GmEntryBase () {}
};

#endif //__GM_BASE_CLASS_H__