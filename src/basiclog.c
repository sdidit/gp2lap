#ifndef _BASICLOG_H
#define _BASICLOG_H

#include "stdinc.h"
#include "typedefs.h"
#include "basiclog.h"


char strbuf[MAXLOGSTRSIZE];

static FILE *lfh = NULL;
static unsigned char log_flags = 0;
static char log_name[_MAX_PATH];

void LogEnd()
{
	if (lfh) {
		fclose(lfh);
		lfh = NULL;
	}
}

void LogFlush()
{
	if (lfh) {
		fflush(lfh);

// Doesn't seem to work:
//		fclose(lfh);
//		lfh = fopen(log_name, "a+");
	}
}

unsigned char LogStart(unsigned char flags, char *lfname)
{
	LogEnd();
	VisLogStart();
	log_flags = flags;
	if (isoff(log_flags, BLF_DISABLED)) {
		memset(strbuf, 0, sizeof(strbuf));
		strncpy(log_name, lfname, _MAX_PATH-1);
		log_name[_MAX_PATH-1] = 0;
		lfh = fopen(log_name, "a+");
		return (lfh != NULL);
	}
        return 1 /*TRUE*/;
}

void LogLine(char *str)
{
	VisLogLine(str);	// even when Logfile disabled
	if (lfh) {
		fputs(str, lfh);
		if (ison(log_flags, BLF_FLUSHALWAYS))
			LogFlush();
	}
}


#endif
