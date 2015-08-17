//
// Zip CRC Calculator
// Author:yeyouqun@163.com
// 2009-11-10
//

#include <gmpre.h>
#include <engine/defs.h>
#include <engine/zipcrc.h>

static const ubyte4 magicNumber = 0xEDB88320;

ubyte4 GmZipCRCor::_table[256];

void GmZipCRCor::initTable()
{
	for (ubyte4 i = 0; i < 256; i++)
	{
		ubyte4 r = i;
		
		for (int j = 0; j < 8; j++)
			if (r & 1) 
				r = (r >> 1) ^ magicNumber;
			else     
				r >>= 1;
				
		GmZipCRCor::_table[i] = r;
	}
}

class GmTableInitiator
{
public:
	GmTableInitiator()
	{
		GmZipCRCor::initTable();
	}
}g_zipabCoreInitiator;

void GmZipCRCor::Update (const void *data, ubyte4 size)
{
	ubyte4 v = _value;
	const unsigned char *p = (const unsigned char *)data;
	
	for (; size > 0 ; size--, p++)
		v = _table[((unsigned char)(v)) ^ *p] ^ (v >> 8);
		
	_value = v;
}
