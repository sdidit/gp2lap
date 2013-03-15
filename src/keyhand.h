#ifndef _KEYHAND_H
#define _KEYHAND_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#include "keys.h"
#include "pages.h"
#include "keyqueue.h"   // wegen KeyEvent

#define AGAINSTLDR  0
#define AGAINSTYOU  1
#define AGAINSTSEL  2

#define KBHANDLERMAX    4
#define KBHANDLERERR    KBHANDLERMAX+1


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

typedef void (*OnActivatePageModuleFunc)(void);
typedef void (*KbdHandlerFunc)(KeyEvent *);
typedef void (*ReDrawHandlerFunc)(int);


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned char picbuf[0x4B000];
extern volatile unsigned long activepage;
extern volatile unsigned long screenshotnow;

extern unsigned long tablepage[2];
extern unsigned char qwhatopp[3];


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void RemoveAllPages();

unsigned long GetKbdHandlerCnt();
unsigned long RegisterKbdHandler(void *, void *);
void CallRecentKbdHandler(KeyEvent *);
void CallAllKbdHandlers(KeyEvent *);
void RemoveAllKbdHandlers();

unsigned long SearchKbdHandler(void *);
unsigned long RemoveKbdHandler(void *);


void SaveThisPage(void *, void *, unsigned long);
void ReDrawAllPages(void *);

void SelectNextPage(unsigned char sessionmode);

//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#define REL(key)  (key | 0x80)  // make release code of key
#define PAGEISACTIVE(page) (activepage & page)
#define PAGESETON(page)  (activepage |= page);
#define PAGESETOFF(page)  (activepage &= ~page);


#endif

