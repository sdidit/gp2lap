#include "stdinc.h"
#include "gp2lap.h"
#include "gp2str.h"
#include "gp2misc.h"
#include "gp2glob.h"
#include "basiclog.h"
#include "misc.h"
#include "gp2hook.h"
#include "gp2log.h"
#include "prflog.h"
#include "track.h"
#include "cfgmain.h"
#include "keys.h"
#include "keyhand.h"
#include "trackinf.h"						 // for FrankOnTrackFileLoad()
#include "trackmap.h"
#include "carinfo.h"
#include "miscahf.h"
#include "vislog.h"
#include "pages.h"

#ifdef SOCKCLNT
#include "sockclnt.h"
#include "podcar.h"
#endif

#ifdef TEST
#include "fonts/myfntlib.h"
extern fontdef fntarrn6;
#endif
#include "fonts/myfntlib.h"
#include "svga/gp2color.h"


static void ForceShowLogPage(void) { PAGESETON(PAGE_LOG); VisLogUpdate(); }


#define LOGF_LOGPO			0x01
#define LOGF_LOGPERF		0x02
#define LOGF_EXTLOAD		0x04
#define LOGF_LAPLOAD		0x08	// lapdata were loaded from game
#define LOGF_INCOMPATIBLE	0x10	// gp2's state has been changed after a game with other checksums was loaded
#define LOGF_MAPUSED		0x20	// track map used
#define LOGF_NEWLOAD		0x40	// loaded games saved by gp2lap 1.12+
static DWORD log_flags = 0;

// HOF2.5
#define HOF25_SLOWDOWNFRAMES	11	// number of frames the car must slowdown
BYTE opt_hof25 = FALSE;

#define HOF25F_MUSTSLOWDOWN	0x01
#define HOF25F_SLOWEDDOWN	0x02
#define HOF25F_NEXTLAPVALID	0x04
#define HOF25_F4USED		0x08

static DWORD hof25_flags = HOF25F_NEXTLAPVALID;


#define TRF_CUSTOM			0x01
#define TRF_HASSEGRANGES	0x02
#define TRF_WRONGSLOT		0x04
static DWORD tr_flags = 0;

// Spa '98
BYTE opt_spa98 = FALSE;
static WORD yspd = 0x8000;
static BOOL yspd_up = TRUE;
static DWORD spa98timer = 0;

static BOOL keep_dist_unit = FALSE;
static short oldKPH = 0;

#define HILIGHT_WHEEL(cs,w)			cs->damageLeds|=1<<(5-(w))
#define IS_HILIGHTED_WHEEL(cs,w)	(cs->damageLeds&(1<<(5-(w))))
#define UNHILIGHT_WHEEL(cs,w)		cs->damageLeds&=~(1<<(5-(w)))

static const BYTE wheel_show_bits[GP2_NUM_WHEELS] = { 16, 14, 9, 7 };
#define IS_DESTRUCTED_WHEEL(cs,w)	(cs->invisibleParts&(1<<wheel_show_bits[w]))
#define DESTRUCT_WHEEL(cs,w)		cs->invisibleParts|=1<<wheel_show_bits[w]
#define REPAIR_WHEEL(cs,w)			cs->invisibleParts&=~(1<<wheel_show_bits[w])

#define START_BLACK_FLAG(cs,t)		on(cs->dispFlags_168,4);cs->timeAlarm0d8=*pCurTime+t*1000

////////////////////////////////////////////////////////////////////////////////////////////////////////////

GP2Car			*pCurrentCS =		NULL;			// lammcall sets this to esi for each hook
DWORD			dwTrackChecksum =	0;				// lammcall sets this in tfl hook
void (__near *fpInitFileHdr)(BYTE type) = NULL;		// BYTE type: file type. pFileHdr points to header
BYTE			*pPitSpeedLimitEnabled = NULL;		// points to opcode byte. Should be 0x74 if pit lane speed limit enabled
BYTE			*pFreezeCode = NULL;

DWORD			dwOldFileBufLen =	0;
   

// For slowdown test
static WORD		prev_speed =		0;
static int		slowdown_count =	0;

static char		strbuf2[256];	// extra temporary log buffer

static GP2LapRecInit initrecbak =	{ 0 };
static GP2LapRecTrack trackrecbak =	{ 0 };
static GP2LapRecSession sesrecbak =	{ 0 };

//----- Statistics -----
DWORD	max_frame_time =			0;
DWORD	total_frames =				0;
DWORD	total_frame_time =			0;
DWORD	num_slomo_frames =			0;
BOOL	dirty =						FALSE;
BYTE	dirty_w[GP2_NUM_WHEELS] =	{ 0,0,0,0 };
DWORD	dirtcount[GP2_NUM_WHEELS] =	{ 0,0,0,0 };
BOOL	drv_aids_set =				FALSE;
BYTE	cur_drv_aids =				0xff;
DWORD	aid_chg_cnt =				0;
BYTE	cur_dlevel =				3;
BOOL	dlevel_chg =				FALSE;


void ResetStats(void)
{
	dirty = FALSE;
	memset(dirty_w, 0, sizeof(dirty_w));
	memset(dirtcount, 0, sizeof(dirtcount));
	total_frames = 0;
	total_frame_time = 0;
	max_frame_time = 0;
	num_slomo_frames = 0;
	drv_aids_set = FALSE;
	cur_drv_aids = 0xff;
	aid_chg_cnt = 0;
	cur_dlevel = *pDetailLevel;
	dlevel_chg = FALSE;
}


//----- Helper functions -----

// For each flag set in flags, increase its nibble counter in *cntp (8 counters of 4 bits).
// Max counter value is 15. The counters won't wrap.
// Returns TRUE if one or more counters overflowed.
BOOL IncNibbleCounters(BYTE flags, DWORD *cntp)
{
	int bit;
	BOOL of = FALSE;
	DWORD newcnt;

	if (!flags || !cntp)
		return FALSE;
	if (*cntp == 0xffffffff)
		return TRUE;

	newcnt = *cntp;
	for (bit=0; bit<8; bit++) {
		DWORD c = newcnt & 0xf;								 // get low 4 bits
		if (flags & (1<<bit)) {
			if (c < 0xf)
				c++;									   // increase counter
			else
				of = TRUE;
		}
		newcnt = (newcnt>>4) | (c<<28);	// shift out low 4 bits, shift in new counter value in high 4 bits
	}
	*cntp = newcnt;
	return of;
}


BOOL AddNibbleCounters(DWORD cnt, DWORD *cntp)
{
	int bit;
	BOOL of = FALSE;
	DWORD newcnt;

	if (!cnt || !cntp)
		return FALSE;
	if (*cntp == 0xffffffff)
		return TRUE;

	newcnt = *cntp;
	for (bit=0; bit<8; bit++) {
		DWORD c = newcnt & 0xf;								 // get low 4 bits
		DWORD sc = cnt & 0xf;								 // get low 4 bits
		if (sc) {
			c += sc;
			if (c > 0xf) {
				c = 0xf;
				of = TRUE;
			}
		}
		newcnt = (newcnt>>4) | (c<<28);	// shift out low 4 bits, shift in new counter value in high 4 bits
		cnt >>= 4;
	}
	*cntp = newcnt;
	return of;
}

WORD CalcPO(double frTime)
{
	if (!pFrameTime || !*pFrameTime)
		return 0;
	return (WORD) ((100.0 * frTime) / (double) *pFrameTime);
}

double CalcFPS(double frTime)
{
	if (frTime > 0)
		return 256.0 / frTime;
	return 0;
}


static BYTE GetFastLapPos(BYTE id)
{
	int pos = 0;
	while (pos < 40 && pFastLapCars[pos] != id)
		pos++;
	return pos < 40 ? pos+1 : 0;
}


static char *GetDriverName(BYTE id)
{
	id &= 0x3f;
	if (id > 0)
		id--;
	if (id >= GP2_MAX_NUMCARS)
		return "???";
	return pDriverNames[id];
}

static char *GetTeamName(BYTE id)
{
	id--;
	if (id >= GP2_MAX_NUMTEAMS)
		return "???";
	return pTeamNames[id];
}

static char *GetEngineName(BYTE id)
{
	id--;
	if (id >= GP2_MAX_NUMTEAMS)
		return "???";
	return pEngineNames[id];
}


static GP2CSx *GetCarSetup(BYTE id)
{
	id &= 0x3f;
	if (id > 0)
		id--;
	if (id >= GP2_MAX_NUMCARS)
		return NULL;
	return pCarSetups+id;
}



///////////////////////////////////////////////////////////////////////////////////////////

static WORD GetAvgPO(void)
{
	return total_frames > 0 ? CalcPO((double) total_frame_time / (double) total_frames) : 0;
}


static void CalcChecksums(DWORD *init_sum, DWORD *shape_sum, DWORD *damage_sum, DWORD *cc_sum)
{
	if (ardwInitVals)
		Log_Encrypt((BYTE*) ardwInitVals, 1948, (BYTE*) init_sum);
	if (pPlayerHPCode) {
		Log_Encrypt(pPlayerHPCode, 4, (BYTE*) init_sum); // use team hp for player or not 1
		Log_Encrypt(pPlayerHPCode+0x0e, 4, (BYTE*) init_sum); // player race power
		Log_Encrypt(pPlayerHPCode+0x15, 4, (BYTE*) init_sum); // player qual power
		Log_Encrypt(pPlayerHPCode+0x16c, 5, (BYTE*) init_sum);	// use team hp for player or not 2
	}
	if (pCarShape)
		Log_Encrypt((BYTE*) pCarShape, 54336, (BYTE*) shape_sum);
	if (pRetireLimit && pDamageTrack && pDamageCars) {
		Log_Encrypt((BYTE*) pRetireLimit, 4, (BYTE*) damage_sum);
		Log_Encrypt((BYTE*) pDamageTrack, 164, (BYTE*) damage_sum);
		Log_Encrypt((BYTE*) pDamageCars, 420, (BYTE*) damage_sum);
	}
	if (pTeamHPQual && pPlayerHPCode) {
#if 0
		FILE *f = fopen("\\cc.bin", "w+b");
		if (f) {
			fwrite(pTeamHPQual, 1, 440, f);
			fwrite(pPlayerHPCode+0x43, 1, 4, f);
			fwrite(pPlayerHPCode+0x5a, 1, 4, f);
			fclose(f);
		}
#endif
		Log_Encrypt((BYTE*) pTeamHPQual, 440, (BYTE*) cc_sum);
		Log_Encrypt(pPlayerHPCode+0x43, 4, (BYTE*) cc_sum); // team power base 1
		Log_Encrypt(pPlayerHPCode+0x5a, 4, (BYTE*) cc_sum); // team power base 2
	}
#ifdef TEST
	sprintf(strbuf, "  Init, car shape, damage, cc checksum: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", *init_sum, *shape_sum, *damage_sum, *cc_sum);
	LogLine(strbuf);
#endif
}


static WORD GetPitSpeedLimit(void) { return (*pPitSpeedLimitEnabled == 0x74) ? *pPitSpeedLimit : 0; }

static BOOL StoreChecksums(void)
{
	DWORD init_sum = 0, shape_sum = 0, damage_sum = 0, cc_sum = 0;
	GP2LapRecInit *initrec = (GP2LapRecInit*) Log_GetRecBuf(GP2LRTYPE_INIT);
	CalcChecksums(&init_sum, &shape_sum, &damage_sum, &cc_sum);
	if (initrec->init_sum != init_sum ||
		initrec->carshape_sum != shape_sum ||
		initrec->damage_sum != damage_sum ||
//		initrec->cc_sum != cc_sum ||
		initrec->pit_speed_fact != GetPitSpeedLimit()
	   ) {
		initrec->init_sum = init_sum;
		initrec->carshape_sum = shape_sum;
		initrec->damage_sum = damage_sum;
		initrec->cc_sum = cc_sum;
		initrec->pit_speed_fact = GetPitSpeedLimit();
		return TRUE;
	}
	return FALSE;
}

static void ResetLogs()
{
	Log_Reset();
	PrfLog_Reset();
	LogLine("Log buffer cleared\n");
}

//----- Hooked functions -----

void __near InitGP2Hook(void)
{
	GP2LapRecInit *rec = NULL;
	DWORD *pULongCfg = NULL;

	// F8 = freeze (instead of F8 + NUM5 + S)
	*(WORD*)(IDAtoFlat(0x3482e)) = 0x6074;
	*(WORD*)(IDAtoFlat(0x34a60)) = 0x6074;

	InitTrackMap();

	pULongCfg = GetCfgULong("KeepDistUnit");
	keep_dist_unit = pULongCfg && *pULongCfg;

	ResetStats();
	LogLine("Initing...\n");

	rec = (GP2LapRecInit*) Log_GetRecBuf(GP2LRTYPE_INIT);
	StoreChecksums();
	GetLogTime(&rec->day_time);
	// Log version info. GP2's exe version is added (in msb) in misc.c, as it's not available here yet
	rec->gp2lap_version = (GP2LAP_VER_MAJOR << 16) | (GP2LAP_VER_MINOR << 8) | GP2LAP_VER_BETA;
	rec->log_version = MAKE_LOG_VERSION(GP2LOG_VERSION, GP2LOG_VERSION_MINOR);
	rec->opt_flags = 0;
	setbits(rec->opt_flags, LIOF_HOF25, opt_hof25);
	rec->cockpit_zoom = (WORD)(*(long*)(IDAtoFlat(0x3856d)));
	rec->future = 0;
}


static char *GetNextTrackTag(char *buf)
{
	while (*buf != '|')
		buf++;
	return buf+1;
}


// Track file loaded
// NOTE: Can't check session mode here, isn't set yet!
void __near TFLHook(void)
{
	ResetStats();
	if (isoff(log_flags, LOGF_LAPLOAD))
		ResetLogs();
	off(log_flags, LOGF_LOGPERF);
	off(tr_flags, TRF_CUSTOM | TRF_HASSEGRANGES | TRF_WRONGSLOT);
	if (pTrackBuf && pTrackFileLength && pTrackNr && pTrackIndex) {
		GP2LapRecTrack *trackrec = (GP2LapRecTrack*) Log_GetRecBuf(GP2LRTYPE_TRACK);
		DWORD len = *pTrackFileLength;
		DWORD slot = *pTrackIndex;
		DWORD nr = *pTrackNr; // normally used everywhere, taken from table with slot as index
		DWORD *bufd = (DWORD*) pTrackBuf;
		BOOL wrong_csum = FALSE;
		
		sprintf(strbuf, "Track %d from slot %d loaded:\n"
#ifdef TEST
						"  length %d, checksum 0x%08x\n"
#endif
				, nr+1, slot+1
#ifdef TEST
				, len, dwTrackChecksum
#endif
			   );
		LogLine(strbuf);
		if (trackrec) {
			trackrec->nr = nr;
			trackrec->slot = slot;
			trackrec->csum = dwTrackChecksum;
			strcpy(trackrec->name, "<no track name>");
			trackrec->tyre_wear_factor = arTrackTyreWear[nr];
		}
		// Look for track editor tags
		if (bufd[0] != 0x32504723 || bufd[1] != 0x4f464e49) {	// #GP2INFO
			sprintf(strbuf, "  track file is a standard GP2 track file (%s)\n", GP2_GetTrackString(*pTrackNr));
			LogLine(strbuf);
			wrong_csum = slot >= GP2_NUM_TRACKS || dwTrackChecksum != GP2_GetDefTrackChecksum(slot);
		} else {
			// This is a Track Editor file
			char *hof25_txt = NULL;
			int hof25_txt_len = 0;
			int ti_laps = 0, ti_slot = 0;
			char *info = GetNextTrackTag(pTrackBuf);
			on(tr_flags, TRF_CUSTOM);
			on(trackrec->nr, LTF_CUSTOM);

			//---- Modif Frank 11/99 -----
			FrankOnTrackFileLoad(pTrackBuf);
			//----------------------------

			// Dump track info tags
			while (info-pTrackBuf < len-1 && info[0] != '#') {	// # = end of tag list
				char *info2 = GetNextTrackTag(info);
				int taglen = info2-info-1, fill, fi;
				BOOL nametag = FALSE, desctag = FALSE, slottag = FALSE, lapstag = FALSE;
#ifdef TEST
				strcpy(strbuf, "    ");
				strncat(strbuf, info, taglen);
				strcat(strbuf, ":");
#endif
				if (!strnicmp(info, "name", taglen)) {
					if (trackrec)
						nametag = TRUE;
				}
				else if (!strnicmp(info, "desc", taglen))
					desctag = TRUE;
				else if (!strnicmp(info, "slot", taglen))
					slottag = TRUE;
				else if (!strnicmp(info, "laps", taglen))
					lapstag = TRUE;
				fill = max(14, taglen+1);
				fi = strlen(strbuf);
				while (fill-taglen > 0) {
					strbuf[fi++] = ' ';
					fill--;
				}
				strbuf[fi] = 0;
				info = GetNextTrackTag(info2);
				taglen = info-info2-1;
#ifdef TEST
				strncat(strbuf, info2, taglen);
				strcat(strbuf, "\n");
				LogLine(strbuf);
#endif
				if (nametag) {
					int namelen = min(taglen, GP2LAP_TRACK_NAME_LEN-1);
					strncpy(trackrec->name, info2, namelen);
					trackrec->name[namelen] = 0;
				}
				if (lapstag)
					ti_laps = atoi(info2);
				if (opt_hof25 && desctag) {
					// Look for hof25 info
					int desci = 0;
					while (desci < taglen) {
						if (*((DWORD*)(info2+desci)) == GP2_SEGINFO_SIG) {
							int hof25_i = 0;
							hof25_txt_len = 0;
							desci += 4;
							hof25_txt = info2+desci;
							do {
								if (hof25_txt[hof25_i] == '@') {
									on(tr_flags, TRF_HASSEGRANGES);	// @SEG...@ found
									hof25_txt_len = hof25_i;
									break;
								}
							} while ((++hof25_i + desci) < taglen);
							break;
						}	// @SEG
						desci++;
					}	// while not end of desc tag
				}	// desc tag found
				if (slottag) {
					ti_slot = atoi(info2);
					if (opt_hof25) {
#ifdef TEST
						sprintf(strbuf, "    [HOF2.5] Slot = %d, real slot = %d\n", ti_slot, slot+1);
						LogLine(strbuf);
#endif
						setbits(tr_flags, TRF_WRONGSLOT, ti_slot != (slot+1));
					}
				}
			}	// while not end of tags
			if (ison(tr_flags, TRF_HASSEGRANGES))
				ProcessTrackSegRanges(hof25_txt, hof25_txt_len);
			sprintf(strbuf, "  track file is a GP2 Track Editor file (%s; slot %d; %d laps)\n", trackrec->name, ti_slot, ti_laps);
			LogLine(strbuf);
		}	// #GP2INFO
		if (opt_hof25) {
			if (ison(tr_flags, TRF_CUSTOM) && ison(tr_flags, TRF_HASSEGRANGES)) {
				if (ison(tr_flags, TRF_WRONGSLOT)) {
					MBTELL(ID_HOF25_TRACKWRONGSLOT, ID_MENU_CURCIRCUIT);
#ifdef TEST
					LogLine("    [HOF2.5] Track file is NOT in correct slot!\n");
#endif
				}
			}
			else if (ison(tr_flags, TRF_CUSTOM) || wrong_csum) {
				MBTELL(ID_HOF25_TRACKNOTCOMPLIANT, ID_MENU_CURCIRCUIT);
#ifdef TEST
				LogLine("    [HOF2.5] Track file is NOT HOF25-compliant!\n");
#endif
			}
		}
	}	// various sanity checks
}


// Load of data file
// File info contains body, length in dec_len
void __near LDFHook(void)
{
	GP2FileInfo *info = *ppFileInfo;
	GP2LapRecInit *curinitrec = (GP2LapRecInit*) Log_GetRecBuf(GP2LRTYPE_INIT);
	GP2LapRecTrack *curtrackrec = (GP2LapRecTrack*) Log_GetRecBuf(GP2LRTYPE_TRACK);

	off(log_flags, LOGF_EXTLOAD | LOGF_LAPLOAD | LOGF_NEWLOAD);
	memcpy(&initrecbak, curinitrec, sizeof(initrecbak));
	memcpy(&trackrecbak, curtrackrec, sizeof(trackrecbak));
	memcpy(&sesrecbak, Log_GetRecBuf(GP2LRTYPE_SESSION), sizeof(sesrecbak));
	if (info && info->buf_ptr) {
		BYTE type = GP2_FILE_TYPE(pFileHdr->type);
		sprintf(strbuf, "Data file '%s' loaded\n", info->file_name);
		LogLine(strbuf);
		if (!GP2_FILE_ISCRUNCHED(pFileHdr->type))
			LogLine("  File is not compressed\n");
		if (GP2_FILE_TYPE_ISHOTLAP(type) || GP2_FILE_TYPE_ISGAME(type)) {
			BOOL incomp = ison(log_flags, LOGF_INCOMPATIBLE);
			restore_gp2string(ID_HOTLAPLOADED);
			restore_gp2string(ID_GAMELOADED);
			on(log_flags, LOGF_INCOMPATIBLE);  // override later if compatible
			if (*(((DWORD*)(info->buf_ptr+info->dec_len))-1) == GP2LOGSIG) {
				DWORD os = *(((DWORD*)(info->buf_ptr+info->dec_len))-2);
				DWORD hash = *(((DWORD*)(info->buf_ptr+info->dec_len))-3);
				GP2LapRecInit *initrec = (GP2LapRecInit*)(info->buf_ptr+os);
				if (initrec->hdr.type == GP2LRTYPE_INIT) {
					// check version
					int majver = GET_GP2LAPVER_MAJOR(initrec->gp2lap_version);
					int minver = GET_GP2LAPVER_MINOR(initrec->gp2lap_version);
					if (majver == 1 && minver >= 7 || majver > 1) {
						GP2LapRecTrack *trackrec = (GP2LapRecTrack*)(initrec+1);
						memcpy(&initrecbak, initrec, sizeof(initrecbak));
						on(log_flags, LOGF_EXTLOAD);
						setbits(log_flags, LOGF_NEWLOAD, majver == 1 && minver >= 14 || majver > 1);
						if (trackrec->hdr.type == GP2LRTYPE_TRACK) {
							GP2LapRecSession *sesrec = (GP2LapRecSession*)(trackrec+1);
							memcpy(&trackrecbak, trackrec, sizeof(trackrecbak));
							if (sesrec->hdr.type == GP2LRTYPE_SESSION) {
								DWORD newhash = 0;
								BYTE *recptr = (BYTE*)(sesrec+1);
								long len = (long)(info->buf_ptr+info->dec_len-12-recptr);
								memcpy(&sesrecbak, sesrec, sizeof(sesrecbak));
								if (GP2_FILE_TYPE_ISGAME(type) && len > 0) {
									LogLine("Extended game loaded; copying lap buffer\n");
									Log_CopyLapData(recptr, len);
									on(log_flags, LOGF_LAPLOAD);
								}
								Log_Encrypt(info->buf_ptr+os, info->dec_len-12-os, (BYTE*)&newhash);
								// check if any relevant data has changed, and inherit it
								// changed in 1.12: clear flag when loaded game is compatible
								// changed in 1.12: check hash code
								if (hash != newhash ||
									ison(sesrecbak.flags, LSF_INCOMPATIBLE) ||
									initrecbak.init_sum != curinitrec->init_sum ||
									initrecbak.carshape_sum != curinitrec->carshape_sum ||
									initrecbak.damage_sum != curinitrec->damage_sum ||
//									initrecbak.cc_sum != curinitrec->cc_sum ||
									initrecbak.pit_speed_fact != curinitrec->pit_speed_fact
								   )
									on(log_flags, LOGF_INCOMPATIBLE);
								else if (!incomp || GP2_FILE_TYPE_ISGAME(type))
									off(log_flags, LOGF_INCOMPATIBLE);
							}
						}
					}
				}
				if (opt_hof25) {
					if (GP2_FILE_TYPE_ISHOTLAP(type))
						set_gp2string(ID_HOTLAPLOADED, "\x11""GP2Lap Extended Hot Lap loaded");
					else
						set_gp2string(ID_GAMELOADED, "\x11""GP2Lap Extended Game Loaded");
				}
			} else {
				if (opt_hof25)
					LogLine("[HOF25] Warning: not a GP2Lap extended file!\n");
			}
		}
#ifdef TEST
		else {
			sprintf(strbuf, "Load of data file: %d not a hotlap or game\n", type);
			LogLine(strbuf);
		}
#endif
	}
#ifdef TEST
	else
		LogLine("Load of data file: no information buffer or data file!\n");
#endif
	if (keep_dist_unit)
		oldKPH = *pIsKPH;
}



// Save hotlap intercept.
// Returns TRUE if hotlap may be saved
static BOOL OnSaveHotlap(GP2LapRecLap *lap)
{
	BOOL ok = TRUE;
	long msg_id = -1;
	char mode = *pSessionMode;

	// HOF2.5 real-time validation checks
	if (opt_hof25 && mode != 0x80) {
		GP2LapRecSession *ses = (GP2LapRecSession*) Log_GetRecBuf(GP2LRTYPE_SESSION);
		
#ifdef TEST
		strcpy(strbuf, "[HOF25] Verifying lap data");
#endif

#if 0
		// Track tests (*** DISABLED ***)
		if (ison(tr_flags, TRF_CUSTOM)) {
			if (isoff(tr_flags, TRF_HASSEGRANGES)) {
				strcat(strbuf, ": not a hof2.5 compliant track");
				ok = FALSE;
			} else if (ison(tr_flags, TRF_WRONGSLOT)) {
				msg_id = ID_HOF25_WRONGSLOT;
#ifdef TEST
				strcat(strbuf, ": custom track in wrong slot");
#endif
				ok = FALSE;
			}
		}
#endif

		// Session tests
		if (ok && ison(ses->flags, LSF_LOADED) && isoff(ses->flags, LSF_HOF25LOAD)) {
			msg_id = ID_HOF25_NOEXTLOAD;
#ifdef TEST
			strcat(strbuf, ": no extended hof2.5 load");
#endif
			ok = FALSE;
		}

		// Lap tests
		if (ok && ison(lap->flags, LLF_DETAILCHANGE)) {
			msg_id = ID_HOF25_DETAILEVELCHANGED;
#ifdef TEST
			strcat(strbuf, ": detail level changed");
#endif
			ok = FALSE;
		}
		if (ok && ison(lap->flags, LLF_NOSLOWDOWN)) {
			msg_id = ID_HOF25_NOSLOWDOWN;
#ifdef TEST
			strcat(strbuf, ": no slowdown before start of lap");
			ok = FALSE;
#endif
		}
		if (ok && ison(lap->flags, LLF_F4USED)) {
			msg_id = ID_HOF25_F4USED;
#ifdef TEST
			strcat(strbuf, ": indestructability was used");
#endif
			ok = FALSE;			
		}
	}
#ifdef TEST
	if (!ok) {
		strcat(strbuf, "\n");
		LogLine(strbuf);
	}
#endif
	if (msg_id != -1)
		MBTELL(msg_id, ID_MENU_SAVEHOTLAP);
	return ok;
}

void SaveGLxData(char *name)
{
	BYTE *my_data = NULL;
	long len = 0;
	char fname[16] = {0};
	FILE *fp;
	int i;

	strncpy(fname, name, 15);
	i = 0;
	while (i<12 && fname[i] != '.') i++;
	if (i >= 12)
		return;
	fname[i+1] = 'G';
	fname[i+2] = 'L';
	fname[i+4] = 0;
	
	fp = fopen(fname, "wb");
	if (fp) {
		// Write sig
		DWORD sig = GP2LOGSIG_GLX;
		DWORD ver = GP2LOG_GLX_VERSION;
		fwrite(&sig, 1, 4, fp);
		fwrite(&ver, 1, 4, fp);
		// pDriverNames, (pTeamNames, pEngineNames, pCar->teamNr), pCarSetups
		fwrite(pDriverNames, 1, GP2_MAX_NUMCARS * GP2_LONGNAME_LEN, fp);
		fwrite(pCarSetups + GP2_MAX_NUMCARS, 1, GP2_MAX_NUMCARS * sizeof(GP2CSx), fp);
		my_data = Log_GetInitData(&len);
		if (my_data && len>0) {
			DWORD hash = 0;
			fwrite(my_data, 1, len, fp);
			Log_Encrypt(my_data, len, (BYTE*)&hash);
			my_data = opt_log_cc ? Log_GetCCLapData(&len) : Log_GetLapData(&len);
			if (my_data && len>0) {
				Log_Encrypt(my_data, len, (BYTE*)&hash);
				fwrite(my_data, 1, len, fp);
			}
			fwrite(&hash, 1, 4, fp);
		}
		fclose(fp);
	}
}


#define APPEND_WORD(p,l,d)		{*((WORD*)p)=d;p+=sizeof(WORD);l+=sizeof(WORD);}
#define APPEND_DWORD(p,l,d)		{*((DWORD*)p)=d;p+=sizeof(DWORD);l+=sizeof(DWORD);}
#define APPEND_DATA(p,l,d,sz)	{memcpy(p,d,sz);p+=sz;l+=sz;}

void __near SDFHook(void)
{
	GP2FileInfo *info = *ppFileInfo;
	GP2LapRecSession *rec = (GP2LapRecSession*) Log_GetRecBuf(GP2LRTYPE_SESSION);
	if (ison(log_flags, LOGF_INCOMPATIBLE))
		on(rec->flags, LSF_INCOMPATIBLE);	// in case session was not started/loaded

    sprintf(strbuf, "Saving data file '%s'\n", info->file_name);
	LogLine(strbuf);
	
	restore_gp2string(ID_HOTLAPSAVED);
	if (opt_log_race)
		restore_gp2string(ID_GAMESAVED);

	dwOldFileBufLen = 0;
	if (info) {
		long len = 0, ext_len = 0;
		BYTE *append_ptr = info->buf_ptr + info->buf_len;
		BYTE *ext_data = append_ptr;
		dwOldFileBufLen = info->buf_len;

		sprintf(strbuf, "  Type: %s\n", GP2_GetFileTypeString(info->type));
		LogLine(strbuf);
		switch (GP2_FILE_TYPE(info->type))
		{
		case GP2_FILETYPE_HOTLAP:
		case GP2_FILETYPE_HOTLAP_L: {
				BYTE *my_data = NULL;
				DWORD hl_nr = *pHotlapSel+1;
				GP2LapRecLap *hotlap = NULL;
				// look back in history to find accompanying lap
				hotlap = Log_FindLap(hl_nr);
				if (hotlap && (OnSaveHotlap(hotlap))) {
					my_data = Log_GetInitData(&len);
					if (my_data && len>0)
						APPEND_DATA(append_ptr,info->buf_len,my_data,len)
					APPEND_DATA(append_ptr,info->buf_len,hotlap,sizeof(GP2LapRecLap))
					set_gp2string(ID_HOTLAPSAVED, "\x11""GP2Lap Extended Hot Lap Saved");
				}
			}
			break;
		case GP2_FILETYPE_ULPRAC:
		case GP2_FILETYPE_QRACE:
		case GP2_FILETYPE_NCGAME:
		case GP2_FILETYPE_CHGAME:
		case GP2_FILETYPE_QRACE_L:
		case GP2_FILETYPE_NCGAME_L:
		case GP2_FILETYPE_CHGAME_L:
			if (opt_log_race) {
				BYTE *my_data = NULL;

				dwOldFileBufLen = info->buf_len;
				my_data = Log_GetInitData(&len);
				if (my_data && len>0)
					APPEND_DATA(append_ptr,info->buf_len,my_data,len)
				my_data = Log_GetLapData(&len);
				if (my_data && len>0)
					APPEND_DATA(append_ptr,info->buf_len,my_data,len)
				if (append_ptr > ext_data)
					set_gp2string(ID_GAMESAVED, "\x11""GP2Lap Extended Game Saved");
				if (opt_log_glx)
					SaveGLxData(info->file_name);
			}
			break;
		//	case GP2_FILETYPE_PERF:	// Perf data is handled in a different hook
		}

		sprintf(strbuf, "  Length: %d\n", info->buf_len);
		LogLine(strbuf);
		ext_len = info->buf_len - dwOldFileBufLen;

		if (dwOldFileBufLen && dwOldFileBufLen < info->buf_len) {
			// Finally, a hash code, the offset of my data and a signature
			Log_Encrypt(ext_data, (long)(append_ptr-ext_data), append_ptr);
#ifdef TEST
			sprintf(strbuf, "  Hash code: 0x%08x\n", *(DWORD*)append_ptr);
			LogLine(strbuf);
#endif
			append_ptr += 4;
			info->buf_len += 4;

			APPEND_DWORD(append_ptr,info->buf_len,dwOldFileBufLen);
			APPEND_DWORD(append_ptr,info->buf_len,GP2LOGSIG);
			if (info->max_len < info->buf_len)
				info->max_len = info->buf_len;
			if (info->min_len > info->buf_len)
				info->min_len = info->buf_len;
			// also store it at other location that gp2 uses if data is too large to crunch,
			// for example, when gp2video is used.
			*pFileDataLen2 -= *pFileDataLen;
			*pFileDataLen = info->buf_len;
			*pFileDataLen2 += *pFileDataLen;
#ifdef TEST
			sprintf(strbuf, "  Extended data length: %d\n", ext_len);
			LogLine(strbuf);
#endif
		}
	}	
}


void __near SPDFHook(void)
{
	restore_gp2string(ID_PERFSAVED);
	if (opt_log_perf) {
		GP2FileInfo *info = *ppFileInfo;
		// First dump track segments to file
		GP2LapRecTrack *trackrec = (GP2LapRecTrack*) Log_GetRecBuf(GP2LRTYPE_TRACK);
		if (trackrec)
			DumpTrackSegData(trackrec->nr, trackrec->csum, trackrec->name);

		if (info && GP2_FILE_TYPE(info->type) == GP2_FILETYPE_PERF) {
			GP2PAxInfo *prf_info = (GP2PAxInfo*) info->buf_ptr;
			long ext_len = 0;
			BYTE *prf_data = PrfLog_GetPerfData(&ext_len);
			dwOldFileBufLen = info->buf_len;
			// Put version in useless time jiffies
			prf_info->time_jifs = GP2LOG_PERF_VERSION;
			// Put track checksum into unused data_start field
			prf_info->data_start = trackrec ? trackrec->csum : 0;

			if (prf_data && ext_len>0 && (dwOldFileBufLen+ext_len < GP2_EXTPERF_MAXLEN-100)) {
				int setup_len = sizeof(GP2CSx);
				GP2CSx *prf_setup = pCurrentCS ? GetCarSetup(GP2_CAR_ID(pCurrentCS->id)) : pCarSetups-2;
				BYTE *save_buf = PrfLog_GetPerfSaveBuf();	// own save buffer, as gp2's buffer might be too small
				BYTE *append_ptr = save_buf + dwOldFileBufLen;
				memcpy(save_buf, info->buf_ptr, dwOldFileBufLen);	// gp2's perf data
				info->buf_ptr = save_buf;
				// append my perf data
//				ext_len = sizeof(GP2LapRecPerf) * (*((short*)info->buf_ptr));
				APPEND_DATA(append_ptr,info->buf_len,prf_data,ext_len);
				// also append the car setup
				APPEND_DATA(append_ptr,info->buf_len,prf_setup,setup_len);
				// append old length and sig
				APPEND_DWORD(append_ptr,info->buf_len,dwOldFileBufLen);
				APPEND_DWORD(append_ptr,info->buf_len,GP2LOGSIG);
				if (info->max_len < info->buf_len)
					info->max_len = info->buf_len;
				if (info->min_len > info->buf_len)
					info->min_len = info->buf_len;
				LogLine("Extending perf data\n");
				set_gp2string(ID_PERFSAVED, "\x11""GP2Lap Extended Performance Lap Data Saved");
			}
		}
	}
}


// Log common session data
static void LogSessionData(GP2LapRecSession *rec)
{
	static DWORD track_last_csum = 0;
	DWORD *pCfgULong;
	
	if (ison(log_flags, LOGF_INCOMPATIBLE))
		on(rec->flags, LSF_INCOMPATIBLE);
	rec->mode = *pSessionMode;
	rec->frame_time = (BYTE) *pFrameTime;
	rec->opt_perc = (BYTE) *pRaceDistPerc;
	setbits(rec->flags, LSF_SH, *pSteeringHelp);
	setbits(rec->flags, LSF_OLH, *pOppLockHelp);
	// TODO: get these
	rec->opt_fail = 0;
	rec->future[0] = 0;
	rec->future[1] = 0;
	rec->future[2] = 0;
	rec->future[3] = 0;
	ResetStats();
	off(log_flags, LOGF_LOGPO | LOGF_LOGPERF | LOGF_MAPUSED | LOGF_EXTLOAD | LOGF_LAPLOAD | LOGF_NEWLOAD);
	if (opt_hof25) {
		on(hof25_flags, HOF25F_NEXTLAPVALID);
		off(hof25_flags, HOF25_F4USED);
	}

	sprintf(strbuf2, " %s", GP2_GetSessionString(*pSessionMode));
	
	// Spa '98
	if (opt_spa98) {
		yspd = 0x8000;
		yspd_up = TRUE;
		spa98timer = 30*25;
	}
	if (track_last_csum != dwTrackChecksum) {
		track_last_csum = dwTrackChecksum;
		OnTrackChanged();				// give track map a chance to do stuff
	}

	pCfgULong = GetCfgULong("MenuTrackMap");
	if (pCfgULong && *pCfgULong) {
		GP2LapRecTrack *trackrec = (GP2LapRecTrack*) Log_GetRecBuf(GP2LRTYPE_TRACK);
		DumpTrackSegData(trackrec->nr, trackrec->csum, trackrec->name);
	}
}



// Start of session
void __near SOSHook(void)
{
	GP2LapRecSession *rec = (GP2LapRecSession*) Log_GetRecBuf(GP2LRTYPE_SESSION);
	ResetLogs();
	strcpy(strbuf, "Start of session: ");
	strbuf2[0] = 0;
	if (rec) {
		rec->flags = 0;
		LogSessionData(rec);
	}
	strcat(strbuf, strbuf2);
	strcat(strbuf, "\n");
	LogLine(strbuf);
	if (opt_hof25) {
		off(hof25_flags, HOF25F_MUSTSLOWDOWN);
		on(hof25_flags, HOF25F_SLOWEDDOWN);
	}
}



// Load of session
void __near LOSHook(void)
{
	GP2LapRecSession *rec = (GP2LapRecSession*) Log_GetRecBuf(GP2LRTYPE_SESSION);
	if (isoff(log_flags, LOGF_LAPLOAD))
		ResetLogs();
	strcpy(strbuf, "Load of session: ");
	strbuf2[0] = 0;
	if (rec) {
		rec->flags = LSF_LOADED;
		if (ison(log_flags, LOGF_EXTLOAD)) {
			GP2LapRecTrack *curtrackrec = (GP2LapRecTrack*) Log_GetRecBuf(GP2LRTYPE_TRACK);
			BOOL prev_loaded = ison(sesrecbak.flags, LSF_LOADED);
			// inherit extended load
			if (!prev_loaded || ison(sesrecbak.flags, LSF_EXTLOAD))
				on(rec->flags, LSF_EXTLOAD);
			// inherit extended load with hof25
			if (ison(initrecbak.opt_flags, LIOF_HOF25) && (!prev_loaded || ison(sesrecbak.flags, LSF_HOF25LOAD)) )
				on(rec->flags, LSF_HOF25LOAD);
			if (ison(log_flags, LOGF_LAPLOAD))
				on(rec->flags, LSF_LAPLOAD);
			// inherit new load
			if (ison(log_flags, LOGF_NEWLOAD) && (!prev_loaded || ison(sesrecbak.flags, LSF_NEWLOAD)))
				on(rec->flags, LSF_NEWLOAD);
			if (trackrecbak.nr != curtrackrec->nr ||
				trackrecbak.slot != curtrackrec->slot ||
				trackrecbak.csum != curtrackrec->csum ||
				trackrecbak.tyre_wear_factor && trackrecbak.tyre_wear_factor != curtrackrec->tyre_wear_factor
			   )
				on(log_flags, LOGF_INCOMPATIBLE);
		}
		LogSessionData(rec);
	}

	strcat(strbuf, strbuf2);
	strcat(strbuf, "\n");
	LogLine(strbuf);
	if (opt_hof25) {
		on(hof25_flags, HOF25F_MUSTSLOWDOWN);
		off(hof25_flags, HOF25F_SLOWEDDOWN);
#ifdef TEST
		strcpy(strbuf, "Testing slowdown...");
#endif
		prev_speed = 0;
		slowdown_count = 0;
	}
}


static DWORD GetSessionTime(void)
{
	return *pCurTime - *pSesStartTime;
}


static void LogLapData(GP2LapRecLap *rec)
{
	int w;
	WORD avg_po = 0;
	WORD max_po = 0;
	WORD slomo_time = 0;
	if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
		avg_po = GetAvgPO();
		max_po = CalcPO(max_frame_time);
		slomo_time = (WORD)((*pFrameTime * num_slomo_frames * 100) / 256);
	}
//	BYTE *ws = pWheelStructs;
	rec->car_id = pCurrentCS->id;
	rec->nr = pCurrentCS->lapNr;
	rec->race_pos = (pCurrentCS->racePos >> 1) + 1;
	rec->qual_pos = GetFastLapPos(pCurrentCS->id);
	if (ison(rec->flags, LLF_PIT)) {
		rec->ses_time = GetSessionTime();
		// For a pit stop, Frank's lap time is set either before or after the car enters the pit,
		// so it's a bit useless to log it. Create own time.
		rec->lap_split1 = 0x10000000;
		rec->lap_split2 = 0x10000000;
		rec->lap_time = *pCurTime - pCurrentCS->timeLapStart;	// without correction, but that's no problem
	} else {
		if (*pSessionMode & 0x80)
			rec->ses_time = pRaceTimes[(GP2_CAR_ID(pCurrentCS->id) - 1)];
		else
			rec->ses_time = GetSessionTime();
		rec->lap_split1 = pCurrentCS->timeLastSpl1;
		rec->lap_split2 = pCurrentCS->timeLastSpl2;
		rec->lap_time = pCurrentCS->timeLast;
	}
	rec->weight = pCurrentCS->weight;
	rec->fuel_laps = pCurrentCS->fuelLoadLaps;
	rec->avg_po = avg_po;
	rec->max_po = max_po;
	rec->slomo_time = slomo_time;
	rec->grip = pCurrentCS->grip;
	for (w=0; w<GP2_NUM_WHEELS; w++)
		rec->tyre_wear[w] = pCurrentCS->tyreWear[w];
	rec->aid_chg_cnt = 0;
	if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
		rec->aid_chg_cnt = aid_chg_cnt;
		setbits(rec->flags, LLF_OFFTRACK, dirty);
		setbits(rec->flags, LLF_DETAILCHANGE, dlevel_chg);
		setbits(rec->flags, LLF_MAPUSED, ison(log_flags, LOGF_MAPUSED));
	}
	// TODO: get these
	rec->plank_wear[0] = 0;
	rec->plank_wear[1] = 0;

	if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
		sprintf(strbuf2, " car %2d, pos %2d, time %s, PO: %3d/%3d%%, %.1fs slomo",
				GP2_CAR_ID(pCurrentCS->id),
				(pCurrentCS->racePos >> 1) + 1,
				GP2_TIMESTR(pCurrentCS->timeLast),
				(int) avg_po,
				(int) max_po,
				(double) slomo_time / 100.0);
		if (dlevel_chg) {
			strcat(strbuf2, " (DC)");
		}
		ResetStats();
		on(log_flags, LOGF_LOGPO);
		off(log_flags, LOGF_MAPUSED);
	}
}



// Start of lap (actually logged as a 'end of lap')
void __near SOLHook(void)
{
	if (pCurrentCS && (opt_log_cc || GP2_CAR_ISPLAYER(pCurrentCS->id))) {
		int w;
		if (!*pIsReplay) {
			GP2LapRecLap *rec = (GP2LapRecLap*) Log_GetRecBuf(GP2LRTYPE_LAP);
			if (rec) {
				char mode = *pSessionMode;
				rec->flags = 0;
				// First, check some hof25 mandatory slowdown
				if (opt_hof25 && mode != 0x80 && GP2_CAR_ISPLAYER(pCurrentCS->id)) {
					if (isoff(hof25_flags, HOF25F_NEXTLAPVALID)) {
						on(rec->flags, LLF_NOSLOWDOWN);	// DQ for hof2.5
						START_BLACK_FLAG(pCurrentCS, 5);
						// May log next lap again
						on(hof25_flags, HOF25F_NEXTLAPVALID);
						off(hof25_flags, HOF25F_MUSTSLOWDOWN);
					} else if (ison(hof25_flags, HOF25F_MUSTSLOWDOWN) && isoff(hof25_flags, HOF25F_SLOWEDDOWN)) {
						off(hof25_flags, HOF25F_NEXTLAPVALID);
						LogLine("[HOF2.5] Warning: No slowdown before start of lap! Lap will be DQ.\n");
						ForceShowLogPage();
					}
					off(hof25_flags, HOF25F_MUSTSLOWDOWN);
					slowdown_count = 0;
					setbits(rec->flags, LLF_F4USED, ison(hof25_flags, HOF25_F4USED));
					off(hof25_flags, HOF25_F4USED);
				}

				if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
					sprintf(strbuf, "End lap %2d:", pCurrentCS->lapNr-1);
					strbuf2[0] = 0;
				}
				LogLapData(rec);
				if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
					Log_AddPlayerRec(rec, rec->hdr.type);
					strcat(strbuf, strbuf2);
					strcat(strbuf, "\n");
					LogLine(strbuf);
				}
			}
		}	// replay
#ifdef TEST
		if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
			if (*pIsReplay) {
				sprintf(strbuf, "End lap %2d during replay\n", pCurrentCS->lapNr-1);
				LogLine(strbuf);
			}
			if (*pIsAccTime) {
				sprintf(strbuf, "End lap %2d during accelerated time\n", pCurrentCS->lapNr-1);
				LogLine(strbuf);
			}
		}
#endif
				// Remove wheel hilighting if they're still visible
		for (w=0; w<GP2_NUM_WHEELS; w++)
			if (!IS_DESTRUCTED_WHEEL(pCurrentCS, w))
				UNHILIGHT_WHEEL(pCurrentCS, w);
	}
}



// Pit in (actually log a lap record with pit flag set)
void __near PIHook(void)
{
	if (pCurrentCS && (opt_log_cc || GP2_CAR_ISPLAYER(pCurrentCS->id)) && (*pSessionMode & 0x80)) {
		if (!*pIsReplay) {
			GP2LapRecLap *rec = (GP2LapRecLap*) Log_GetRecBuf(GP2LRTYPE_LAP);
			if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
				sprintf(strbuf, "Pit in  %2d:", pCurrentCS->lapNr-1);
				strbuf2[0] = 0;
			}
			if (rec) {
				rec->flags = LLF_PIT;
				LogLapData(rec);
			}
			if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
				Log_AddPlayerRec(rec, rec->hdr.type);
				strcat(strbuf, strbuf2);
				strcat(strbuf, "\n");
				LogLine(strbuf);
			}
		}
	}
}



// Pit out
void __near POHook(void)
{
	if (!*pIsReplay && pCurrentCS && (opt_log_cc || GP2_CAR_ISPLAYER(pCurrentCS->id)) && (*pSessionMode & 0x80)) {
		GP2LapRecPitOut *rec = (GP2LapRecPitOut*) Log_GetRecBuf(GP2LRTYPE_PITOUT);
		if (rec) {
			rec->car_id = pCurrentCS->id;
			rec->nr = pCurrentCS->lapNr;
			rec->race_pos = (pCurrentCS->racePos >> 1) + 1;
			rec->qual_pos = GetFastLapPos(pCurrentCS->id);
			rec->ses_time = GetSessionTime();
			rec->weight = pCurrentCS->weight;
			rec->fuel_laps = pCurrentCS->fuelLoadLaps;
			rec->tyre_set = arCurTyreSet[GP2_CAR_ID(pCurrentCS->id)-1];
		}
		if (GP2_CAR_ISPLAYER(pCurrentCS->id)) {
			sprintf(strbuf, "Pit out %2d: car %2d, pos %2d\n",
					pCurrentCS->lapNr-1,
					GP2_CAR_ID(pCurrentCS->id),
					(pCurrentCS->racePos >> 1) + 1);
			LogLine(strbuf);
			Log_AddPlayerRec(rec, rec->hdr.type);
			ResetStats();
		}
	}
	on(log_flags, LOGF_LOGPO);
}


// Fatal crash
void __near RetireHook(void)
{
	if (!*pIsReplay) {
		if (pCurrentCS && (opt_log_cc || GP2_CAR_ISPLAYER(pCurrentCS->id))) {
			GP2LapRecRetire *rec = (GP2LapRecRetire *) Log_GetRecBuf(GP2LRTYPE_RETIRE);
			if (rec) {
				rec->car_id = pCurrentCS->id;
				rec->lap_nr = pCurrentCS->lapNr;
				rec->race_pos = (pCurrentCS->racePos >> 1) + 1;
				rec->qual_pos = GetFastLapPos(pCurrentCS->id);
				rec->ses_time = GetSessionTime();
				rec->segment = pCurrentCS->pSeg ? pCurrentCS->pSeg->nr : -1;
			}
			if (GP2_CAR_ISPLAYER(pCurrentCS->id))
				Log_AddPlayerRec(rec, rec->hdr.type);
			if (!opt_spa98) {
				sprintf(strbuf, "Car %d crashed\n", GP2_CAR_ID(pCurrentCS->id));
				LogLine(strbuf);
			}
		}
	}
}


static void MaybeLogSpecial(void)
{
	BYTE drv_aids = pCurrentCS->drvAids;
	WORD surface = 0;
	int i = 0;

	if (*pIsAccTime)
		return;
	
	if (opt_spa98 && spa98timer > 0) {
		spa98timer--;
		if ((*pSessionMode & 0x80)) {
			if (pCurrentCS->lapNr >= 1) {
				if (yspd_up) {
					yspd++;
					if (yspd == 0x8000)
						yspd = 0x8001; // prevent divide by zero in gp2's code
					else if (yspd > 0x8040)
						yspd_up = FALSE;
				} else {
					yspd--;
					if (yspd == 0x8000)
						yspd = 0x7fff; // prevent divide by zero in gp2's code
					else if (yspd < 0x7fc0)
						yspd_up = TRUE;
				}
				pCurrentCS->ySpeed = yspd<<16;
			} else
				yspd_up = pCurrentCS->racePos < 20;		// start to go backwards if race pos < 10
		}
	}
	
	for (i=GP2_NUM_WHEELS-1; i>=0; i--) {
		BYTE *ws = pWheelStructs+GP2_WHEELSTRUCT_SIZE*i;
		BYTE stype = ws[0x2d];
		if (!*pIsReplay) {
			surface <<= 3;
			surface |= stype & 7;
		}
		if (opt_hof25) {
			char mode = *pSessionMode;
			if (mode != 0x80) {
				WORD speed = pCurrentCS->speed;
				if (GP2_SURF_VERYOFFTRACK(stype) &&
					pCurrentCS->pSeg &&
					(isoff(tr_flags, TRF_CUSTOM) || ison(tr_flags, TRF_HASSEGRANGES))
				   ) {
					// only for hof2.5 in unlimited practice
					WORD seg = pCurrentCS->pSeg->nr;
					int sr = 0;
					const SegRange *ranges = GetCurSegInfo(*pTrackNr, *pSteeringHelp != 0, ison(tr_flags, TRF_CUSTOM));
					while (ranges[sr].from) {
						if (seg >= ranges[sr].from && seg < ranges[sr].to) {
							if (opt_hof25) {
#ifdef TEST
								START_BLACK_FLAG(pCurrentCS, 10);
#else
								DESTRUCT_WHEEL(pCurrentCS, i);
#endif
							}
							HILIGHT_WHEEL(pCurrentCS, i);
						}
						sr++;
					}
				}
				if (!*pIsReplay) {
					// Test if car slowed down
					if (ison(hof25_flags, HOF25F_MUSTSLOWDOWN) && isoff(hof25_flags, HOF25F_SLOWEDDOWN)) {
						if (speed < prev_speed) {
							if (++slowdown_count > HOF25_SLOWDOWNFRAMES) {
#ifdef TEST
								LogLine("Slowed down\n");
#endif
								on(hof25_flags, HOF25F_SLOWEDDOWN);
								slowdown_count = 0;
							}
						}
					}
					prev_speed = speed;
				}
			}
		}
		if (GP2_SURF_OFFTRACK(stype) && !*pIsReplay) {
			dirty = TRUE;
			dirty_w[i] = 1;
			dirtcount[i]++;
		}
	}
	if (!*pIsReplay) {
		if (!drv_aids_set)
			drv_aids_set = TRUE;
		else if (cur_drv_aids != drv_aids) {
			IncNibbleCounters(cur_drv_aids ^ drv_aids, &aid_chg_cnt);
		}
		cur_drv_aids = drv_aids;
		if (ison(drv_aids, 1<<3))
			on(hof25_flags, HOF25_F4USED);
		if (*pDetailLevel != cur_dlevel)
			dlevel_chg = TRUE;
		cur_dlevel = *pDetailLevel;
	}
}



////////////////////////////////////////////////////////////////////////////////////////////////

// Fast forward in replay
static BOOL doFastMotion = FALSE;

void ToggleFastMotion(void)
{
#ifndef TEST
	if (*pIsReplay) {
#endif
		doFastMotion = !doFastMotion;
		*pFrameWaitCode = doFastMotion ? 0x9090 : 0xf372;
#ifndef TEST
	}
#endif
}


#ifdef SOCKCLNT
void DoSendPacket()
{
	int w;
	PodCarStruct podcs;

	podcs.time = *pCurTime;
	podcs.pos[0] = GP2_XY_FACTOR * (double)pCurrentCS->xPos;
	podcs.pos[1] = GP2_XY_FACTOR * (double)pCurrentCS->yPos;
	podcs.pos[2] = GP2_XY_FACTOR * (double)pCurrentCS->zPos;
	podcs.angle[0] = 0;//(double)pCurrentCS->xAngle;
	podcs.angle[1] = 0;//(double)pCurrentCS->yAngle;
	podcs.angle[2] = ((double)pCurrentCS->zAngle * PI) / 32768.0;
	podcs.speed[0] = GP2_SPEED_FACTOR * (double)((long)pCurrentCS->xSpeed >> 16);
	podcs.speed[1] = GP2_SPEED_FACTOR * (double)((long)pCurrentCS->ySpeed >> 16);
	podcs.speed[2] = GP2_SPEED_FACTOR * (double)((long)pCurrentCS->zSpeed >> 16);
	podcs.carspeed = GP2_SPEED_FACTOR * (double)pCurrentCS->speed;
	for (w=0; w<GP2_NUM_WHEELS; w++) {
		podcs.wheelSpeed[w] = GP2_SPEED_FACTOR * (double)((long)pCurrentCS->wheelSpeed[w]>>8);
	}
	podcs.gear = (char)pCurrentCS->gear;
	podcs.revs = pCurrentCS->revs;
	podcs.steer = ((double)pCurrentCS->convSteer*-35.0*PI)/(180.0*6336.0);

	sendDataPacket(&podcs, sizeof(PodCarStruct));
}
#endif


// End of frame
void __near EOFHook(void)
{
	pCurrentCS = NULL;
	if (ppPlayerCS && *ppPlayerCS)
		pCurrentCS = *ppPlayerCS;
		
	if (pCurrentCS && GP2_CAR_ISPLAYER(pCurrentCS->id)) {
		MaybeLogSpecial();
#ifdef SOCKCLNT
		if (!*pIsAccTime)
			DoSendPacket();
#endif
	}

	if (PAGEISACTIVE(PAGE_MAP)) {
		on(log_flags, LOGF_MAPUSED);
		DrawTrackMap();
	}
	if (PAGEISACTIVE(PAGE_CARINFO)) {
		DrawCarInfo();
	}
	
#ifdef TEST
	if (*ppSelectedCS) {
		GP2Car *pSelCar = *ppSelectedCS;
		sprintf(strbuf, "angle %6ld", (long)pSelCar->zAngle);
		writefnt(strbuf, &fntarrn6, &picbuf, 0, 8, 31);
	}
#endif

	if (ison(log_flags, LOGF_LOGPO) && !*pIsReplay && !*pPaused) {
		// Process number of frames, total frame time and maximum frame time
		DWORD frTime = *pRealFrameTime & 0x07ff;
		DWORD cut_off;
		total_frames++;
		total_frame_time += frTime;
		cut_off = 3*(total_frame_time/total_frames);
		if ((frTime>*pFrameTime) && (frTime<cut_off))
			num_slomo_frames++;
		if (frTime>max_frame_time && (frTime<cut_off))
			max_frame_time = frTime;
	}
#ifndef TEST
	if (doFastMotion && !*pIsReplay) {
		doFastMotion = FALSE;
		*pFrameWaitCode = 0xf372;
	}
#endif
}


#define DISTFACTOR	495160
static BYTE perflap = 0;

// Perf data
void __near PrfHook(void)
{
	if (opt_log_perf && pCurrentCS) {
		// pPerfFlag is cleared by GP2 when the car crosses the start line and perfing must be started
		if (*pPerfFlag) {
			off(log_flags, LOGF_LOGPERF);
			perflap = 0;
		} else if (isoff(log_flags, LOGF_LOGPERF)) {
			on(log_flags, LOGF_LOGPERF);
			perflap = pCurrentCS->lapNr;
		}
		if (ison(log_flags, LOGF_LOGPERF) && perflap == pCurrentCS->lapNr) {
			GP2LapRecPerf *rec = PrfLog_GetPerfRecBuf();
			if (rec) {
				int w;
				rec->pos_x = pCurrentCS->xPos;
				rec->pos_y = pCurrentCS->yPos;
				for (w=0; w<GP2_NUM_WHEELS; w++)
					rec->wheel_speed[w] = (BYTE) ((pCurrentCS->wheelSpeed[w] >> 7) & 0xff);
			}
		}
	}
}



// Entering cockpit
void __near ECPHook(void)
{
#ifdef SOCKCLNT
	DWORD *pULongCfg = NULL;
	char *pStrCfg = NULL;
	LogLine("Trying to connect to server...\n");
	pStrCfg = GetCfgString("podIP");
	if (pStrCfg && strlen(pStrCfg) > 0) {
		WORD podPort = 5001;
		pULongCfg = GetCfgULong("podPort");
		if (pULongCfg)
			podPort = (WORD) *pULongCfg;
		if (connectToServer(pStrCfg, podPort)) {
			sprintf(strbuf, "  Successfully connected to server '%s' on port %d\n", pStrCfg, podPort);
			LogLine(strbuf);
		} else {
			sprintf(strbuf, "*** Unable to connect to server '%s' on port %d\n", pStrCfg, podPort);
			LogLine(strbuf);
		}
	} else {
		sprintf(strbuf, "*** No ip address given: please specify podIP in .cfg file\n");
		LogLine(strbuf);
	}
#endif
	InitTrackMap();
	if (keep_dist_unit)
		*pIsKPH = oldKPH;
}


// Leaving cockpit
void __near LCPHook(void)
{
#ifdef SOCKCLNT
	LogLine("Disconnecting from server\n");
	disconnectFromServer();
#endif
}


// Hook function pointers. Called from lammcall.
void (__near *fpInitGP2Code)(void) =	InitGP2Hook;
void (__near *fpSOLCode)(void) =		SOLHook;
void (__near *fpPICode)(void) =			PIHook;
void (__near *fpPOCode)(void) =			POHook;
void (__near *fpRetireCode)(void) =		RetireHook;
void (__near *fpTFLCode)(void) =		TFLHook;
void (__near *fpLDFCode)(void) =		LDFHook;
void (__near *fpSDFCode)(void) =		SDFHook;
void (__near *fpSPDFCode)(void) =		SPDFHook;
void (__near *fpSOSCode)(void) =		SOSHook;
void (__near *fpLOSCode)(void) =		LOSHook;
void (__near *fpECPCode)(void) =		ECPHook;
void (__near *fpLCPCode)(void) =		LCPHook;
void (__near *fpEOFCode)(void) =		EOFHook;
void (__near *fpPrfCode)(void) =		PrfHook;
