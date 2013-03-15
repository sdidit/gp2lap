#include "../stdinc.h"
#include "../gp2lap.h"     // 08.11.99 von Rene
#include "../gp2glob.h"    // 08.11.99 von Rene
#include "../misc.h"
#include "gp2struc.h"
#include "timinstd.h"


struct gp2drivernames __MyDrivers;
struct singlecarstruct *tmpcar, *ldrcar;


//----------------------------------------------------------------------

void MakeMyDrivers(struct gp2drivernames *tab)
{
  unsigned long i, ii, isdopp;
  unsigned char vorname[50];
  unsigned char name[50];
  unsigned char vorname2[50];
  unsigned char name2[50];

  for (i = 0; i < 40; i++) {
    //if (sscanf(tab->driver[i], "%s %[^\n]", vorname, name) == 2) { // 2 recognized args
    switch ( sscanf(AHFGetStrNrESI(GP2STRCOUNT+i), "%s %[^\n]", vorname, name) ) {

     case 2 : // 2 recognized args

      isdopp = 0;

      for (ii = 0; ii < 40; ii++) {
        if (ii != i) {
          //if (sscanf(tab->driver[ii], "%s %[^\n]", vorname2, name2) == 2) {
            if (sscanf(AHFGetStrNrESI(GP2STRCOUNT+ii), "%s %[^\n]", vorname2, name2) == 2) {
            if (strcmp(name, name2) == 0) {
              isdopp = 1;
              break;
            }
          }
        }
      }
      vorname[1] = 0x20;  // Leerzeichen hinter 1. Buchstaben
      _bprintf(__MyDrivers.driver[i], sizeof(__MyDrivers.driver[i]),
                "%.2s%s", isdopp ? vorname : "", name);

      break;

     case 1 :  // 1 recognized arg, j.i. just "Jon"
      _bprintf(__MyDrivers.driver[i], sizeof(__MyDrivers.driver[i]),
                "%s", vorname);
      break;

     default :
      _bprintf(__MyDrivers.driver[i], sizeof(__MyDrivers.driver[i]),
                "X Unknown");
    } // switch

  }
  for (i = 40; i < 256; i++)
   _bprintf(__MyDrivers.driver[i], sizeof(__MyDrivers.driver[i]),
             "Driver res 0x%02X", i);

} // MakeMyDrivers()

//----------------------------------------------------------------------

char *GetMyDriverName(unsigned char carid)
// öbergabe: car_id
// RÅckgabe: ptr auf Namen
{
  return __MyDrivers.driver[(carid & 0x3F)-1];
} // GetMyDriverName()


//======================================================================


int __myDecompTime(long *h, long *m, long *s, long *ms, unsigned long *t)
{
  if (!myGP2_TIME_ISVALID(*t))
    return 0;
  *h = *t / 3600000; *m = (*t % 3600000) / 60000; *s = (*t % 60000) / 1000; *ms = *t % 1000;
  return 1;
}

//---------------------------

char *myGetTimeString(unsigned long *t)
{
#define GTSSTRINGS  3
  long h = 0, m = 0, s = 0, ms = 0;
  static char str[GTSSTRINGS][16];
  static unsigned char internalcnt = 0;  // for more function call in *one* parameter list

  internalcnt++;
  internalcnt %= GTSSTRINGS;
  if (__myDecompTime(&h, &m ,&s, &ms, t))
    if (h != 0) {
      sprintf(&str[internalcnt][0], "%lu:%lu:%02lu.%03lu", h, m, s, ms); // "4h 58:32.394"
    }
    else {
      sprintf(&str[internalcnt][0], "%lu:%02lu.%03lu", m, s, ms);      // "58:32.394"
    }
  else
    strcpy(&str[internalcnt][0], "-:--.---");

  return &str[internalcnt][0];
}

//----------------------------------------------------------------------

char *myGetTimeStringSlim(unsigned long *t)
{
  long h = 0, m = 0, s = 0, ms = 0;
  static char str[16];

  if (__myDecompTime(&h, &m ,&s, &ms, t)) {
    if (m != 0)
      sprintf(str, "%lu:%02lu.%03lu", m, s, ms);
    else
      sprintf(str, "%lu.%03lu", s, ms);
  }
  else
    strcpy(str, "-:--.---");

  return str;
}

//----------------------------------------------------------------------

unsigned long CalcTimeDiff(unsigned long t1, unsigned long t2)
{
  if (myGP2_TIME_ISVALID(t1) && myGP2_TIME_ISVALID(t2)) {
    return (t1 > t2) ? (t1 - t2) : (t2 - t1);
  }
  else
    return myGP2_TIME_INV_MASK;
}


//======================================================================


struct singlecarstruct *Carid2Carstruct(unsigned char carid)
// Warning: call with carid & 0x3F !!!
{
  unsigned char i;
  for (i = 0; i < 26; i++) {
    if ( (GP2_Cars->car[i].car_id & 0x3F) == carid )
      return &GP2_Cars->car[i];
  }
  return NULL;
}

//----------------------------------------------------------------------

unsigned char Carid2Qualpos(unsigned char carid)
// Warning: call with carid & 0x3F !!!
// Rueckgabe: 0==polesitter, 1==zweiter, etc.
{
  unsigned char i;
  for (i = 0; i < 26; i++) {
   if ((GP2_FastestCars->car[i] & 0x3F) == carid) return i;
  }
  return 0xFF;
}


//----------------------------------------------------------------------

unsigned char CalcLapsDown(struct singlecarstruct *cif, struct singlecarstruct *car)
// call to get the nr of laps down compared to cif (make sure cif is really in front!)
// RÅckgabe: laps down, 0 == same lap
//           250 == one of the carstructs was NULL
//           251 == one of the cars had no track segment (curSeg)
// problems: nur in box scheint manchmal zu spinnen (wohl wegen lapnumber dort)
{
  static unsigned long ciftsegs, cartsegs;

  if (cif == NULL || car == NULL)
    return 250;

  if (cif->curSeg == NULL || car->curSeg == NULL)
    return 251;

  // eigentl. nicht ganz richtig, da lapnumber==angefangene Runde
  ciftsegs = *pNumTrackSegs * cif->lapnumber;
  cartsegs = *pNumTrackSegs * car->lapnumber;
  ciftsegs += SEG_KILLSTATEBITS( SEG_HASEQUALSEG(cif->curSeg->nr) ? cif->curSeg->equalseg->nr : cif->curSeg->nr );
  cartsegs += SEG_KILLSTATEBITS( SEG_HASEQUALSEG(car->curSeg->nr) ? car->curSeg->equalseg->nr : car->curSeg->nr );

  return ( CalcValueDiff(ciftsegs, cartsegs) / *pNumTrackSegs );

} // CalcLapsDown()

//----------------------------------------------------------------------

unsigned char GetNumRunners(void)
// call to get the nr of runners in the race (see IDA: PrnRunnersNPos)
// RÅckgabe: #Fahrer // am Anfang des Rennens natuerl. #Starter
{
  unsigned char i, runners;

  runners = 0;
  for (i = 0; i < 26; i++) {
    //if ( ((GP2_Cars->car[i].remove_from_track & 0x20) == 0) || (GP2_Cars->car[i].unk_5E & 1) )
    if ( CARISRUNNING(GP2_Cars->car[i]) )
      runners++;
  }
  return runners;

} // GetNumRunners()

//----------------------------------------------------------------------

unsigned char GetNumCarsInLeadlap(void)
// to get the # of cars in the leadlap (f.i. for At the line: laptime)
{
  unsigned char i, carcount;

  carcount = 1;
  ldrcar = Carid2Carstruct(POS2CARID(0)); // get the leader

  if (ldrcar->lapnumber == 0)  // race hasn't started yet
    return 0;

  //------ for all cars -------
  for (i = 0; i < 26; i++) {
    if ( (GP2_Cars->car[i].car_id != ldrcar->car_id) &&
          CARISRUNNING(GP2_Cars->car[i]) &&
         (GP2_Cars->car[i].lapnumber == ldrcar->lapnumber) )
      carcount++;
  }

  return carcount;

} // GetNumCarsInLeadlap()

//----------------------------------------------------------------------

static unsigned long lastlaps[26][3];
static unsigned long lastpositions[26];
static unsigned long lastlapnumbers[26];

void CreateSessionImage(void)
// create a fingerprint of the current session
{
  unsigned char i;

  //------ for all cars -------
  for (i = 0; i < 26; i++) {
    lastlaps[i][0] = GP2_Cars->car[i].lastlap_split1;
    lastlaps[i][1] = GP2_Cars->car[i].lastlap_split2;
    lastlaps[i][2] = GP2_Cars->car[i].lastlap_time;
    lastpositions[i] = GP2_Cars->car[i].racepos;
    lastlapnumbers[i] = GP2_Cars->car[i].lapnumber;
  }

} // CreateSessionImage()

//----------------------------------------------------------------------

unsigned long SessionImageChanged(void)
// ...to verify if a new session has been started/loaded
// 1 == new session, 0 == the same session
{
  unsigned char i;
  unsigned long retval;

  retval = 0;
  //------ for all cars -------
  for (i = 0; i < 26; i++) {
    if (lastlaps[i][0] != GP2_Cars->car[i].lastlap_split1) retval = 1;
    if (lastlaps[i][1] != GP2_Cars->car[i].lastlap_split2) retval = 1;
    if (lastlaps[i][2] != GP2_Cars->car[i].lastlap_time) retval = 1;
    if (lastpositions[i] != GP2_Cars->car[i].racepos) retval = 1;
    if (lastlapnumbers[i] != GP2_Cars->car[i].lapnumber) retval = 1;
  }

  return retval;

} // SessionImageChanged()
