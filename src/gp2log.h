#ifndef GP2LOG_H
#define GP2LOG_H


///////////////////////////////////////////////////////////////////////////////
// Binary logging
//

#include <time.h>
#include "gp2def.h"

#pragma pack(1)

// Versions
#define GP2LAP_VER_MAJOR		1
#define GP2LAP_VER_MINOR		15
#define GP2LAP_VERSION			"1.15"
#define GP2LAP_VER_BETA			0
#define GP2LAP_BETA_VERSION		"0"
#define GP2LOG_VERSION			7					// WARNING: maximum of 31
#define GP2LOG_VERSION_MINOR	5					// WARNING: maximum of 7
#define GP2LOG_GLX_VERSION		3

#define GP2LAP_NAME				"GP2Lap"
#define GP2LAP_EMAIL			"gp2lap@gmx.net"
#define GP2LAP_HOMEPAGE			"http://rsdi.home.xs4all.nl/gp2lap.html"
#define GP2LAP_COPYRIGHT_STR	"Copyright (C) 1998-2001 Frank Ahnert, Rene Smit"

#define GP2LOG_DEBUG_FILENAME	"gp2lap.log"
#define GP2LAP_TXT_FILENAME		"gp2lap.txt"

#define GP2_EXE_FILENAME		"gp2.exe"

#define MAKE_LOG_VERSION(major,minor)	((major)|((minor)<<5))
#define MAKE_LOG_VERSION(major,minor)	((major)|((minor)<<5))
#define GET_LOGVER_MAJOR(ver)			((ver)&0x1f)
#define GET_LOGVER_MINOR(ver)			(((ver)>>5)&0x07)
#define GET_GP2LAPVER_GP2EXE(ver)		(((ver)>>24)&0xff)
#define GET_GP2LAPVER_MAJOR(ver)		(((ver)>>16)&0xff)
#define GET_GP2LAPVER_MINOR(ver)		(((ver)>>8)&0xff)
#define GET_GP2LAPVER_BETA(ver)			((ver)&0xff)
#define CHECK_LOG_VER(ver,major,minor)	(GET_LOGVER_MAJOR(ver)>(major)||(GET_LOGVER_MAJOR(ver)==(major)&&GET_LOGVER_MINOR(ver)>=(minor)))

#ifdef TEST
#	define GP2LAP_VERSION_STR	GP2LAP_NAME" v"GP2LAP_VERSION" beta "GP2LAP_BETA_VERSION" ("__DATE__")"
#	define GP2LAP_BASICVERSION	GP2LAP_VERSION"."GP2LAP_BETA_VERSION
#else
#	define GP2LAP_VERSION_STR	GP2LAP_NAME" v"GP2LAP_VERSION" ("__DATE__")"
#	define GP2LAP_BASICVERSION	GP2LAP_VERSION // for MenuDrawing patch, just a few chars!
#endif

#define GP2LAP_BANNER_STR		"\n"GP2LAP_VERSION_STR"\n"GP2LAP_COPYRIGHT_STR"\n"\
								"You may not redistribute this program; see "GP2LAP_TXT_FILENAME" for details.\n"\
								"Please report problems to <"GP2LAP_EMAIL">, and visit "GP2LAP_HOMEPAGE"\n\n"

#define GP2LOGSIG				0xde510d90

#define GP2LAP_TRACK_NAME_LEN	38  // including termination zero

#define GP2LRTYPE_INIT			1
#define GP2LRTYPE_TRACK			2
#define GP2LRTYPE_SESSION		3
#define GP2LRTYPE_CAR			4
#define GP2LRTYPE_LAP			5
#define GP2LRTYPE_PITOUT		7
#define GP2LRTYPE_RETIRE		8
#define GP2LRTYPE_INVALID		0

#define RecType(r)	(((GP2LapRecHdr*)r)->type)


// My own date/time struct
typedef struct GP2LOGTIMESTAMP
{
	BYTE	year;  // - 1980
	BYTE	month;
	BYTE	day;
	BYTE	hour;
	BYTE	minute;
	BYTE	second;
} GP2LogTimeStamp;


// Log header for each rec
typedef struct GP2LAPRECHDR
{
	BYTE	type;
} GP2LapRecHdr;


/////////////////////////////////////////////////////////////////
// Log record definitions

#define LIOF_HOF25		0x01
#define LIOF_CRUNCHED	0x02

typedef struct GP2LAPRECINIT
{
	GP2LapRecHdr hdr;
	BYTE		log_version;
	GP2LogTimeStamp day_time;
	DWORD		init_sum;
	DWORD		carshape_sum;
	DWORD		damage_sum;
	DWORD		cc_sum;
	WORD		pit_speed_fact;
	DWORD		gp2lap_version;
	BYTE		opt_flags;  // see LIOF_ #defines above
	WORD		cockpit_zoom;
	BYTE		future;  // reserved
} GP2LapRecInit;


// Track record flags (high 4 bits of track nr)
#define LTF_CUSTOM		0x80  // track editor file (#GP2INFO tag found)

typedef struct GP2LAPRECTRACK
{
	GP2LapRecHdr hdr;
	BYTE		nr;  // track nr in lo nibble. See LTF_ #defines above for hi nibble
	BYTE		slot;  // log slot too, because this can be edited in .exe
	DWORD		csum;  // used to check if track file isn't edited
	char		name[GP2LAP_TRACK_NAME_LEN];
	WORD		tyre_wear_factor;  // added in 1.14; GP2LAP_TRACK_NAME_LEN shortened by 2
} GP2LapRecTrack;



// Session log record flags
#define LSF_SH				0x01  // steering help on
#define LSF_OLH				0x02  // opposite lock help on
#define LSF_LOADED			0x04  // session was loaded
#define LSF_EXTLOAD			0x08  // loaded game was saved by gp2lap
#define LSF_HOF25LOAD		0x10  // loaded game was saved by gp2lap and used hof25 mode
#define LSF_INCOMPATIBLE	0x20  // game state potentially or for sure altered by load
#define LSF_LAPLOAD			0x40  // loaded game contained laps
#define LSF_NEWLOAD			0x80  // loaded game was saved by gp2lap 1.12 or newer

// beta 12: changed considerably
typedef struct GP2LAPRECSESSION
{
	GP2LapRecHdr hdr;
	BYTE		mode;
	BYTE		frame_time;
	BYTE		opt_perc;
	DWORD		flags;  // see LSF_ #defines above
	WORD		opt_fail;
	DWORD		future[4];  // reserved
} GP2LapRecSession;



typedef struct GP2LAPRECCAR
{
	GP2LapRecHdr hdr;
	BYTE		car_id;
	BYTE		team_id;
	WORD		grip_factor;
	WORD		engine_power;
	char		driver_name[GP2_LONGNAME_LEN];
	char		team_name[GP2_SHORTNAME_LEN];
	char		engine_name[GP2_SHORTNAME_LEN];
} GP2LapRecCar;



// Lap log record flags
#define LLF_OFFTRACK		0x01  // at least once on grass or in sand
#define LLF_PIT				0x02  // car is in pit
#define LLF_DETAILCHANGE	0x04  // at least one detail change
#define LLF_NOSLOWDOWN		0x08  // hof2.5 only: no slowdown after loading and before start of lap
#define LLF_F4USED			0x10  // hof2.5 only: F4 was used during lap
#define LLF_MAPUSED			0x20  // track map was on

typedef struct GP2LAPRECLAP
{
	GP2LapRecHdr hdr;
	BYTE		nr;
	BYTE		car_id;
	BYTE		flags;  // see LLF_ #defines above
	BYTE		race_pos;
	BYTE		qual_pos;
	DWORD		ses_time;
	DWORD		lap_split1;
	DWORD		lap_split2;
	DWORD		lap_time;
	WORD		weight;
	WORD		fuel_laps;
	WORD		grip;
	DWORD		aid_chg_cnt;
	DWORD		tyre_wear[GP2_NUM_WHEELS];
	DWORD		plank_wear[2];  // TODO: find it!
	WORD		avg_po;
	WORD		max_po;
	WORD		slomo_time;  // in 1/100th secs
	WORD		future[3];  // reserved
} GP2LapRecLap;



typedef struct GP2LAPRECPITOUT
{
	GP2LapRecHdr hdr;
	BYTE		nr;
	BYTE		car_id;
	BYTE		reserved;
	BYTE		race_pos;
	BYTE		qual_pos;
	DWORD		ses_time;
	WORD		weight;
	WORD		fuel_laps;
	BYTE		tyre_set;
} GP2LapRecPitOut;


typedef struct GP2LAPRECRETIRE
{
	GP2LapRecHdr hdr;
	BYTE		lap_nr;
	BYTE		car_id;
	BYTE		race_pos;
	BYTE		qual_pos;
	DWORD		ses_time;
	BYTE		reason;
	WORD		segment;
	DWORD		future[2];  // reserved
} GP2LapRecRetire;



// Options
extern BYTE opt_log_race;
extern BYTE opt_log_cc;
extern BYTE opt_log_glx;

BOOL	Log_Create();
void	Log_Kill();
void	Log_Reset();
void	Log_Encrypt(BYTE *data, long len, BYTE *csum);  // csum 4 BYTEs long
void	*Log_GetRecBuf(int rectype);
void	Log_AddPlayerRec(void *rec, int rectype);
void	Log_CopyLapData(const BYTE *buf, int len);
BYTE	*Log_GetLapData(long *lenp);
BYTE	*Log_GetCCLapData(long *lenp);
BYTE	*Log_GetInitData(long *lenp);
GP2LapRecLap *Log_FindLap(DWORD lapnr);

// Helper functions
extern void GetLogTime(GP2LogTimeStamp *t);


/////////////////////////////////////////////////////////////////////

#pragma pack()
#endif
