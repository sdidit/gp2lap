#include <stdio.h>
#include "dpmi.h"
#include "timing/timinstd.h"
#include "timing/gp2struc.h"
#include "fonts/myfntlib.h"
#include "vislog.h" // wegen OnVisLogKeyHit()
#include "basiclog.h"  // wegen LogLine()
#include "trackmap.h"
#include "gp2hook.h"
#include "carinfo.h"
#include "timing/atl.h"
#include "svga/gp2pics.h" // wegen SaveScreenShotNow()
#include "keyqueue.h"
#include "keyhand.h"


static KeyEvent actkbd;
static unsigned long ii, hcnt = 0;
static long i;
static KbdHandlerFunc kbhandler[KBHANDLERMAX];
static ReDrawHandlerFunc rdhandler[KBHANDLERMAX];


//-----------------------------------------------------------------------

void KeyMainHandler()   // called for every frame or so...
{

  if ( KeyEventDequeue(&actkbd) ) {  // only if keys present

      // LogLine("KeyEvent available\n");

      CallRecentKbdHandler(&actkbd); // Warning: including the activate key event

      // activating a page (including its kbd handler) ?
      switch (actkbd.scancode) {

        // Note:
        // It is planned, that all kbd handlers get registered within
        // this switch statement following a certain keystroke. From that
        // point on, they'll get all key events, including the key for
        // removing "themselves".
        //
        // Due to the mymemset(&picbuf, 0, ...) behaviour of all PAGE modules
        // we also need ReDraw routines for each module
        //
        // Rene, maybe you can take a look at your OnModuleTrackMapKeyHit() etc...
        //       (currently the track map doesn't get disabled by itself (only with '0'))

        case KEY_1:
            if (!PAGEISACTIVE(PAGE_CARINFO)) {
                if (!actkbd.lctrlactive) RemoveAllPages();
                RegisterKbdHandler(&OnModuleCarInfoKeyHit, &OnCarInfoReDraw);
                PAGESETON(PAGE_CARINFO);
                OnActivateCarInfo();
            }
            break;
            
        case KEY_2:
            if (!PAGEISACTIVE(PAGE_LOG)) {
              if (!actkbd.lctrlactive) RemoveAllPages();
              RegisterKbdHandler(&OnVisLogKeyHit, &OnVisLogReDraw);
              PAGESETON(PAGE_LOG);
              OnActivateVisLog();
            }
            break;

        case KEY_8:
            if (!PAGEISACTIVE(PAGE_MAP)) {
              if (!actkbd.lctrlactive) RemoveAllPages();
              RegisterKbdHandler(&OnModuleTrackMapKeyHit, &OnTrackMapReDraw);
              PAGESETON(PAGE_MAP);
              OnActivateTrackMap();
            }
            break;

        case KEY_9:
            if (!PAGEISACTIVE(PAGE_ATTHELINE)) {
              if (!actkbd.lctrlactive) RemoveAllPages();
              RegisterKbdHandler(&OnATLKeyHit, &OnATLReDraw);
              PAGESETON(PAGE_ATTHELINE);
              OnActivateATL();
            }
            break;


        case KEY_0:
            RemoveAllPages();
            break;

        // additional stuff

        case KEY_F12:

            SaveScreenShotNow();
            break;

        case KEY_F:
            ToggleFastMotion(); // see gp2hook.c
            break;

      } // switch

  } // key present


} // KeyMainHandler()
void (*fpKeyMainHandler)() = KeyMainHandler;





//-----------------------------------------------------------------------

void RemoveAllPages(void)
{
  activepage = PAGE_NONE;
  RemoveAllKbdHandlers();
  mymemset(&picbuf, 0, sizeof(picbuf));

} // RemoveAllPages()



//-----------------------------------------------------------------------

// NOCH WOANDERS HIN!!!

void SelectNextPage(unsigned char sessionmode /*, struct mykdbstatus..*/ )
{
  if (sessionmode & 0x80) {  // race
    if (actkbd.lshiftactive) {
      if (tablepage[sessionmode >> 7]-- == 0)
        tablepage[sessionmode >> 7] = RATL_END;
    }
    else {
      if (++tablepage[sessionmode >> 7] > RATL_END)
        tablepage[sessionmode >> 7] = RATL_START;
    }
  }
  else {  // no race
    if (actkbd.lshiftactive) {
      if (tablepage[sessionmode >> 7]-- == 0)
        tablepage[sessionmode >> 7] = QATL_END;
    }
    else {
      if (++tablepage[sessionmode >> 7] > QATL_END)
        tablepage[sessionmode >> 7] = QATL_START;
    }
  }

} // SelectNextPage()









//-----------------------------------------------------------------------
// The Handler stuff.
// Note: Though being called "Kbd" it's for both, Kbd and Redrawing
//-----------------------------------------------------------------------

unsigned long GetKbdHandlerCnt(void)
{
  return hcnt;

} // GetKbdHandlerCnt()

//-----------------------------------------------------------------------

unsigned long RegisterKbdHandler(void *hptr, void *rdptr)
// register a new kbd handler
// šbergabe: hptr == handler function
// Rckgabe: 0 == failed (table full), 1 == success

// EVTL: optionales Release Code blocking noch mit rein

{
  if (hcnt >= KBHANDLERMAX)
    return 0;                  // nix mehr frei

  if (SearchKbdHandler(hptr) != KBHANDLERERR)
    return 0;                  // schon registriert

  for (i = (KBHANDLERMAX-1); i > 0; i--) {
    kbhandler[i] = kbhandler[i-1];
    rdhandler[i] = rdhandler[i-1];
  }

  //if (kbhandler[i] == NULL) {
  kbhandler[0] = hptr;
  rdhandler[0] = rdptr;
  hcnt++;


  return 1;

} // RegisterKbdHandler()

//-----------------------------------------------------------------------

void CallRecentKbdHandler(KeyEvent *mks)
{
  if (hcnt > 0) {
    if (kbhandler[0] != NULL)
      kbhandler[0](mks);
  }

} // CallRecentKbdHandler()

//-----------------------------------------------------------------------

void CallAllKbdHandlers(KeyEvent *mks)
{
  static unsigned long tmpcnt;

  tmpcnt = hcnt;

  if (tmpcnt) {

    for (i = 0; (tmpcnt) && (i < KBHANDLERMAX); i++)
      if (kbhandler[i] != NULL) {
        kbhandler[i](mks);
        tmpcnt--;
      }

  }

} // CallAllKbdHandlers()

//-----------------------------------------------------------------------

void RemoveAllKbdHandlers(void)
{
  for (i = 0; i < KBHANDLERMAX; i++) {
    kbhandler[i] = NULL;
    rdhandler[i] = NULL;
  }
  hcnt = 0;

} // RemoveAllKbdHandlers()

//-----------------------------------------------------------------------

// UMBENENNEN: SearchHandler oder so
unsigned long SearchKbdHandler(void *hptr)
// get a registered kbd/redraw handler's index within kbdhandler[]
// šbergabe: hptr == handler function
// Rckgabe: < KBHANDLERMAX == index, KBHANDLERERR == can't find hptr
{
  if (hcnt > 0) {
    for (i = 0; i < KBHANDLERMAX; i++)
      if (kbhandler[i] == hptr ||
          rdhandler[i] == hptr) return i;
  }

  return KBHANDLERERR;

} // SearchKbdHandler()

//-----------------------------------------------------------------------

unsigned long RemoveKbdHandler(void *hptr)
// call to remove a registered kbd handler
// šbergabe: hptr == handler function
// Rckgabe: 0 == failed, 1 == success
{
  if ( (hcnt > 0) && ((i = SearchKbdHandler(hptr)) != KBHANDLERERR) ) {

    for (ii = i; ii < (KBHANDLERMAX-1); ii++) {
      kbhandler[ii] = kbhandler[ii+1];
      rdhandler[ii] = rdhandler[ii+1];
    }

    kbhandler[KBHANDLERMAX-1] = NULL;
    rdhandler[KBHANDLERMAX-1] = NULL;
    hcnt--;
    return 1;
  }

  return 0;

} // RemoveKbdHandler()


//-----------------------------------------------------------------------


void SaveThisPage(void *dest, void *src, unsigned long lines)
{

  if (hcnt > 0) {
  //if (hcnt > 1) { // optimization: it only make sense, if we have at least to handlers

    mymemcpy(dest, src, lines*640);

  }

} // SaveThisPage()

//-----------------------------------------------------------------------

void ReDrawAllPages(void *caller)
// call to redraw all active pages, except caller's
// ACHTUNG: muss auch gehen, falls caller gar nicht mehr in TAB..., oder z.B. caller==NULL
// šbergabe: caller == &OnReDreaw handler of the caller, i.e. for making sure not to redraw caller
// Rckgabe: -
{
  static unsigned long caller_priority;

  if (hcnt > 0) {
  //if (hcnt > 1) {   // optimization: it only make sense, if we have at least to handlers

/*
    sprintf(strbuf, "ReDrawAllPages() called by %P\n" \
                    "  kbhandler: [ %P | %P | %P | %P ]\n" \
                    "  rdhandler: [ %P | %P | %P | %P ]\n" \
                    , caller,
                    kbhandler[0], kbhandler[1], kbhandler[2], kbhandler[3],
                    rdhandler[0], rdhandler[1], rdhandler[2], rdhandler[3] );
    printf(strbuf); fflush(stdout);
*/

    //-------------------------------------------------
    // Note: one priority ([caller_priority]) already drawn
    //       (i > caller_priority): to be drawn "below" current picbuf[] stuff
    //       (i < caller_priority): to be drawn "above" current picbuf[] stuff
    // Note: if caller==NULL: caller_priority = KBHANDLERMAX+1 (i.e. all remaining pages have higher priority)
    caller_priority = SearchKbdHandler(caller);


    // draw higher priorities
    for (i = caller_priority-1; i >= 0; i--) {
      if ( (rdhandler[i] != NULL) && (rdhandler[i] != caller) ) {
        //sprintf(strbuf, "    (%u) now redrawing %P (above)\n", i, rdhandler[i]);
        //printf(strbuf); fflush(stdout);
        rdhandler[i]( 0 /* !below */ );
      }
    }

    // draw lower priorities
    for (i = caller_priority+1; i < hcnt; i++) {
      if ( (rdhandler[i] != NULL) && (rdhandler[i] != caller) ) {
        //sprintf(strbuf, "    (%u) now redrawing %P (below)\n", i, rdhandler[i]);
        //printf(strbuf); fflush(stdout);
        rdhandler[i]( 1 /* below */);
      }
    }

  } // if hcnt > 1

} // ReDrawAllPages()
