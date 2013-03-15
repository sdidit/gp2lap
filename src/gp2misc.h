#ifndef GP2MISC_H
#define GP2MISC_H

#include "typedefs.h"
#include "gp2def.h"


DWORD		GP2_GetDefTrackChecksum(int track);
int			GP2_Crunch(const BYTE *src, BYTE *dest, DWORD src_length);
int			GP2_Decrunch(const BYTE *src, BYTE *dest, DWORD src_length);
int			GP2_CrunchHWJam(const BYTE *src, BYTE *dst, int sz);
int			GP2_DecrunchHWJam(const BYTE *src, BYTE *dst, int sz);
void		GP2_CalcChecksum(const BYTE *src, long src_length, WORD *sum, WORD *cycle);
int			GP2_DecompTime(long *h, long *m, long *s, long *ms, DWORD t);
// Amiga date counts days since 1-1-1978
void		GP2_FromAmigaDate(DWORD adate, DWORD *year, DWORD *month, DWORD *day);
void		GP2_ToAmigaDate(DWORD *adate, DWORD year, DWORD month, DWORD day);
WORD		GP2_GetRARBValue(int index);
WORD		GP2_GetFARBValue(int index);
const GP2CSx *GP2_GetDefaultSetup();


///////////////////////////////////////////////////////////////////////////////////////////

#endif
