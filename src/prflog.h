#ifndef PRFLOG_H
#define PRFLOG_H


///////////////////////////////////////////////////////////////////////////// //
// Binary logging
//

#include "gp2def.h"

#pragma pack(1)

#define GP2LOG_PERF_VERSION		7


typedef struct GP2LAPRECPERF
{
	long	pos_x;
	long	pos_y;
	BYTE	wheel_speed[GP2_NUM_WHEELS]; // extra 7 bits wheel speed precision
} GP2LapRecPerf;




// Options
extern BYTE opt_log_perf;

BOOL	PrfLog_Create();
void	PrfLog_Kill();
void	PrfLog_Reset();
GP2LapRecPerf *PrfLog_GetPerfRecBuf();
BYTE	*PrfLog_GetPerfData(long *lenp);
BYTE	*PrfLog_GetPerfSaveBuf();


/////////////////////////////////////////////////////////////////////

#pragma pack()
#endif
