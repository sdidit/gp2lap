#include <i86.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <conio.h>
#include "basiclog.h"
#include "dpmi.h"

unsigned long LogDpmiInfo = 0;   // by default no logging

//---- verwendet von dpmi_init() ----------------
extern void cdecl _GETDS();
extern void cdecl cstart_();

unsigned int dpmi_virtual_memory = 0;   // set at init - indicating if VMM supported
unsigned int dpmi_available_memory = 0;
unsigned int dpmi_physical_memory = 0;
unsigned int dpmi_dos_memory = 0;       // set at init - how much dos mem avail


//---- variables for dpmi_real_call() ----------------
int dos_stack_initialized = 0;
ubyte * dos_stack = NULL;
ubyte * dos_stack_top = NULL;
#define DOS_STACK_SIZE (4*1024)  // A big ol' 4K stack!!!

//---- variables for dpmi_(un)lock_region() ----------
int total_bytes = 0;  // global (locked bytes)

//---- for tmp low mem buffer to be allocated --------
#define RUNTIMEDOSMEMSIZE 1024
static void *dpmi_dos_buffer = NULL;    // global for tmp dos rm buffer
static ushort dpmi_dos_selector = 0;    // global for tmp dos rm buffer

//------------------------------------------

ulong GetLogDpmiInfo() { return LogDpmiInfo; }
void SetLogDpmiInfo(ulong val) { LogDpmiInfo = val; }

//------------------------------------------

unsigned long dpmi_find_dos_memory(void)
// öbergabe: -
// RÅckgabe: frei dos speicher in bytes
{
  union REGS r;

  memset(&r, 0, sizeof(r));
  r.x.eax = 0x0100;       // DPMI allocate DOS memory
  r.x.ebx = 0xffff;       // Number of paragraphs requested
  int386 (0x31, &r, &r);
  //if ( (r.x.eax & 0xffff) == 0x08 )
  //if ( (r.x.eax & 0xffff) == 0x08 )
  // Note: call should never succeed, so don't worry...
  if ( r.x.cflag )
    return ((r.x.ebx & 0xffff)*16);  // EBX == size of largest avail block in para's
  else
    return 640*1024;

} // dpmi_find_dos_memory()

//------------------------------------------

void *dpmi_real_malloc( uint size, ushort *selector )
// öbergabe: size == zu allokierende Grî·e (Bytes)
//           selector == ptr auf ushort
// RÅckgabe: linaddr auf allok. Speicher und selector
{
  union REGS r;

  if (LogDpmiInfo) {
    sprintf(strbuf, "  [DPMI] ALLOC LOW %u bytes: ", size);
    LogLine(strbuf);
  }

  memset(&r, 0, sizeof(r));
  r.x.eax = 0x0100;               // DPMI allocate DOS memory
  r.x.ebx = (size + 15) >> 4;     // Number of paragraphs requested
  int386 (0x31, &r, &r);

  if (r.x.cflag) {  // Failed
    if (LogDpmiInfo) {
      sprintf(strbuf, "Failed (0x%X)\n", r.w.ax);
      LogLine(strbuf);
    }
    return ((uint) 0);
  }

  if (selector != NULL) {
    *selector = r.x.edx & 0xFFFF;  // set selector
  }

  if (LogDpmiInfo) {
    sprintf(strbuf, "Ok (%04X:%P)\n", *selector, (r.x.eax & 0xFFFF) << 4);
    LogLine(strbuf);
  }

  return (void *) ((r.x.eax & 0xFFFF) << 4);  // return linaddr

} // dpmi_real_malloc()

//------------------------------------------

unsigned long dpmi_real_free( ushort selector )
// öbergabe: selector des zu befreienden DOS speichers
// RÅckgabe: 1 == ok, sonst 0
{
  union REGS r;

  if (LogDpmiInfo) {
    sprintf(strbuf, "  [DPMI] FREE LOW selector: %04X ", selector);
    LogLine(strbuf);
  }

  memset(&r, 0, sizeof(r));
  r.x.eax = 0x0101;            // DPMI free DOS memory
//  r.x.ebx = selector;          // haha, he made a mistake
  r.x.edx = selector;          // Selector to free
  int386 (0x31, &r, &r);

  if (r.x.cflag) {  // Failed
    sprintf(strbuf, "Failed (0x%X)\n", r.w.ax);
  }
  else {
    sprintf(strbuf, "Ok\n");
  }

  if (LogDpmiInfo) LogLine(strbuf);

  return (r.x.cflag) ? 0 : 1;

} // dpmi_real_free()

//------------------------------------------

void dpmi_real_int386x( ubyte intno, dpmi_real_regs *rregs )
// öbergabe: intno == Interrupt
//           rregs == zu verwendende realmode call structure
// RÅckgabe: -
{
   union REGS regs;
   struct SREGS sregs;
   memset(&regs, 0, sizeof(regs));
   memset(&sregs, 0, sizeof(sregs));

/* Use DMPI call 300h to issue the DOS interrupt */

   regs.w.ax = 0x0300;
   regs.h.bl = intno;
   regs.h.bh = 0;
   regs.w.cx = 0;
   sregs.es = FP_SEG(rregs);
   regs.x.edi = FP_OFF(rregs);
   int386x( 0x31, &regs, &regs, &sregs );

} // dpmi_real_int386x()

//------------------------------------------

ulong dpmi_real_call(dpmi_real_regs * rregs)
// calls rm procedure (with retf frame) at rregs->es:rregs->di
// öbergabe: intno == Interrupt
//           rregs == zu verwendende realmode call structure
// RÅckgabe: 0 == failed, 1 == success
{
  ushort temp_selector;  // for DOS stack
  union REGS regs;
  struct SREGS sregs;

  if ( !dos_stack_initialized ) {
    //--- ok, allocate some dos stack ----
    dos_stack_initialized = 1;
    dos_stack = dpmi_real_malloc( DOS_STACK_SIZE, &temp_selector );
    if ( dos_stack == NULL ) {
      printf( "dpmi: Error allocating real mode stack!\n" );
      dos_stack_top = NULL;
    } else {
      dos_stack_top = &dos_stack[DOS_STACK_SIZE];
    }
  }

  // Give this puppy a stack!!!, i.e. set stack within realmode call struct
  if ( dos_stack_top )    {
    rregs->ss = DPMI_real_segment(dos_stack_top);
    rregs->sp = DPMI_real_offset(dos_stack_top);
  }

    /* Use DMPI call 301h to call real mode procedure */
        memset(&regs,0,sizeof(regs));
        memset(&sregs,0,sizeof(sregs));
   regs.w.ax = 0x0301;
   regs.h.bh = 0;
   regs.w.cx = 0;
   sregs.es = FP_SEG(rregs);
   regs.x.edi = FP_OFF(rregs);
   int386x( 0x31, &regs, &regs, &sregs );
   if ( regs.x.cflag ) {
     //exit(regs.w.ax);
     printf( "dpmi: Error calling rm proc (0x%X)\n", regs.w.ax );
     return 0;
   }
   return 1;

} // dpmi_real_call()

//------------------------------------------

ulong dpmi_unlock_region(void *address, unsigned length)
// you should lock first! (i.e. total_bytes)
// öbergabe: address == lin addr to unlock
//           length == ...
// RÅckgabe: 0 == failed, otherwise success
{
  union REGS regs;
  unsigned long linear;

  linear = (unsigned long) address;

  total_bytes -= length;
  //mprintf( 1, "DPMI unlocked %d bytes\n", total_bytes );

  if (LogDpmiInfo) {
    sprintf(strbuf, "  [DPMI] UNLOCK REGION  %P, %u bytes: ", linear, length);
    LogLine(strbuf);
  }

  memset(&regs, 0, sizeof(regs));
  regs.w.ax = 0x601;                      // DPMI Unlock Linear Region
  regs.w.bx = (linear >> 16);             // Linear address in BX:CX
  regs.w.cx = (linear & 0xFFFF);

  regs.w.si = (length >> 16);             // Length in SI:DI
  regs.w.di = (length & 0xFFFF);
  int386 (0x31, &regs, &regs);

  if (regs.w.cflag)   // Failed
    sprintf(strbuf, "Failed (0x%X)\n", regs.w.ax);
  else
    sprintf(strbuf, "Ok  (%u bytes still locked)\n", total_bytes);
  if (LogDpmiInfo) LogLine(strbuf);

  return (!regs.w.cflag);                 // Return 0 if can't unlock

} // dpmi_unlock_region()

//------------------------------------------

ulong dpmi_lock_region(void *address, unsigned length)
// öbergabe: address == lin addr to lock
//           length == ...
// RÅckgabe: 0 == failed, otherwise success
{
  union REGS regs;
  unsigned long linear;

  linear = (unsigned long) address;

  total_bytes += length;
  //mprintf( 1, "DPMI Locked down %d bytes\n", total_bytes );

  if (LogDpmiInfo) {
    sprintf(strbuf, "  [DPMI] LOCK REGION  %P, %u bytes: ", linear, length);
    LogLine(strbuf);
  }

  memset(&regs, 0, sizeof(regs));
  regs.w.ax = 0x600;                      // DPMI Lock Linear Region
  regs.w.bx = (linear >> 16);             // Linear address in BX:CX
  regs.w.cx = (linear & 0xFFFF);

  regs.w.si = (length >> 16);             // Length in SI:DI
  regs.w.di = (length & 0xFFFF);
  int386 (0x31, &regs, &regs);

  if (regs.w.cflag)   // Failed
    sprintf(strbuf, "Failed (0x%X)\n", regs.w.ax);
  else
    sprintf(strbuf, "Ok\n");
  if (LogDpmiInfo) LogLine(strbuf);

  return (! regs.w.cflag);                // Return 0 if can't lock

} // dpmi_lock_region()

//------------------------------------------

ulong dpmi_modify_selector_base( ushort selector, void * address )
// set segment base address
{
  union REGS regs;
  unsigned long linear;

  linear = (unsigned long)address;

  memset(&regs, 0, sizeof(regs));
  regs.w.ax = 0x0007;                     // DPMI Change Selector Base Addres
  regs.w.bx = selector;                   // Selector to change
  regs.w.cx = (linear >> 16);             // Base address
  regs.w.dx = (linear & 0xFFFF);
  int386 (0x31, &regs, &regs);            // call dpmi
  if (regs.w.cflag)
    return 0;                             // Return 0 if error

  return 1;

} // dpmi_modify_selector_base()

//------------------------------------------

ulong dpmi_modify_selector_limit( ushort selector, int size  )
// set segment limit
{
  union REGS regs;
  unsigned long segment_limit;

  segment_limit = (unsigned long) size;

  memset(&regs,0,sizeof(regs));
  regs.w.ax = 0x0008;                     // DPMI Change Selector Limit
  regs.w.bx = selector;                     // Selector to change
  regs.w.cx = (segment_limit >> 16);        // Size of selector
  regs.w.dx = (segment_limit & 0xFFFF);
  int386 (0x31, &regs, &regs);            // call dpmi
  if (regs.w.cflag)
    return 0;                             // Return 0 if error

  return 1;

} // dpmi_modify_selector_limit()

//------------------------------------------

ulong dpmi_allocate_selector( void *address, int size, ushort *selector )
// allocate ldt descriptor (mod base and limit too)
{
  union REGS regs;

  memset(&regs, 0, sizeof(regs));
  regs.w.ax = 0;                        // DPMI Allocate Selector
  regs.w.cx = 1;                          // Allocate 1 selector
  int386 (0x31, &regs, &regs);            // call dpmi
  if (regs.w.cflag)
    return 0;                           // Return 0 if error
  *selector = regs.w.ax;

  if ( !dpmi_modify_selector_base( *selector, address ) )
    return 0;

  if ( !dpmi_modify_selector_limit( *selector, size ) )
    return 0;

   //mprintf( 0, "Selector 0x%4x has base of 0x%8x, size %d bytes\n", *selector, linear,segment_limit);

  return 1;

} // dpmi_allocate_selector()

//------------------------------------------

void dpmi_close()
// shutdown a dpmi_init()'ed system
{
  if (dpmi_dos_selector != 0) {
    if (LogDpmiInfo) {
      sprintf(strbuf, "- Now freeing runtime dos mem (%u bytes)\n", RUNTIMEDOSMEMSIZE);
      LogLine(strbuf);
    }
    // hmm, why not free dos mem, done by host?, well I'll add it anyway
    /*----*/  dpmi_real_free(dpmi_dos_selector); /*-----*/
    dpmi_dos_buffer = NULL;
    dpmi_dos_selector = 0;
  }

  // AFAIK done by host
  //if (LogDpmiInfo) LogLine("- Now unlocking whole image\n");
  //dpmi_unlock_region( &__begtext, &end - &__begtext));

} // dpmi_close()

//------------------------------------------

ulong dpmi_init(int verbose)
// öbergabe: verbose == flag ob screen ausgabe
// RÅckgabe: 0 == success
//           1 == error allocating runtime dos mem
//           2 == Error locking _GETDS_
//           3 == Error locking _GETCS_
//           4 == Error locking _chain_intr
//           5 == Error locking whole LE image...
{
    union REGS regs;
    struct SREGS sregs;
    mem_data mi;   // for DPMI Get Memory Info

    if (LogDpmiInfo)
      LogLine("- Dpmi Init:\n");

    //--- get amount of dos memory ---
    dpmi_dos_memory = dpmi_find_dos_memory();
    if (LogDpmiInfo) {
      sprintf(strbuf, "  %u bytes dos mem available\n", dpmi_dos_memory);
      LogLine(strbuf);
    }

    //--- lock whole LE image ---
    // always fails under win95 due to overlapping sel's -> change that
    //if (LogDpmiInfo)
    //  LogLine("  Now locking whole image\n");
    //if (!dpmi_lock_region(&__begtext, &end - &__begtext)) {
    //  return 5;
    //}

    //--- allocate runtime available low dos mem (1K) ---
    if (LogDpmiInfo) {
      sprintf(strbuf, "  Now allocating runtime dos mem (%u bytes)\n", RUNTIMEDOSMEMSIZE);
      LogLine(strbuf);
    }
    dpmi_dos_buffer = dpmi_real_malloc(RUNTIMEDOSMEMSIZE, &dpmi_dos_selector);
    if (!dpmi_dos_buffer) {
      dpmi_dos_selector = 0;
      //if (verbose) printf( "dpmi: Error allocating runtime dos mem\n");
      //exit(1);
      return 1;
    }

    //--- to be called at shutdown ------
    atexit(dpmi_close);

    //--- check dpmi -----
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = 0x400;         // DPMI Get version info
    int386( 0x31, &regs, &regs );

    if (!regs.w.cflag) {
      sprintf(strbuf, "  DPMI V%d.%d, CPU:%d, VMM:", regs.h.ah, regs.h.al, regs.h.cl);
      if (LogDpmiInfo) LogLine(strbuf);
      if (verbose) printf(strbuf);

      sprintf(strbuf, "no");
      if (regs.w.bx & 0x04) { // VMM supported?
        dpmi_virtual_memory = 1;
        sprintf(strbuf, "yes");
        if (verbose) printf(strbuf);
      } else {
        if (verbose) printf(strbuf);
      }
      if (LogDpmiInfo) LogLine(strbuf);
    }

    //--- Find available memory -------
    memset(&regs, 0, sizeof(regs));
    memset(&sregs, 0, sizeof(sregs));
    regs.x.eax = 0x500;          // DPMI Get Memory Info
    sregs.es = FP_SEG(&mi);
    regs.x.edi = FP_OFF(&mi);
    int386x( 0x31, &regs, &regs, &sregs );

    if (!regs.w.cflag) {
      if (verbose) printf( ", P:%dK", mi.largest_lockable_pages*4 );
      if (dpmi_virtual_memory)
        if (verbose) printf( ", A:%dK", mi.largest_block_bytes/1024 );
      //dpmi_physical_memory = mi.largest_lockable_pages*4096;
      //dpmi_available_memory = mi.largest_block_bytes;
      dpmi_physical_memory = mi.total_physical_pages*4096;
      dpmi_available_memory = mi.total_pages * 4096;

      if (LogDpmiInfo) {
        sprintf(strbuf, "\n  --- Current DPMI memory information ---\n" \
                          "  largest_available_block: %u bytes\n" \
                          "  maximum_unlocked_page_allocation: %u\n", mi.largest_block_bytes, mi.max_unlocked_page_allocation );
        LogLine(strbuf);
        sprintf(strbuf, "  maximum_locked_page_allocation: %u\n" \
                        "  total_pages: %u\n" \
                        "  total_unlocked_pages: %u\n", mi.largest_lockable_pages, mi.total_pages, mi.unlocked_pages );
        LogLine(strbuf);
        sprintf(strbuf, "  free_pages: %u\n" \
                        "  total_physical_pages: %u\n" \
                        "  free_linear_pages: %u\n" \
                        "  paging_size_pages: %u\n", mi.unused_physical_pages, mi.total_physical_pages, mi.free_linear_pages, mi.paging_size_pages);
        LogLine(strbuf);
      }
    } else {
      if (LogDpmiInfo) LogLine("\n  --- Current DPMI memory information ---\n  Failed\n");
      if (verbose) printf( "MemInfo failed!" );
      dpmi_physical_memory = 16*1024*1024;            // Assume 16 MB
      dpmi_available_memory = 16*1024*1024;           // Assume 16 MB
    }

    if (!dpmi_lock_region( _GETDS, 4096 )) {
      //printf( "dpmi: Error locking _GETDS" );
      if (LogDpmiInfo) LogLine("  Error locking _GETDS_\n");
      //exit(1);
      return 2;
    }
    if (!dpmi_lock_region( cstart_, 4096 )) {
      //printf( "Error locking cstart" );
      if (LogDpmiInfo) LogLine("  Error locking _GETCS_\n");
      //exit(1);
      return 3;
    }
    if (!dpmi_lock_region( _chain_intr, 4096 )) {  // lock int chain
      //printf( "Error locking _chain_intr" );
      if (LogDpmiInfo) LogLine("  Error locking _chain_intr\n");
      //exit(1);
      return 4;
    }

    if (LogDpmiInfo)
      LogLine("- Dpmi init successfully\n");

    return 0;

} // dpmi_init()

//------------------------------------------

void *dpmi_get_temp_low_buffer( int size )
// returns linaddr of the runtime low dos memory (usually 1K -> RUNTIMEDOSMEMSIZE)
// öbergabe: size == needed size in bytes
// RÅckgabe: linaddr of temp buffer
{
  if ( dpmi_dos_buffer == NULL ) return NULL;
  if ( size > RUNTIMEDOSMEMSIZE ) return NULL;

  return dpmi_dos_buffer;

} // dpmi_get_temp_low_buffer()

//------------------------------------------

int dpmi_set_pm_handler(unsigned intnum, void far *isr )
// öbergabe: intnum == ...
//           isr == far ptr (16:32) to ISR
// RÅckgabe: 0 == failed, otherwise 1
{
   union REGS regs;
   memset(&regs, 0, sizeof(regs));

/* Use DMPI call 204h to get pm interrrupt */
   regs.w.ax = 0x0205;
   regs.h.bl = intnum;
   regs.w.cx = FP_SEG(isr);
   regs.x.edx = FP_OFF(isr);
   int386( 0x31, &regs, &regs );
   if (!regs.w.cflag)
     return 0;
   return 1;

} // dpmi_set_pm_handler()
