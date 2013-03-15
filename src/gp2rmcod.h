#ifndef _RMCODE_H
#define _RMCODE_H

#include "typedefs.h"

#define SIZE_GP2SENDBUF 120

#pragma pack (push, 1)

typedef struct gp2sendbuf {
  unsigned char data[SIZE_GP2SENDBUF];
} gp2sendbuf;

typedef struct gp2ctbs {    // IDA: t_CmdsToBeSend
  gp2sendbuf slot[8];
} gp2sendslots;

typedef struct rmcodedef {
    BYTE    keytab[16]; 	 // 0000
    BYTE    field_10;		 // 0010
    BYTE    field_11;		 // 0011	 ; was mit keys??
    BYTE    field_12;		 // 0012	 ; was mit keys??
    BYTE    field_13;		 // 0013	 ; was mit keys??
    BYTE    field_14;		 // 0014	 ; was mit keys??
    BYTE    bDOSCritErr;	 // 0015	 ; FF falls ja
    WORD    bDOSCritErrCode;	 // 0016
    DWORD   pOrgInt15;		 // 0018
    DWORD   pOrgInt1B;		 // 001C
    DWORD   pOrgInt23;		 // 0020
    DWORD   pOrgInt24;		 // 0024
    WORD    wComPortAddr;	 // 0028
    BYTE    bUseRCTS;		 // 002A	 ; should be: use RTS/CTS in modem link
				 //		 ; controlled via cmdl switch
    BYTE    SendBuf[120];	 // 002B
    BYTE    RcvBuf[256];	 // 00A3
    WORD    wRcvPtr;		 // 01A3	 ; (head)
    WORD    wRcvProcPtr;	 // 01A5	 ; der "Verarbeitungs"-Ptr (tail)
    WORD    wSendPtr;		 // 01A7
    BYTE    bBytesToSend;	 // 01A9
    BYTE    bLSRstate;		 // 01AA
    BYTE    bMSRstate;		 // 01AB
    BYTE    bSendKlar;		 // 01AC	 ; wird u.a. gesetzt, wenn kein int pending
				 //		 ; wird 0 falls:
				 //		 ; BytestoSend==0,
				 //		 ; kein CTS gesetzt
    BYTE    bRcvKlar;		 // 01AD	 ; wird z.B. auf 0 gesetzt, wenn auch RTS gel”scht wird wegen RcvBuf low
    BYTE    bool_1AE;		 // 01AE	 ; at 91F56, 91e53
    WORD    counter_1AF;	 // 01AF	 ; some counter
    BYTE    field_1B1[10];	 // 01B1
    BYTE    t_VesaInfo[256];	 // 01BB	 ; for vesainfo and vesamodeinfo
    BYTE    field_2BB[33];	 // 02BB
    BYTE    NewRmInt15[187];	 // 02DC
    BYTE    NewRmInt1B; 	 // 0397
    BYTE    NewRmInt23; 	 // 0398
    BYTE    NewRmInt24[13];	 // 0399
    BYTE    NewRmComIsr[84];	 // 03A6
    WORD    mdmJmpTab[4];	 // 03FA
    BYTE    aubMdmStatusChg[14]; // 0402
    BYTE    subMdmTxDEmpty[50];  // 0410
    BYTE    subMdmDataAvl[76];	 // 0442
    BYTE    subMdmLineChg[11];	 // 048E
    BYTE    subUnk[58]; 	 // 0499
    BYTE    rmcodeend[12];	 // 04D3	 ; 'rm code end',0
} rmcodedef;

#pragma pack (pop)

#endif
