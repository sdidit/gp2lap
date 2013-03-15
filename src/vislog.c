#include "stdinc.h"
#include "fonts/myfntlib.h"
#include "dpmi.h"  // wegen mymemset() etc.
#include "miscahf.h"  // wegen atmp
#include "keys.h"
#include "keyhand.h"  // wegen PAGEISACTIVE, PAGE_LOG etc.
#include "svga/vesa.h"
#include "svga/gp2color.h"
#include "vislog.h"



static unsigned long firstline, vislogLineNr;
static unsigned long vislogStartingOfs, vislogDispMaxLines;

static unsigned long vislogstrcnt = VISLOGSTRCNTMAX;  // should be VISLOGSTRCNTMIN..VISLOGSTRCNTMAX
static struct VisLogLine Lines[VISLOGSTRCNTMAX];

// PS: bei 194 Zeilen picbuf werden nur max. 18 Zeilen dargestellt

unsigned char ReDrawBufVisLog[0x4b000];

//------------------------------------------

void VisLogUpdate(void)
{
  static unsigned long i, lastnr;
  static int actline;

  if ((InCockpitNow /*noetig da sonst GPF*/) && PAGEISACTIVE(PAGE_LOG)) {

    //--- 11/99 neu ---
    //mymemset(&picbuf, 0, sizeof(picbuf));  // gpf'd damit sporadisch
    memset(&picbuf, 0, sizeof(picbuf));  // den Bildschirmausgabebuffer leeren


    if (*GP2_use_svga)
      WRITEVERD7K("---- The last log lines ----", &picbuf, VLOGSVGAROW1Y, VLOGSVGACOL2X, GP2YELLOW);
    else
      wwritefnt("---- The last log lines ----", &fntgp2_8x8, &picbuf, 4, 2, 31, 320, 0);

    lastnr = 0xffffffff;
    actline = firstline;
    for (i = vislogDispMaxLines; i > 0; i--) {

      if (--actline < 0) actline = vislogstrcnt-1;
      if ((Lines[actline].number) && (Lines[actline].number < lastnr)) {
        lastnr = Lines[actline].number;
        if (*GP2_use_svga) {
            _bprintf(atmp, sizeof(atmp), "%0.3u: ", lastnr);
            WRITEVERD7KR(&atmp, &picbuf, VLOGSVGAROW2Y + ((i-1)*(fntverd7k.ydim-1)), VLOGSVGACOL2X, GP2YELLOW);
            WRITEARRN6(&Lines[actline].line[0], &picbuf, VLOGSVGAROW2Y + ((i-1)*(fntarrn6.ydim-1)), VLOGSVGACOL2X, GP2WHITE);
        }
        else
          wwritefnt(&Lines[actline].line[0], &fntgp2_8x8, &picbuf, 12 + ((i-1)*fntgp2_8x8.ydim), 2, 31, 320, 0);
      }
    }

    SaveThisPage(&ReDrawBufVisLog, &picbuf, GetCopySvgaLinesNum());
    ReDrawAllPages(&OnVisLogReDraw);
  }

} // VisLogUpdate();

//------------------------------------------

void VisLogStart()
{
  mymemset((void *)&Lines, 0, sizeof(Lines));
  firstline = 0;
  vislogLineNr = 1;
  vislogDispMaxLines = VLOGLINETICK;
  vislogStartingOfs = 0;
  VisLogLine("Welcome, VisualLog Output has been started.");

} // VisLogStart()

//------------------------------------------

void VisLogLine(char *logstr)
{
  mymemcpy((void *)&Lines[firstline].line[0], logstr, VISLOGSTRLEN);
  Lines[firstline++].number = vislogLineNr++;

  firstline %= vislogstrcnt;
  VisLogUpdate();

} // VisLogLine()

//------------------------------------------

void VisLogSetStrCount(unsigned long cnt)
// parameters: cnt==new number of debug lines, should be VISLOGSTRCNTMIN<=cnt<=VISLOGSTRCNTMAX
{
  //--- 11/99 ignored ----
  //vislogstrcnt = (cnt > VISLOGSTRCNTMAX) ? VISLOGSTRCNTMAX : cnt;
  //vislogstrcnt = (vislogstrcnt < VISLOGSTRCNTMIN) ? VISLOGSTRCNTMIN : vislogstrcnt;

  VisLogStart();

} // VisLogSetStrCount()

//------------------------------------------

unsigned long VisLogGetStrCount(void)
// parameters: -
{
  return vislogstrcnt;

} // VisLogGetStrCount()

//----------------------------------------------------------------------

void OnVisLogReDraw(int below)
{
  //VisLogUpdate();  // probably fast enough, to simply update again.
  //                 // although VisLog and Trackmap together should raise PO somehow...

  if (below)
    mypicinsertbelow(&picbuf, &ReDrawBufVisLog, GetCopySvgaLinesNum());
  else
    mypicinsertabove(&picbuf, &ReDrawBufVisLog, GetCopySvgaLinesNum());

} // OnVisLogReDraw()

//----------------------------------------------------------------------

void OnActivateVisLog(void)
{
  VisLogUpdate();
}

//----------------------------------------------------------------------

void OnVisLogKeyHit(KeyEvent *mks)
{
  switch (mks->scancode) {

      case KEY_2 :

          if (mks->lctrlactive && GetKbdHandlerCnt() > 1) {
            // only if we're the in multi page mode and CTRL active
            PAGESETOFF(PAGE_LOG);
            RemoveKbdHandler(&OnVisLogKeyHit);
            KeyEventClear(mks);
            // a bit clumsy, but it works...
            mymemset(&picbuf, 0, sizeof(picbuf));
            ReDrawAllPages(&OnVisLogReDraw);
          }
          else {
            vislogDispMaxLines += VLOGLINETICK;
            vislogDispMaxLines %= (VLOGLINETICK * 5);

            if (vislogDispMaxLines == 0)
              vislogDispMaxLines = VLOGLINETICK;

            VisLogUpdate();
          }

          break;
/*
      case KEY_TICK :

          //(mks->lshiftactive) ?
          vislogStartingOfs++;
          VisLogUpdate();

          break;
*/
  }

} // OnVisLogKeyHit()

