//
//
// Gimu defs
// Authour:yeyouqun@163.com
// 2009-11-8
//
//
#ifndef __GM_DEFS_H__
#define __GM_DEFS_H__
#define		MAX_BLOCK_SIZE					(16 * 1024 * 1024)

typedef unsigned int		ubyte4;
typedef signed int			byte4;
typedef unsigned short int	ubyte2;
typedef signed short int	byte2;
typedef unsigned __int64	ubyte8;
typedef signed __int64		byte8;
typedef unsigned long		ulong;
typedef ubyte4				dword;
typedef ubyte2				word;
#ifdef _WIN32
	#if defined(_M_X64) || defined(_WIN64)
		#define SIZET_TO_UBYTE4(value) static_cast<ubyte4>((value))
	#else
		#define SIZET_TO_UBYTE4(value) (value)
	#endif //
#else
#error Not support platform now!!!
#endif //

#ifdef _WIN32
typedef wchar_t				char_t;
#endif //

#define dtoc(d,n) ((d >> n) & 0xff)
#define ctod(d,c,n) ((d = c, d &= 0xff) << n)


inline ubyte4 UByte4ToChars (char * pData, const ubyte4 Data)
{
	assert (pData);
	pData[0] = dtoc (Data, 0);
	pData[1] = dtoc (Data, 8);
	pData[2] = dtoc (Data, 16);
	pData[3] = dtoc (Data, 24);
	return sizeof (ubyte4);
}

inline ubyte4 UByte2ToChars (char * pData, const ubyte2 Data)
{
	assert (pData);
	pData[0] = dtoc (Data, 0);
	pData[1] = dtoc (Data, 8);
	return sizeof (ubyte2);
}

inline ubyte4 UByte8ToChars (char * pData, const ubyte8 Data)
{
	assert (pData != 0);
	pData[0] = dtoc (Data, 0);
	pData[1] = dtoc (Data, 8);
	pData[2] = dtoc (Data, 16);
	pData[3] = dtoc (Data, 24);
	pData[4] = dtoc (Data, 32);
	pData[5] = dtoc (Data, 40);
	pData[6] = dtoc (Data, 48);
	pData[7] = dtoc (Data, 56);
	return sizeof (ubyte8);
}

inline ubyte4 Byte4ToChars (char * pData, const byte4 Data)
{
	return UByte4ToChars (pData, (ubyte4)Data);
}

inline ubyte4 Byte2ToChars (char * pData, const byte2 Data)
{
	return UByte2ToChars (pData, (ubyte2)Data);
}

inline ubyte4 Byte8ToChars (char * pData, const byte8 Data)
{
	return UByte8ToChars (pData, (ubyte8)Data);
}

inline ubyte4 CharsToUByte8 (const char * pData, ubyte8 * pDest)
{
	assert (pData != 0 && pDest != 0);
	*pDest = 0;
	ubyte8 Data;
	*pDest |= ctod (Data, pData[0], 0);
	*pDest |= ctod (Data, pData[1], 8);
	*pDest |= ctod (Data, pData[2], 16);
	*pDest |= ctod (Data, pData[3], 24);
	*pDest |= ctod (Data, pData[4], 32);
	*pDest |= ctod (Data, pData[5], 40);
	*pDest |= ctod (Data, pData[6], 48);
	*pDest |= ctod (Data, pData[7], 56);
	return sizeof (ubyte8);
}

inline ubyte4 CharsToUByte4 (const char * pData, ubyte4 * pDest)
{
	assert (pData != 0 && pDest != 0);
	*pDest = 0;
	ubyte4 Data;
	*pDest |= ctod (Data, pData[0], 0);
	*pDest |= ctod (Data, pData[1], 8);
	*pDest |= ctod (Data, pData[2], 16);
	*pDest |= ctod (Data, pData[3], 24);
	return sizeof (ubyte4);
}

inline ubyte4 CharsToUByte2 (const char * pData, ubyte2 * pDest)
{
	assert (pData != 0 && pDest != 0);
	*pDest = 0;
	ubyte2 Data;
	*pDest |= ctod (Data, pData[0], 0);
	*pDest |= ctod (Data, pData[1], 8);
	return sizeof (ubyte2);
}

inline ubyte4 CharsToByte8 (const char * pData, byte8 * pDest)
{
	return CharsToUByte8 (pData, (ubyte8*)pDest);
}

inline ubyte4 CharsToByte4 (const char * pData, byte4 * pDest)
{
	return CharsToUByte4 (pData, (ubyte4*)pDest);
}

inline ubyte4 CharsToByte2 (const char * pData, byte2 * pDest)
{
	return CharsToUByte2 (pData, (ubyte2*)pDest);
}

struct ReleaseMemory
{
	template <typename ReleaseType>
	void operator () (ReleaseType * pType)
	{
		delete pType;
	}
};

#define DECL_NO_COPYASSIGN_CONSTRUCTOR(class_name) \
	private: \
	class_name (const class_name &); \
	class_name & operator = (const class_name &);

#define UNUSED(param)

#define UNKOWN_EXCEPTION_STR() wxString::Format (_("IDS_UNKNOWN_EXCEPTION"),__FILE__,__LINE__)

struct ENGINE_EXPORT GmDirectoryInfo
{
	GmDirectoryInfo () : Files (0), Dirs (0), Size (0), CompressSize (0) {}
	ubyte4 Files;
	ubyte4 Dirs;
	ubyte8 Size;
	ubyte8 CompressSize;
	GmDirectoryInfo & operator += (const GmDirectoryInfo & info)
	{
		Files += info.Files;
		Dirs += info.Dirs;
		Size += info.Size;
		CompressSize += info.CompressSize;
		return *this;
	}
};

#define READ_BLOCK_LEN	(5*1024*1024)

#define SINGLETON_DECL(type)			\
	private:							\
	static type * m_pInstance;			\
	public:								\
	static type * GetInstance ();		\
	static void DeleteInstance ()

#define SIGNLETON_IMPL(type)			\
	type * type::m_pInstance = 0;		\
	type * type::GetInstance ()			\
	{									\
		if (m_pInstance == 0) {			\
			m_pInstance = new type ();	\
		}								\
		return m_pInstance;				\
	}									\
										\
	void type::DeleteInstance ()		\
	{									\
		if (m_pInstance != 0) {			\
			delete m_pInstance;			\
			m_pInstance = 0;			\
		}								\
	}									\
	typedef int		_____________gm____
	 //just for semi-colon endding.

#define TimeTToWxDateTime(timet) (time_t)((timet)/1000)

#include <engine/classbase.h>

#define STORE_DATA_TO			0
#define GET_DATA_BACK			1

//
// 当设置下面的标志（在 GmUifDirEntry 或者 GmLeafEntry 的 Flags 标志中，这个标志，用来做分析或者其他
// 扩展功能时使用。
//
#define NODE_COLLECTED_FLAG 0x80000000
//
// 这个标志用于分析最新时间树（用于确定执行任务需要处理的文件树）时，标志哪些文件在源树（原来处理过的文件所
// 生成的一棵树）存在，而在当前分析树中不存在。这样有利于应用这个标志来做进一步的处理，如同步时，确定要不要
// 同步被删除的文件，需要用到这个标志。
//
#define NODE_IN_TARGET_TREE 0x40000000

#endif //__GM_DEFS_H__
