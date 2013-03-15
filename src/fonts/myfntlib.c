#include <string.h>
#include "myfntlib.h"


unsigned long fntscrwidth = 640;  // default: 640


// include some fonts
#include "arrn6.fnt"
#include "gp2_8x8.fnt"
#include "verd7fk.fnt"
#include "verd7k.fnt"
#include "arial20.fnt"


//---- private functions ------
//none

//--------------------------------------------------------

void initfont(fontdef *fnt)
{
  unsigned long i, x, row, maxx;

  for (i = 0; i < 256; i++) {  // for all chars
    maxx = 0;
    for (row = 0; row < fnt->ydim; row++) {
      for (x = 0; x < fnt->xdim; x++) {
        if (fnt->fntchars[i][(row*fnt->xdim) + x] != 0)
          if (x > maxx) maxx = x;
      }
    }
    fnt->usedxdim[i] = ++maxx;  // Koordinate 0 entspricht ja Breite 1
  }

  fnt->usedxdim[0x20] = fnt->usedxdim[0x5F] - 1; // (' ') kriegt die Breite von ('_') - 1
  //chxdim[0x20] = chxdim[0x5F] - 1; // (' ') kriegt die Breite von ('_') - 1

  // Ziffern will ich non-proport. (4 ist breiteste)
  maxx = 0;
  for (i = 0x30; i <= 0x39; i++)  // breiteste Zahl bestimmen, idR '4'
    if (fnt->usedxdim[i] > maxx) maxx = fnt->usedxdim[i];

  for (i = 0x30; i <= 0x39; i++)  // diese Breite jetzt allen Ziffern zuweisen
    fnt->usedxdim[i] = maxx;
}

//--------------------------------------------------------

//---------------------------------------------------------
//---- Routinen fuer'n einen fontdef fnt ------------------
//---------------------------------------------------------

void writefnt(char *str,
               fontdef *fnt,
                unsigned char *vbufofs, // Pointer auf Bildpuffer
                 unsigned long y, unsigned long x,  // Zeile, Spalte
                  unsigned char color)
// x,y == Position to begin drawing string at
{
  static unsigned char ch;
  static unsigned long xdim, ydim, i, j, actchar, vbuftmp;
  static unsigned char *fntchar;

  actchar = 0;
  xdim = fnt->xdim;
  ydim = fnt->ydim;

  //--------- fÅr alle Zeichen im String --------------
  while ((ch = str[actchar++]) != 0) {      // ch := aktuelles Zeichen

    fntchar = fnt->fntchars[ch];
    vbuftmp = (fntscrwidth*y) + x;

    //-------- fÅr alle 11 Zeilen des Charakterbitmaps ------------
    for (j = 0; j < ydim; j++) {
      //--------- fÅr alle 10 Pixelbytes in dieser Zeile -----------
      for (i = 0; i < xdim/*fnt->usedxdim[ch] muesste faster sein*/; i++) {

        //noch nicht optimiert:
        //if ( fnt->fntchars[ch][(j*xdim)+i] != 0 )
        //  vbufofs[((y+j) * fntscrwidth) + (x+i)] = color;
        if ( fntchar[i] != 0 )
          vbufofs[vbuftmp + i] = color;
      }
      vbuftmp += fntscrwidth;
      fntchar += xdim;
    }
    x += (fnt->usedxdim[ch] + fnt->optspacing);  // Ausgabe X Position um gewisse Zeichenbreite erhîhen
  }
}  // writefnt

//--------------------------------------------------------

void writefntr(char *str,
                fontdef *fnt,
                 unsigned char *vbufofs, // Pointer auf Bildpuffer
                  unsigned long y, unsigned long x,  // Zeile, Spalte
                   unsigned char color)
// x,y == Position to end drawing string at
{
// sollte die result. Startposition < 0 sein ==> bei 0 angefangen

  static unsigned long length, xdimwholestr;

  length = 0;
  xdimwholestr = 0;

  //--------- fÅr alle Zeichen im String (rÅckwÑrts) -----------
  while (str[length] != 0x00) {
    xdimwholestr += fnt->usedxdim[(unsigned char)str[length++]] + fnt->optspacing;
                    // PS: SPACING muss beachtet werden
  }

  writefnt(str, fnt, vbufofs,
            y, (xdimwholestr > x /*passt scho...*/) ? 0 : (x - xdimwholestr),
            color);
}  // writefntr()








//--------------------------------------------------------
// Testzwecke - fuers 'Version x.xx" auf Logo schreiben eingefuehrt
//-------------------
void wwritefnt(char *str,
                fontdef *fnt,
                 unsigned char *vbufofs, // Pointer auf Bildpuffer
                  unsigned long y, unsigned long x,  // Zeile, Spalte
                   unsigned char color,
                    unsigned short width,
                     unsigned short indivspacing)
// wie writefnt, nur mit indiv spacing und width statt sturer fntscrwidth
{
  static unsigned char ch;
  static unsigned long xdim, ydim, i, j, actchar, vbuftmp;
  static unsigned char *fntchar;

  actchar = 0;
  xdim = fnt->xdim;
  ydim = fnt->ydim;

  //--------- fÅr alle Zeichen im String --------------
  while ((ch = str[actchar++]) != 0) {      // ch := aktuelles Zeichen

    fntchar = fnt->fntchars[ch];
    vbuftmp = (width*y) + x;

    //-------- fÅr alle 11 Zeilen des Charakterbitmaps ------------
    for (j = 0; j < ydim; j++) {
      //--------- fÅr alle 10 Pixelbytes in dieser Zeile -----------
      for (i = 0; i < xdim/*fnt->usedxdim[ch] muesste faster sein*/; i++) {

        if ( fntchar[i] != 0 )
          vbufofs[vbuftmp + i] = color;
      }
      vbuftmp += width;
      fntchar += xdim;
    }
    x += (fnt->usedxdim[ch] + indivspacing); // Ausgabe X Position um eine Zeichenbreite erhîhen
  }
}  // wwritefnt()

//--------------------------------

void wwritefntr(char *str,
                 fontdef *fnt,
                  unsigned char *vbufofs, // Pointer auf Bildpuffer
                   unsigned long y, unsigned long x,  // Zeile, Spalte
                    unsigned char color,
                     unsigned short width,
                      unsigned short indivspacing)
// wie writefntr, nur mit indiv spacing und width statt sturer fntscrwidth
{
// sollte die result. Startposition < 0 sein ==> bei 0 angefangen

  static unsigned long length, xdimwholestr;

  length = 0;
  xdimwholestr = 0;

  //--------- fÅr alle Zeichen im String (rÅckwÑrts) -----------
  while (str[length] != 0x00) {
    xdimwholestr += fnt->usedxdim[(unsigned char)str[length++]] + indivspacing;
                    // PS: SPACING muss beachtet werden
  }

  wwritefnt(str, fnt, vbufofs,
             y, (xdimwholestr > x /*passt scho...*/) ? 0 : (x - xdimwholestr),
             color, width, indivspacing);
}  // wwritefntr()


