#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basiclog.h"
#include "misc.h"                // wegen AHFGetStrNrESI()
#include "timing/gp2struc.h"     // wegen ErwRundzahl
#include "trackinf.h"
#include "miscahf.h"
#include "cfgmain.h"


static Gp2InfoTags acttags; // noch EINSPAREN!!!
static Gp2InfoTags TrackTags[16];

static unsigned char tagbuffer[4096];

// new segs from within custom track file
long int Split1Seg = -1;
long int Split2Seg = -1;

// pointers to data structures within gp2
unsigned char (*LapsPerTrack)[16] = NULL;
struct ErwRundzahl (*t_ErwRundzahl)[16] = NULL;
unsigned short (*GripTable)[16] = NULL;

//--------------------------------------------------------------

void CatchTrackInfos()
// patches the gp2 strings to contain almost all info provided by the f1ctxx.dat file
// called from AfterGp2Init() or so...
{
  static FILE *fin;
  unsigned long i;
  static char buffer[2000];

  for (i = 0; i < 16; i++) {
    _bprintf(buffer, sizeof(buffer), "%s\\%s", GetGP2Dir(), TrackNrToFilename(i, NULL));

    if (fin = fopen(buffer, "rb")) {
      if (fread(&buffer, sizeof(buffer), 1, fin) == 1) {

        //printf("Now parsing [%s\\%s]\n", GetGP2Dir(), TrackNrToFilename(i, NULL)); fflush(stdout);

        if ( ParseGp2InfoTags(&buffer, &TrackTags[i]) ) {

          //printf("Parsing success with [%s\\%s]\n", GetGP2Dir(), TrackNrToFilename(i, NULL));

          set_gp2string(0x59F + i, &TrackTags[i].name); // "Elhart Lake"
          set_gp2string(0x5BF + i, &TrackTags[i].country); // ComboBox (Drive Options) "USA"
          set_gp2string(0x5DF + i, &TrackTags[i].country);
          set_gp2string(0x5FF + i, &TrackTags[i].country); // Car Setup Std menu
          set_gp2string(0x61F + i, &TrackTags[i].country); // Select Circuit
          set_gp2string(0x63F + i, &TrackTags[i].country); // once "GB" instead of "Great Britain"??

          set_gp2string(0x65F + i, &TrackTags[i].adjectivepre);  // f.i. "Brazilian "
          set_gp2string(0x67F + i, &TrackTags[i].adjectivepost); // f.i. " of Brazil"

          // now set the laps for track [i]
          LapsPerTrack = (unsigned char *)IDACodeReftoDataRef(0x1708f);
          (*LapsPerTrack)[i] = (TrackTags[i].laps > 0) ? TrackTags[i].laps : DEFAULTLAPS;  // DEFAULTLAPS laps on error

          // now set the length for track [i]
          t_ErwRundzahl = (struct ErwRundzahl *)IDACodeReftoDataRef(0x6b844);
          (*t_ErwRundzahl)[i].length = (TrackTags[i].length > 0) ? METERSTOGCLENGTH(TrackTags[i].length) : DEFAULTLENGTH; // DEFAULTLENGTH on error (see IDA 0x81949)
          // here we have number of laps again. not set by the trackmanagers, but prob. useful
          (*t_ErwRundzahl)[i].laps = (TrackTags[i].laps > 0) ? TrackTags[i].laps : DEFAULTLAPS; // 13 laps on error

          // now set the GripLevel for track [i]
          GripTable = (unsigned short *)IDACodeReftoDataRef(0x76f84);  // viktor.gars@swipnet.se calls it tyrewear
          (*GripTable)[i] = (TrackTags[i].tyrewear > 0) ? TrackTags[i].tyrewear : DEFAULTTYREWEAR;
        }

        //sprintf(strbuf, "TrackName=[%s] Country=[%s]\n", custTrackNames[i], custTrackCountry[i]);
        //LogLine(strbuf);
      }

          // Rene 04-01-2000
          TrackTags[i].checksum = 0;
          fseek(fin, -4, SEEK_END);
          fread(&TrackTags[i].checksum, 1, sizeof(DWORD), fin);

      fclose(fin);
    }
    else { // can't open

      _bprintf(TrackTags[i].name, sizeof(TrackTags[i].name), "%s", TrackNrToFilename(i, NULL));
      _bprintf(TrackTags[i].country, sizeof(TrackTags[i].country), "(File not found)");

      set_gp2string(0x59F + i, &TrackTags[i].name);
      set_gp2string(0x5BF + i, &TrackTags[i].country);
      set_gp2string(0x5DF + i, &TrackTags[i].country);
      set_gp2string(0x5FF + i, &TrackTags[i].country);
      set_gp2string(0x61F + i, &TrackTags[i].country);
      set_gp2string(0x63F + i, &TrackTags[i].country);

      set_gp2string(0x65F + i, &TrackTags[i].adjectivepre);  // f.i. "Brazilian "
      set_gp2string(0x67F + i, &TrackTags[i].adjectivepost); // f.i. " of Brazil"
    }
  }

} // CatchTrackInfos()


//--------------------------------------------------------------

#pragma aux TrackNrToFilename parm [ECX] [ESI] caller value [ESI];
char *TrackNrToFilename(unsigned long tracknr, char *files /*not used*/)
{
  static char filename[32];

  _bprintf(filename, sizeof(filename), "f1ct%02u", tracknr+1);

  if ( GetCfgString(filename) ) { // switch found

     //printf("liefere [%s]\n", GetCfgString(filename));
     return (char *)GetCfgString(filename);
  }
  else {  // switch not in .cfg file
    _bprintf(filename, sizeof(filename), "circuits\\f1ct%02u.dat", tracknr+1);
  }

  return &filename;

} // TrackNrToFilename()
char * (*fpTrackNrToFilename)() = TrackNrToFilename;

//--------------------------------------------------------------

/*
char *GetTrackFileStr(unsigned long tracknr)
{


} // GetTrackFileStr()
*/

//--------------------------------------------------------------


void FrankOnTrackFileLoad(char *TrackBuf)
{
  ParseGp2InfoTags(TrackBuf, &acttags);

  //sprintf(strbuf, " Name=%s,Cntry=%s,Laps=%d,Slot=%d,Tyre=%d,LenM=%d,Spl1=0x%04X,Spl2=0x%04X\n",
  //                acttags.name, acttags.country,
  //                acttags.laps, acttags.slot, acttags.tyrewear,
  //                acttags.length,
  //                acttags.split1, acttags.split2);
  //LogLine(strbuf);

  Split1Seg = acttags.split1;
  Split2Seg = acttags.split2;

} // FrankOnTrackFileLoad()

//--------------------------------------------------------------

void InitGp2InfoTags(Gp2InfoTags *pTags)
{
  if (pTags) {
    memset(pTags, 0xFF, sizeof(Gp2InfoTags));  // init the tag buffer
    pTags->name[0] = 0;
    pTags->country[0] = 0;
    pTags->author[0] = 0;
    pTags->event[0] = 0;
    pTags->adjectivepre[0] = 0;
    pTags->adjectivepost[0] = 0;
  }

} // InitGp2InfoTags()

//--------------------------------------------------------------

unsigned long ParseGp2InfoTags(char *filebuf, Gp2InfoTags *pTags)
// Rueckgabe: 0 == error
{
//#define PGITDEBUG

  static char *t, *tvalue;

  if (!pTags) return 0;
  InitGp2InfoTags(pTags);

  memcpy(&tagbuffer, filebuf, sizeof(tagbuffer));  // 2nd buffer, cause strtok() modifies...

  #ifdef PGITDEBUG
    printf("\n\nPGITEntry: %.512s\n", filebuf); fflush(stdout);
  #endif

  t = strtok(tagbuffer, "|#"); // search first token

  // the first token must be GP2INFOTAG
  if (stricmp(t, GP2INFOTAG) != 0)  // test, if it's a gp2info section
    return 0;    // no valid gp2info section

  while (t = strtok(NULL, "|#")) {   // get next token

    // ok, t holds our actual token

    // printf(" ActualToken: [%.120s]\n", t); fflush(stdout);

    // WARNING: TAGS HAVE TO BE IN EXACTLY THIS ORDER (OR NOT???)

    if (stricmp(t, SPLIT1TAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;  // dunno, why that's necessary -> not cleared internally!?
      pTags->split1 = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->split1 = -1;
    }

    else if (stricmp(t, SPLIT2TAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;
      pTags->split2 = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->split2 = -1;
    }

    else if (stricmp(t, LAPSTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;
      pTags->laps = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->laps = -1;
    }

    else if (stricmp(t, SLOTTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;
      pTags->slot = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->slot = -1;
    }

    else if (stricmp(t, TYRETAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;
      pTags->tyrewear = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->tyrewear = -1;
    }

    else if (stricmp(t, LENGTHTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;
      pTags->length = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->length = -1;
    }

    else if (stricmp(t, YEARTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      errno = 0;
      pTags->year = strtoul(tvalue, NULL, 0);
      if ( errno != 0 ||
           tvalue > t+strlen(t)+1 ) pTags->year = -1;
    }

    else if (stricmp(t, NAMETAG) == 0) {
      tvalue = strtok(NULL, "|#");
      if (tvalue > t+strlen(t)+1) continue;
      _bprintf(pTags->name, MAXTAGSTRLEN, "%s", tvalue);
    }

    else if (stricmp(t, COUNTRYTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      if (tvalue > t+strlen(t)+1) continue;
      _bprintf(pTags->country, MAXTAGSTRLEN, "%s", (tvalue > t+strlen(t)+1) ? "(unknown Country)" : tvalue);

      //---- set the adjective ----
      // Warning: using gp2 string 0x0327 here: " von ", " of ", " van ", " de ", " di ",
      _bprintf(pTags->adjectivepost, MAXTAGSTRLEN, "%s%s", AHFGetStrNrESI(0x0327), pTags->country);
      //_bprintf(pTags->adjectivepre, MAXTAGSTRLEN, "%sian ", pTags->country);   // not correct! ;)
    }

    else if (stricmp(t, AUTHORTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      if (tvalue > t+strlen(t)+1) continue; // q&d fix against |Author||Year|1982| resulting into Author=Year
      _bprintf(pTags->author, MAXTAGSTRLEN, "%s", (tvalue > t+strlen(t)+1) ? "(unknown Author)" : tvalue);
    }

    else if (stricmp(t, EVENTTAG) == 0) {
      tvalue = strtok(NULL, "|#");
      if (tvalue > t+strlen(t)+1) continue;
      _bprintf(pTags->event, MAXTAGSTRLEN, "%s", (tvalue > t+strlen(t)+1) ? "(unknown Event)" : tvalue);
    }

  }

  if(pTags->name[0] == 0)    _bprintf(pTags->name, MAXTAGSTRLEN, "?");
  if(pTags->country[0] == 0) _bprintf(pTags->country, MAXTAGSTRLEN, "?");
  if(pTags->author[0] == 0) _bprintf(pTags->author, MAXTAGSTRLEN, "?");
  if(pTags->event[0] == 0) _bprintf(pTags->event, MAXTAGSTRLEN, "?");


  #ifdef PGITDEBUG
    printf("PGITExit: Name=%s,Cntry=%s,Laps=%d,Slot=%d,Tyre=%d,LenM=%d,Spl1=0x%04X,Spl2=0x%04X,Author=%s,Event=%s\n",
                  pTags->name, pTags->country,
                  pTags->laps, pTags->slot, pTags->tyrewear,
                  pTags->length,
                  pTags->split1, pTags->split2,
                  pTags->author, pTags->event); fflush(stdout);
  #endif


  return 1;

} // ParseGp2InfoTags()

//--------------------------------------------------------------

Gp2InfoTags *GetTrackTags(unsigned char tracknr)
// tracknr == 0..15
{

  return &TrackTags[tracknr];

} // GetTrackTags()
