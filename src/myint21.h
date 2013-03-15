#ifndef _MYINT21_H
#define _MYINT21_H


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

#pragma pack (push, 1)

struct rmcode {                // structure of rm21code.inc

  unsigned char code1[(7*16) + 14];
  unsigned char oldintofs[4];
  //unsigned short oldintofs;
  //unsigned short oldintseg;
  //unsigned char watcom_newstr[16];
  //------- die zu patchenden 16 Bytes --------------
  unsigned char moveax;
  unsigned char LammCallAdr[4];
  unsigned char calleaxnop[3];
  //------- die originalen 16 WatcomBytes --------------
  unsigned char watcom_searchstr[8];
  unsigned char code2[11];

};

#pragma pack (pop)


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------


extern void __interrupt RcvGP2Call(void);
void remove_int21_hook(void);
void *install_int21_hook(void);



#endif
