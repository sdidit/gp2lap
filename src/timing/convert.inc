#include "../pages.h"

#define RATL_START   0
#define RATL_BEHIND  0
#define RATL_LAPTIME 1
#define RATL_END     1

#define QATL_START   0
#define QATL_SECTORS 0
#define QATL_SECDIFF 1
#define QATL_IDEAL   2
#define QATL_END     2


#include "../gp2rmcod.h"

unsigned char *GP2_use_svga = NULL;
unsigned char *GP2_RaceMode = NULL;
struct gp2drivernames *GP2_DriverNames = NULL;
struct gp2order *GP2_LapsRunByCar = NULL;  // keine order sondern car als index
struct gp2array40 *GP2_CaridTeamTab = NULL;
struct gp2order *GP2_RaceOrder = NULL;
struct gp2order *GP2_FastestCars = NULL;

struct gp2timetable *GP2_RaceTimes = NULL;
struct gp2timetable *GP2_BestLaptimes = NULL;
struct gp2timetable *GP2_BestSplit1 = NULL;
struct gp2timetable *GP2_BestSplit2 = NULL;
struct gp2timetable *GP2_QualTimesComb = NULL;
struct gp2timetable *GP2_QualTimesSes1 = NULL;
struct gp2timetable *GP2_QualTimesSes2 = NULL;
struct singlecarstruct **pCarInView = NULL;
struct singlecarstruct **pPlayerCar = NULL;

unsigned short *GP2_NumCarsRunning = NULL;
unsigned long  *GP2_NumStarters = NULL;
unsigned short *GP2_LapsInThisRace = NULL;
struct gp2seg *GP2_MistSeg = NULL;
struct gp2cartable *GP2_Cars = NULL;
unsigned char *GP2_TranspTab = NULL;
unsigned char *pCameraViewSrc = NULL;
unsigned long *pRCRTable = NULL;

unsigned char *pRcvLnkCmd = NULL;
gp2sendslots *t_CmdsToBeSend = NULL;
unsigned long InCockpitNow = 0;         // set if in cockpit (race engine, otherwise 0)

//------ some cfg switches --------------
unsigned long SupressFastestLap = 0;    // by default: no
unsigned long atlAcrossTop = 0;    // by default: no

#include "timing/gp2struc.h"
#include "timing/timinpic.h"
#include "timing/codehook.inc"


//-----------------------------------------------------------------------

void FrankSoftDone(void);

void FrankSoftInit(void)
{
  //--- 9/98 ---------
  initfont( &fntarrn6 );
  initfont( &fntverd7fk );
  initfont( &fntverd7k );
  initfont( &fntgp2_8x8 );
  initfont( &fntarial20 );


  if (GetCfgULong("atlNoFastestLap"))
    SupressFastestLap = *GetCfgULong("atlNoFastestLap");

  if (GetCfgULong("atlSplit1Color"))      SetRBehindCol0(*GetCfgULong("atlSplit1Color"));
  if (GetCfgULong("atlSplit2Color"))      SetRBehindCol1(*GetCfgULong("atlSplit2Color"));
  if (GetCfgULong("atlLineColor"))        SetRBehindCol2(*GetCfgULong("atlLineColor"));
  if (GetCfgULong("atlAcrossTop"))        atlAcrossTop = *GetCfgULong("atlAcrossTop");
  if (GetCfgULong("atlMarkTopX"))         SetLaptimeTopCount(*GetCfgULong("atlMarkTopX"));
  if (GetCfgULong("logDebugScreenLines")) VisLogSetStrCount(*GetCfgULong("logDebugScreenLines"));
  if (GetCfgULong("DrawLines"))           SetCopySvgaLinesNum(*GetCfgULong("DrawLines"));

  if (GetCfgULong("LogDpmiInfo"))
    SetLogDpmiInfo(*GetCfgULong("LogDpmiInfo"));

  if (GetCfgULong("UseIpxLink"))
    UseIpxLink = *GetCfgULong("UseIpxLink");
  if (GetCfgULong("LogIpxInfo"))
    LogIpxInfo = *GetCfgULong("LogIpxInfo");
  if (GetCfgULong("LogIpxOutpackets"))
    LogIpxOutpackets = *GetCfgULong("LogIpxOutpackets");
  if (GetCfgULong("LogIpxInpackets"))
    LogIpxInpackets = *GetCfgULong("LogIpxInpackets");

  if (GetCfgULong("LogLnkCmdsIn"))
    LogLnkCmdsIn = *GetCfgULong("LogLnkCmdsIn");
  if (GetCfgULong("LogLnkCmdsOut"))
    LogLnkCmdsOut = *GetCfgULong("LogLnkCmdsOut");
  if (GetCfgULong("LogLnkCmdsProcess"))
    LogLnkCmdsProcess = *GetCfgULong("LogLnkCmdsProcess");

  if (GetCfgULong("ForcePMBanking"))
    ForcePMBanking = *GetCfgULong("ForcePMBanking");
  if (GetCfgULong("ForceLFBuffering"))
    ForceLFBuffering = *GetCfgULong("ForceLFBuffering");
  if (GetCfgULong("UseOldNetworkCode"))
    UseOldNetworkCode = *GetCfgULong("UseOldNetworkCode");

  if (UseIpxLink) {
    sprintf(strbuf, "- IpxLink: using %s network code\n", UseOldNetworkCode ? "OLD" : "NEW");
    LogLine(strbuf);
  }


  #ifdef TEST
    sprintf(strbuf, "- ForceLFBuffering == %s\n", ForceLFBuffering ? "yes" : "no");
    LogLine(strbuf);
  #endif

  atexit( FrankSoftDone );

} // FrankSoftInit()

//-----------------------------------------------------------------------

void FrankSoftDone(void)
{
  if (LogIpxInfo && LinkEstablished) {
    sprintf(strbuf, "- IpxLink: pcnt: %u lecb: %P lsz: %u nerr: %u mecb: %u mpck: %u me: %u\n",
      statsaddr->packetcount,
      statsaddr->lastecb,
      statsaddr->lastsize,
      statsaddr->neterrors,
      statsaddr->missedecbs,
      statsaddr->missedpackets,
      statsaddr->packetsfromme);
    LogLine(strbuf);
  }

} // FrankSoftDone()

//-----------------------------------------------------------------------

void __near _cdecl AHFAfterGp2Init(void)
// called after general gp2 init
{
   long *tmplongp;
   ulong *crtp = GetCfgULong("CarRemoveTime");

   // 12/98 Rene
   ulong *lsp = GetCfgULong("UseLocalizedStrings");
   init_gp2_version(lsp && *lsp);

#ifdef TEST
   sprintf(strbuf, "- GP2Lap __begtext at 0x%08X\n" \
                   "- GP2Lap end       at 0x%08X (%u bytes)\n", &__begtext, &end, &end-&__begtext);
   LogLine(strbuf);
   printf(strbuf); fflush(stdout);
   sprintf(strbuf, "- GP2CodeStart at 0x%08X\n", GP2_CodeStartAdr-0x10020);
   LogLine(strbuf);
   printf(strbuf); fflush(stdout);
#endif  // !

   //--- 10/98 --- Increase car remove time -----
   if ( crtp && *crtp ) {
         if ( *(ulong *)(IDAtoFlat(0x2B101)) == 20000) { // default of 20 secs?
       *(ulong *)(IDAtoFlat(0x2B101)) = *crtp;  // 3600000 == 1h
         sprintf(strbuf, "- CarRemoveTime set to %u\n", *crtp);
         LogLine(strbuf);
     }
   }

   //--- 9/98 ---- SET RMCODE PTR WITHIN MY IPX ESR ---
   if ( LinkEstablished ) {
     if (*ppDosRmCode && !((ulong)(*ppDosRmCode) & 0xFFF00000) ) {   // gp2's rmcode ptr set and poiting to low mem?
       // set the 16:16 adr of the gp2 rmcode within my esr
       *(ulong *)((ulong)(esrrmaddr) + (ulong)(EsrRmcodeOfs)) =
                       ( (DPMI_real_segment(*ppDosRmCode) << 16) +
                          DPMI_real_offset(*ppDosRmCode) );
     }
   }

  //--- 8/98 --- Don't del times in pits ---------
  memset(IDAtoFlat(0x1634a), 0x90, 17);  // suppress reseting timeLapStart & timeLast

  //--- 10/98 --- Show splits in race ---------
  //if ( GetCfgULong("SplitsInRace") && *GetCfgULong("SplitsInRace") )
  //--- 11/99 Ignore, because ATLBehind doesn't work properly with Splits diabled
  memset(IDAtoFlat(0x1611d), 0x90, 13);

#ifdef TEST
  //--- 10/98 --- dump the Strings ---------
  if ( GetCfgULong("LogAllStrings") && *GetCfgULong("LogAllStrings") )
    dump_all_gp2strings();
#endif

  //--- 10/98 --- chg some unk rcr vals ---------
  if (pRCRTable) {
    if (GetCfgULong("VGARcr1")) pRCRTable[0] = *GetCfgULong("VGARcr1");
    if (GetCfgULong("SVGARcr1")) pRCRTable[1] = *GetCfgULong("SVGARcr1");
    if (GetCfgULong("VGARcr2")) pRCRTable[2] = *GetCfgULong("VGARcr2");
    if (GetCfgULong("SVGARcr2")) pRCRTable[3] = *GetCfgULong("SVGARcr2");
    if (GetCfgULong("VGARcr3")) pRCRTable[4] = *GetCfgULong("VGARcr3");
    if (GetCfgULong("SVGARcr3")) pRCRTable[5] = *GetCfgULong("SVGARcr3");
  }

  if (*GetCfgULong("CatchTrackInfos"))
    CatchTrackInfos();  // try to catch Names from custom track files

  //--- 9/99 --- set fisheye zoom for cokpit mode (default: 256) ----
  tmplongp = GetCfgLong("CockpitZoom");
  if ( tmplongp && *tmplongp ) {
    memset(IDAtoFlat(0x3829d), 0x90, 5);  // wipeout max check for 0x1F00 of ChgViewPlane
    memset(IDAtoFlat(0x382a9), 0x90, 5);  // wipeout min check for 0x80 of ChgViewPlane
    if (*tmplongp > 0x1F00) *tmplongp = 0x1F00;
    else if (*tmplongp < 64) *tmplongp = 64;
    *(long *)(IDAtoFlat(0x3856d)) = *tmplongp;
    sprintf(strbuf, "- CockpitZoom set to %d\n", *tmplongp);
    LogLine(strbuf);
  }

  //--- 9/99 --- dexor confidential strings ---------
  dexor(&securehpmsg);  // "(%u hp)"


} // AHFAfterGp2Init()

void (__near _cdecl *fpAHFAfterGp2Init)(void) = AHFAfterGp2Init;


//-----------------------------------------------------------------------


//--- 9/99 --- before gp2 destroyes all best splittimes ----
//--- called at the end of each lap, similar to AHFCrossingSplit(0), but
//--- much earlier, before much of the calc's are done -> see IDA 0x160c6
#pragma aux AHFBefDestrBest parm [ESI] caller;
void __near _cdecl AHFBefDestrBest( struct singlecarstruct *actcar )
{
  static unsigned char id;
  id = actcar->car_id & 0x3F;

  if (actcar->lastlap_time < actcar->bestlap_time) {
    // ok, gp2 will overwrite actual best sector info's soon, so back them up
    sicprevbest[id][0] = actcar->bestlap_split1;
    sicprevbest[id][1] = actcar->bestlap_split2;
    sicprevbest[id][2] = actcar->bestlap_time;

    //-----------------------------------
    //---- stats for the currently, (but probably no longer) leading car ----
    //---- immer bezogen auf car, das gerade finishline passiert (id) ----
    //-----------------------------------
    if (!(*GP2_RaceMode & 0x80)) {
      GetBestSplitsSegs(&prevldrsplits[id][0], &prevldrsegs[id][0], QPOS2CARID(0), Carid2Carstruct(QPOS2CARID(0)));
      sicprevbest[id][3] = (unsigned long) QPOS2CARID(0);  // actual (probably previous) polesitter

/*
      if (actcar->car_id & 0x80) { // nur fuer human
        sprintf(strbuf, "prevldr: 0x%02x (%s %s %s -> %s)\n", sicprevbest[id][3],
                                                       myGetTimeString( &prevldrsegs[id][0] ),
                                                       myGetTimeString( &prevldrsegs[id][1] ),
                                                       myGetTimeString( &prevldrsegs[id][2] ),
                                                       myGetTimeString( &prevldrsplits[id][2] ));
        LogLine(strbuf);
      }
*/
    }
  }

  // die splitzeiten der eben beendeten runde retten
  prevlapsplits[id][0] = actcar->lastlap_split1;
  prevlapsplits[id][1] = actcar->lastlap_split2;
  prevlapsplits[id][2] = actcar->lastlap_time;

/*
  if (actcar->lastlap_split1 < idealsegs[id][0])
    idealsegs[id][0] = actcar->lastlap_split1;
  if (actcar->lastlap_split2 - actcar->lastlap_split1 < idealsegs[id][1])
    idealsegs[id][1] = actcar->lastlap_split2 - actcar->lastlap_split1;
  if (actcar->lastlap_time - actcar->lastlap_split2 < idealsegs[id][2])
    idealsegs[id][2] = actcar->lastlap_time - actcar->lastlap_split2;
*/

} // AHFBefDestrBest()

void (__near _cdecl *fpAHFBefDestrBest)(struct singlecarstruct *) = AHFBefDestrBest;


//-----------------------------------------------------------------------
