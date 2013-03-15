#ifndef _FIXINT2F_H
#define _FIXINT2F_H


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------

//-----------------
//--- published ---
//-----------------

//--- geht komischerweise auch ohne publix machen??? ----
//--- function pointers to all functions ----
extern unsigned long (__near _cdecl *fpAHFGetOpSystem)(unsigned long);
extern void (__near _cdecl *fpAHFInitDivers)(unsigned long);
extern unsigned short (__near _cdecl _saveregs *fpAHFInt2f1600)(void);
extern unsigned short (__near _cdecl*fpAHFInt2f1500)(void);
extern unsigned short (__near _cdecl _saveregs *fpAHFInt2f150b)(unsigned short);
extern unsigned short (__near _cdecl _saveregs *fpAHFInt2f4a33)(void);


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

void InitFixInt2F(void);

//--------------------------
//--- special parameters ---
//--------------------------

#pragma aux AHFGetOpSystem parm [EAX] caller value [EAX];
#pragma aux AHFInt2f1600 value [AX];
#pragma aux AHFInt2f1500 value [CX /*leider nur eins*/];
#pragma aux AHFInt2f150b parm [ECX] caller value [AX]; //leider nur eins
#pragma aux AHFInt2f4a33 value [AX];
#pragma aux AHFInitDivers parm [EAX] caller;

unsigned long __near _cdecl AHFGetOpSystem(unsigned long);
unsigned short __near _cdecl _saveregs AHFInt2f1600(void);
unsigned short __near _cdecl AHFInt2f1500(void);
unsigned short __near _cdecl _saveregs AHFInt2f150b(unsigned short);
unsigned short __near _cdecl _saveregs AHFInt2f4a33(void);
void __near _cdecl AHFInitDivers(unsigned long);



#endif
