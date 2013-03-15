#ifndef _TRACKINF_H
#define _TRACKINF_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define GP2INFOTAG "GP2INFO"        // #GP2INFO|Name|Sepang|Country|Malaysia
#define NAMETAG    "Name"           // |Name|Sepang|
#define COUNTRYTAG "Country"        // |Country|Malaysia|

#define AUTHORTAG  "Author"
#define YEARTAG    "Year"
#define EVENTTAG   "Event"

#define LAPSTAG    "Laps"           // |Laps|56|
#define SLOTTAG    "Slot"           // |Slot|1|
#define TYRETAG    "Tyre"           // |Tyre|25000|
#define LENGTHTAG  "LengthMeters"   // |LengthMeters|5544|

#define SPLIT1TAG  "Split1"         // |Split1|0x0|Split2|0x0|
#define SPLIT2TAG  "Split2"


#define MAXTAGSTRLEN    40

// the following may be used, if certain track tags are missing

#define DEFAULTLAPS      13
#define DEFAULTLENGTH    4310    // i.e. 1313m
#define DEFAULTTYREWEAR  0x4EAC  // the default for Brazil


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

typedef struct _Gp2InfoTags {
        //char *name;
        //char *country;
        char name[MAXTAGSTRLEN];
        char country[MAXTAGSTRLEN];

        char author[MAXTAGSTRLEN];
        char event[MAXTAGSTRLEN];
        char adjectivepre[MAXTAGSTRLEN];   // f.i. "Brazilian "
        char adjectivepost[MAXTAGSTRLEN];   // f.i. " of Europe"

        long year;

        long laps;      // for the following members: -1 == not set
        long slot;
        long tyrewear;
        long length;
        long split1;
        long split2;
        DWORD checksum;  // Rene 04-01-2000 (for track pic in menu)
} Gp2InfoTags;


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- published ---
//-----------------

extern long int Split1Seg;    // noch aendern -> wieder weg
extern long int Split2Seg;    // noch aendern -> wieder weg


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void CatchTrackInfos();
char *TrackNrToFilename(unsigned long, char *);

void FrankOnTrackFileLoad(char *);
unsigned long ParseGp2InfoTags(char *, Gp2InfoTags *);

Gp2InfoTags *GetTrackTags(unsigned char);


//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#define GCLENGTHTOMETERS(l)  ((l * 1000) / 3282)
#define METERSTOGCLENGTH(l)  ((l * 3282) / 1000)



#endif
