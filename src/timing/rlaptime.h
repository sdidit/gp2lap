#ifndef _RLAPTIME_H
#define _RLAPTIME_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define TOPCOUNTMIN 3
#define TOPCOUNTMAX 9
#define TOPCOUNTDEFAULT 6

#define _RLWHOLEBOARDX 2
#define _RLWHOLEBOARDY 2


// spacing between tablets when atlAcrossTop active
#define _RLTABLETSPACINGX  20 // ARROWXDIM==15
// otherwise
#define _RLTABLETSPACINGY  1


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned char picbuf[0x4B000];
extern char atmp[360];

extern unsigned long FIRSTPOS;  // fÅr Anzeige in der Tabelle
extern unsigned long LASTPOS;
extern unsigned long atlAcrossTop;

//-----------------
//--- published ---
//-----------------

extern unsigned char myorder[40];

//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void __near MakeLaptimeBoard(unsigned char);
void MakeTopX(unsigned char);
void InitMyLineOrder();
void SetLaptimeTopCount(unsigned long);
unsigned long GetLaptimeTopCount();


#endif

