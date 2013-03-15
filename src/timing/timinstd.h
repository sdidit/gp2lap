#ifndef _TIMINSTD_H
#define _TIMINSTD_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define GP2BLACK      1
#define GP2GREY       16
#define GP2LIGHTGREY  27
#define GP2WHITE      31
#define GP2RED        44
#define GP2GREEN      111
#define GP2BLUE       128
#define GP2BROWN      38
#define GP2PURPLE     255


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- imported ----
//-----------------

extern struct gp2order *GP2_FastestCars;
extern struct gp2order *GP2_RaceOrder;
extern struct gp2cartable *GP2_Cars;

//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void MakeMyDrivers(struct gp2drivernames *);
char *GetMyDriverName(unsigned char);

int __myDecompTime(long *, long *, long *, long *, unsigned long *);
char *myGetTimeString(unsigned long *);
char *myGetTimeStringSlim(unsigned long *);
unsigned long CalcTimeDiff(unsigned long, unsigned long);

struct singlecarstruct *Carid2Carstruct(unsigned char);
unsigned char Carid2Qualpos(unsigned char);
unsigned char CalcLapsDown(struct singlecarstruct *, struct singlecarstruct *);
unsigned char GetNumRunners();
unsigned char GetNumCarsInLeadlap();

void CreateSessionImage();
unsigned long SessionImageChanged();

//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#define CalcValueDiff(v1, v2)  ( (v1 > v2) ? (v1 - v2) : (v2 - v1) )

#define myGP2_TIME_INV_MASK     0xf0000000UL
#define myGP2_TIME_ISVALID(t)   (!((t) & myGP2_TIME_INV_MASK))

#define GP2TIMEINVMASK     myGP2_TIME_INV_MASK
#define GP2TIMEVALID(t)   (!((t) & myGP2_TIME_INV_MASK))  // as myGP2_TIME_ISVALID()

#define QPOS2CARID(pos) (GP2_FastestCars->car[pos] & 0x3F)  // nur unteren 6 bits
#define QPOS2CARIDFULL(pos) (GP2_FastestCars->car[pos])  // vollst., nicht nur unteren 6 bits
#define POS2CARID(pos) (GP2_RaceOrder->car[pos] & 0x3F)  // nur unteren 6 bits
#define POS2CARIDFULL(pos) (GP2_RaceOrder->car[pos])  // vollst., nicht nur unteren 6 bits

#define SEG_KILLSTATEBITS(segnr) (segnr & 0x07FF) // 5 oberen bits killen
#define SEG_HASEQUALSEG(segnr) (segnr & 0x2000)

#define HUMANCARID(id) (id & 0x80)
#define LINKEDCARID(id) (id & 0x40)

#define CARISRUNNING(car) ( ((car.remove_from_track & 0x20) == 0) || (car.unk_5E & 1) )
#define PCARISRUNNING(pcar) ( ((pcar->remove_from_track & 0x20) == 0) || (pcar->unk_5E & 1) )


#endif
