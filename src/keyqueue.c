#include <stdio.h>
#include "dpmi.h"
#include "keyqueue.h"


static KeyEvent keyqueue[KEYQUEUESIZE];
static int head = 0, tail = 0;
static unsigned long KeyQueueMutex = 0;    // flag, zugriff auf keyqueue

//-----------------------------------------------------------------------

int KeyEventEnqueue(KeyEvent *srckey)
// 0 == error, 1 sonst
{
  if ( srckey == NULL || KeyQueueFull() )
    return 0;

  if (CanEnterCriticalSection(&KeyQueueMutex)) {

    //mymemcpy((void *)&keyqueue[tail], (void *)srckey, sizeof(KeyEvent));
    keyqueue[tail].scancode = srckey->scancode;
    keyqueue[tail].key = srckey->key;
    keyqueue[tail].lshiftactive = srckey->lshiftactive;
    keyqueue[tail].lctrlactive = srckey->lctrlactive;

    tail++;
    tail %= KEYQUEUESIZE;

    LeaveCriticalSection(&KeyQueueMutex);
  }

  return 1;

} // KeyEventEnqueue()

//-----------------------------------------------------------------------

int KeyEventDequeue(KeyEvent *destkey)
// 0 == error, 1 sonst
{
  if ( destkey == NULL || KeyQueueEmpty() )
    return 0;

  if (CanEnterCriticalSection(&KeyQueueMutex)) {

    //mymemcpy((void *)destkey, (void *)&keyqueue[head], sizeof(KeyEvent));
    destkey->scancode = keyqueue[head].scancode;
    destkey->key = keyqueue[head].key;
    destkey->lshiftactive = keyqueue[head].lshiftactive;
    destkey->lctrlactive = keyqueue[head].lctrlactive;

    head++;
    head %= KEYQUEUESIZE;

    LeaveCriticalSection(&KeyQueueMutex);
  }

  return 1;

} // KeyEventDequeue()

//-----------------------------------------------------------------------

void KeyEventClear(KeyEvent *destkey)
{
    destkey->scancode = 0;
    destkey->key = 0;
    destkey->lshiftactive = 0;
    destkey->lctrlactive = 0;

} // KeyEventClear()

//-----------------------------------------------------------------------

int KeyQueueEmpty()
// !=0 == leer,
{
  return (head == tail);

} // KeyQueueEmpty()

//-----------------------------------------------------------------------

int KeyQueueFull()
{
  return ( (head == tail+1) ||
           ( (tail == KEYQUEUESIZE-1) &&
             (head == 0)                 )
         );

} // KeyQueueFull()

//-----------------------------------------------------------------------

void KeyQueueInit()
{
  head = tail = 0;

} // KeyQueueInit()
