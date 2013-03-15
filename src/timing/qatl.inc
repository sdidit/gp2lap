//---- 09/99 fuer UpdateLapStat() ------------

static struct singlecarstruct *tmpcar, *ldrcar;

// temporary stuff to be used within UpdateLapStat()
static unsigned long i, row, tabletofs;
static unsigned char civid, ldrcarid, oppcarid, tmpcarid;  // sind & 0x3f'ed
static unsigned char color;
static DWORD timediff;

static unsigned char *col1text[5] = { "Sector 1", "Sector 2", "Sector 3", "Laptime", "Currently" };


// polesitters best laptime (splits and segments)
static DWORD ldrsplits[3], ldrsegs[3];
// opponent car's best laptime (splits and segments)
static DWORD oppsplits[3], oppsegs[3];
// car in view best laptime (splits and segments) and actual segments
static DWORD civbestsplits[3], civbestsegs[3], civsegs[3];


// actual (probably previous) polesitters best laptime (splits and segments)
DWORD prevldrsplits[0x40][3], prevldrsegs[0x40][3]; // gesetzt von AHFBefDestrBest, siehe dort
unsigned long sicprevbest[0x40][4];   // gesetzt von AHFBefDestrBest, siehe dort
                                      // [..][3]==aktuelle polesitter carid

// fuer rbehind.h benoetigt
DWORD prevlapsplits[0x40][3];         // lastlap_splitx wird ja bei zieldurchfahrt geloescht, set by AHFBefDestrBest, siehe dort
unsigned char ldratsections[3];       // das car, was als erstes den x. split passiert hat (0==split1,...2==line), set by AHFCrossingSplit()
                                      // sind & 0x3f'ed;

// ideal segments for every car
unsigned long idealsegs[0x40][3];   // gesetzt von AHFBefDestrBest, siehe dort

//-------------------------------------

unsigned long tablepage[2]; // RaceMode >> 7 als index, gibt subpage zu activepage an
unsigned char qwhatopp[3];  // gibt den darzustellenden Opponenten an,
                            // [0]==0 -> leader, [0]==1 -> MichSelbst (civ)
                            // [0]==2 -> das car in CaridTeamTab[ [1] ]
                            // PS: [2]==carid aus CaridTeamTab[ [1] ],
                            //     [1] laeuft also von 0..25 oder so....
#define AGAINSTLDR  0
#define AGAINSTYOU  1
#define AGAINSTSEL  2

//-----------------------------------------------------------------------


// some positions within the pic
#define SECROW1POSY  16
#define SECCOL1POSX  3   // for left aligned text
#define SECCOL2POSX  107 // for right aligned text
#define SECCOL3POSX  161 // for right aligned text

#define POLEBESTCOL  GP2RED   // new overall besttime (pole)
#define PERSBESTCOL  GP2GREEN // new personal besttime
#define OPPOBESTCOL  GP2BLUE  // faster than opponent car

//-----------------------------------------------------------------------

void MakeSectorHeader(char *ltext, char *rtext, unsigned char lcolor, unsigned char rcolor)
{
  insertpicat(SECTORPICX, SECTORPICY, SECTORPICXDIM, SECTORPICYDIM, &picsectors, &picbuf);
  writefnt(ltext, &fntverd7fk, &picbuf, SECTORPICY + 2, SECTORPICX+4, lcolor);
  writefntr(rtext, &fntverd7k, &picbuf, SECTORPICY + 2, SECTORPICX+161, rcolor);
}

//------------------------------

void GetBestSplitsSegs(DWORD splits[3], DWORD segs[3], unsigned char carid /* da acar NULL sein kann */, struct singlecarstruct *acar)
{
  //---- aquire splittimes for acar ----
  if (acar == NULL) { // car nicht mehr auf piste -> carstruct net mehr da, aber Zeiten in GP2_Best*
    splits[0] = GP2_BestSplit1->car[(carid & 0x3f)-1];
    splits[1] = GP2_BestSplit2->car[(carid & 0x3f)-1];
    splits[2] = GP2_BestLaptimes->car[(carid & 0x3f)-1];
  }
  else {  // kann Zeiten aus carstruct holen
    splits[0] = acar->bestlap_split1;
    splits[1] = acar->bestlap_split2;
    splits[2] = acar->bestlap_time;

    //--- Bugfix 9/99: obwohl acar != NULL ist kann bestzeit trotzdem nur in tables gef, werden
    if (!myGP2_TIME_ISVALID(splits[0]))
      if (myGP2_TIME_ISVALID(GP2_BestLaptimes->car[(carid & 0x3f)-1])) {
        splits[0] = GP2_BestSplit1->car[(carid & 0x3f)-1];
        splits[1] = GP2_BestSplit2->car[(carid & 0x3f)-1];
        splits[2] = GP2_BestLaptimes->car[(carid & 0x3f)-1];
      }
  }

  //---- calc segment times for acar ----
  if (myGP2_TIME_ISVALID(splits[0])) {
    segs[0] = splits[0];
    segs[1] = CalcTimeDiff(splits[1], splits[0]);
    segs[2] = CalcTimeDiff(splits[2], splits[1]);
  }
  else {
    segs[0] = segs[1] = segs[2] = splits[0];
  }
}  // GetSplitsSegs()

//------------------------------

unsigned char IdealPosition(DWORD idealtime)
// rueckgabe: 1..26
{
  for (i = 0; i < 26; i++)  // for all 26 positions
    if (GP2_BestLaptimes->car[QPOS2CARID(i)-1] >= idealtime)  // gleichheit einfachhalber so behandeln
      return (i+1);  // position liefern
  return 255;
}

//------------------------------

char *Pos2String(unsigned char carpos)
// carpos == 1..26
{
  static unsigned char *stndrd[4] = { "th", "st", "nd", "rd" };
  static char zz, z, posbuf[10];
  zz = carpos / 10; z = carpos % 10;
  _bprintf(posbuf, sizeof(posbuf), "%u%s", carpos, zz == 1 ? stndrd[0] : (z < 4 ? stndrd[z] : stndrd[0]) );
  return &posbuf;
}

//------------------------------

// Probleme:
// 2 sofort raus & auf besttime kurs: section werden nicht rot gekennz.
// 3 rote zeiten, werden gegen oppcar die -:--.--- haben nur schwarz angezeigt (aehnlich prob 2)
void UpdateLapStat(struct singlecarstruct *cariv, unsigned long boardmode)
/* šbergabe: cariv = das CarInView (auf das sich alles bezieht)
             boardmode == [QATL_SECTORS, QATL_SECDIFF]
   Rckgabe: nix
*/
{
  static struct singlecarstruct *leader, *oppcar;
  static unsigned char *heads[2][3] = {
    { "Polesitter (Sectors)", "Yourself (Sectors)", "Selected (Sectors)" },
    { "Polesitter (Diffs)", "Yourself (Diffs)", "Selected (Diffs)" }
  };

  //-----------------------------------
  //---- stats for the leading car ----
  //-----------------------------------

  leader = Carid2Carstruct(QPOS2CARID(0));
  ldrcarid = QPOS2CARID(0);
  GetBestSplitsSegs(&ldrsplits, &ldrsegs, ldrcarid, leader);

  //------------------------------------
  //---- stats for the opponent car ----
  //------------------------------------

  switch (qwhatopp[0]) {
    case AGAINSTLDR : oppcar = leader; oppcarid = ldrcarid; break;
    case AGAINSTYOU : oppcar = cariv; oppcarid = cariv->car_id & 0x3f; break;
    case AGAINSTSEL : oppcar = Carid2Carstruct(qwhatopp[2] & 0x3f); oppcarid = qwhatopp[2] & 0x3f; break;
  }
  GetBestSplitsSegs(&oppsplits, &oppsegs, oppcarid, oppcar);


  //------------------------------------
  //---- stats for the car in view -----
  //------------------------------------

  GetBestSplitsSegs(&civbestsplits, &civbestsegs, cariv->car_id, cariv);

  //---- zusaetzlich noch fuer akt. lap ----
  if (myGP2_TIME_ISVALID(cariv->lastlap_split1)) {
    civsegs[0] = cariv->lastlap_split1;
    civsegs[1] = CalcTimeDiff(cariv->lastlap_split2, cariv->lastlap_split1);
    civsegs[2] = CalcTimeDiff(cariv->lastlap_time, cariv->lastlap_split2);
  }
  else {
    civsegs[0] = civsegs[1] = civsegs[2] = cariv->lastlap_split1;
  }

  //----------------------------------------------------
  //---- bei Verbesserg. fuer einen sector lang mal ----
  //---- die bestzeit gelten lassen --------------------
  //----------------------------------------------------
  // Mglk: cariv verbessert seine eigene (polesitter, yourself) oder eine fremden (polesitter)

  if ( (cariv->splitNr == 2) &&
       (cariv->lastlap_time == cariv->bestlap_time) )    // war das eben eine civ schnellste?
  switch (qwhatopp[0]) {

    case AGAINSTLDR : // against polesitter
        if (Carid2Qualpos(cariv->car_id & 0x3f) == 0) { // haben wir jemanden (evtl. uns selbst) von POLE verdraengt?
          // wenn ja, ihn nochmal kurz zum leader machen
          oppcarid = (unsigned char)sicprevbest[cariv->car_id & 0x3f][3];  //  wer stand vorher auf pole?
          if (oppcarid == (cariv->car_id & 0x3f)) {  // haben uns selbst verdraengt
            //LogLine("Haben uns selbst von pole verdraengt\n");
            if (myGP2_TIME_ISVALID(sicprevbest[oppcarid][0])) {
              oppsplits[0] = sicprevbest[oppcarid][0];
              oppsplits[1] = sicprevbest[oppcarid][1];
              oppsplits[2] = sicprevbest[oppcarid][2];
              oppsegs[0] = oppsplits[0];
              oppsegs[1] = CalcTimeDiff(oppsplits[1], oppsplits[0]);
              oppsegs[2] = CalcTimeDiff(oppsplits[2], oppsplits[1]);
              ldrsplits[0] = oppsplits[0]; ldrsplits[1] = oppsplits[1]; ldrsplits[2] = oppsplits[2];
              ldrsegs[0] = oppsegs[0];     ldrsegs[1] = oppsegs[1];     ldrsegs[2] = oppsegs[2];
            }
          }
          else { // haben jemand anderen verdraengt
            //sprintf(strbuf, "Haben 0x%02x von pole verdraengt\n", oppcarid);
            //LogLine(strbuf);
            oppsplits[0] = prevldrsplits[cariv->car_id & 0x3f][0];
            oppsplits[1] = prevldrsplits[cariv->car_id & 0x3f][1];
            oppsplits[2] = prevldrsplits[cariv->car_id & 0x3f][2];
            oppsegs[0] = prevldrsegs[cariv->car_id & 0x3f][0];
            oppsegs[1] = prevldrsegs[cariv->car_id & 0x3f][1];
            oppsegs[2] = prevldrsegs[cariv->car_id & 0x3f][2];
            ldrsplits[0] = oppsplits[0]; ldrsplits[1] = oppsplits[1]; ldrsplits[2] = oppsplits[2];
            ldrsegs[0] = oppsegs[0];     ldrsegs[1] = oppsegs[1];     ldrsegs[2] = oppsegs[2];
          }
        }
        break;

    case AGAINSTSEL : // against selcected (szenario: haben cariv selected)
        if (oppcarid != (cariv->car_id & 0x3f)) break; //...sonst zu AGAINSTYOU

    case AGAINSTYOU : // against yourself (szenario: haben eben unsere zeit verbessert)
        // krame meine alte Bestzeit raus...
        if (myGP2_TIME_ISVALID(sicprevbest[oppcarid][0])) {  // ueberhaupt schon eine prev best da?
          oppsplits[0] = sicprevbest[oppcarid][0];
          oppsplits[1] = sicprevbest[oppcarid][1];
          oppsplits[2] = sicprevbest[oppcarid][2];
          oppsegs[0] = oppsplits[0];
          oppsegs[1] = CalcTimeDiff(oppsplits[1], oppsplits[0]);
          oppsegs[2] = CalcTimeDiff(oppsplits[2], oppsplits[1]);
          if ( oppcarid == (unsigned char)sicprevbest[oppcarid][3] ) { // war ich vorher gar polesitter?
            // wenn ja, mich nochmal kurz zum leader machen
            ldrsplits[0] = oppsplits[0]; ldrsplits[1] = oppsplits[1]; ldrsplits[2] = oppsplits[2];
            ldrsegs[0] = oppsegs[0];     ldrsegs[1] = oppsegs[1];     ldrsegs[2] = oppsegs[2];
          }
        }
        break;
  }


  //------------------------------------
  //---- Do some initial Output --------
  //------------------------------------

  //--- 12/99
  //mymemset(&picbuf, 0, sizeof(picbuf));  // clear screenbuf

  MakeSectorHeader("Against", heads[boardmode&1][qwhatopp[0] & 3], GP2WHITE, GP2LIGHTGREY);
  tabletofs = SECTORPICY + SECROW1POSY;

  //------------------------------------
  //---- Output text for Column 1 ------
  //------------------------------------

  row = 1;
  for (i = 0; i < 5; i++) {
    _bprintf(atmp, sizeof(atmp), "%s", col1text[i]);
    WRITEVERD7K(&atmp, &picbuf, tabletofs + (row++)*(fntverd7k.ydim+1), SECTORPICX+SECCOL1POSX, GP2BLACK);
  }

  //------------------------------------
  //---- Output for the opp car --------
  //------------------------------------

  row = 1;
  for (i = 0; i < 3; i++) {
    _bprintf(atmp, sizeof(atmp), "%s", myGetTimeStringSlim( &oppsegs[i] ));
    WRITEARRN6R(&atmp, &picbuf, tabletofs + (row++)*(fntarrn6.ydim+1), SECTORPICX+SECCOL2POSX, GP2BLACK);
  }
  _bprintf(atmp, sizeof(atmp), "%.12s", myGetTimeString( &oppsplits[2] ));
  WRITEARRN6R(&atmp, &picbuf, tabletofs + (row++)*(fntarrn6.ydim+1), SECTORPICX+SECCOL2POSX, GP2BLACK);
  // nicht anzeigen, wenn noch kein polesitter
  if ((qwhatopp[0] != AGAINSTLDR) || myGP2_TIME_ISVALID(oppsplits[2])) {
    _bprintf(atmp, sizeof(atmp), "%.6s", GetMyDriverName(oppcarid));
    WRITEVERD7KR(&atmp, &picbuf, tabletofs + ( 0 )*(fntverd7k.ydim), SECTORPICX+SECCOL2POSX, GP2WHITE);
    _bprintf(atmp, sizeof(atmp), "%s  ", Pos2String(Carid2Qualpos(oppcarid)+1));
    WRITEVERD7FKR(&atmp, &picbuf, tabletofs + (row)*(fntverd7fk.ydim+1), SECTORPICX+SECCOL2POSX, GP2BLACK);
  }

  //------------------------------------
  //---- Output for the car in view ----
  //------------------------------------

  row = 0;
  _bprintf(atmp, sizeof(atmp), "%.6s", GetMyDriverName(cariv->car_id));
  WRITEVERD7KR(&atmp, &picbuf, tabletofs + (row)*(fntverd7k.ydim), SECTORPICX+SECCOL3POSX, GP2WHITE);
  for (i = 0; i <= cariv->splitNr; i++) {
    row++;
    switch (boardmode) {
      case QATL_SECTORS :
        _bprintf(atmp, sizeof(atmp), "%s", myGetTimeStringSlim( &civsegs[i] ) ); break;
      case QATL_SECDIFF :
        timediff = CalcTimeDiff(civsegs[i], oppsegs[i]);
        _bprintf(atmp, sizeof(atmp), "%c%s", (myGP2_TIME_ISVALID(civsegs[i]) && myGP2_TIME_ISVALID(oppsegs[i])) ?
                          ( civsegs[i] < oppsegs[i] ? '-' : '+' ) : 0x20, myGetTimeStringSlim( &timediff )); break;
    }
    // schnell noch color bestimmen
    if (myGP2_TIME_ISVALID(ldrsegs[i]) && myGP2_TIME_ISVALID(oppsegs[i]))
      switch (boardmode) {
        case QATL_SECTORS :
          if (civsegs[i] < ldrsegs[i]) color = POLEBESTCOL;
          else if (civsegs[i] < oppsegs[i]) color = OPPOBESTCOL;
          //else if (civsegs[i] < oppsegs[i]) color = (qwhatopp[0] == AGAINSTYOU) ? PERSBESTCOL : OPPOBESTCOL; // falls oppcar man selber -> persbest
               else if (civsegs[i] < civbestsegs[i] && myGP2_TIME_ISVALID(civbestsegs[i])) color = PERSBESTCOL; // validchk muss hier hin
                    else color = GP2BLACK;
          break;
        case QATL_SECDIFF :
          if (civsegs[i] < oppsegs[i]) color = GP2RED;  // hier gibts nur rot und normal
          else color = GP2BLACK;
          break;
      }
    else
      color = GP2BLACK;
    WRITEARRN6R(&atmp, &picbuf, tabletofs + (row)*(fntarrn6.ydim+1), SECTORPICX+SECCOL3POSX, color);
  }

  if (cariv->splitNr == 2) {  // show now laptime too
    row++;
    switch (boardmode) {
      case QATL_SECTORS :
        _bprintf(atmp, sizeof(atmp), "%.12s", myGetTimeString( (DWORD*)&cariv->lastlap_time )); break;
      case QATL_SECDIFF :
        timediff = CalcTimeDiff(cariv->lastlap_time, oppsplits[2]);
        _bprintf(atmp, sizeof(atmp), "%c%s", myGP2_TIME_ISVALID(oppsplits[2]) && myGP2_TIME_ISVALID(cariv->lastlap_time) ?
                          ( cariv->lastlap_time < oppsplits[2] ? '-' : '+' ) : 0x20, myGetTimeStringSlim( &timediff )); break;
    }
    // schnell noch color bestimmen
    if ( myGP2_TIME_ISVALID(cariv->lastlap_time) && myGP2_TIME_ISVALID(ldrsplits[2]) && myGP2_TIME_ISVALID(oppsplits[2]) )
      switch (boardmode) {
        case QATL_SECTORS :
          if (cariv->lastlap_time < ldrsplits[2]) color = POLEBESTCOL;
          else if (cariv->lastlap_time < oppsplits[2]) color = OPPOBESTCOL;
               else if ((cariv->lastlap_time < civbestsplits[2]) && myGP2_TIME_ISVALID(civbestsplits[2])) color = PERSBESTCOL; // validchk muss hier hin
                    else color = GP2BLACK;
          break;
        case QATL_SECDIFF :
          if (cariv->lastlap_time < oppsplits[2]) color = GP2RED;
          else color = GP2BLACK;
          break;
      }
    else
      color = GP2BLACK;
    WRITEARRN6R(&atmp, &picbuf, tabletofs + (row)*(fntarrn6.ydim+1), SECTORPICX+SECCOL3POSX, color);
  }
  row = 5;
  _bprintf(atmp, sizeof(atmp), "%s  ", Pos2String(Carid2Qualpos(cariv->car_id & 0x3f)+1));
  WRITEVERD7FKR(&atmp, &picbuf, tabletofs + (row)*(fntverd7fk.ydim+1), SECTORPICX+SECCOL3POSX, GP2BLACK);


  //------------------------------------
  //---- Output for test purposes ------
  //------------------------------------
/*
  row = 0;
  _bprintf(atmp, sizeof(atmp), "civspl: %X,%X,%X oppspl: %X,%X,%X oppsgs: %X,%X,%X",
                                cariv->lastlap_split1, cariv->lastlap_split2, cariv->lastlap_time,
                                oppsplits[0], oppsplits[1], oppsplits[2],
                                oppsegs[0], oppsegs[1], oppsegs[2] );
  WRITEARRN6(&atmp, &picbuf, tabletofs + (row)*(fntarrn6.ydim), SECTORPICX+200, GP2WHITE);
*/

}  // UpdateLapStat()

//-----------------------------------------------------------------------

static char securehpmsg[] = {
       '(' ^ XORVAL,
       '%' ^ XORVAL,
       'u' ^ XORVAL,
       ' ' ^ XORVAL,
       'h' ^ XORVAL,
       'p' ^ XORVAL,
       ')' ^ XORVAL, 0
};
// Bugs:
// (1) ...
void MakeIdealBoard(struct singlecarstruct *cariv)
// Uebergabe: cariv = das CarInView (auf das sich alles bezieht)
// Rueckgabe: nix
{
  static DWORD idealtime;

  civid = cariv->car_id & 0x3f;

  //------------------------------------
  //---- Do some initial Output --------
  //------------------------------------

  //--- 12/99
  //mymemset(&picbuf, 0, sizeof(picbuf));  // clear screenbuf

  MakeSectorHeader("Ideal Laptime", "", GP2WHITE, GP2LIGHTGREY);
  tabletofs = SECTORPICY + SECROW1POSY;

  //------------------------------------
  //---- Output text for Column 1 ------
  //------------------------------------

  row = 1;
  for (i = 0; i < 4; i++) {
    _bprintf(atmp, sizeof(atmp), "%s", col1text[i]);
    WRITEVERD7K(&atmp, &picbuf, tabletofs + (row++)*(fntverd7k.ydim+1), SECTORPICX+SECCOL1POSX, GP2BLACK);
  }
  WRITEVERD7K("Virtual", &picbuf, tabletofs + (row)*(fntverd7k.ydim+1), SECTORPICX+SECCOL1POSX, GP2BLACK);


  //------------------------------------
  //---- Output for his besttime -------
  //------------------------------------

  GetBestSplitsSegs(&civbestsplits, &civbestsegs, civid, cariv);

  row = 0;
  _bprintf(atmp, sizeof(atmp), securehpmsg, (cariv->enginepower*790) >> 14);
  WRITEVERD7KR(&atmp, &picbuf, tabletofs + (row++)*(fntverd7k.ydim), SECTORPICX+SECCOL2POSX, GP2WHITE);
  for (i = 0; i < 3; i++) {
    _bprintf(atmp, sizeof(atmp), "%s", myGetTimeStringSlim( &civbestsegs[i] ));
    WRITEARRN6R(&atmp, &picbuf, tabletofs + (row++)*(fntarrn6.ydim+1), SECTORPICX+SECCOL2POSX, GP2BLACK);
  }
  _bprintf(atmp, sizeof(atmp), "%.12s", myGetTimeString( &civbestsplits[2] ));
  WRITEARRN6R(&atmp, &picbuf, tabletofs + (row++)*(fntarrn6.ydim+1), SECTORPICX+SECCOL2POSX, GP2BLACK);

  _bprintf(atmp, sizeof(atmp), "%s  ", Pos2String(Carid2Qualpos(civid)+1));
  WRITEVERD7FKR(&atmp, &picbuf, tabletofs + (row++)*(fntverd7fk.ydim+1), SECTORPICX+SECCOL2POSX, GP2BLACK);


  //------------------------------------
  //---- calcs for the ideal stuff -----
  //------------------------------------

  if (myGP2_TIME_ISVALID(idealsegs[civid][0]) && myGP2_TIME_ISVALID(idealsegs[civid][1]) && myGP2_TIME_ISVALID(idealsegs[civid][2]) ) {
    //--- evtl. noch ein paar verbesserungen vornehmen ---
    //--- nach Laden eines saved games
    if (civbestsegs[0] < idealsegs[civid][0]) idealsegs[civid][0] = civbestsegs[0];
    if (civbestsegs[1] < idealsegs[civid][1]) idealsegs[civid][1] = civbestsegs[1];
    if (civbestsegs[2] < idealsegs[civid][2]) idealsegs[civid][2] = civbestsegs[2];
    idealtime = idealsegs[civid][0] + idealsegs[civid][1] + idealsegs[civid][2];
  }
  else
    idealtime = 0x10000000;


  //------------------------------------
  //---- Output the ideal stuff --------
  //------------------------------------

  row = 0;
  _bprintf(atmp, sizeof(atmp), "%.6s", GetMyDriverName(civid));
  WRITEVERD7KR(&atmp, &picbuf, tabletofs + (row++)*(fntverd7k.ydim), SECTORPICX+SECCOL3POSX, GP2WHITE);

  for (i = 0; i < 3; i++) {
    _bprintf(atmp, sizeof(atmp), "%s", myGetTimeStringSlim( &idealsegs[civid][i] ));
    if (myGP2_TIME_ISVALID(idealsegs[civid][i]) && myGP2_TIME_ISVALID(civbestsegs[i]))
      color = (idealsegs[civid][i] < civbestsegs[i]) ? GP2RED : GP2BLACK;
    WRITEARRN6R(&atmp, &picbuf, tabletofs + (row++)*(fntarrn6.ydim+1), SECTORPICX+SECCOL3POSX, color);
  }

  _bprintf(atmp, sizeof(atmp), "%.12s", myGetTimeString( (DWORD*)&idealtime ));
  if (myGP2_TIME_ISVALID(civbestsplits[i]))
    color = (idealtime < civbestsplits[i]) ? GP2RED : GP2BLACK;
  WRITEARRN6R(&atmp, &picbuf, tabletofs + (row++)*(fntarrn6.ydim+1), SECTORPICX+SECCOL3POSX, color);

  if (myGP2_TIME_ISVALID(idealtime)) {
    _bprintf(atmp, sizeof(atmp), "%s  ", Pos2String(IdealPosition(idealtime)));
    WRITEVERD7FKR(&atmp, &picbuf, tabletofs + (row++)*(fntverd7fk.ydim+1), SECTORPICX+SECCOL3POSX, GP2BLACK);
  }

}  // MakeIdealBoard()


