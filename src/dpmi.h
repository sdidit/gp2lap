#ifndef _DPMI_H
#define _DPMI_H


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

typedef signed char byte;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef ubyte bool;

#pragma pack (push, 1)

typedef struct dpmi_real_regs {
    uint edi;
    uint esi;
    uint ebp;
    uint reserved_by_system;
    uint ebx;
    uint edx;
    uint ecx;
    uint eax;
    ushort flags;
    ushort es,ds,fs,gs,ip,cs,sp,ss;
} dpmi_real_regs;

typedef struct mem_data {   // for DPMI Get Memory Info (function 0x500)
  int     largest_block_bytes;
  int     max_unlocked_page_allocation;
  int     largest_lockable_pages;
  int     total_pages;
  int     unlocked_pages;
  int     unused_physical_pages;
  int     total_physical_pages;
  int     free_linear_pages;
  int     paging_size_pages;
  int     reserved[3];       // i.e. 12 bytes
} mem_data;

#pragma pack (pop)


//--------------------------------------------------------
//------ global variables --------------------------------
//--------------------------------------------------------


//---- vom Compiler bereitgestellte Infos ------------
extern unsigned char __begtext;  // Start of 32 Bit CODE segment
extern unsigned char end;        // End of BSS segment


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

ulong GetLogDpmiInfo();
void SetLogDpmiInfo(ulong);

unsigned long dpmi_find_dos_memory(void);

void *dpmi_real_malloc(uint, ushort *);
unsigned long dpmi_real_free(ushort);

void dpmi_real_int386x(ubyte, dpmi_real_regs *);
ulong dpmi_real_call(dpmi_real_regs *);

ulong dpmi_unlock_region(void *, unsigned);
ulong dpmi_lock_region(void *, unsigned);

ulong dpmi_modify_selector_base(ushort, void *);
ulong dpmi_modify_selector_limit(ushort, int);
ulong dpmi_allocate_selector(void *, int, ushort *);

void dpmi_close();
ulong dpmi_init(int);

void *dpmi_get_temp_low_buffer(int);
int dpmi_set_pm_handler(unsigned, void far *);


//--------------------------------------------------------
//------ some macros -------------------------------------
//--------------------------------------------------------

#define DPMI_real_segment(P)    ((((uint) (P)) >> 4) & 0xFFFF)
#define DPMI_real_offset(P)     (((uint) (P)) & 0xF)



#include "inl_asm.inc"

#endif

