#include <stdio.h>
#include "timinpic.h"
#include "../fonts/myfntlib.h"
#include "timinstd.h"
#include "gp2struc.h"
#include "../dpmi.h"  // wegen mymemset()
#include "rlaptime.h"


#define LASTLAPSMAXIDX   30
#define MYORDERINITVAL   0x00   // such a carid doesn't exist

//---- always order-indexed!!! ----
unsigned char myorder[40];  // Order *jeweils am* Lapende, indiz. nach Position, inhalt ist carid
unsigned long mylastlaps[LASTLAPSMAXIDX+1]; // indiz. nach Position; paar mehr und letztes dummy feld
unsigned char mytopx[TOPCOUNTMAX];          // three fastest of the last lap

unsigned long atltopcount = TOPCOUNTDEFAULT;  // should be TOPCOUNTMIN..TOPCOUNTMAX


//-------------------------------------------------------------

void MakeLaptimeHeader(void)
{
  insertpicat(_RLWHOLEBOARDX+_LEFTHDRX, _RLWHOLEBOARDY+_LEFTHDRY, LEFTHDRXDIM, LEFTHDRYDIM, &picattheline, &picbuf);
  insertpicat(_RLWHOLEBOARDX+_RIGHTHDRX, _RLWHOLEBOARDY+_RIGHTHDRY, RIGHTHDRXDIM, RIGHTHDRYDIM, &piclaptime, &picbuf);

} // MakeLaptimeHeader()

//-------------------------------------------------------------

void __near MakeLaptimeBoard(unsigned char caropos /*kann eigentl. weg*/)
// external timing info from:
// - myorder[]
// problems:
// - nochmal genau ueberpruefen, wo myorder[] ueberall gesetzt wird, an allen sections???
// - myorder[] besser in mylineorder[] umbenennen
{
  static unsigned long i, p, tab_yofs, tab_xofs;
  static struct singlecarstruct *tmpcar;
  static unsigned char CarsInLeadlap;


  //mymemset(&picbuf, 0, sizeof(picbuf));  // den Bildschirmausgabebuffer leeren
  MakeLaptimeHeader();


  if ( (CarsInLeadlap = GetNumCarsInLeadlap())== 0 ||  // dann nur header ausgegeben
       (myorder[0] == MYORDERINITVAL) )          // noch frisch init., also nix uebern strich
    return;

  MakeTopX(CarsInLeadlap);

  //--- evtl Pfeil nach oben setzen ---
  if (FIRSTPOS > 0)
    insertpicat(_RLWHOLEBOARDX+_TABLETX+TABCOL1POSX, _RLWHOLEBOARDY+_TABLETY-(ARROWYDIM+1), ARROWXDIM,ARROWYDIM, &picarrowup, &picbuf);


  //if (carpos >= FIRSTPOS  &&  carpos <= LASTPOS)
  for (i = FIRSTPOS; i < CarsInLeadlap && i <= LASTPOS; i++) {  // bis zum aktuellen car

    //--- evtl neue table einblenden -------
    if ((i % 5) == 0) {
      if (atlAcrossTop) {
        tab_xofs = _RLWHOLEBOARDX+_TABLETX + ((i-FIRSTPOS)/5)*(TABLETXDIM+_RLTABLETSPACINGX);
        tab_yofs = _RLWHOLEBOARDY+_TABLETY;
      }
      else {
        tab_xofs = _RLWHOLEBOARDX+_TABLETX;
        tab_yofs = _RLWHOLEBOARDY+_TABLETY + ((i-FIRSTPOS)/5)*(TABLETYDIM+_RLTABLETSPACINGY);
      }
      insertpicat(tab_xofs, tab_yofs, TABLETXDIM, TABLETYDIM, &pictablet, &picbuf);
      tab_yofs += TABROW1POSY;  // den Schriftofs innerhalb des tabs bestimmen
    }

    tmpcar = Carid2Carstruct(myorder[i] & 0x3f);  // car, das als i. uebern strich fuhr

    if (tmpcar == NULL) { // sollte eigentl. nicht passieren
      //_bprintf(atmp, sizeof(atmp), "Error: C2C returned NULL for i==%u, cid==0x%02x", i, myorder[i]);
      //WRITEVERD7K(&atmp, &picbuf, _RLWHOLEBOARDY+_LEFTHDRY+3, _RLWHOLEBOARDX+TABLETXDIM+6, GP2WHITE);
      return;
    }

    _bprintf(atmp, sizeof(atmp), "%02d.", i+1);
    WRITEARRN6(&atmp, &picbuf, tab_yofs + (i%5)*(fntarrn6.ydim-1), tab_xofs+TABCOL1POSX, GP2WHITE);

    _bprintf(atmp, sizeof(atmp), "%-15.15s", GetMyDriverName(tmpcar->car_id) );
    WRITEARRN6(&atmp, &picbuf, tab_yofs + (i%5)*(fntarrn6.ydim-1), tab_xofs+TABCOL2POSX, HUMANCARID(tmpcar->car_id) ? GP2RED : GP2BLACK);

    _bprintf(atmp, sizeof(atmp), "%.10s", myGetTimeString(&tmpcar->lastlap_time) );
    WRITEARRN6R(&atmp, &picbuf, tab_yofs + (i%5)*(fntarrn6.ydim-1), tab_xofs+TABCOL3POSX, GP2BLACK);


    //---- mark current top3 (if in view) -----
    for (p = 0; p < atltopcount; p++) {
      if (i == mytopx[p]) {
        insertpicat(tab_xofs+_TOP3MARKERX, tab_yofs+(i%5)*(fntarrn6.ydim-1), TOP3XDIM,TOP3YDIM, &pictop3, &picbuf);
        _bprintf(atmp, sizeof(atmp), "%.1d", p+1);
        WRITEARRN6(&atmp, &picbuf, tab_yofs+(i%5)*(fntarrn6.ydim-1), tab_xofs+_TOP3MARKERX+7, GP2BLACK);
      }
    }


  } // for


  //--- evtl Pfeil nach unten setzen ---
  if (CarsInLeadlap > LASTPOS+1)
    insertpicat(tab_xofs+TABCOL1POSX, tab_yofs+TABLETYDIM-1, ARROWXDIM,ARROWYDIM, &picarrowdown, &picbuf);


} // MakeLaptimeBoard()


//-------------------------------------------------------------

void MakeTopX(unsigned char carcnt)
// parameters: carcnt==#cars in leadlap
{
  unsigned long i, ii, p;
  struct singlecarstruct *tmpcar;

  //---- extract last laptimes -----
  for (i = 0; i < carcnt; i++) { // for all relevant cars
    tmpcar = Carid2Carstruct(myorder[i] & 0x3f);  // car, das als i. uebern strich fuhr (1. entspr. i==0)
    if (tmpcar == NULL) break; // leave silently
    mylastlaps[i] = tmpcar->lastlap_time;
  }

  //---- make a dummy entry and let all point to him ----
  mylastlaps[LASTLAPSMAXIDX] = GP2TIMEINVMASK;
  for (i = 0; i < atltopcount; i++)
    mytopx[i] = LASTLAPSMAXIDX;

  //---- make current top3 -----
  for (i = 0; i < carcnt; i++) { // for all relevant cars
    if (GP2TIMEVALID(mylastlaps[i])) {
      for (ii = 0; ii < atltopcount; ii++) {
        if (mylastlaps[i] < mylastlaps[mytopx[ii]]) {   // position found?
          for (p = (atltopcount-1); p > ii; p--)  // make room
            mytopx[p] = mytopx[p-1];
          mytopx[ii] = i;  // insert
          break;
        }
      }
    }
  }

} // MakeTopX()

//-------------------------------------------------------------

void InitMyLineOrder(void)
{
  mymemset(&myorder, MYORDERINITVAL, sizeof(myorder));

} // InitMyLineOrder()

//-------------------------------------------------------------

void SetLaptimeTopCount(unsigned long tc)
// parameters: tc==new value, should be TOPCOUNTMIN<=tc<=TOPCOUNTMAX
{
  atltopcount = (tc > TOPCOUNTMAX) ? TOPCOUNTMAX : tc;
  atltopcount = (atltopcount < TOPCOUNTMIN) ? TOPCOUNTMIN : atltopcount;

} // SetLaptimeTopCount()

//-------------------------------------------------------------

unsigned long GetLaptimeTopCount(void)
// parameters: -
{
  return atltopcount;

} // GetLaptimeTopCount()

