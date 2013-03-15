void mypicinsertabove(char *dest, char *src, unsigned long lines);
#pragma aux mypicinsertabove =  \
      "cld" \
           /*    "mov     esi,esi             /* ESI := ZIEL (gp2 buf) */ \
           /*    "mov     edi,ds:_picbufptr   /* EDI := QUELLE (baseptr mypicbuf) /* \
           /*    "mov     ecx,dword ptr ds:_CopySvgaLinesNum /* */ \
               "imul    ecx,160"       /* == (CopySvgaLinesNum*640) >> 2 */ \
\
               "sub     edi,esi"       /* EDI := Differenz ESI zu EDI */ \
               "mov     ebp,edi"       /* EBP := Differenz */ \
\
"NextSrch:      jecxz   JumpFertig" \
               "xor     eax,eax" \
               "add     edi,esi" \
               "mov     edx,edi" \
               "repz    scasd" \
               "jz      JumpFertig" /* just set on ecx == 0 */  \
\
               "sub     edi,4" \
               "sub     edi,edx" \
               "add     esi,edi" \
               "mov     edi,ebp" \
\
"looptop: "\
      "mov eax,[esi+edi]"      /* read sprite source bits (former [EDI]) */ \
      "mov ebx,7f7f7f7fh" \
      "and ebx,eax" \
      "add ebx,7f7f7f7fh" \
      "or  ebx,eax"            /* 80808080 bits have non-zero status of bytes */ \
      "and ebx,80808080h" \
      "shr ebx,7"              /* move to 01010101 bits  */ \
      "add ebx,7f7f7f7fh"      /* 80==on or 7f==off values in each byte  */ \
      "and ebx,7f7f7f7fh"      /* 00==on or 7f==off masks  */ \
      "lea eax,[ebx+ebx]"      /* eax has 00==on or fe==off masks. (P5: AGI stall) */ \
      "or  eax,ebx"            /* eax has 00==on or ff==off masks  */ \
      "mov ebx,eax" \
      "not ebx"                /* ebx has 00==off or ff==on masks  */ \
      "and eax,[esi]"          /* background (original) bits (gp2buf) */ \
      "or  eax,[esi+edi]"      /* merge with sprite bits for final result */ \
      "mov [esi],eax"          /* draw! */ \
      "add esi,4" \
      "cmp dword ptr [esi+edi],0" \
      "je  NextSrch" \
      "dec ecx" \
      "jnz looptop" \
\
"JumpFertig: " \
parm [ESI] [EDI] [ECX] modify exact [EAX EBX ECX EDX ESI EDI EBP];


//------------------------------------------------------------------


void mypicinsertbelow(char *dest, char *src, unsigned long lines);
#pragma aux mypicinsertbelow =  \
      "cld" \
           /*    "mov     esi,esi             /* ESI := ZIEL (gp2 buf) */ \
           /*    "mov     edi,ds:_picbufptr   /* EDI := QUELLE (baseptr mypicbuf) /* \
           /*    "mov     ecx,dword ptr ds:_CopySvgaLinesNum /* */ \
               "imul    ecx,160"       /* == (CopySvgaLinesNum*640) >> 2 */ \
\
               "sub     edi,esi"       /* EDI := Differenz ESI zu EDI */ \
               "mov     ebp,edi"       /* EBP := Differenz */ \
\
"NextSrch:      jecxz   JumpFertig" \
               "xor     eax,eax" \
               "add     edi,esi" \
               "mov     edx,edi" \
               "repz    scasd" \
               "jz      JumpFertig" /* just set on ecx == 0 */  \
\
               "sub     edi,4" \
               "sub     edi,edx" \
               "add     esi,edi" \
               "mov     edi,ebp" \
\
"looptop: "\
      "mov eax,[esi]"          /* read target bits ([ESI]) */ \
      "mov ebx,7f7f7f7fh" \
      "and ebx,eax" \
      "add ebx,7f7f7f7fh" \
      "or  ebx,eax"            /* 80808080 bits have non-zero status of bytes */ \
      "and ebx,80808080h" \
      "shr ebx,7"              /* move to 01010101 bits  */ \
      "add ebx,7f7f7f7fh"      /* 80==on or 7f==off values in each byte  */ \
      "and ebx,7f7f7f7fh"      /* 00==on or 7f==off masks  */ \
      "lea eax,[ebx+ebx]"      /* eax has 00==on or fe==off masks. (P5: AGI stall) */ \
      "or  eax,ebx"            /* eax has 00==on or ff==off masks  */ \
      "mov ebx,eax" \
      "not ebx"                /* ebx has 00==off or ff==on masks  */ \
      "and eax,[esi+edi]"      /* src bits drueber */ \
      "or  eax,[esi]"          /* merge with target bits for final result */ \
      "mov [esi],eax"          /* draw! */ \
      "add esi,4" \
      "cmp dword ptr [esi+edi],0" \
      "je  NextSrch" \
      "dec ecx" \
      "jnz looptop" \
\
"JumpFertig: " \
parm [ESI] [EDI] [ECX] modify exact [EAX EBX ECX EDX ESI EDI EBP];
