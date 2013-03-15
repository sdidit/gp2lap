#include "timing.h"


// include the pictures
#include "pictures/lefthdr.inc"
#include "pictures/righthdr.inc"
#include "pictures/tablet.inc"
#include "pictures/top3.inc"
#include "pictures/arrows.inc"
#include "pictures/sectors.inc"


//--------------------------------------------------------

void insertpicat(unsigned long posx, unsigned long posy,
                  unsigned long dx, unsigned long dy,
                   unsigned char pic[], unsigned char buf[])
//---- assuming we're in fntscrwidth x ... mode ----------
{
  static unsigned long i, x, y;

  i = 0;
  for (y = posy; y < (posy+dy); y++) {
    for (x = posx; x < (posx+dx); x++) {
      buf[fntscrwidth*y + x] = pic[i++];
    }
  }

  //buf[640*y + x] = GP2_TranspTab[palidx+pic[i++]];
}

//--------------------------------------------------------

void winsertpicat(unsigned long posx, unsigned long posy,
                   unsigned long dx, unsigned long dy,
                    unsigned char pic[], unsigned char buf[],
                     unsigned long width)  // zeilenbreite
//---- assuming we're in width x ... mode ------------
{
  static unsigned long i, x, y;

  i = 0;
  for (y = posy; y < (posy+dy); y++) {
    for (x = posx; x < (posx+dx); x++) {
      buf[width*y + x] = pic[i++];
    }
  }
}

//--------------------------------------------------------
