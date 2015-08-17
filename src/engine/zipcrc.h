//
// Zip CRC Calculator
// Author:yeyouqun@163.com
// 2009-11-10
//

#ifndef __ZIPCRC_H__
#define __ZIPCRC_H__

class GmZipCRCor
{
	ubyte4 _value;
	static ubyte4 _table[256];
	static void initTable();	
	friend class GmTableInitiator;
public:
	GmZipCRCor ():  _value(0xFFFFFFFF) { ; }
	void Init () { _value = 0xFFFFFFFF; }
	void Update(const void *data, ubyte4 size);
	ubyte4 GetDigest() const { return _value ^ 0xFFFFFFFF; } 
};

#endif //__ZIPCRC_H__
