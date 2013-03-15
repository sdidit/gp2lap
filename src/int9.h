#ifndef _INT9_H
#define _INT9_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#include "keys.h"


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned char picbuf[0x4B000];
extern unsigned long InCockpitNow;

//-----------------
//--- published ---
//-----------------

extern void (__interrupt __far *OldInt9)();


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void __interrupt __far MyInt9();



#endif
