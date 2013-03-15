#include "timinstd.h"
#include "rbehind.h"
#include "rlaptime.h"

//----------------------------------------------------------------------

#include "atl.h"
#include "qatl.inc"

void OnATLReDraw(void);

//----------------------------------------------------------------------


//---- Zusatzinfos fuer ATL pages Verwaltung ----
unsigned long FIRSTPOS = 0;  // fr Anzeige in der Tabelle
unsigned long LASTPOS = 4;

//---- always order-indexed!!! ----
unsigned long CarCount = 0; // kann eigentlich weg
unsigned char LastCarPosCrossing;


//----------------------------------------------------------------------

void OnActivateATL(void)
{
  UpdateBoard();
}

//----------------------------

void OnATLKeyHit(KeyEvent *mks)   // KOMMT HIER WIEDER WEG!
{
  switch (mks->scancode) {

    case KEY_TAB :
                  SelectNextPage(*GP2_RaceMode);
                  UpdateBoard();
                  break;

    case KEY_9:
                  if (mks->lctrlactive && GetKbdHandlerCnt() > 1) {
                    // only if we're the in multi page mode and CTRL active
                    PAGESETOFF(PAGE_ATTHELINE);
                    RemoveKbdHandler(&OnATLKeyHit);
                    KeyEventClear(mks);
                    // a bit clumsy, but it works...
                    mymemset(&picbuf, 0, sizeof(picbuf));
                    ReDrawAllPages(&OnATLReDraw);
                  }
                  else {
                    switch (LASTPOS-FIRSTPOS) {
                      case 4  :
                      case 9  : LASTPOS += 5;  break; // board with 2 tabs
                      case 14 : LASTPOS -= 10; break; // board with 3 tabs (back to 1 tab)
                    }
                    UpdateBoard();
                  }
                  break;

    case KEY_TICK :       // '^'
                  if (PAGEISACTIVE(PAGE_ATTHELINE)) {
                    // Rene 12-12-99: support shift+tick
                    if (*GP2_RaceMode & 0x80) {  // in race
                      int board_size = LASTPOS-FIRSTPOS+1;
                      if (mks->lshiftactive) {
                        if (FIRSTPOS < board_size) {
                          FIRSTPOS = 30-board_size;
                          LASTPOS = 29;
                        } else {
                          FIRSTPOS -= board_size;
                          LASTPOS -= board_size;
                        }
                      } else {
                        FIRSTPOS += board_size;
                        LASTPOS += board_size;
                        if (FIRSTPOS > 30-board_size) {
                          FIRSTPOS = 0;
                          LASTPOS = board_size-1;
                        }
                      }
                        /* old:
                      switch (LASTPOS-FIRSTPOS) {

                        case 4  : // board with one tab
                                  FIRSTPOS += 5;
                                  LASTPOS += 5;
                                  if (FIRSTPOS > 25) {
                                    FIRSTPOS = 0;
                                    LASTPOS = 4;
                                  }
                                  break;

                        case 9  : // board with 2 tabs
                                  FIRSTPOS += 10;
                                  LASTPOS += 10;
                                  if (FIRSTPOS > 20) {
                                    FIRSTPOS = 0;
                                    LASTPOS = 9;
                                  }
                                  break;

                        case 14 : // board with 3 tabs
                                  FIRSTPOS += 15;
                                  LASTPOS += 15;
                                  if (FIRSTPOS > 15) {
                                    FIRSTPOS = 0;
                                    LASTPOS = 14;
                                  }
                                  break;
                      }  // switch()
                      */
                    }  // was in race
                    else {  // not in race

                      //--------------------------------------------
                      //---- die '^' umschaltung fuer qualstats ----
                      //--------------------------------------------

                      if ((tablepage[0 /*non-race*/]==QATL_SECTORS) || (tablepage[0]==QATL_SECDIFF)) {
                        //---- change opponent car ----
                        //---- oppcar durch plaz.reihenfolge bestimmen...

                        if (mks->lshiftactive) {
                          if (qwhatopp[0] == AGAINSTSEL) { // sind gerade im indiv. opp mode
                            if (Carid2Qualpos(qwhatopp[2]) == 0)
                              qwhatopp[0] = AGAINSTYOU;     // wieder yourself als opp
                            else {
                              qwhatopp[1] = Carid2Qualpos(qwhatopp[2]);
                              qwhatopp[1]--;
                              qwhatopp[2] = QPOS2CARID(qwhatopp[1]);
                            }
                          }
                          else {
                            if (qwhatopp[0]-- == AGAINSTLDR) { // waren wir Against ldr?
                              //--- 9/99 test ------------
                              qwhatopp[0] = AGAINSTSEL;
                              qwhatopp[1] = 25;                                // Position
                              qwhatopp[2] = QPOS2CARID(25); // car_id dahin
                            }
                          }
                        }  // lshift active
                        else {
                          if (qwhatopp[0] == AGAINSTSEL) { // sind gerade im indiv. opp mode
                            if (Carid2Qualpos(qwhatopp[2]) >= 25)
                              qwhatopp[0] = AGAINSTLDR;     // wieder leader als opp
                            else {
                              qwhatopp[1] = Carid2Qualpos(qwhatopp[2]);
                              qwhatopp[1]++;
                              qwhatopp[2] = QPOS2CARID(qwhatopp[1]);
                            }
                          }
                          else {
                            qwhatopp[0]++;
                            if (qwhatopp[0] == AGAINSTSEL) { // eben Against selected betreten?
                              qwhatopp[1] = 0;                                 // Position
                              qwhatopp[2] = QPOS2CARID(0); // car_id dahin
                            }
                          }
                        }  // no lshift
                      }
                    }

                    UpdateBoard();

                  }
                  break;

  }

} // OnATLKeyHit()

//----------------------------------------------------------------------

void InitMyTiming(void)
{
  static unsigned char i;

  mymemset(&picbuf, 0, sizeof(picbuf));

  InitMyLineOrder();

  //---- make my driver names --------
  MakeMyDrivers(GP2_DriverNames); // everytime

  //---- make some inits --------
  CarCount = 0;
  LastCarPosCrossing = 0;


  //--- 09/99 ----
  qwhatopp[0] = AGAINSTLDR;   // begin with leader as opp
  for (i = 0; i < 0x40; i++) { // reset certain times...
    sicprevbest[i][0] = sicprevbest[i][1] = sicprevbest[i][2] = 0x10000000;
    sicprevbest[i][3] = 0xFF;

    //---- die idealsegs[][] belegen, da game geladen wurde --------
    //idealsegs[i][0] = idealsegs[i][1] = idealsegs[i][2] = 0x10000000;
    GetBestSplitsSegs(&oppsplits/*dummy*/, &idealsegs[i][0], i, Carid2Carstruct(i));
    if (tmpcar = Carid2Carstruct(i)) {
      if (tmpcar->lastlap_split1 < idealsegs[i][0]) idealsegs[i][0] = CalcTimeDiff(tmpcar->lastlap_split1, 0);
      if (tmpcar->lastlap_split2-tmpcar->lastlap_split1 < idealsegs[i][1]) idealsegs[i][1] = CalcTimeDiff(tmpcar->lastlap_split2, tmpcar->lastlap_split1);
      if (tmpcar->lastlap_time-tmpcar->lastlap_split2 < idealsegs[i][2]) idealsegs[i][2] = CalcTimeDiff(tmpcar->lastlap_time, tmpcar->lastlap_split2);
    }
  }
    /*
    sprintf(strbuf, "toideal: carid==0x1C (%s %s %s)\n",
        myGetTimeString( &idealsegs[0x1C][0] ),
        myGetTimeString( &idealsegs[0x1C][1] ),
        myGetTimeString( &idealsegs[0x1C][2] ));
    LogLine(strbuf);
    */

  //------ inits for rbehind.h --------
  for (i = 0; i <= 2; i++)
    ldratsections[i] = 0;
  for (i = 0; i < 0x40; i++) {
    prevlapsplits[i][0] = myGP2_TIME_INV_MASK;
    prevlapsplits[i][1] = myGP2_TIME_INV_MASK;
    prevlapsplits[i][2] = myGP2_TIME_INV_MASK;
  }

  //------ do some output if neccessary --------
  UpdateBoard();


} // InitMyTiming()

//-------------------------------------------------------------

//----- someone's crossing start/finish line --------
#pragma aux AHFCrossingSplit parm [EAX] [ESI] caller;
void __near _cdecl AHFCrossingSplit( unsigned long section,
                                      struct singlecarstruct *actcar )
// šbergabe: section: (0 == line, 1 == split1, 2 == split2)
{
  static unsigned long ii, p;
  static unsigned char carpos;

  //----- get actual car's position -----------
  carpos = actcar->racepos >> 1;   // also 1. == 0x00, 2. == 0x01 usw.

  //----- erste car speichern, dass split passierte (fuer rbehind.h) ----
  if (carpos == 0) // natuerl. nur fuer leader
    switch (section) {
      case 0 : ldratsections[2] = actcar->car_id & 0x3F; break;
      case 1 : ldratsections[0] = actcar->car_id & 0x3F; break;
      case 2 : ldratsections[1] = actcar->car_id & 0x3F; break;
    }



  switch (section) {

    case 0 :

        if (*GP2_RaceMode & 0x80) {   // only for race
          //---- leader crossing start/finish line? ----------
          if (carpos == 0) {
            InitMyLineOrder();
            CarCount = 0;
          }

          //----- store some info about that car -----------
          myorder[carpos] = actcar->car_id;

          //---- make sure, that it's not a lapped car ------
          if (carpos == CarCount) {

            LastCarPosCrossing = carpos;
            CarCount++;

            //------ do some output if neccessary --------
            //UpdateBoard();

          } // make sure, not a lapped car


          // for every car crossing
          UpdateBoard();

        }
        break;  // passed line

    case 1 :

        UpdateBoard();
        break;  // passed 1st split

    case 2 :

        UpdateBoard();
        break;  // passed 2nd split

  } // switch(section)


  if ((*GP2_RaceMode & 0x80) == 0) {   // it's not the race...

    civid = actcar->car_id & 0x3f;
    // any new ideal sector times?
    switch (section) {
      case 0 : if ( myGP2_TIME_ISVALID(actcar->lastlap_time) &&
                    (actcar->lastlap_time - actcar->lastlap_split2 < idealsegs[civid][2]) )
                 idealsegs[civid][2] = actcar->lastlap_time - actcar->lastlap_split2;
               break;
      case 1 : if ( myGP2_TIME_ISVALID(actcar->lastlap_split1) &&
                    (actcar->lastlap_split1 < idealsegs[civid][0]) )
                 idealsegs[civid][0] = actcar->lastlap_split1;
               break;
      case 2 : if ( myGP2_TIME_ISVALID(actcar->lastlap_split2) &&
                    (actcar->lastlap_split2 - actcar->lastlap_split1 < idealsegs[civid][1]) )
                 idealsegs[civid][1] = actcar->lastlap_split2 - actcar->lastlap_split1;
               break;
    }

    UpdateBoard();
  }

} // AHFCrossingSplit()

//-------------------------------------------------------------

//----- Start of Session --------
void __near _cdecl AHFSOSHook(void)
{
  InitMyTiming();
}

//-------------------------------------------------------------

//----- Change of Position (also called when car crosses line) ------
void __near _cdecl AHFCOPHook(void)
{
  //--- update board with every chg of position ---
  UpdateBoard();

}

//-------------------------------------------------------------

unsigned char ReDrawBufATL[0x4b000];

void UpdateBoard(void)
{
  if (!PAGEISACTIVE(PAGE_ATTHELINE))
    return;

  mymemset(&picbuf, 0, sizeof(picbuf));  // den Bildschirmausgabebuffer leeren

  if (*GP2_RaceMode & 0x80) {  // only for race

     switch (tablepage[1]) {
       case RATL_BEHIND :
                MakeBehindBoard(*GP2_NumStarters-1);
                break;
       case RATL_LAPTIME :
                MakeLaptimeBoard(LastCarPosCrossing);
                break;
     }
  }
  else {  // not a race
     switch (tablepage[0]) {
       case QATL_SECTORS :
                UpdateLapStat(*pCarInView, QATL_SECTORS);
                break;
       case QATL_SECDIFF :
                UpdateLapStat(*pCarInView, QATL_SECDIFF);
                break;
       case QATL_IDEAL :
                MakeIdealBoard(*pCarInView);
                break;
     }

  }

  SaveThisPage(&ReDrawBufATL, &picbuf, GetCopySvgaLinesNum());
  ReDrawAllPages(&OnATLReDraw);

} // UpdateBoard()

//-------------------------------------------------------------

void OnATLReDraw(void)
{
  mypicinsertabove(&picbuf, &ReDrawBufATL, GetCopySvgaLinesNum());
} // OnATLReDraw()

//-------------------------------------------------------------

void __near _cdecl AHFCarInViewUpd(void)
//--- CarInView has changed (via camera or inside cockpit) -----
{
  UpdateBoard();
}

//-------------------------------------------------------------

#pragma aux AHFPitLaneArea parm [ESI] caller;
void __near _cdecl AHFPitLaneArea(struct singlecarstruct *car)
//--- "car" is entering/leaving the pitlane area (see car->unk_16A.b3) -----
{
  //if (car->unk_16A & 0x08)
  //  sprintf(strbuf, "car #%u entered pitlane area\n", car->car_id & 0x3f);
  //else
  //  sprintf(strbuf, "car #%u left pitlane area\n", car->car_id & 0x3f);
  //LogLine(strbuf);

  UpdateBoard();
}

//-------------------------------------------------------------

#pragma aux AHFGotoCockpit parm [EAX] caller;
#define CPITREPLAY  0x80
#define CPITENTER   0x00
#define CPITLEAVE   0x01

#include "gp2hook.h"
#include "gp2glob.h"

long int OrgSplit1Seg = -1;  // split segs found before modifying
long int OrgSplit2Seg = -1;

void __near _cdecl AHFGotoCockpit(unsigned long when)
{
  static unsigned long i;

  if ( (when & 0x03) == CPITENTER ) {

    InCockpitNow = 1;
    fntscrwidth = *GP2_use_svga ? 640 : 320;

    // entering
    sprintf(strbuf, "- Entering Cockpit now (%s%s)\n",
      *GP2_use_svga ? "svga" : "vga",
      (when & CPITREPLAY) ? ", replay" : "");
    LogLine(strbuf);


    //---- 11/99 war frueher in (geloeschter) AHFEnterCockpit() ----
    if (SessionImageChanged()) {
      #ifdef TEST
        LogLine("- SessionImage has changed\n");
      #endif
      InitMyTiming();   // i.e. start of session (reset all)
    }
    #ifdef TEST
      else {
        LogLine("- SessionImage has NOT changed\n");
      }
    #endif

    //---- PL001/25.03.99 experimental: move split segments -----
    //---- Rene, please leave the LogLine()'s in here

    if (*GetCfgULong("CatchSplitSegs")) {

      //sprintf(strbuf, "- Number of segments: 0x%04X\n", *pNumTrackSegs);
      //LogLine(strbuf);
      for (i = 0; i < *pNumTrackSegs; i++) {  // search current splits
        if (pTrackSegs[i].fl_split & 1) if (OrgSplit1Seg < 0) OrgSplit1Seg = i;
        if (pTrackSegs[i].fl_split & 2) if (OrgSplit2Seg < 0) OrgSplit2Seg = i;
      }

      if ( (Split1Seg > 0) && (Split2Seg > 0) ) {  // only if both valid

          // kill all split marks
          for (i = 0; i < *pNumTrackSegs; i++) pTrackSegs[i].fl_split &= 0xFC;

          // setting the two splits now
          for (i = 0; i < 8; i++)  // have to mark 8 segs
            if (Split1Seg+i < *pNumTrackSegs)  // watch out for overflow
              pTrackSegs[Split1Seg+i].fl_split |= 0x01;  // make split1 seg
          for (i = 0; i < 8; i++)  // have to mark 8 segs
            if (Split2Seg+i < *pNumTrackSegs)  // watch out for overflow
              pTrackSegs[Split2Seg+i].fl_split |= 0x02;  // make split2 seg

          sprintf(strbuf, "- Splits set to seg 0x%04X (nr=%04X) and 0x%04X (nr=%04X) of 0x%04X\n",
            Split1Seg, pTrackSegs[Split1Seg].nr, Split2Seg, pTrackSegs[Split2Seg].nr, *pNumTrackSegs);
          LogLine(strbuf);

      } else {
          sprintf(strbuf, "- Splits left at seg 0x%04X (nr=%04X) and 0x%04X (nr=%04X) of 0x%04X\n",
            OrgSplit1Seg, pTrackSegs[OrgSplit1Seg].nr, OrgSplit2Seg, pTrackSegs[OrgSplit2Seg].nr, *pNumTrackSegs);
          LogLine(strbuf);
      }
      Split1Seg = -1;  // mark this info dirty till next track file gets read
      Split2Seg = -1;
      OrgSplit1Seg = -1;
      OrgSplit2Seg = -1;

    } // if "CatchSplitSegs"...
    //---- PL001 ----

    //---- 09/99 some info about user car -------
    //sprintf(strbuf, "- Car Nr.%02u horsepower==%u\n",
    //  ((struct singlecarstruct *)(*pPlayerCar))->car_id & 0x3f,
    //  (((struct singlecarstruct *)(*pPlayerCar))->enginepower* 790) >> 14 );
    //LogLine(strbuf);


    // 11/99 Hier evtl. mal noch ein UpdateBoard() machen

  }
  else {
    // leaving
    InCockpitNow = 0;
    fntscrwidth = 640;  // always, when in menu mode (either 640x480x16 or ~x256)

    CreateSessionImage();

    sprintf(strbuf, "- Leaving Cockpit now (%s%s)\n",
      *GP2_use_svga ? "svga" : "vga",
      (when & CPITREPLAY) ? ", replay" : "");
    LogLine(strbuf);
  }

} // AHFGotoCockpit()

//-------------------------------------------------------------

void (__near _cdecl *fpAHFGotoCockpit)(unsigned long) = AHFGotoCockpit;

void (__near _cdecl *fpAHFCrossingSplit)(unsigned long, struct singlecarstruct *) = AHFCrossingSplit;
void (__near _cdecl *fpAHFSOSCode)(void) = AHFSOSHook;
void (__near _cdecl *fpAHFCOPCode)(void) = AHFCOPHook;
void (__near _cdecl *fpAHFCarInViewUpd)(void) = AHFCarInViewUpd;
void (__near _cdecl *fpAHFPitLaneArea)(struct singlecarstruct *) = AHFPitLaneArea;
