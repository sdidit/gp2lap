#ifndef _MISCAHF_H
#define _MISCAHF_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define XORVAL 0x48


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned long GP2_CodeStartAdr;

//-----------------
//--- published ---
//-----------------

extern char atmp[360];


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

char *updown (char *);
char *dexor(char *);
unsigned char *IDAtoFlat(unsigned long);
unsigned char *IDACodeReftoDataRef(unsigned long);


//void mypicinsertbelow(char *, char *, unsigned long);
//void mypicinsertabove(char *, char *, unsigned long);


//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#include "mypicins.inc"



#endif
