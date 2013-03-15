#ifndef _TIMING_H
#define _TIMING_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define LEFTHDRX 23
#define LEFTHDRY 3

#define RIGHTHDRX (LEFTHDRX+LEFTHDRXDIM)
#define RIGHTHDRY (LEFTHDRY)

#define TABLETX 3
#define TABLETY 18

// picture dimensions
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
