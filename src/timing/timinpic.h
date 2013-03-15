#ifndef _TIMING_H
#define _TIMING_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

//--- PICTURE DIMENSIONS ---

#define ARROWXDIM 15
#define ARROWYDIM 8
#define LEFTHDRXDIM 88
#define LEFTHDRYDIM 16
#define RIGHTHDRXDIM 68
#define RIGHTHDRYDIM 16
#define SECTORPICX 2
#define SECTORPICY 2
#define SECTORPICXDIM 164
#define SECTORPICYDIM 88
#define TABLETXDIM 176
#define TABLETYDIM 55
#define TOP3XDIM 15
#define TOP3YDIM 11

//--- POSITIONS WITHIN WHOLEBOARD ---

// position of the headers within WHOLEBOARD
#define _LEFTHDRX    20
#define _LEFTHDRY    0
#define _RIGHTHDRX   _LEFTHDRX + LEFTHDRXDIM
#define _RIGHTHDRY   0

// position of the tablet within WHOLEBOARD
#define _TABLETX     0
#define _TABLETY     LEFTHDRYDIM-1  // damit nicht 2facher schwarzer Rand

// position of the yellow pic within WHOLEBOARD
#define _TOP3MARKERX _TABLETX+TABLETXDIM+2

//--- POSITIONS WITHIN TABLET ---

#define TABROW1POSY  2
#define TABCOL1POSX  3   // for left aligned text
#define TABCOL2POSX  24  // for left aligned text
#define TABCOL3POSX  173 // for right aligned text


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern  unsigned long fntscrwidth;

//-----------------
//--- published ---
//-----------------

extern unsigned char picarrowup[];
extern unsigned char picarrowdown[];
extern unsigned char picattheline[];
extern unsigned char piclaptime[];
extern unsigned char picbehind[];
extern unsigned char picsectors[];
extern unsigned char pictablet[];
extern unsigned char pictop3[];


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void insertpicat(unsigned long, unsigned long,
                  unsigned long, unsigned long,
                   unsigned char [], unsigned char []);
void winsertpicat(unsigned long, unsigned long,
                   unsigned long, unsigned long,
                    unsigned char [], unsigned char [],
                     unsigned long);


#endif
