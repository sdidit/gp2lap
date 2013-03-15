#include "stdinc.h"
#include "prflog.h"

/*
The perf buffer		car pos, angle, etc. should also be logged during replay!
*/

BYTE opt_log_perf = FALSE;

static GP2LapRecPerf	*perfbuf;
static WORD				numperfs;
static BYTE				*perfsavebuf;

#define perf_buflen		3000
#define PerfBufSize()	((long)(numperfs*sizeof(GP2LapRecPerf)))


BOOL PrfLog_Create()
{
	if (opt_log_perf) {
		perfbuf = calloc(perf_buflen, sizeof(GP2LapRecPerf));
		if (!perfbuf) {
			fprintf(stderr, "*** GP2Lap: error allocating perf data buffer\n");
			PrfLog_Kill();
			return FALSE;
		}
		numperfs = 0;
		perfsavebuf = calloc(GP2_EXTPERF_MAXLEN, 1);
		if (!perfsavebuf) {
			fprintf(stderr, "*** GP2Lap: error allocating perf data save buffer\n");
			PrfLog_Kill();
			return FALSE;
		}
	}
	return TRUE;
}

void PrfLog_Kill()
{
	numperfs = 0;
	if (perfsavebuf) {
		free(perfsavebuf);
		perfsavebuf = NULL;
	}
	if (perfbuf) {
		free(perfbuf);
		perfbuf = NULL;
	}
}

void PrfLog_Reset()
{
	numperfs = 0;
}


GP2LapRecPerf *PrfLog_GetPerfRecBuf()
{
	void *recpos = NULL;
	if (opt_log_perf && perfbuf) {
		recpos = (void*) &perfbuf[numperfs];
		numperfs++;
		// bounds checks
		if (numperfs >= perf_buflen)
			numperfs--;
	}
	return recpos;
}

BYTE *PrfLog_GetPerfData(long *lenp)
{
	*lenp = PerfBufSize();
	return (BYTE *) perfbuf;
}

BYTE *PrfLog_GetPerfSaveBuf()
{
	return perfsavebuf;
}


