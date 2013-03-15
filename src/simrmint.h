struct rminfo {                // Format of DPMI real mode call structure:
                               // Offset  Size    Description     (Table 2819)
    long EDI;                  //  00h    DWORD   EDI
    long ESI;                  //  04h    DWORD   ESI
    long EBP;                  //  08h    DWORD   EBP
    long reserved_by_system;   //  0Ch    DWORD   reserved (00h)
    long EBX;                  //  10h    DWORD   EBX
    long EDX;                  //  14h    DWORD   EDX
    long ECX;                  //  18h    DWORD   ECX
    long EAX;                  //  1Ch    DWORD   EAX
    short flags;               //  20h    WORD    flags
    short ES;                  //  22h    WORD    ES
    short DS;                  //  24h    WORD    DS
    short FS;                  //  26h    WORD    FS
    short GS;                  //  28h    WORD    GS
    short IP;                  //  2Ah    WORD    IP
    short CS;                  //  2Ch    WORD    CS
    short SP;                  //  2Eh    WORD    SP
    short SS;                  //  30h    WORD    SS
};


unsigned long get_rmint( unsigned char intnr );
/* parm: intnr
   value: (rm_seg << 16) + rm_ofs (0 if error)
*/
#pragma aux get_rmint = \
             "mov eax,200h" \
             "int 31h" \
             "mov eax,0" \
             "jc gr_err" \
             "shl ecx,16" \
             "mov cx,dx" \
             "mov eax,ecx" \
   "gr_err:   "\
parm[BL] value[EAX] modify [EAX EBX ECX EDX];

unsigned long set_rmint( unsigned char intnr, unsigned long rmvector );
/* parm: (rm_seg << 16) + rm_ofs
   value: 1 == ok, (0 if error)
*/
#pragma aux set_rmint = \
             "mov dx,cx" \
             "shr ecx,16" \
             "mov eax,201h" \
             "int 31h" \
             "mov eax,0" \
             "jc sr_err" \
             "mov eax,1" \
   "sr_err:   "\
parm[BL] [ECX] value[EAX] modify [EAX EBX ECX EDX];



struct rmcode {                // Format of DPMI real mode call structure:
                               // Offset  Size    Description     (Table 2819)
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


void *get_XBRK_structure(void);
/* parm: size == # bytes to alloc in low memory
   value: selector (0 if error)
*/
#pragma aux get_XBRK_structure = \
             "mov eax,0BF02h" \
             "xor esi,esi" \
             "xor edx,edx" \
             "int 15h" \
             "shl edx,4" \
             "add edx,esi" \
value[EDX] modify [EAX EDX ESI];

struct xbrk {
  unsigned long firstbyteadr;        //    00h    DWORD   linear address of first available byte
  unsigned long lastbyteadr;         //    04h    DWORD   linear address of last available byte + 1 ???
  unsigned long rmadrofxbrkstruct;   //    08h    DWORD   real-mode address of XBRK structure???
  unsigned long l_1;                 //    0Ch    DWORD   ???
  unsigned char c_1[2];              //    10h  2 BYTEs   ???
  unsigned short segmentof_;         //    12h    WORD    segment of ???
  unsigned char c_2[8];              //    14h  8 BYTEs   ???
  unsigned char pm_idt[512];         //    1Ch 512 BYTEs  protected-mode IDT
  unsigned char pm_gdt[512];         //   21Ch  N BYTEs   protected-mode GDT
};
