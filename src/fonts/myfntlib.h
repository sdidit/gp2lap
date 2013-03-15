#ifndef _MYFNTLIB_H
#define _MYFNTLIB_H


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

typedef struct {
    unsigned char *fntchars[256];
    unsigned long xdim;
    unsigned long ydim;
    unsigned long optspacing;
    unsigned char *name;
    unsigned char usedxdim[256]; // fuer non-proportional
} fontdef;


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- published ---
//-----------------

extern unsigned long fntscrwidth;   // default: 640

extern fontdef fntarrn6;
extern fontdef fntgp2_8x8;
extern fontdef fntverd7fk;
extern fontdef fntverd7k;
extern fontdef fntarial20;


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

//----- for Init() and Done() -----
void initfont(fontdef *);

//----- output functions -----
//void writefnt(char *, fontdef *, unsigned char *,
//               unsigned short, unsigned short, unsigned char);
void writefnt(char *, fontdef *, unsigned char *,
               unsigned long, unsigned long, unsigned char);
void writefntr(char *, fontdef *, unsigned char *,
                unsigned long, unsigned long, unsigned char);
void wwritefnt(char *, fontdef *, unsigned char *,
                unsigned long, unsigned long, unsigned char, unsigned short, unsigned short);
void wwritefntr(char *, fontdef *, unsigned char *,
                unsigned long, unsigned long, unsigned char, unsigned short, unsigned short);


//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#define WRITEVERD7FK(a, b, c, d, col)  writefnt(a, &fntverd7fk, b, c, d, col)
#define WRITEVERD7FKR(a, b, c, d, col)  writefntr(a, &fntverd7fk, b, c, d, col)

#define WRITEVERD7K(a, b, c, d, col)  writefnt(a, &fntverd7k, b, c, d, col)
#define WRITEVERD7KR(a, b, c, d, col)  writefntr(a, &fntverd7k, b, c, d, col)

#define WRITEARRN6(a, b, c, d, col)  writefnt(a, &fntarrn6, b, c, d, col)
#define WRITEARRN6R(a, b, c, d, col)  writefntr(a, &fntarrn6, b, c, d, col)

#define WRITEARIAL20(a, b, c, d, col)  writefnt(a, &fntarial20, b, c, d, col)
#define WRITEARIAL20R(a, b, c, d, col)  writefntr(a, &fntarial20, b, c, d, col)


#endif

