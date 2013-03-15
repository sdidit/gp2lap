#include <process.h>
#include <direct.h>
#include <time.h>
#include "stdinc.h"
#include "typedefs.h"
#include "basiclog.h"
#include "misc.h"

#include "gp2def.h"
#include "gp2log.h"
#include "prflog.h"
#include "gp2hook.h"

#include "dpmi.h"
#include "fixint2f.h"
#include "miscahf.h"
#include "trackinf.h"

#include "svga/vesa.h"
#include "vislog.h"

#include "fonts/myfntlib.h"						   // the font library
 
#include "cfgmain.h"							  // cfg file handling
#include "cfgdefin.inc"							// custom cfg switches

#include "int9.h"
#include "keyqueue.h"
#include "keyhand.h"
#include "gp2rmcod.h"

#include "myint21.h"

#ifdef SOCKCLNT
#include "sockclnt.h"
#endif

#include "gp2lap.h"

#include "ipx/ipxgp2.inc"


#ifndef __FLAT__
#error FLAT memory model allowed only
#endif



//-----------------------------------------------------------------------

#pragma off (check_stack);

//-----------------------------------------------------------------------
//
//      Display stuff
//

BYTE    *pUseSVGA =     NULL;


//-----------------------------------------------------------------------

struct gp2setupdata *GetSetupPtr(GP2Car *car,                   // ESI
                                                                 void *functionptr);    // EAX
#pragma aux GetSetupPtr =       \
                        "pushad"                \
                        "or esi,esi"    \
                        "jz Bye"                \
                        "or eax,eax"    \
                        "jz Bye"                \
                        "call eax"              \
"Bye:            popad"                 \
parm[ESI] [EAX] value[EAX] modify exact [EAX];


//----------------------------------------------------------------------
//-- Fuer Externe Routine (die von gp2 lockregions angesprungen wird) --
//----------------------------------------------------------------------

DWORD   GP2_Found =             0;
DWORD   GP2_rmFound =           0;  // fuer keytable-Behandlung eingefuehrt
DWORD   GP2_FoundAdr =          0;
WORD    GP2_CSel =              0;
WORD    GP2_DSel =              0;

DWORD   GP2_CodeStartAdr =      0;
DWORD   GP2_CodeEndAdr =        0;
DWORD   GP2_DataStartAdr =      0;
DWORD   GP2_DataEndAdr =        0;
DWORD   GP2_wcodeStartAdr =     0;
DWORD   GP2_wcodeEndAdr =       0;
DWORD   GP2_Data2StartAdr =     0;
DWORD   GP2_Data2EndAdr =       0;

//----- Frank's old hooked stuff ------
BYTE    *fpCalcSetuptoESI =     NULL;
BYTE    *fpGetCaridsSetup =     NULL;
BYTE    *pIsHumanFlag;
BYTE    *pUseAdvSetup;
struct gp2setupdata *pPlayerSetup = NULL;

GP2Car  *pCarStructs =  NULL;
GP2Car  *mycar =                NULL;

//----- important val's for itn handling ------
volatile DWORD   flagfield =    0;
volatile DWORD   activepage =   0;
volatile DWORD   activesub =    0;
volatile DWORD   whatcmd =      0;


//----- makebuffer for string output ------
//char atmp[360];

//----- *the* output buffer ------
BYTE    picbuf[0x4B000];
BYTE    *picbufptr = &picbuf;

//---- vom Compiler bereitgestellte Infos ------------
extern BYTE     __begtext;					/* Start of 32 Bit CODE segment */
//extern BYTE     end;								  /* End of BSS segment */

//---------------------------------------------------------------------------------------------

#include "timing/convert.inc"

//-----------------------------------------------------------------------------

void CloserDone(void)
// put everything in to be done at shutdown
{
	//----- restore interrupts -------
	_disable();
	_dos_setvect(0x09, OldInt9);
	_enable();
}

void CloserInit(void)
// i.e. closer to spawning child
{
	//----- get interrupt -------
	OldInt9 = _dos_getvect(0x09);

	//----- redirect interrupts -------
	_disable();
	_dos_setvect(0x09, MyInt9);
	_enable();

	atexit( CloserDone );
}
 
#define OPT_CFGNAME_STR "cfg:"
char *empty_opt_str = " ";

BOOL GetConfigFileNameOpt(char *namebuf, int buflen, int argc, char *argv[])
{
	int len = strlen(OPT_CFGNAME_STR);
	int i;
	BOOL found = FALSE;
	for (i=1; i<argc; i++) {
		if (!strnicmp(OPT_CFGNAME_STR, argv[i], len)) {
			char *opt_str = argv[i];
			if (strlen(opt_str)>len) {
				strncpy(namebuf, opt_str+len, buflen-1);
				found = TRUE;
			}
			argv[i] = empty_opt_str;	// clear option
		}
	}
	return found;
}


//-----------------------------------------------------------------------------

int main(int argc, const char *argv[])
// return values:
//  1 == error init dpmi / couldn't lock
//  2 == error initing log buffers
//  3 == error installing lammcall rmcode
//  4 == useipxlink requested, but couldn't install
//  5 == error while linking
//  6 == error loading gp2.exe
{
	char *fullname = NULL;
	DWORD *pULongCfg = NULL;
	char *pStrCfg = NULL;
	char cfgnamebuf[_MAX_PATH];
	char *cfgname = NULL;
	char *gp2exename = GP2_EXE_FILENAME;
	char *gp2logname = GP2LOG_DEBUG_FILENAME;
	DWORD dpmicode;
	ubyte log_flags = 0, i /* count var */;
	char tmpbuf[128];


#ifndef TEST
			//----- invoked by our own stub? --------
			// ACHTUNG: "GP2LINT" muss ungerade Anzahl Buchstaben haben (wegen updown & gamma)
	if (strcmp(getenv(updown("GP2LINT")), updown("GAMMA")) != 0) {
							//--- it's not defined, so leave quiet ---
#ifdef TEST
		fprintf(stderr, "GP2LINT not defined!\n");
#endif
		return 140;
	}
#endif

	printf(GP2LAP_BANNER_STR);
#ifdef SOCKCLNT
	printf("This version of GP2Lap was compiled as a socket client.\n");
#endif
#ifndef AUTH
	printf("This version of GP2Lap cannot be used for online leagues that require authentication.\n");
#endif
	//------- Init configuration
	if (GetConfigFileNameOpt(cfgnamebuf, _MAX_PATH, argc, (void*)argv)) {
		cfgname = cfgnamebuf;
		if (!strchr(cfgname, '.'))
			strcat(cfgname, ".cfg");
	} else
		cfgname = GP2LAP_CFG_FILENAME;

	switch (InitCfgValues(cfgname, &paths_to_check, &items, &fullname)) {
		case 0: printf("- Configuration read from: %s\n", fullname); break;
		case 1: fprintf(stderr, "*** can't open %s\n", fullname); break;
		case 2: fprintf(stderr, "*** can't locate %s\n", fullname); break;
	}

	pULongCfg = GetCfgULong("logDebug");
	if (pULongCfg)
		setbits(log_flags, BLF_DISABLED, !*pULongCfg);
	pULongCfg = GetCfgULong("logDebugFlush");
	if (pULongCfg)
		setbits(log_flags, BLF_FLUSHALWAYS, *pULongCfg);
	pStrCfg = GetCfgString("logDebugName");
	if (pStrCfg && strlen(pStrCfg) > 0)     // strlen doesn't seem to do nullptr chk
		gp2logname = pStrCfg;
	if (isoff(log_flags, BLF_DISABLED))
		printf("- Logging debug output to: %s\n", gp2logname);
	pULongCfg = GetCfgULong("hof25Enable");
	if (pULongCfg)
		opt_hof25 = *pULongCfg;
	if (opt_hof25)
		printf("- HOF2.5 mode enabled\n");
	pULongCfg = GetCfgULong("logPerf");
	if (pULongCfg)
		opt_log_perf = *pULongCfg;
	if (opt_log_perf)
		printf("- Extended perfing enabled\n");
	pULongCfg = GetCfgULong("logGLX");
	if (pULongCfg)
		opt_log_glx = *pULongCfg;
	if (opt_log_glx) {
		printf("- GLX log file enabled\n");
		// only enable opt_log_cc if opt_log_glx is TRUE
		pULongCfg = GetCfgULong("logCC");
		if (pULongCfg)
			opt_log_cc = *pULongCfg;
		if (opt_log_cc)
			printf("- Computer car logging enabled\n");
	}
	pULongCfg = GetCfgULong("Spa98");
	if (pULongCfg)
		opt_spa98 = *pULongCfg;
	if (opt_spa98)
		printf("- Spa '98 enabled\n");

	if (!LogStart(log_flags, gp2logname) && isoff(log_flags, BLF_DISABLED))
		fprintf(stderr, "*** error opening logfile '%s'\n", gp2logname);
	atexit(LogEnd);

#ifdef SOCKCLNT
	sockInit();
	atexit(sockExit);
#endif

	//--- init Frank's stuff ------
	init_new_gp2strings();  // init our strings
	FrankSoftInit();  // atexit'ed

	//---- init dpmi before all other stuff now ------
	dpmicode = dpmi_init(0 /* no verbose */); // atexit'ed
	if (dpmicode) {
		fprintf(stderr, "*** dpmi: error %04u\n", dpmicode);
		return 1;
	}

	initvesa();
 
	//---- lock my int9 handler ------
	if (!dpmi_lock_region((void near *)(MyInt9), 4096)) {  // should be enough
		 if (GetLogDpmiInfo())
			 LogLine("- dpmi: error: MI9 can't be locked!\n");
	} else {
		 if (GetLogDpmiInfo())
			 LogLine("- dpmis: MI9 locked\n");
	}

	//============================================
	//======== the complete logging stuff ========
	//============================================
	// Warning: don't change options after starting the log system!
	if (!Log_Create()) {
		fprintf(stderr, "*** error initing log buffers\n");
		return 2;
	}
	atexit(Log_Kill);

	if (!PrfLog_Create()) {
		fprintf(stderr, "*** error initing perf log buffers\n");
		return 2;
	}
	atexit(PrfLog_Kill);

	//=================================================
	//======== RM-Code fuer int21h vorbereiten =========
	//=================================================
	if ( !install_int21_hook() ) { // atexit'ed
		fprintf(stderr, "*** lowp: error e004");
		return 3;
	}

	//============================================
	//======== the complete network stuff ========
	//============================================
	if ( UseIpxLink ) {
		if ( !ipx_basic_init(0) ) // init the ipx  // atexit'ed
			return 4;
		if ( !start_ipx_link() )  // init the link
			return 5;
	}

	CloserInit();   // atexit'ed  // the very last

	//---- ok, send alive to logfile -----------
	_strdate(&tmpbuf);
	sprintf(strbuf,"\n"GP2LAP_NAME" started on %s ", tmpbuf);
	_strtime(&tmpbuf);
	strcat(strbuf, tmpbuf);
	strcat(strbuf, "\n");
	LogLine(strbuf);

	sprintf(strbuf, "- Code start at 0x%08x\n", &__begtext);
	LogLine(strbuf);

	//----- 08/99  for solving the int2F prob---------------
	InitFixInt2F();

	//----- Fremdapplikation starten -------
	putenv("DOS4G=QUIET");
	sprintf(strbuf, "Loading %s...\n", gp2exename);
	LogLine(strbuf);
	printf(strbuf);
	argv[0] = gp2exename;
	if (spawnv(P_WAIT, gp2exename, (void*)argv) < 0) {      // cast to void* to avoid warning about double indirection constness
		sprintf(strbuf, "*** error loading %s: %s\n", gp2exename, strerror(errno));
		LogLine(strbuf);
		fprintf(stderr, strbuf);
	}

#ifdef TEST
	sprintf(strbuf, "flagfield = 0x%08x\n", flagfield);
	LogLine(strbuf);
	if (GP2_Found && ((flagfield & 0xFFF) != 0x7))
		printf("\nflags == 0x%08X\n*** error flag field incorrect\n", flagfield);
#endif

	//--- saying bye now ------
	_strdate(&tmpbuf);
	sprintf(strbuf, GP2LAP_NAME" exiting on %s ", tmpbuf);
	_strtime(&tmpbuf);
	strcat(strbuf, tmpbuf);
	strcat(strbuf, "\n");
	LogLine(strbuf);

	return 0;
}
