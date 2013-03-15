#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../gp2log.h"         // wegen GP2LAP_BASICVERSION
#include "../fonts/myfntlib.h"
#include "../trackinf.h"  // wegen TrackTags
#include "../dpmi.h"      // wegen mymemcpy() etc.
#include "../keyhand.h"	       // wegen ReDrawAllPages()
#include "../cfgmain.h"    // for GetCfgULong()
#include "../miscahf.h"
#include "svgabmp.h"
#include "vesa.h"              // wegen vesa_setpage() etc.
#include "gp2color.h"
#include "gp2pics.h"

// Rene 04-01-2000
#include "../track.h"
#include "../trackmap.h"

unsigned char (*ImageTranspTab)[3][256] = NULL;
unsigned char videopal[3*256];

#include "logo.inc"

//--------------------------------------------------------------

#pragma aux OnDecodePicData parm [EDI] [ECX];
void OnDecodePicData(char *pic, unsigned long picsize)
{
  unsigned short svgaobj = *(unsigned short *)IDACodeReftoDataRef(0x8dccf);

  //printf("called for obj %04u with %d pixels at %P\n", svgaobj, picsize, pic); fflush(stdout);
  //int i;
  //for (i = 0; i < 20*640; i++) pic[i] = GP2MENUBLUE;
  //for (i = 0; i < 420*640; i++) pic[i] = (*ImageTranspTab)[0][pic[i]];

  if ( (svgaobj < 96) && (picsize == 640*480) )
    if (svgaobj % 6 == 0)  // make sure it's the framed track pic, see F1PCSVGA.TXT layout
      OnSelectTrackImage(pic, svgaobj);

} // OnDecodePicData()
void (*fpOnDecodePicData)() = OnDecodePicData;

//--------------------------------------------------------------

//#pragma aux OnImgTranspTabLoad parm [EDI] [ECX];
void OnImgTranspTabLoad()
{
  int i;
  //char *pic = (void *) IDACodeReftoDataRef(0x7ad0b);
  //ImageTranspTab = (void *) IDACodeReftoDataRef(0x8dcb9);

  //for (i = 0; i < 420*640; i++) pic[i] = (*ImageTranspTab)[0][pic[i]];

} // OnImgTranspTabLoad()
void (*fpOnImgTranspTabLoad)() = OnImgTranspTabLoad;

//--------------------------------------------------------------

void OnSelectTrackImage(char *pic, unsigned short picnr)
{
  int row = 0;
  unsigned long tracknr = *(unsigned long *)IDACodeReftoDataRef(0x11a69);

  Gp2InfoTags *track = GetTrackTags(tracknr);


  if (track->name[0]) {  // supposing it's a custom track

    /////////////////////////////////
    // show more custom track details
    /////////////////////////////////

    WRITEVERD7KR("Laps: ", pic, TRKINFO_COL1Y + (row*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUYELLOW);
    _bprintf(atmp, sizeof(atmp), "%d", track->laps); // "?, using %d", DEFAULTLAPS);
    WRITEVERD7K(track->laps>0 ? atmp : "?", pic, TRKINFO_COL1Y + (row++*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUWHITE);

    WRITEVERD7KR("Length: ", pic, TRKINFO_COL1Y + (row*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUYELLOW);
    _bprintf(atmp, sizeof(atmp), "%dm", track->length); // "?, using %dm", GCLENGTHTOMETERS(DEFAULTLENGTH));
    WRITEVERD7K(track->length>0 ? atmp : "?", pic, TRKINFO_COL1Y + (row++*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUWHITE);

    WRITEVERD7KR("Slot: ", pic, TRKINFO_COL1Y + (row*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUYELLOW);
    _bprintf(atmp, sizeof(atmp), "%d", track->slot);
    WRITEVERD7K(track->slot>0 ? atmp : "?", pic, TRKINFO_COL1Y + (row++*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUWHITE);

    WRITEVERD7KR("Tyrewear: ", pic, TRKINFO_COL1Y + (row*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUYELLOW);
    _bprintf(atmp, sizeof(atmp), "%d", track->tyrewear);
    WRITEVERD7K(track->tyrewear>0 ? atmp : "?", pic, TRKINFO_COL1Y + (row++*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUWHITE);

    WRITEVERD7KR("Event, Year: ", pic, TRKINFO_COL1Y + (row*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUYELLOW);
    if (track->year > 0)
      _bprintf(atmp, sizeof(atmp), "%s, %d", track->event, track->year);
    else
      _bprintf(atmp, sizeof(atmp), "%s, ?", track->event);
    WRITEVERD7K(atmp, pic, TRKINFO_COL1Y + (row++*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUWHITE);

    WRITEVERD7KR("Author: ", pic, TRKINFO_COL1Y + (row*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUYELLOW);
    _bprintf(atmp, sizeof(atmp), "%s", track->author);
    WRITEVERD7K(atmp, pic, TRKINFO_COL1Y + (row++*(fntverd7fk.ydim-0)), TRKINFO_COL1X, GP2MENUWHITE);


    /////////////////////////////////
    // load (or calc) track images
    /////////////////////////////////

    OnDrawFramedTrackImage(pic, track);

  }

  // SearchDseg03();

} // OnSelectTrackImage()

//--------------------------------------------------------------

void OnDrawFramedTrackImage(char *canvas, Gp2InfoTags *track)
// image should be drawn to (x=179, y=72) using dimension of 440x330
{
  // Rene 04-01-2000
  DWORD *pCfgULong;
  pCfgULong = GetCfgULong("MenuTrackMap");
  if (pCfgULong && *pCfgULong && track->checksum != 0) {
    TrackSegInfo info;
    if (ReadTrackSegData(track->checksum, &info)) {
      DrawTrackMenuMap(canvas, &info);
      KillTrackSegInfo(&info);
    }
  }
/*
    int x, y, i;
    static FILE *bmpfile = NULL;

    _bprintf(atmp, sizeof(atmp), "%s\\%s", GetGP2Dir(), "test440.bmp");

    if ((bmpfile = fopen(atmp, "rb")) != NULL) {

      if (readstream_svgabmp(440, 330, &picbuf, &videopal, bmpfile) < 1) {
        printf("Error: while decoding [%s]\n", atmp);
        fflush(stdout);
      }
      else {
        //---- das picbuf Bild(440x330) in das framepic(640x480) ----
        for (y = 0; y < 330; y++)
          for (x = 0; x < 440; x++)
            canvas[(y+72)*640 + (x+179)] = picbuf[ (y*440)+x ];  // GP2MENUYELLOW;
      }

      fclose(bmpfile);

    }
    else {
      printf("Error: can't open [%s]\n", atmp); fflush(stdout);
    }
*/

} // OnDrawFramedTrackImage()

//--------------------------------------------------------------


void SaveScreenShotNow(void)
{
  FILE *fout;
  void *dest, *source;

  static unsigned long screenshotcnt = 0;
  unsigned int page = 0;
  signed int scr_size = 640*480;    // for svga
  unsigned long xdim, ydim;

  char *GP2_Zielpalette = (void *) IDACodeReftoDataRef(0x7f624);
  unsigned char *GP2_use_svga = (void *) IDACodeReftoDataRef(0x35b36);

  //---- determine the dims (and allocate svga buffer) ------
  if (*GP2_use_svga) {
    xdim = 640; ydim = 480;
  }
  else {
    xdim = 320; ydim = 200;
  }

  //---- fill workbuf, or if it's VGA it's still there ------
  if (*GP2_use_svga) {
    source = (unsigned char *)0x000A0000;
    dest = &picbuf;
    while(scr_size > 0) {
      vesa_setpage(page, GetSvgaGranularity() /*64*/);
      mymemcpy(dest, source, (scr_size > 65536 ? 65536 : scr_size));
      scr_size -= 65536;
      dest = (void *)((unsigned long)dest + 65536);
      page++;
    }
    dest = &picbuf;
  }
  else
    dest = (unsigned char *)0x000A0000;


  //---- now write dest to file ------
  _bprintf(atmp, sizeof(atmp), "gp2_%04u.bmp", screenshotcnt++);
  if ((fout = fopen(atmp, "wb")) != NULL) {
    savestream_svgabmp(dest, 0 /*xpos*/, 0 /*ypos*/, xdim, ydim,
                       GP2_Zielpalette, fout);
    fclose(fout);
  }

  mymemset(&picbuf, 0, sizeof(picbuf));
  ReDrawAllPages(NULL);

} // SaveScreenShotNow()


//--------------------------------------------------------------

#define LOGOXPOS 499
#define LOGOYPOS 9

void InsertLogoNow(void)
{
  char *pic = (void *) IDACodeReftoDataRef(0x7ad0b);
  int x, y;

  //--- insert version into logo ----
  WRITEVERD7KR(GP2LAP_BASICVERSION, pic, LOGOYPOS + 0, LOGOXPOS + LOGOXDIM-6, GP2MENURED);

  for (y = 0; y < LOGOYDIM; y++)
    for (x = 0; x < LOGOXDIM; x++)
      if (logo[y*LOGOXDIM+x] != 0xFF)
        pic[((LOGOYPOS+y)*640) + LOGOXPOS + x] = logo[y*LOGOXDIM+x];

} // InsertLogoNow()
void (*fpInsertLogoNow)() = InsertLogoNow;


//--------------------------------------------------------------


void SearchDseg03(void)
{
  // temporary used for debugging
  extern unsigned long   GP2_DataStartAdr;
  extern unsigned long   GP2_DataEndAdr;

  static unsigned long i;
  //static char *searchtoken = "ATZ&"; // null terminieren!!!
  static char *searchtoken = "DEADBEAF"; // null terminieren!!!

  printf("\nGP2_DataStartAdr==%P\nGP2_DataEndAdr==%P\nsize==%d bytes\n",
         GP2_DataStartAdr, GP2_DataEndAdr, GP2_DataEndAdr-GP2_DataStartAdr);
  printf("L_DataStart (IDA dseg03 0xc03c8) is at %P\n", IDACodeReftoDataRef(0x14cca));
  fflush(stdout);

  for (i = GP2_DataStartAdr; i <= GP2_DataEndAdr; i++) {

    if (strnicmp((char *)i, searchtoken, strlen(searchtoken)) == 0) {
      printf("SearchToken found at %P (IDA dseg03 0x%x)\n", i, i - GP2_DataStartAdr + 0xc03c8);
      fflush(stdout);
      break;
    }

  }

} // SearchDseg03()
