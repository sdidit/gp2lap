#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gp2def.h"
#include "gp2misc.h"


static const char *type_strings[] =
{
	"Names",
	"Records",
	"Controller config",
	"Car setup",
	"Car setup all tracks",
	"Quick race",
	"Unlimited practice",
	"Non-championship",
	"Championship",
	"Linked quick race",
	"Linked unlimited practice",
	"Linked non-championship game",
	"Linked championship game",
	"Performance analysis",
	"Hotlap",
	"Linked hotlap"
};


#define ENDOFSESSIONMODES	0xff
static BYTE session_modes[] = { 0, 1, 2, 3, 0x40, 0x44, 0x80, ENDOFSESSIONMODES };
static const char *session_strings[] =
{
	"Unlimited practice",
	"Friday (first) practice",
	"Pre-race warmup",
	"Saturday (second) practice",
	"Friday (first) qualify",
	"Saturday (final) qualify",
	"Quick/non-championship/championship race",
	"Unknown session mode"
};

static const char *session_short_strings[] =
{
	"Unlimited practice",
	"Fri practice",
	"Pre-race warmup",
	"Sat practise",
	"Fri qualify",
	"Sat qualify",
	"Race",
	"Unknown"
};

static const char *track_strings[] =
{
	"Interlagos",
	"Aida",
	"Imola",
	"Monte Carlo",
	"Barcelona",
	"Montreal",
	"Magny-Cours",
	"Silverstone",
	"Hockenheim",
	"Hungaroring",
	"Spa-Francorchamps",
	"Monza",
	"Estoril",
	"Jerez",
	"Suzuka",
	"Adelaide"
};

static const char *country_strings[] =
{
	"Brazil",
	"Pacific",
	"San Marino",
	"Monaco",
	"Spain",
	"Canada",
	"France",
	"Great Britain",
	"Germany",
	"Hungary",
	"Belgium",
	"Italy",
	"Portugal",
	"Europe",
	"Japan",
	"Australia"
};


static const char *gp2_exe_ver_strings[GP2_EXE_NUMVERSIONS+1] = {
	"English",
	"French",
	"German",
	"Dutch",
	"Italian",
	"Spanish",
	"American",
	"Unknown"
};


static const char *level_strings[6] = {
	"Rookie", "Amateur", "Semi-Pro", "Pro", "Ace", "Unknown"
};




/**
 *	GP2_GetTrackString	- Get GP2 track name
 */
const char *GP2_GetTrackString(long track)
/*
**	track:	track number
**
**	return:	track string
*/
{
	return (track >= 0 && track < GP2_NUM_TRACKS) ?
		track_strings[track] : "???";
}


/**
 *	GP2_GetCountryString	- Get GP2 country name for track
 */
const char *GP2_GetCountryString(long track)
/*
**	track:	track number
**
**	return:	country string
*/
{
	return (track >= 0 && track < GP2_NUM_TRACKS) ?
		country_strings[track] : "???";
}


/**
 *	GP2_GetFileTypeString	- Get string representation of a GP2 data file type
 */
const char *GP2_GetFileTypeString(long type)
/*
**	type:	file type
**
**	return:	file type string
*/
{
	int file_type = GP2_FILE_TYPE(type);
	if (file_type > sizeof(type_strings) / sizeof(type_strings[0]))
		return "?";

	return type_strings[file_type];
}



/**
 *	GP2_GetSessionString	- Get string representation of a GP2 session mode
 */
const char *GP2_GetSessionString(long mode)
/*
**	mode:	session mode, possible values:
**			0, 1, 2, 3, 0x40, 0x44, 0x80
**
**	return:	session string
*/
{
	int index = 0;
	BYTE m;
	while ((m = session_modes[index]) != mode && m != ENDOFSESSIONMODES)
		index++;
	return session_strings[index];
}



/**
 *	GP2_GetShortSessionString	- Get short string representation of a GP2 session mode
 */
const char *GP2_GetShortSessionString(long mode)
/*
**	mode:	session mode, possible values:
**			0, 1, 2, 3, 0x40, 0x44, 0x80
**
**	return:	session string
*/
{
	int index = 0;
	BYTE m;
	while ((m = session_modes[index]) != mode && m != ENDOFSESSIONMODES)
		index++;
	return session_short_strings[index];
}





/**
 *	GP2_ExeVerString	- Get string representation of the GP2 executable version
 */
const char *GP2_ExeVerString(BYTE ver)
/*
**	ver:	gp2 exe version
**
**	return:	version string
*/
{
	if (ver >= GP2_EXE_NUMVERSIONS)
		ver = GP2_EXE_NUMVERSIONS;
	return gp2_exe_ver_strings[ver];
}




/**
 *	GP2_MakeTimeString	- Make short string representation of a time
 */
char *GP2_MakeTimeString(char *buf, DWORD t)
/*
**	buf:	String buffer for time string. If NULL, a static buffer is used.
**	t:		Time in ms.
**
**	return:	time string containing minutes, seconds and milliseconds.
*/
{
	long m = 0, s = 0, ms = 0;
	static char str[16];
	if (!buf)
		buf = str;
	if (GP2_DecompTime(NULL, &m ,&s, &ms, t))
		sprintf(buf, "%lu:%02lu.%03lu", m, s, ms);
	else
		strcpy(buf, "-:--.---");
	return buf;
}


/**
 *	GP2_MakeLongTimeString	- Make long string representation of a time
 */
char *GP2_MakeLongTimeString(char *buf, DWORD t)
/*
**	buf:	String buffer for time string. If NULL, a static buffer is used.
**	t:		Time in ms.
**
**	return:	time string containing hours, minutes, seconds and milliseconds.
*/
{
	long h = 0, m = 0, s = 0, ms = 0;
	static char str[24];
	if (!buf)
		buf = str;
	if (GP2_DecompTime(&h, &m ,&s, &ms, t))
		sprintf(buf, "%luh%02lum%02lu.%03lus", h, m, s, ms);
	else
		strcpy(buf, "------------");
	return buf;
}


const char *GP2_GetLevelString(int level)
{
	if (level < 0 || level > 4)
		level = 5;
	return level_strings[level];
}

