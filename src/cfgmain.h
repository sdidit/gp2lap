#ifndef _CFGMAIN_H
#define _CFGMAIN_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define CFGQUIET    // supress status messages

//----- define some cfg item types -----
#define TERMITEM    0    // for marking end of itemlist
#define LONGTYPE    1    // signed value
#define ULONGTYPE   2    // unsigned value
#define STRTYPE     3    // string between quotation marks ("), maxlen == 188


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

//----- one item to be stored in memory -----
typedef struct {
    void *value;
    long int type;
    char *name;         // case insensitive
} itemdef;


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

//----- for Init() and Done() -----
long InitCfgValues(const char *, char *[], itemdef *, char **);
void DoneCfgValues(void);

//----- get the value functions -----
long *GetCfgLong(const char *);
unsigned long *GetCfgULong(const char *);
char *GetCfgString(const char *);


#endif







//union mp {
//  char *pText;
//  long *pLong;
//};
