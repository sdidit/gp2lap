#define LOBYTE(w)           ((unsigned char)(w))
#define HIBYTE(w)           ((unsigned char)((unsigned short)(w) >> 8))

#define LOWORD(l)           ((unsigned short)(l))
#define HIWORD(l)           ((unsigned short)((unsigned int)(l) >> 16))


extern void my_cli( void );
#pragma aux my_cli = "cli";

extern void my_sti( void );
#pragma aux my_sti = "sti";

extern void push_all_and_set_ES( void );
#pragma aux push_all_and_set_ES = \
        "pushad" \
/*        "push edi" */ \
/*        "push esi" */ \
        "push ds" \
        "push es" \
        "push ds" \
        "pop es" \
        modify[ESP];

extern void pop_all( void );
#pragma aux pop_all = \
        "pop es" \
        "pop ds" \
/*        "pop esi" */ \
/*        "pop edi" */ \
        "popad" \
        modify[ESP];


unsigned long ahf_dpmi_lock_region( unsigned long linearadr,
                                   unsigned long size );
#pragma aux ahf_dpmi_lock_region = \
             "mov cx, bx" \
             "shr ebx, 16"       /* BX:CX = starting linear address */ \
             "mov di, si" \
             "shr esi, 16 "      /* SI:DI = size of region in bytes */ \
             "mov eax, 600h" \
             "int 31h" \
             "mov eax, 0" \
             "jc dlr_err" \
             "mov eax, 1" \
   "dlr_err: " \
parm[EBX] [ESI] value[EAX] modify [EAX EBX ECX EDX ESI EDI];


unsigned long ahf_dpmi_unlock_region( unsigned long linearadr,
                                     unsigned long size );
#pragma aux ahf_dpmi_unlock_region = \
             "mov cx, bx" \
             "shr ebx, 16"       /* BX:CX = starting linear address */ \
             "mov di, si" \
             "shr esi, 16 "      /* SI:DI = size of region in bytes */ \
             "mov eax, 601h" \
             "int 31h" \
             "mov eax, 0" \
             "jc dur_err" \
             "mov eax, 1" \
   "dur_err: " \
parm[EBX] [ESI] value[EAX] modify [EAX EBX ECX EDX ESI EDI];


unsigned long ahf_dpmi_get_rmint(unsigned char intnr);
// parm: intnr
// value: (rm_seg << 16) + rm_ofs (0 if error)
#pragma aux ahf_dpmi_get_rmint = \
             "mov eax,200h" \
             "int 31h" \
             "mov eax,0" \
             "jc gr_err" \
             "shl ecx,16" \
             "mov cx,dx" \
             "mov eax,ecx" \
   "gr_err:   "\
parm[BL] value[EAX] modify [EAX EBX ECX EDX];


unsigned long ahf_dpmi_set_rmint(unsigned char intnr, unsigned long rmvector);
// parm: (rm_seg << 16) + rm_ofs
// value: 1 == ok, (0 if error)
#pragma aux ahf_dpmi_set_rmint = \
             "mov dx,cx" \
             "shr ecx,16" \
             "mov eax,201h" \
             "int 31h" \
             "mov eax,0" \
             "jc sr_err" \
             "mov eax,1" \
   "sr_err:   "\
parm[BL] [ECX] value[EAX] modify [EAX EBX ECX EDX];



//------ Stack-Verwaltungsfunktionen -------------------------

void __far *getstack( void );
#pragma aux getstack = \
    "mov dx, ss" \
    "mov eax, esp" \
    parm [] value [dx eax] modify exact [];

void setstack( void __far *newstack );
#pragma aux setstack = \
    "mov ss,dx" \
    "mov esp, eax" \
    parm [dx eax] modify exact [];


void mymemcpy(char *dest, char *src, unsigned long len);
#pragma aux mymemcpy =  \
"cld" /* 12/99 probably better */ \
"               mov     eax, ecx        " \
"               sub     ecx, edi        " \
"               sub     ecx, eax        " \
"               and     ecx, 3          " \
"               sub     eax, ecx        " \
"               jle     short LEndBytes " \
"           rep movsb                   " \
"               mov     ecx, eax        " \
"               and     eax, 3          " \
"               shr     ecx, 2          " \
"           rep movsd                   " \
"LEndBytes:     add     ecx, eax        " \
"           rep movsb                   " \
parm [EDI] [ESI] [ECX] modify [EAX ECX ESI EDI];

void mymemset(char *dest, char chr, unsigned long len);
#pragma aux mymemset =  \
"cld" /* 12/99 probably better */ \
               "ror ebx,8" \
               "shld eax,ebx,8" \
               "shld eax,ebx,8" \
               "shld eax,ebx,8" \
               "shld eax,ebx,8" \
"               mov     ebx, ecx        " \
"               sub     ecx, edi        " \
"               sub     ecx, ebx        " \
"               and     ecx, 3          " \
"               sub     ebx, ecx        " \
"               jle     short LEndBytes " \
"           rep stosb                   " \
"               mov     ecx, ebx        " \
"               and     ebx, 3          " \
"               shr     ecx, 2          " \
"           rep stosd                   " \
"LEndBytes:     add     ecx, ebx        " \
"           rep stosb                   " \
parm [EDI] [BL] [ECX] modify [EAX EBX ECX EDI];


//---------------------------------------------------------------------------------------------


//--- MS Windows - WINDOWS ENHANCED MODE INSTALLATION CHECK ------
unsigned short myint2f1600( void );
#pragma aux myint2f1600 = \
             "mov eax,1600h" \
             "int 2fh" \
value[AX] modify [EAX EBX ECX EDX EDX ESI EDI];

//--- CD-ROM - INSTALLATION CHECK ---------
unsigned long myint2f1500( void );
#pragma aux myint2f1500 = \
             "xor ebx,ebx" \
             "mov eax,1500h" \
             "int 2fh" \
             "shl ebx,16" \
             "mov bx,cx" \
value[EBX /*<BX|CX>*/ ] modify [EAX EBX ECX EDX EDX ESI EDI];

//--- CD-ROM v2.00+ - DRIVE CHECK ------
unsigned long myint2f150b( unsigned short drivenr /* 0==A: ...26 */ );
#pragma aux myint2f150b = \
             "xor ebx,ebx" \
             "mov eax,150bh" \
             "int 2fh" \
             "shl ebx,16" \
             "mov bx,ax" \
parm[CX] value[EBX /*<BX|AX>*/ ] modify [EAX EBX ECX EDX EDX ESI EDI];


//--- Windows95 - CHECK MS-DOS VERSION 7 ------
unsigned short myint2f4a33( void );
#pragma aux myint2f4a33 = \
             "push ds" \
             "push es" \
             "mov eax,4a33h" \
             "int 2fh" \
             "pop es" \
             "pop ds" \
value[AX] modify [EAX EBX ECX EDX ESI EDI];


//--- short stuff ------
void movtobx( unsigned short regval );
#pragma aux movtobx = "mov bx,dx" parm[DX] modify exact [DX]; // Umweg leider noetig

void _myint3(void);
#pragma aux _myint3 = "int 3";

//---------------------------------------------------------------------------------------------

unsigned long CanEnterCriticalSection(unsigned long *);
#pragma aux CanEnterCriticalSection = \
             "mov     eax,0" \
             "mov     edx,1" \
             "lock cmpxchg dword ptr [ebx], edx" \
             "mov     eax,0" \
             "jne     CantEnter" \
             "mov     eax,1" \
"CantEnter:   "\
parm[EBX] value[EAX] modify [EAX EBX ECX EDX];


void LeaveCriticalSection(unsigned long *);
#pragma aux LeaveCriticalSection = \
             "mov dword ptr [ebx],0" \
parm[EBX] modify [EBX];
