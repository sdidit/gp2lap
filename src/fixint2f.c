#include "stdinc.h"
#include "dpmi.h"
#include "basiclog.h"
#include "cfgmain.h"
#include "miscahf.h"
#include "fixint2f.h"


//--- function pointers to all functions ----
unsigned long (__near _cdecl *fpAHFGetOpSystem)(unsigned long) = AHFGetOpSystem;
void (__near _cdecl *fpAHFInitDivers)(unsigned long) = AHFInitDivers;
unsigned short (__near _cdecl _saveregs *fpAHFInt2f1600)(void) = AHFInt2f1600;
unsigned short (__near _cdecl*fpAHFInt2f1500)(void) = AHFInt2f1500;
unsigned short (__near _cdecl _saveregs *fpAHFInt2f150b)(unsigned short) = AHFInt2f150b;
unsigned short (__near _cdecl _saveregs *fpAHFInt2f4a33)(void) = AHFInt2f4a33;


//--- internal values for solving the int 2f problem ----
unsigned short ValueInt2f1600 = 0;
unsigned long ValueInt2f1500 = 0;
unsigned long ValueInt2f150b[26];  // for all drives
unsigned short ValueInt2f4a33 = 0;


//--------------------------------------------------------------

void InitFixInt2F(void)
{
  unsigned char i;

  //----- WINDOWS ENHANCED MODE INSTALLATION CHECK -------
  ValueInt2f1600 = myint2f1600();
  ValueInt2f1500 = myint2f1500();
  for (i = 0; i < 26; i++) ValueInt2f150b[i] = myint2f150b(i);
  ValueInt2f4a33 = myint2f4a33();
  //printf("ok\nAX == 0x%04X\n", ValueInt2f1600);

} //InitFixInt2F()

//--------------------------------------------------------------


//--- 8/99 for solving the int 0x2f bug ----
unsigned long __near _cdecl AHFGetOpSystem(unsigned long when)
// called before (when==0)/after (when==1) gp2's OS check
// Rckgabe: 0 == don't detect win
{
  if (when == 0) {
    //--- 08/99 --- disable int 2f based stuff for CalcVideo/CPU speed -----
    memset(IDAtoFlat(0x7b67e), 0x90, 7);  // no win9x enter critical section
    memset(IDAtoFlat(0x7b698), 0x90, 7);  // no win9x leave critical section
    //LogLine(when == 0 ? "Entering GetOS\n" : "Leaving GetOS\n");
    if (GetCfgULong("PureDOS") && *GetCfgULong("PureDOS")) {
      LogLine("- PureDOS activated\n");
      return 0;
    }
  }
  return 1;

} // AHFGetOpSystem()


//--------------------------------------------------------------


//--- 8/99 for solving the int 0x2f bug ----
unsigned short __near _cdecl _saveregs AHFInt2f1600(void)
// called from gp2 instead of int 2f
// šbergabe: -
// Rckgabe: AX == win status
{
  return ValueInt2f1600;  // DO NOT CHANGE this function (only AX should be changed)
                          // _saveregs somehow doesn't seems to work

} // AHFInt2f1600()


//--------------------------------------------------------------


//--- 8/99 for solving the int 0x2f bug ----
unsigned short __near _cdecl AHFInt2f1500(void)
// called from gp2 instead of int 2f
// šbergabe: -
// Rckgabe: BX = number of CD-ROM drive letters used
//           CX = starting drive letter (0=A:)
{
  movtobx(ValueInt2f1500 >> 16);    // DO NOT CHANGE this function
  return (unsigned short)ValueInt2f1500;

} // AHFInt2f1500()


//--------------------------------------------------------------


//--- 8/99 for solving the int 0x2f bug ----
unsigned short __near _cdecl _saveregs AHFInt2f150b(unsigned short drivenr)
// called from gp2 instead of int 2f
// šbergabe: CX = drive number (0=A:)
// Rckgabe: BX = ADADh if MSCDEX.EXE installed
//           AX = support status
{
  movtobx(ValueInt2f150b[drivenr] >> 16);    // DO NOT CHANGE this function
  return (unsigned short)(ValueInt2f150b[drivenr]);

} // AHFInt2f150b()


//--------------------------------------------------------------


//--- 8/99 for solving the int 0x2f bug ----
unsigned short __near _cdecl _saveregs AHFInt2f4a33(void)
// called from gp2 instead of int 2f
// šbergabe: -
// Rckgabe: AX = 0000h for MS-DOS 7.00+
{
  return ValueInt2f4a33;  // DO NOT CHANGE this function (only AX should be changed)
                          // _saveregs somehow doesn't seems to work

} // AHFInt2f4a33()


//--------------------------------------------------------------


//--- 8/99 for indicating any failed inits ----
void __near _cdecl AHFInitDivers(unsigned long when)
// called before (when&1==0)/after (==1) gp2's divers inits
{
  switch (when >> 16) {

    case 1 :    // soundcard init
      if ((when & 1) == 0) {
        sprintf(strbuf, "- GP2: setting up soundcard...");
        LogLine(strbuf);
        printf(strbuf); fflush(stdout);
      } else {
        sprintf(strbuf, "done\n");
        LogLine(strbuf);
        printf(strbuf);
        //getch();
      }
      break;
    case 2 :    // loading system
      if ((when & 1) == 0) {
        sprintf(strbuf, "- GP2: loading system...");
        LogLine(strbuf);
        printf(strbuf); fflush(stdout);
      } else {
        sprintf(strbuf, "done\n");
        LogLine(strbuf);
        printf(strbuf);
      }
      break;
  }

} // AHFInitDivers()
