#include "../stdinc.h"
#include "svgabmp.h"


struct bmpfheader bmpfhdr;
struct bmpiheader bmpihdr;

//--------------------------------------------------------
//------ function implementation -------------------------
//--------------------------------------------------------


long int savestream_svgabmp(unsigned char *pixstream,
                             unsigned long xpos, unsigned long ypos,
                              unsigned long xdim, unsigned long ydim,
                               unsigned char *bmppal, FILE *bmpfile)
{
  unsigned long i = 0;
  long int retcode = 0;

  //---- Die HeaderStrukturen flushen ----------
  memset((void *)&bmpfhdr, 0, sizeof(bmpfhdr));
  memset((void *)&bmpihdr, 0, sizeof(bmpihdr));

  //----- die BMP Headerstruktur fÅllen (nur die nîtigen members) --------
  bmpfhdr.bfType = 0x4D42; // == "BM"
  //bmpfhdr.bfSize = sizeof(bmfhdr) + sizeof(bmpihdr) + 1024 + (xdim*ydim);
  bmpfhdr.bfSize = 1078 + (xdim*ydim);
  bmpfhdr.bfOffBits = 1078;

  //----- die BMP Infostruktur fÅllen (nur die nîtigen members) --------
  bmpihdr.biSize = sizeof(bmpihdr);   // == 40
  bmpihdr.biWidth = xdim;
  bmpihdr.biHeight = ydim;
  bmpihdr.biPlanes = 1;
  bmpihdr.biBitCount = 8;
  bmpihdr.biCompression = 0;          // == BI_RGB (bmp is not compressed)
  bmpihdr.biSizeImage = (xdim*ydim);  // i.d.R. 0x4B000
  bmpihdr.biClrUsed = 256;
  bmpihdr.biClrImportant = 256;

  //------ BMP header speichern ------------
  if (fwrite(&bmpfhdr, sizeof(bmpfhdr), 1, bmpfile) == 1) {
    if (fwrite(&bmpihdr, sizeof(bmpihdr), 1, bmpfile) == 1) {

      //------ BMP farbpalette speichern ------------
      for (i = 0; i < 256; i++) {
        fputc(bmppal[(i*3) + 2] << 2, bmpfile);
        fputc(bmppal[(i*3) + 1] << 2, bmpfile);
        fputc(bmppal[(i*3) + 0] << 2, bmpfile);
        fputc(0x00, bmpfile);
        if (ferror(bmpfile)) return 0;
      }

      //------ BMP bilddaten speichern ------------

      // swap_lines(640, 480, pixstream);
      // if (fwrite(pixstream, 640*480, 1, bmpfile) == 1)
      //   return 1;

      retcode = 1; // erstmal auf Ok setzen
      for (i = (ypos + ydim); i > ypos; i--) {  // fÅr alle Zeilen
        if (fwrite(pixstream+((i-1)*xdim)+xpos, xdim, 1, bmpfile) != 1) {
          retcode = 0;  // fehler
          break;
        }
      }

    }
  }

  return retcode;

}  // savestream_svgabmp()

//-----------------------------------------------------------------------

long int readstream_svgabmp(unsigned long xsize, unsigned long ysize,
                             unsigned char *pixstream,
                              unsigned char *bmppal, FILE *bmpfile)
// RÅckgabe: 0 == general reading failure
//           -1 == not a bmp file
//           -2 == wrong dims (either 640x480x8 or 440x330x8)
//           -3 == is compressed
//           -4 == can't handle core bmp's
{
  unsigned long i;

  //----- globalen Header einlesen -------
  if (fread(&bmpfhdr, sizeof(bmpfhdr), 1, bmpfile) != 1)
    return 0;

  if (bmpfhdr.bfType != 0x4D42)
    return -1;

  //----- Bildbeschreibungs header einlesen -------
  if (fread(&bmpihdr, sizeof(bmpihdr), 1, bmpfile) != 1)
    return 0;

  //----- Bilddimensionen checken -------
  if ((bmpihdr.biWidth == xsize) && (bmpihdr.biHeight == ysize) && (bmpihdr.biBitCount == 8) ) {

    //----- Checken, ob keine Kompression -------
    if (bmpihdr.biCompression == 0) {

      if (bmpihdr.biSize != 40) return -4;

      //----- jetzt kommen Palettedaten -------
      for (i = 0; i < 256; i++) {
        bmppal[(i*3) + 2] = fgetc(bmpfile) >> 2;
        bmppal[(i*3) + 1] = fgetc(bmpfile) >> 2;
        bmppal[(i*3) + 0] = fgetc(bmpfile) >> 2;
        fgetc(bmpfile);
        if (ferror(bmpfile)) return 0;
      }

      //----- zu den Bilddaten seeken -------
      if (fseek(bmpfile, bmpfhdr.bfOffBits, SEEK_SET) != 0)
        return 0;

      //----- die Bilddaten einlesen -------
      if (fread(pixstream, xsize*ysize, 1, bmpfile) == 1) {
      //if (fread(&picbuf, 6, 1, bmpfile) == 1) {
        swap_lines(xsize, ysize, pixstream);
        return 1;
      }

    } else return -3;

  } else return -2;

  return 0;

}  // readstream_svgabmp()

//-----------------------------------------------------------------------

unsigned char pixline[640];

void swap_lines(unsigned long xdim, unsigned long ydim, unsigned char *pic640480ptr)
// for swapping the 640*480 Data for BMP
{
  unsigned long i = 0;
  for (i = 0; i < (ydim >> 1) /*240*/; i++) {
    memcpy((void *)&pixline, (void *)&pic640480ptr[i*xdim], xdim);  // vorne nach pixelline
    memcpy((void *)&pic640480ptr[i*xdim], (void *)&pic640480ptr[((ydim-1) - i)*xdim], xdim);  // von hinten nach vorne
    memcpy((void *)&pic640480ptr[((ydim-1) - i)*xdim], (void *)&pixline, xdim);  // von pixline nach hinten
  }

}  // swap_lines()


