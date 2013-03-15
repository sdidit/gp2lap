#ifndef _RBEHIND_H
#define _RBEHIND_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define _RBWHOLEBOARDX 2
#define _RBWHOLEBOARDY 2


// spacing between tablets when atlAcrossTop active
#define _RBTABLETSPACINGX  20 // only for compatib. with rlaptime
// otherwise
#define _RBTABLETSPACINGY  1


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned char picbuf[0x4B000];
extern char atmp[360];

extern unsigned char *pNumLapsDone; // BYTE // used for "x/y laps completed
extern unsigned short *GP2_LapsInThisRace;
extern struct gp2timetable *GP2_RaceTimes;
extern unsigned long *GP2_NumStarters;
extern unsigned long atlAcrossTop;

extern unsigned long FIRSTPOS;
extern unsigned long LASTPOS;

extern unsigned char ldratsections[3];
extern unsigned long prevlapsplits[0x40][3];


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void SetRBehindCol0(unsigned char);
void SetRBehindCol1(unsigned char);
void SetRBehindCol2(unsigned char);
unsigned char GetRBehindCol0(void);
unsigned char GetRBehindCol1(void);
unsigned char GetRBehindCol2(void);

void __near MakeBehindBoard(unsigned char);



#endif

