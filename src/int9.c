#include <stdio.h>
#include <conio.h>
#include "dpmi.h"
#include "keyqueue.h"
#include "int9.h"


void (__interrupt __far *OldInt9)(); // fuer originalen int9 haendler


static unsigned long MyIntActive = 0;    // flag, ob mein handler aktiv
static KeyEvent actkbd;


//-----------------------------------------------------------------------


void __interrupt __far MyInt9()
{
  if ((inp(0x64) & 0x01) && InCockpitNow) {  // ueberhaupt taste vorhanden *und* im Cockpit?

  if (CanEnterCriticalSection(&MyIntActive)) {

    /*--- get the keys -----*/
    actkbd.scancode = inp(0x60);
    actkbd.key = actkbd.scancode & 0x7F;  // kill potential release bit

    switch (actkbd.key) {
      case KEY_LEFTSHIFT :
        actkbd.lshiftactive = (actkbd.scancode & 0x80) ? 0 : 1;
        break;
      case KEY_LEFTCTRL :
        actkbd.lctrlactive = (actkbd.scancode & 0x80) ? 0 : 1;
        break;
    }

    // small optimization (no keyboard steering, throttle. brakes keys)
    if ((actkbd.key != KEY_A) &&
        (actkbd.key != KEY_COMMA) &&
        (actkbd.key != KEY_PERIOD) &&
        (actkbd.key != KEY_Z) &&
        (actkbd.key != KEY_LEFTSHIFT) &&
        (actkbd.key != KEY_LEFTCTRL)) {

      KeyEventEnqueue(&actkbd);

    } // if !(keyboard driving keys)

    LeaveCriticalSection(&MyIntActive);
  }
  }

  OldInt9();

} // MyInt9()
