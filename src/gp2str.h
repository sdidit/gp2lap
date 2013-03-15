#ifndef GP2STR_H
#define GP2STR_H

#include "typedefs.h"


#define GP2_FPTIMESTR(t)			GP2_MakeFPTimeString(NULL,t)
#define GP2_MakeFPTimeString(b,t)	GP2_MakeTimeString(b,((int)((t)*1000.0)))
#define GP2_TIMESTR(t)				GP2_MakeTimeString(NULL,t)
#define GP2_LONGTIMESTR(t)			GP2_MakeLongTimeString(NULL,t)

/* gp2str.c */
char		*GP2_MakeTimeString(char *buf, DWORD t);
char		*GP2_MakeLongTimeString(char *buf, DWORD t);
const char	*GP2_GetSessionString(long mode);
const char	*GP2_GetShortSessionString(long mode);
const char	*GP2_GetTrackString(long track);
const char	*GP2_GetCountryString(long track);
const char	*GP2_GetFileTypeString(long type);
const char	*GP2_ExeVerString(BYTE ver);
const char	*GP2_GetLevelString(int level);


///////////////////////////////////////////////////////////////////////////////////////////

#endif
