#ifndef CRC32_H
#define CRC32_H

/* $Id: crc32.h,v 1.6 2005/11/07 11:15:09 gleixner Exp $ */

//#include <stdint.h>

extern const unsigned long crc32_table[256];

/* Return a 32-bit CRC of the contents of the buffer. */

static inline unsigned long
crc32(unsigned long val, const void *ss, int len)
{
	const unsigned char *s = (unsigned char*)ss;
        while (--len >= 0)
                val = crc32_table[(val ^ *s++) & 0xff] ^ (val >> 8);
        return val;
}


#endif
