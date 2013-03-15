#ifndef _KEYQUEUE_H
#define _KEYQUEUE_H


//--------------------------------------------------------
//------ constants ---------------------------------------
//--------------------------------------------------------

#define KEYQUEUESIZE    16


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

#pragma pack (push, 1)

typedef struct mykbdstatus {
  unsigned char scancode;     // actual scancode within int handler
  unsigned char key;          // actual scancode & 0x7f within int handler
  //unsigned char released;     // whether scancode has been a release code
  unsigned char lshiftactive; // >0 == active
  unsigned char lctrlactive;  // >0 == active
} KeyEvent;

#pragma pack (pop)


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------

int KeyEventEnqueue(KeyEvent *);
int KeyEventDequeue(KeyEvent *);
void KeyEventClear(KeyEvent *);
int KeyQueueEmpty();
int KeyQueueFull();
void KeyQueueInit();





#endif

