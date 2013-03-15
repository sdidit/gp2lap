#ifndef _VISLOG_H
#define _VISLOG_H


#include "keyqueue.h"   // wegen KeyEvent

//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define VISLOGSTRLEN 128

#define VISLOGSTRCNTMIN 3
#define VISLOGSTRCNTMAX 40
#define VISLOGSTRCNTDEFAULT 12


#define VLOGSVGAROW1Y   8
#define VLOGSVGAROW2Y   18
#define VLOGSVGACOL1X   8
#define VLOGSVGACOL2X   30

#define VLOGLINETICK    5


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

struct VisLogLine {
  char line[VISLOGSTRLEN];
  unsigned long number;    // 0 == not used
};


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned char picbuf[0x4B000];
extern unsigned char *GP2_use_svga;

extern unsigned long InCockpitNow;
extern volatile unsigned long activepage;


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void VisLogUpdate();
void VisLogStart();
void VisLogLine(char *);
void VisLogSetStrCount(unsigned long);
unsigned long VisLogGetStrCount();
void OnVisLogReDraw(int);
void OnActivateVisLog();

// warum bei verwendung von <struct mykbdstatus> kein fehler????????
void OnVisLogKeyHit(KeyEvent *);


#endif

