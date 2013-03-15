#include <stdio.h>
#include "timinpic.h"
#include "../fonts/myfntlib.h"
#include "gp2struc.h"
#include "timinstd.h"
#include "../dpmi.h"  // wegen mymemset()
#include "atl.h"  // wegen OnATLKeyHit
#include "rbehind.h"


// colors for times depending on sections
unsigned char rbcolsplit0 = 135; // some sort of light blue
unsigned char rbcolsplit1 = 125; // some sort of blue
unsigned char rbcolsplit2 = 151; // some sort of dark blue


//-------------------------------------------------------------

void SetRBehindCol0(unsigned char color) { rbcolsplit0 = color; }
void SetRBehindCol1(unsigned char color) { rbcolsplit1 = color; }
void SetRBehindCol2(unsigned char color) { rbcolsplit2 = color; }
unsigned char GetRBehindCol0(void) { return rbcolsplit0; }
unsigned char GetRBehindCol1(void) { return rbcolsplit1; }
unsigned char GetRBehindCol2(void) { return rbcolsplit2; }

//-------------------------------------------------------------

void MakeBehindHeader(void)
{
  insertpicat(_RBWHOLEBOARDX+_LEFTHDRX, _RBWHOLEBOARDY+_LEFTHDRY, LEFTHDRXDIM, LEFTHDRYDIM, &picattheline, &picbuf);
  insertpicat(_RBWHOLEBOARDX+_RIGHTHDRX, _RBWHOLEBOARDY+_RIGHTHDRY, RIGHTHDRXDIM, RIGHTHDRYDIM, &picbehind, &picbuf);
}

//------------------------------

void __near MakeBehindBoard(unsigned char carpos)
// parameters: carpos == info up to which position (25 == up to 26th place)
// external timing info from:
// - ldratsections[]
// - prevlapsplits[]
// problems:
// - wenn leader mit 30 sec vorsprung an Pit kommt, kriegen am standplatz alle anderen -1L (siehe etwa gp2_rb01.bmp)
{
  static unsigned long i, tab_yofs, tab_xofs;
  static unsigned char color;
  static DWORD timediff;
  static DWORD ldrracetime;
  static unsigned char lapsdown;
  static struct singlecarstruct *tmpcar, *ldrcar;
  static unsigned char ldrcarid, tmpcarid;


  //mymemset(&picbuf, 0, sizeof(picbuf));  // den Bildschirmausgabebuffer leeren

  MakeBehindHeader();

  //--- evtl Pfeil nach oben setzen ---
  if (FIRSTPOS > 0)
    insertpicat(_RBWHOLEBOARDX+_TABLETX+TABCOL1POSX, _RBWHOLEBOARDY+_TABLETY-(ARROWYDIM+1), ARROWXDIM,ARROWYDIM, &picarrowup, &picbuf);


  for (i = FIRSTPOS; (i <= carpos) && (i <= LASTPOS); i++) {  // bis zum aktuellen car

    //--- evtl neue table einblenden -------
    if ((i % 5) == 0) {
      if (atlAcrossTop) {
        tab_xofs = _RBWHOLEBOARDX+_TABLETX + ((i-FIRSTPOS)/5)*(TABLETXDIM+_RBTABLETSPACINGX);
        tab_yofs = _RBWHOLEBOARDY+_TABLETY;
      }
      else {
        tab_xofs = _RBWHOLEBOARDX+_TABLETX;
        tab_yofs = _RBWHOLEBOARDY+_TABLETY + ((i-FIRSTPOS)/5)*(TABLETYDIM+_RBTABLETSPACINGY);
      }
      insertpicat(tab_xofs, tab_yofs, TABLETXDIM, TABLETYDIM, &pictablet, &picbuf);
      tab_yofs += TABROW1POSY;  // den Schriftofs innerhalb des tabs bestimmen
    }

    _bprintf(atmp, sizeof(atmp), "%02d.", i+1);
    WRITEARRN6(&atmp, &picbuf, tab_yofs + (i%5)*(fntarrn6.ydim-1), tab_xofs+TABCOL1POSX, GP2WHITE);
    _bprintf(atmp, sizeof(atmp), "%-15.15s", GetMyDriverName(POS2CARID(i)) );
    WRITEARRN6(&atmp, &picbuf, tab_yofs + (i%5)*(fntarrn6.ydim-1), tab_xofs+TABCOL2POSX, HUMANCARID(POS2CARIDFULL(i)) ? GP2RED : GP2BLACK);
    // mit anderem Font
    //WRITEVERD7K(&atmp, &picbuf, tab_yofs + (i%5)*(fntverd7k.ydim-1), tab_xofs+TABCOL2POSX, HUMANCARID(POS2CARIDFULL(i)) ? GP2RED : GP2BLACK);

    //---- aktuelles car i der for-schleife ----
    tmpcar = Carid2Carstruct(POS2CARID(i)); // evtl NULL check noch
    tmpcarid = tmpcar->car_id & 0x3f;

    //---- ersten finden, der splitNr passiert hat ----
    ldrcar = Carid2Carstruct(ldratsections[tmpcar->splitNr]);
    if (ldrcar == NULL) // fuer race anfang, da ldratsections noch nicht gesetzt
      ldrcar = Carid2Carstruct(POS2CARID(0));  // den wirkl. momentan fuehrenden nehmen
    ldrcarid = ldrcar->car_id & 0x3f;


    // berechnen, wie lange er bis dahin unterwegs war
    ldrracetime = GP2_RaceTimes->car[ldrcarid-1];

    // kalks sind nur fuer noch nicht ueberrundete tmpcar's valid
    if (ldrcar->lapnumber > tmpcar->lapnumber) { // ldr ist schon wieder uebern strich
      ldrracetime -= ldrcar->lastlap_time;
      if (tmpcar->splitNr < 2)
        ldrracetime += prevlapsplits[ldrcarid][tmpcar->splitNr];
    }
    else
      if (tmpcar->splitNr < 2)
        ldrracetime += ldrcar->lastlap[tmpcar->splitNr];



    timediff = GP2_RaceTimes->car[tmpcarid-1];
    switch (tmpcar->splitNr) {
      case 0 : color = rbcolsplit0;
               timediff += tmpcar->lastlap_split1;
               break;
      case 1 : color = rbcolsplit1;
               timediff += tmpcar->lastlap_split2;
               break;
      case 2 : color = rbcolsplit2;
               //timediff += tmpcar->lastlap_time; // da GP2_RaceTimes noch nicht aktual.
               break;
    }


    timediff = CalcTimeDiff(ldrracetime, timediff);


    //if (tmpcar->remove_from_track & 0x20) { // aendern!!!
    //BOOL running = isoff(car->flags_90, 32)  ||  ison(car->flags_5E, 1);
    if ( !(!(tmpcar->remove_from_track & 0x20) || (tmpcar->unk_5E & 1)) ) { // Rene: That's what gp2 is using for it's number of cars running.
      color = GP2BLACK;
      _bprintf(atmp, sizeof(atmp), "Out"); // "OUT (Lap %02d)", tmpcar->lapnumber);
    }
    else if (tmpcar->unk_16A & 0x08) {
      color = GP2BLACK;
      _bprintf(atmp, sizeof(atmp), "Pit");
    }
    else if ((lapsdown = CalcLapsDown(ldrcar, tmpcar)) > 0) {
      color = GP2BLACK;
      _bprintf(atmp, sizeof(atmp), "-%uL", lapsdown);
    }
    else {
      _bprintf(atmp, sizeof(atmp), "%s%.12s",
        (ldrcarid == tmpcarid) ? "" : "+",
        (ldrcarid == tmpcarid) ? myGetTimeString( &ldrracetime ) :
                                 myGetTimeStringSlim( &timediff )
      );
    }
    if (tmpcar->lapnumber > 0)
      WRITEARRN6R(&atmp, &picbuf, tab_yofs + (i%5)*(fntarrn6.ydim-1), tab_xofs+TABCOL3POSX, color);



  } // for

  //---- give some leader x/y laps completed info ----
  _bprintf(atmp, sizeof(atmp), "Lap %u/%u", *pNumLapsDone, *GP2_LapsInThisRace);
  WRITEVERD7K(&atmp, &picbuf, _RBWHOLEBOARDY+_LEFTHDRY+3, _RBWHOLEBOARDX+TABLETXDIM+6, GP2WHITE);

  //--- evtl Pfeil nach unten setzen ---
  if (carpos > LASTPOS)
    insertpicat(tab_xofs+TABCOL1POSX, tab_yofs+TABLETYDIM-1, ARROWXDIM,ARROWYDIM, &picarrowdown, &picbuf);

} // MakeBehindBoard()


//-------------------------------------------------------------





































// verschiedenes:

    // some debug output
    //if (tmpcar->car_id & 0x80) {
    //  _bprintf(atmp, sizeof(atmp), "tmpcar==%P (id=0x%02x), ldrcar==%P (id=0x%02x)", tmpcar, tmpcarid, ldrcar, ldrcarid);
    //  WRITEARRN6R(&atmp, &picbuf, 10, 600, GP2WHITE);
    //}

    // some debug output
    //if (tmpcar->car_id == 0x9c) {
    //  _bprintf(atmp, sizeof(atmp), "ldrcar for car #28 is: %u", ldrcarid);
    //  WRITEARRN6R(&atmp, &picbuf, 10, 600, GP2WHITE);
    //  _bprintf(atmp, sizeof(atmp), "real ldrracetime is %s", myGetTimeString( &ldrracetime ));
    //  WRITEARRN6R(&atmp, &picbuf, 10+(1*11), 600, GP2WHITE);
    //}

    // some debug output
    //if (tmpcar->car_id & 0x80) {
    //if (tmpcar->car_id == 0x9c) {
    //  _bprintf(atmp, sizeof(atmp), "car #28 racetime upto split%u was %s", tmpcar->splitNr, myGetTimeString( &timediff ));
    //  WRITEARRN6R(&atmp, &picbuf, 10+(3*11), 600, GP2WHITE);
    //}

    // some debug output
    //if (tmpcar->car_id == 0x9c) {
    //  _bprintf(atmp, sizeof(atmp), "ldrracetime upto split%u was %s", tmpcar->splitNr, myGetTimeString( &ldrracetime ));
    //  WRITEARRN6R(&atmp, &picbuf, 10+(2*11), 600, GP2WHITE);
    //}

    // some debug output
    //if (tmpcar->car_id == 0x9c) {
    //  _bprintf(atmp, sizeof(atmp), "resulting gap ldrcar<-->#28 is %s", myGetTimeString( &timediff ));
    //  WRITEARRN6R(&atmp, &picbuf, 10+(4*11), 600, GP2WHITE);
    //}
