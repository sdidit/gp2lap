unsigned int vesa_videostate( unsigned char subfunc,
                            unsigned short states,
                          unsigned short dossegm,
                        void *rmintbuf );
/* öbergabe: subfunc == (siehe oben)
             states == welche states sollen berÅcksichtigt werden (siehe oben)
             dossegm == DOS-BufferSegment (von rmint-funktion zu fÅllen (ES:BX))
             rmintbuf == dpmi call (simulate rmint)
   RÅckgabe: bei subfunc == 00h
                 EBX := benîtigte Grî·e in Bytes
                 EBX == 0, wenn Fehler
             bei subfunc == 01h und 02h
                 EBX == 0, wenn Fehler
                 EBX == 1, wenn successfull
*/

#pragma aux vesa_videostate = \
             "push eax"\
             "push ecx"\
             "push edx"\
             "push edi"\
             "push esi"\
             "push ebp"\
     /*----- setup rmint-buffer ----------*/ \
             "mov [edi+22h],ax"     /* rmint ES := dossegm */ \
             "mov word ptr [edi+10h],0000h"  /* rmint BX := Ofs fÅr dossegm */ \
             "mov [edi+28],4F04h"   /* rmint EAX := 4F04 */ \
             "mov [edi+24],ecx"     /* rmint ECX := states */ \
             "mov [edi+20],edx"     /* rmint EDX := subfunc */ \
     /*----- issue rmint via DPMI call ----------*/ \
             "mov eax, 300h"        /* simulate rmint */ \
             "mov ebx, 10h"         /* interrupt 10h */ \
             "xor ecx, ecx"         /* copy 0 words stack from pm to rm */ \
             "int 31h" \
     /*----- rmint-fehlerauswertung im rmi-buf ----------*/ \
             "xor ebx,ebx"     /* erstmal Fehleranzeige setzen */ \
             "cmp word ptr [edi+28], 004Fh" /* rmi.AX == 004Fh? */ \
             "jne vhv_exit"    /* EBX ist schon 0, zeigt fehler an */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             "cmp dl, 0"       /* welche subfunc war gesetzt? */ \
             "jne WithBuf"   \
             "mov bx, word ptr [edi+10h]"   /* rmi.BX = size of 64Byte-Blocks needed */ \
             "shl ebx, 6"      /* BX := BX * 64 */ \
             "jmp vhv_exit"  \
   "WithBuf:  mov ebx, 1"      /* Fehlerfrei signalisieren */ \
  "vhv_exit:  pop ebp" \
             "pop esi" \
             "pop edi" \
             "pop edx" \
             "pop ecx" \
             "pop eax" \
parm[DL] [CX] [AX] [EDI] value[EBX] modify [EAX EBX ECX EDX EDI];

//------------------------------------------------------------------------
//------------------------------------------------------------------------

unsigned int vesa_modeinfo ( unsigned short vmode,
                          void *linsegaddr,
                        void *rmintbuf );
/* öbergabe: vmode == videomodus
             linsegaddr == DOS-BufferSegment (von rmint-funktion zu fÅllen (ES:BX))
             rmintbuf == dpmi call (simulate rmint)
   RÅckgabe: 0 == failed
                 ES:DI (rm)buffer filled
             1 == successfull
*/

#pragma aux vesa_modeinfo = \
             "push eax"\
             "push ecx"\
             "push edx"\
             "push edi"\
             "push esi"\
             "push ebp"\
     /*----- setup rmint-buffer ----------*/ \
             "shr eax,4"        /* cnvrt linear segaddr to rm seg */ \
             "mov [edi+22h],ax"     /* rmint ES := dossegm */ \
             "mov word ptr [edi],0h"  /* rmint DI := Ofs fÅr dossegm */ \
             "mov [edi+28],4F01h"   /* rmint EAX := 4F01 */ \
             "mov [edi+24],cx"      /* rmint ECX := vmode */ \
     /*----- issue rmint via DPMI call ----------*/ \
             "mov eax, 300h"        /* simulate rmint */ \
             "mov ebx, 10h"         /* interrupt 10h */ \
             "xor ecx, ecx"         /* copy 0 words stack from pm to rm */ \
             "int 31h" \
     /*----- rmint-fehlerauswertung im rmi-buf ----------*/ \
             "xor ebx,ebx"     /* erstmal Fehleranzeige setzen */ \
             "cmp word ptr [edi+28], 004Fh" /* rmi.AX == 004Fh? */ \
             "jne vm_exit"     /* EBX ist schon 0, zeigt fehler an */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             "mov ebx, 1"      /* Fehlerfrei signalisieren */ \
  "vm_exit:   pop ebp" \
             "pop esi" \
             "pop edi" \
             "pop edx" \
             "pop ecx" \
             "pop eax" \
parm[CX] [EAX] [EDI] value[EBX] modify [EAX EBX ECX EDX EDI];

//------------------------------------------------------------------------
//------------------------------------------------------------------------

unsigned int vesa_getinfo ( void *linsegaddr,
                        void *rmintbuf );
/* öbergabe: dossegm == DOS-BufferSegment (von rmint-funktion zu fÅllen (ES:BX))
             rmintbuf == dpmi call (simulate rmint)
   RÅckgabe: 0 == failed
                 ES:DI (rm)buffer filled
             1 == successfull
*/

#pragma aux vesa_getinfo = \
             "push eax"\
             "push ecx"\
             "push edx"\
             "push edi"\
             "push esi"\
             "push ebp"\
     /*----- setup rmint-buffer ----------*/ \
             "shr eax,4"        /* cnvrt linear segaddr to rm seg */ \
             "mov [edi+22h],ax"     /* rmint ES := dossegm */ \
             "mov word ptr [edi],0h"  /* rmint DI := Ofs fÅr dossegm */ \
             "mov [edi+28],4F00h"   /* rmint EAX := 4F01 */ \
     /*----- issue rmint via DPMI call ----------*/ \
             "mov eax, 300h"        /* simulate rmint */ \
             "mov ebx, 10h"         /* interrupt 10h */ \
             "xor ecx, ecx"         /* copy 0 words stack from pm to rm */ \
             "int 31h" \
     /*----- rmint-fehlerauswertung im rmi-buf ----------*/ \
             "xor ebx,ebx"     /* erstmal Fehleranzeige setzen */ \
             "cmp word ptr [edi+28], 004Fh" /* rmi.AX == 004Fh? */ \
             "jne vm_exit"     /* EBX ist schon 0, zeigt fehler an */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             "mov ebx, 1"      /* Fehlerfrei signalisieren */ \
  "vm_exit:   pop ebp" \
             "pop esi" \
             "pop edi" \
             "pop edx" \
             "pop ecx" \
             "pop eax" \
parm[EAX] [EDI] value[EBX] modify [EAX EBX ECX EDX EDI];

//------------------------------------------------------------------------

void *vesa_getpmfunc5( void *rmintbuf );
/* öbergabe: dossegm == DOS-BufferSegment (von rmint-funktion zu fÅllen (ES:BX))
             rmintbuf == dpmi call (simulate rmint)
   RÅckgabe: 0 == failed
                 ES:DI (rm)buffer filled
             !0 == Adresse der PM-Func
*/

#pragma aux vesa_getpmfunc5 = \
             "push eax"\
             "push ecx"\
             "push edx"\
             "push edi"\
             "push esi"\
             "push ebp"\
     /*----- setup rmint-buffer ----------*/ \
            /* "mov [edi+22h],ax"     /* rmint ES := dossegm */ \
           /*  "mov word ptr [edi],0h"  /* rmint DI := Ofs fÅr dossegm */ \
             "mov [edi+28],4F0Ah"   /* rmint EAX := 4F01 */ \
     /*----- issue rmint via DPMI call ----------*/ \
             "mov eax, 300h"        /* simulate rmint */ \
             "mov ebx, 10h"         /* interrupt 10h */ \
             "xor ecx, ecx"         /* copy 0 words stack from pm to rm */ \
             "int 31h" \
     /*----- rmint-fehlerauswertung im rmi-buf ----------*/ \
             "xor ebx,ebx"     /* erstmal Fehleranzeige setzen */ \
             "cmp word ptr [edi+28], 004Fh" /* rmi.AX == 004Fh? */ \
             "jne vg5_exit"     /* EBX ist schon 0, zeigt fehler an */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             "movzx ebx,word ptr [edi+22h]"  /*es */ \
             "shl ebx, 4" \
             "movzx eax,word ptr [edi+00h]" /*di */ \
             "add ebx,eax" \
             "movzx eax,[ebx]"  /* function5 Ofset dazu */ \
             "add ebx,eax" \
          /*   "mov ebx, 1"      /* Fehlerfrei signalisieren */ \
  "vg5_exit:  pop ebp" \
             "pop esi" \
             "pop edi" \
             "pop edx" \
             "pop ecx" \
             "pop eax" \
parm[EDI] value[EBX] modify [EAX EBX ECX EDX EDI];

//------------------------------------------------------------------------

//------------------------------------------------------------------------

unsigned int vesa_setpage ( unsigned int vpage,
                          unsigned short vesa_granularity );
/*
   RÅckgabe: 0 == successful
             1 == failed
*/
#pragma aux vesa_setpage = \
             "shl eax, 6"     /* vpage := vpage * 64 */ \
                              /* ACHTUNG: es koennte(?) sein da· der Faktor */ \
                              /*  nicht immer 64 ist . Auf jeden Fall */ \
                              /*  ist im Mode 101h vesamodeinfo->WindowSize == 64 */ \
             "xor edx, edx" \
             "div bx"         /* vpage / vesa_granularity */ \
             "mov dx, ax"     /* DX := window address in video memory (in granularity units)*/ \
             "push dx" \
     /*----- select video memory window (for window A) -------*/ \
             "mov eax, 4F05h"\
             "xor ebx, ebx"\
             "xor ecx, ecx"\
             "int 10h" \
             "pop dx" \
             "mov ebx,0"      /* erstmal Fehleranzeige setzen */ \
             "cmp ax,004Fh" \
             "jne vs_exit" \
     /*----- select video memory window (for window B) -------*/ \
             "mov eax, 4F05h"\
             "mov ebx, 1"\
             "xor ecx, ecx"\
             "int 10h" \
             "mov ebx,0"      /* erstmal Fehleranzeige setzen */ \
             "cmp ax,004Fh" \
             "jne vs_exit" \
             "mov ebx, 1"     /* fehlerfrei signalisieren */ \
  "vs_exit:  " \
parm[EAX] [BX] value[EBX] modify [EAX EBX ECX EDX];

//--------- auf den vertical retrace warten (-> no flicker) ----------

extern void waitforvrt( void );

#pragma aux waitforvrt = \
             /*"push eax"*/ \
             /*"push edx" */ \
             "mov edx,03DAh" \
    "wfv_n1:  in  al,dx" \
             "and al,8" \
             "jnz wfv_n1"               /* wait until vertical retrace starts */ \
    "wfv_n2:  in  al,dx" \
             "and al,8" \
             "jz wfv_n2"               /* wait until vertical retrace ends */ \
             /*"pop edx"*/ \
             /*"pop eax"*/ \
parm[] modify [EAX EDX];

//------------------------------------------------------------------------

unsigned char *physaddrmapping( void *physaddr,   // should be above 1MB
                                  unsigned long sizeinbytes,
                                    void *errcode );
/* öbergabe: physaddr == physical address to map
             sizeinbytes == ....
             errcode == ptr to word for errorcode
   RÅckgabe: NULL == failed
             otherwise returning linear addr wich maps the requested phys mem
*/
#pragma aux physaddrmapping = \
             "mov cx,bx"\
             "shr ebx,16"       /* BX:CX := physical address */ \
             "mov di,si"\
             "shr esi,16"       /* SI:DI := size in bytes */ \
     /*----- issue DPMI call ----------*/ \
             "mov eax,800h"     /* dpmi function */ \
             "int 31h" \
     /*----- fehlerauswertung ----------*/ \
             "mov word ptr [edx],ax" \
             "mov eax,0"        /* erstmal Fehleranzeige setzen */ \
             "jc pam_exit"      /* EAX ist schon 0, zeigt fehler an */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             "mov word ptr [edx],0" \
             "shl ebx,16" \
             "mov bx,cx"        /* EBX := BX:CX (i.e. lin addr which maps the req. memory) */ \
             "mov eax,ebx" \
  "pam_exit:" \
parm[EBX] [ESI] [EDX] value[EAX] modify [EAX EBX ECX EDX ESI EDI];

//------------------------------------------------------------------------

unsigned long freephysaddrmapping( void *linmapaddr,
                                     void *errcode );
/* öbergabe: linmapaddr == linear address returned by dpmi function 800h
             errcode == ptr to word for errorcode
   RÅckgabe: 0 == failed
             1 == successfull
*/
#pragma aux freephysaddrmapping = \
             "mov cx,bx"\
             "shr ebx,16"       /* BX:CX := lin address */ \
     /*----- issue DPMI call ----------*/ \
             "mov eax,801h"     /* dpmi function */ \
             "int 31h" \
     /*----- fehlerauswertung ----------*/ \
             "mov word ptr [edx],ax" \
             "mov ebx,0"        /* erstmal Fehleranzeige setzen */ \
             "jc fpam_exit"     /* EAX ist schon 0, zeigt fehler an */ \
     /*----- RÅckgabewerte aufbereiten ------------*/ \
             "mov word ptr [edx],0" \
             "mov ebx, 1"     /* fehlerfrei signalisieren */ \
 "fpam_exit:" \
parm[EBX] [EDX] value[EBX] modify [EAX EBX ECX EDX ESI EDI];

//------------------------------------------------------------------------

unsigned short video_set_mode( unsigned short vmode );
/* öbergabe: vmode == ....
   RÅckgabe: 0 == successfull
             1 == failed
             2 == not supported
*/
#pragma aux video_set_mode = \
             "cmp bx,100"  \
             "jae VesaNeed"  \
\
             "xor eax,eax"  \
             "mov al,bl" \
             "int 10h" \
\
             "xor ebx,ebx" \
             "jmp vsm_exit" \
  "VesaNeed:  mov eax,4F02h"  \
             "int 10h"        /*AL := 4F, wenn mode supported*/ \
                              /*AH := 00, wenn succesfull*/ \
                              /*      01,      failed*/ \
             "mov ebx,0" \
             "cmp ax,004Fh" \
             "je  vsm_exit"     /* success */  \
\
             "mov bl,1" \
             "cmp al,4Fh" \
             "je  vsm_exit"     /* just failed */ \
\
             "mov bl,2" \
  "vsm_exit: " \
parm[BX] value[BX] modify [EAX EBX];

//------------------------------------------------------------------------
