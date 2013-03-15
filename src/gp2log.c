#include "stdinc.h"
#include "gp2log.h"
#include "basiclog.h"
#include "misc.h"
#ifdef AUTH
#include "auth.h"
#endif

//----- Binary logging -----


/*
- the checksum buffer	checks on physics and track, etc.
- the session buffer	game mode, frame rate, options, driver names, etc.
- the hotlap buffer		nothing yet...
- the lap buffer		lap times, grip, pitstops, retirements...
*/


typedef struct GP2LOGENV
{
// for racing and hotlapping
	GP2LapRecInit		initrec;									 // static
	GP2LapRecTrack		trackrec;									 // static
	GP2LapRecSession	sessionrec;									 // static
// for racing
	BYTE				*lapbuf;									// dynamic
	BYTE				*lapbufpos;
	BYTE				*cclapbuf;									// dynamic
	BYTE				*cclapbufpos;
} GP2LogEnv;

static GP2LogEnv logenv;


#define lap_bufsize 	10000
#define cclap_bufsize	(lap_bufsize)*GP2_NUM_CARS_RACE

BYTE opt_log_race =			TRUE;
BYTE opt_log_glx =			FALSE;
BYTE opt_log_cc =			FALSE;	// only works if opt_log_glx is true also
	



static void *Log_Alloc(DWORD num, DWORD sz)
{
	void *buf = calloc(num, sz);
	if (!buf)
		fprintf(stderr, "*** GP2Lap: error allocating %d bytes of memory\n", num*sz);
	return buf;
}



BOOL Log_Create()
{
	memset(&logenv, 0, sizeof(GP2LogEnv));
	logenv.initrec.hdr.type = GP2LRTYPE_INIT;
	logenv.trackrec.hdr.type = GP2LRTYPE_TRACK;
	logenv.sessionrec.hdr.type = GP2LRTYPE_SESSION;

	// allocate lap buffer
	logenv.lapbuf = Log_Alloc(lap_bufsize, 1);
	if (!logenv.lapbuf) { Log_Kill(); return FALSE; }
	logenv.lapbufpos = logenv.lapbuf;
	if (opt_log_cc) {
		logenv.cclapbuf = Log_Alloc(cclap_bufsize, 1);
		if (!logenv.cclapbuf) { Log_Kill(); return FALSE; }
		logenv.cclapbufpos = logenv.cclapbuf;
	}
	
#ifdef AUTH
	if (!Auth_Init())
		return FALSE;
#endif
	return TRUE;
}


void Log_Kill()
{
#ifdef AUTH
	Auth_Kill();
#endif

	if (logenv.lapbuf) {
		free(logenv.lapbuf);
		logenv.lapbuf = NULL;
	}
	logenv.lapbufpos = NULL;
	if (logenv.cclapbuf) {
		free(logenv.cclapbuf);
		logenv.cclapbuf = NULL;
	}
	logenv.cclapbufpos = NULL;
}


void Log_Reset()
{
	logenv.lapbufpos = logenv.lapbuf;
	logenv.cclapbufpos = logenv.cclapbuf;
}


void Log_Encrypt(BYTE *data, long len, BYTE *csum)
{
#ifdef AUTH
	Auth_Encrypt(data, len, csum);
#else
	memset(csum, 0, 4);
#endif
}



long Log_GetRecSize(int rectype)
{
	switch (rectype) {
		case GP2LRTYPE_LAP:		return sizeof(GP2LapRecLap);
		case GP2LRTYPE_PITOUT:	return sizeof(GP2LapRecPitOut);
		case GP2LRTYPE_RETIRE:	return sizeof(GP2LapRecRetire);
		case GP2LRTYPE_TRACK:	return sizeof(GP2LapRecTrack);
		case GP2LRTYPE_SESSION:	return sizeof(GP2LapRecSession);
		case GP2LRTYPE_INIT:	return sizeof(GP2LapRecInit);
	}
	return 0;
}


BYTE *Log_GetInitData(long *lenp)
{
	BYTE *dat;

	dat = (BYTE *) &logenv.initrec;
	*lenp = sizeof(GP2LapRecInit) +
			sizeof(GP2LapRecTrack) +
			sizeof(GP2LapRecSession);
	return dat;
}



GP2LapRecLap *Log_FindLap(DWORD lapnr)
{
	BYTE *lap = NULL;
	BYTE *bufptr = logenv.lapbuf;
	int type;
	long sz;

#ifdef TEST
	sprintf(strbuf, "Finding lap %d. Buffer contains", lapnr-1);
#endif
	while (bufptr < logenv.lapbufpos &&
		   (type = ((GP2LapRecHdr*) bufptr)->type) != GP2LRTYPE_INVALID &&
		   (sz = Log_GetRecSize(type)) > 0
		  )	{
		if (type == GP2LRTYPE_LAP) {
			BYTE buflapnr = ((GP2LapRecLap*)bufptr)->nr;
#ifdef TEST
			char tmp[8];
			sprintf(tmp, " %d", buflapnr-1);
			strcat(strbuf, tmp);
#endif
			if (buflapnr == (BYTE) lapnr)
				lap = bufptr;
			// Find the last one, so keep on searching
		}
		bufptr += sz;
	}
#ifdef TEST
	strcat(strbuf, "\n");
	LogLine(strbuf);
#endif
	return (GP2LapRecLap *) lap;
}




BYTE *Log_GetLapData(long *lenp)
{
	*lenp = (long) (logenv.lapbufpos - logenv.lapbuf);
	return logenv.lapbuf;
}

BYTE *Log_GetCCLapData(long *lenp)
{
	*lenp = (long) (logenv.cclapbufpos - logenv.cclapbuf);
	return logenv.cclapbuf;
}



static void *Log_GetLapRecBuf(int rectype)
{
	if (opt_log_cc) {
		void *recpos = logenv.cclapbufpos;
		int recsize = Log_GetRecSize(rectype);
		if (recsize > 0) {
			logenv.cclapbufpos += recsize;
			// bounds check. TODO: realloc
			if ((logenv.cclapbufpos-logenv.cclapbuf) > cclap_bufsize)
				logenv.cclapbufpos = logenv.cclapbuf;
			return recpos;
		}
	} else {
		void *recpos = logenv.lapbufpos;
		int recsize = Log_GetRecSize(rectype);
		if (recsize > 0) {
			logenv.lapbufpos += recsize;
			// bounds check. TODO: realloc
			if ((logenv.lapbufpos-logenv.lapbuf) > lap_bufsize)
				logenv.lapbufpos = logenv.lapbuf;
			return recpos;
		}
	}
	return NULL;
}


void Log_CopyLapData(const BYTE *buf, int len)
{
	logenv.lapbufpos = logenv.lapbuf;
	if (len > 0 && len <= lap_bufsize) {
		memcpy(logenv.lapbuf, buf, len);
		logenv.lapbufpos += len;
	}
	if (opt_log_cc) {
		logenv.cclapbufpos = logenv.cclapbuf;
		if (len > 0 && len <= cclap_bufsize) {
			memcpy(logenv.cclapbuf, buf, len);
			logenv.cclapbufpos += len;
		}
	}
}


void *Log_GetRecBuf(int rectype)
{
	void *recpos = NULL;

	switch (rectype) {
		case GP2LRTYPE_INIT:	recpos = (void*) &logenv.initrec; break;
		case GP2LRTYPE_TRACK:	recpos = (void*) &logenv.trackrec; break;
		case GP2LRTYPE_SESSION:	recpos = (void*) &logenv.sessionrec; break;
		case GP2LRTYPE_LAP:
		case GP2LRTYPE_PITOUT:
		case GP2LRTYPE_RETIRE:	recpos = Log_GetLapRecBuf(rectype); break;
	}

	if (recpos)
		((GP2LapRecHdr*) recpos)->type = rectype;
	return recpos;
}

void Log_AddPlayerRec(void *rec, int rectype)
{
	if (rec && opt_log_cc) {
		void *recpos = logenv.lapbufpos;
		int recsize = Log_GetRecSize(rectype);
		if (recsize > 0) {
			logenv.lapbufpos += recsize;
			if ((logenv.lapbufpos-logenv.lapbuf) > lap_bufsize)
				logenv.lapbufpos = logenv.lapbuf;
			memcpy(recpos, rec, recsize);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////


void GetLogTime(GP2LogTimeStamp *t)
{
	struct dostime_t dt;
	struct dosdate_t dd;
	_dos_gettime(&dt);
	_dos_getdate(&dd);
	t->year = (BYTE) (dd.year-1980);
	t->month = dd.month;
	t->day = dd.day;
	t->hour = dt.hour;
	t->minute = dt.minute;
	t->second = dt.second;
}
