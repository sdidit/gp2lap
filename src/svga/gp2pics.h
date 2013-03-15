#ifndef _GP2PICS_H
#define _GP2PICS_H


#include "../trackinf.h"   // for Gp2InfoTags

//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define TRKINFO_COL1X  250
#define TRKINFO_COL1Y  411


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern unsigned char picbuf[0x4B000];
extern char atmp[360];


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void OnDecodePicData(char *, unsigned long);
void OnImgTranspTabLoad();
void OnSelectTrackImage(char *, unsigned short);
void OnDrawFramedTrackImage(char *, Gp2InfoTags *);

void SaveScreenShotNow();
void InsertLogoNow();

void SearchDseg03();


#endif
