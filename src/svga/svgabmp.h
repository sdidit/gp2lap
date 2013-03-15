#ifndef _SVGABMP_H
#define _SVGABMP_H


//--------------------------------------------------------
//------ data types --------------------------------------
//--------------------------------------------------------

#pragma pack (push, 1)

struct bmpfheader {    /* bmfh  == 14 Bytes */
  unsigned short bfType;       // 0x4D42 == "BM"
  unsigned long bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned long bfOffBits;
};

struct bmpiheader {    /* bmih  == 40 Bytes */
  unsigned long biSize;
  long biWidth;
  long biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned long biCompression;
  unsigned long biSizeImage;
  long biXPelsPerMeter;
  long biYPelsPerMeter;
  unsigned long biClrUsed;
  unsigned long biClrImportant;
};

#pragma pack (pop)

//------ nicht mehr gebraucht ------
//unsigned char bmph[] = {
//0x42, 0x4D, 0x36, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x04, 0x00, 0x00, 0x28, 0x00,
//0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
//0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
//0x00, 0x00, 0x00, 0x01, 0x00, 0x00 };  // 54 Byte


//--------------------------------------------------------
//------ function prototypes -----------------------------
//--------------------------------------------------------


long int savestream_svgabmp(unsigned char *pixstream,
                             unsigned long xpos, unsigned long ypos,
                              unsigned long xdim, unsigned long ydim,
                               unsigned char *bmppal, FILE *bmpfile);
long int readstream_svgabmp(unsigned long xsize, unsigned long ysize,
                             unsigned char *pixstream,
                              unsigned char *bmppal, FILE *bmpfile);
void swap_lines(unsigned long xdim, unsigned long ydim, unsigned char *pic640480ptr);



#endif

