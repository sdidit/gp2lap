#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gp2log.h"
#include "gp2def.h"




// Checksums of default tracks
static DWORD track_def_csum[GP2_NUM_TRACKS] = {
	0xfb84d4a0, 0xf0bd9661, 0x875c0066, 0x25531329,
	0x23c26589, 0xd9779bef, 0x6caa1499, 0x883cafd9,
	0x6dceb94b, 0xe28ed299, 0xbeeef45f, 0x453e95a3,
	0x1b26e5d8, 0xbbc5995d, 0x42cb8229, 0x9f1f4008
};

DWORD GP2_GetDefTrackChecksum(int track)
{
	if (track >=0 && track < GP2_NUM_TRACKS)
		return track_def_csum[track];
	return 0;
}


#define GP2_CRUNCH_LENGTH_MIN	3
#define GP2_CRUNCH_LENGTH_MAX	10

static const BYTE *FindPattern(const BYTE *buf, int buflen, const BYTE *pat, int *patlen)
{
	int maxlen = *patlen, findlen = GP2_CRUNCH_LENGTH_MIN-1, b;
	const BYTE *findloc = NULL;
	const BYTE *bufptr;
	for (bufptr=buf+buflen-1; bufptr>=buf; --bufptr) {
		for (b=0; b<maxlen && bufptr[b] == pat[b]; b++);
		if (b > findlen) {
			findloc = bufptr;
			findlen = b;
			if (findlen == maxlen)
				break;
		}
	}
	if (findlen < GP2_CRUNCH_LENGTH_MIN)
		return NULL;
	*patlen = findlen;
	return findloc;
}


/**
 *	GP2_Crunch	- Crunch data using the GP2 crunch method
 */
int GP2_Crunch(const BYTE *src, BYTE *dest, DWORD src_length)
/*
**	src:		Source buffer with data to be crunched.
**	dest:		Destination buffer to receive crunched data.
**	src_length:	Length of src.
**
**	return:		Destination length.
*/
{
	BYTE *type_mask, *dest_start = dest;
	const BYTE *src_end = src + src_length, *start = src, *begin, *back;
	int n;
	const DWORD *gp2lap_end = (DWORD*)src_end;

	/* Mark as being crunched by me (don't mind the hash code) */
	if (gp2lap_end[-1] == GP2LOGSIG) {
		GP2LapRecInit *init_rec = (GP2LapRecInit*) (src + gp2lap_end[-2]);
		if (init_rec->hdr.type == GP2LRTYPE_INIT)
			init_rec->opt_flags |= LIOF_CRUNCHED;
	}

	while (src < src_end) {
		type_mask = dest;
		*dest++ = 0;
		for (n=0; n<8 && src<src_end; n++) {
			int length = GP2_CRUNCH_LENGTH_MAX;
			if (src_end - src < length)
				length = src_end - src;
			begin = src - 0x2000;
			if (begin < start)
				begin = start;
			back = FindPattern(begin, src-begin, src, &length);
			if (!back)
				*dest++ = *src++;
			else {
				*(WORD*)dest = (src - back - 1) | ((length-3) << 13);
				dest += 2;
				src += length;
				*type_mask |= 1<<n;
				printf("%1d", length-3);
			}
		}
	}
	printf("\n");
	return dest - dest_start;
}


/**
 *	GP2_Decrunch	- Decrunch data using the GP2 decrunch method
 */
int GP2_Decrunch(const BYTE *src, BYTE *dest, DWORD src_length)
/*
**	src:		Source buffer with data to be decrunched.
**	dest:		Destination buffer to receive decrunched data.
**				If NULL, just the length is returned.
**	src_length:	Length of src.
**
**	return:		Destination length.
*/
{
	BYTE type_mask, *back, length;
	WORD os;
	const BYTE* src_end = src + src_length;
	int dest_len = 0, n, i;

	while (src < src_end) {
		type_mask = *src++;
		for (n=0; n<8 && src<src_end; n++) {
			if (type_mask & 1) {
				/* Command */
				if (dest) {
					os = *(WORD*)src;
					src += 2;
					back = dest - (os & 0x1fff) - 1;
					length = (os >> 13) + 3;
					for (i=0; i<length; i++)
						*dest++ = *back++;
					dest_len += length;
				} else {
					src++;
					dest_len += ((*src++) >> 5) + 3;
				}
			} else {
				/* Data */
				if (dest)
					*dest++ = *src;
				src++;
				dest_len++;
			}
			type_mask >>= 1;
		}
	}
	return dest_len;
}



/**
 *	GP2_CalcChecksum	- Calculate checksum using the GP2 checksum method
 */
void GP2_CalcChecksum(const BYTE *src, long src_length, WORD *sum, WORD *cycle)
/*
**	src:		Source buffer from which checksum must be calculated
**	src_length:	Length of src
**
**	sum:		Will receive the sum part of the checksum (must be initialized to zero)
**	cycle:		Will receive the cycle part of the checksum (must be initialized to zero)
*/
{
	BYTE c;
	while (src_length-- > 0) {
		c = *src++;
		*sum += c;
		*cycle = (*cycle << 3) + (*cycle >> 13) + c;
	}
}




// buf: points to (word) search buffer
// sz: size of buffer (in words)
// rle_len: receives the length of the block (in words)
// returns: (word) offset into buf where the rle block begins
static int GetRLEBlock(const WORD *buf, int sz, int *rle_len)
{
	int len = sz;
	int rle = 0;
	int rlen = 0;
	int i = 1;
	// skip data until same words are found
	while (i < len && buf[i] != buf[i-1])
		i++;
	if (i != len) {
		WORD data;
		// found same words
		rle = i - 1;
		rlen = 2;
		data = buf[i++];
		// check for more (although gp3 jams don't go further than 0x7f,
		// length of 0x80 is the maximum)
		while (rlen < 0x80 && i < len && buf[i] == data) {
			rlen++;
			i++;
		}
	}
	*rle_len = rlen;
	return rle;
}


// returns: num bytes copied
static int CopyDataBlocks(BYTE *dst, const WORD *src, int sz)
{
	int d, i;
	if (sz <= 0)
		return 0;
	d = 0;
	for (i=0; i<sz; i++) {
		int b = sz-i;
		if (b > 0x7f)
			b = 0x7f;
		if (dst)
			dst[d] = (BYTE)(0x80 + b);
		d++;
		if (dst)
			memcpy(dst+d, src+i, b * 2);
		d += b * 2;
		i += b;
	}
	return d;
}


/**
 *	GP2_CrunchHWJam	- Crunch hardware jam data
 */
int GP2_CrunchHWJam(const BYTE *src, BYTE *dst, int sz)
/*
**	src:	Points to jam file buffer, except the first 8 bytes
**  dst:	Points to buffer with proper size to be filled. Can be NULL to get size in advance.
**	sz:		Length of src buffer
**	return:	Num bytes encoded.
*/
{
	int s = 0;	// num read
	int d = 0;	// num written
	while (s < sz) {
		// find next rle block
		int rle_len = 0;	// receives rle block length in words
		int rle = GetRLEBlock((WORD*)(src+s), (sz-s)/2, &rle_len);
		if (rle_len > 0) {
			// rle block found
			if (rle > 0) {
				// first write data blocks upto the rle block
				d += CopyDataBlocks(dst ? dst+d : NULL, (WORD*)(src+s), rle);
				s += rle * 2;
			}
			// write rle block: count (byte), value (word)
			if (dst)
				dst[d] = rle_len;
			d++;
			if (dst)
				*(WORD*)(dst+d) = *(WORD*)(src+s);
			d += 2;
			s += rle_len * 2;
		} else {
			// no rle block found anymore, copy blocks until end
			d += CopyDataBlocks(dst ? dst+d : NULL, (WORD*)(src+s), (sz-s)/2);
			s = sz;
		}
	}
	return d;
}


/**
 *	GP2_DecrunchHWJam	- Decrunch hardware jam data
 */
int GP2_DecrunchHWJam(const BYTE *src, BYTE *dst, int sz)
/*
**	src:	Points to jam data
**  dst:	Points to buffer with proper size to be filled. Can be NULL to get size in advance.
**	sz:		Length of src buffer
**	return:	Num bytes decoded.
*/
{
	int written = 0, read = 0, block_size, i;
	BYTE c;
	WORD w, *wdst;

	while (read < sz) {
		c = src[read++];
		if (c > 0x80) {
			c -= 0x80;
			block_size = c * 2;
			if (dst != NULL)
				memcpy(dst + written, src + read, block_size);
			read += block_size;
			written += block_size;
		} else {
			w = *(WORD*)(src + read);
			read += 2;
			if (dst != NULL) {
				wdst = (WORD*)(dst + written);
				for (i=0; i<c; i++)
					*wdst++ = w;
			}
			written += c * 2;
		}
	}
	return written;
}





BOOL GP2_DecompTime(long *h, long *m, long *s, long *ms, DWORD t)
{
	if (!GP2_TIME_ISVALID(t))
		return FALSE;

	if (h) {
		*h = t / 3600000;
		t = t % 3600000;
	}
	if (m) {
		*m = t / 60000;
		t = t % 60000;
	}
	if (s) {
		*s = t / 1000;
		t = t % 1000;
	}
	if (ms)
		*ms = t;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////

static unsigned char arDaysInMonth[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static DWORD DaysInYear(DWORD y)
{
	// Note: modulo 400 not counted!
	return (!(y%4) && ((y%100) /*|| !(y%400)*/ )) ? 366 : 365;
}

static DWORD DaysInMonth(DWORD y, DWORD m)
{
	return m == 2 ? DaysInYear(y)-337 : (int) arDaysInMonth[m-1];
}

void GP2_FromAmigaDate(DWORD adate, DWORD *year, DWORD *month, DWORD *day)
{
	*year = 1978;
	for (;;) {
		DWORD yd = DaysInYear(*year);
		if (adate < yd)
			break;
		(*year)++;
		adate -= yd;
	}
	*month = 1;
	for (;;) {
		DWORD md = DaysInMonth(*year, *month);
		if (adate < md)
			break;
		(*month)++;
		adate -= md;
	}
	*day = adate+1;
}

void GP2_ToAmigaDate(DWORD *adate, DWORD year, DWORD month, DWORD day)
{
	DWORD y = 1978, m = 1;
	*adate = 0;
	while (y != year) {
		*adate += DaysInYear(y);
		y++;
	}
	while (m != month) {
		*adate += DaysInMonth(y, m);
		m++;
	}
	*adate += day-1;
}



WORD GP2_GetRARBValue(int index)
{
	static const WORD RARB_VALUES[] = { 0, 50, 100, 150, 200, 300, 400, 550, 750, 1000, 1250 };
	if (index >= 0 && index < GP2_NUM_ARB_VALUES)
		return RARB_VALUES[index];
	return 0;
}

WORD GP2_GetFARBValue(int index)
{
	static const WORD FARB_VALUES[] = { 0, 500, 1000, 1500, 2000, 3000, 4000, 5500, 7500, 10000, 12500 };
	if (index >= 0 && index < GP2_NUM_ARB_VALUES)
		return FARB_VALUES[index];
	return 0;
}


static const BYTE GP2_DEFAULT_SETUP[] = {
	0x0A, 0x0C, 0x1C, 0x23, 0x2A, 0x30, 0x36, 0x3B,
	0x02, 0x64, 0x00, 0x00, 0x6E, 0x6E, 0x6E, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x0F, 0x0F, 0x15, 0x15,
	0x0F, 0x0F, 0x15, 0x15, 0x64, 0x64, 0x8C, 0x8C,
	0x74, 0x74, 0x40, 0x40, 0x02, 0x00, 0x05, 0x00,
};

const GP2CSx *GP2_GetDefaultSetup()
{
	return (const GP2CSx *) GP2_DEFAULT_SETUP;
}

