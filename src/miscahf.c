#include <ctype.h>
#include "miscahf.h"


//----- makebuffer for string output ------
char atmp[360];


//-------------------------------------------------

char *updown (char *str)
{
  char *strstart = str;
  unsigned int i = 1;

  while (*str) {
    if (i++ % 2)
      *str = tolower(*str);
    str++;
  }
  return strstart;

} // updown()

//-------------------------------------------------

char *dexor(char *str)
{
  char *strstart = str;
  while (*str) {
    //printf("%c -> %c", *str, *str ^ XORVAL); puts("");
    *str++ ^= XORVAL;
  }
  return strstart;

} // dexor()

//-------------------------------------------------

unsigned char *IDAtoFlat(unsigned long ida_adr)
{
  return (unsigned char *)((GP2_CodeStartAdr-0x10020) + ida_adr);
}

//-------------------------------------------------

unsigned char *IDACodeReftoDataRef(unsigned long ida_adr)
{
  return (unsigned char *)(*(unsigned long *)(IDAtoFlat(ida_adr)));
}

//---------------------------------------------------------------------------------------------

/*
void mypicinsertbelow(char *dest, char *src, unsigned long lines)
{
  static unsigned long i;

  for (i = 0; i < lines*640; i++) {

    if (*dest == 0x00)
      *dest++ = *src++;
    else {
      dest++;
      src++;
    }

  }

} // mymeminsertbelow()
*/

//---------------------------------------------------------------------------------------------

/*
void mypicinsertabove(char *dest, char *src, unsigned long lines)
{
  static unsigned long i;

  for (i = 0; i < lines*640; i++) {

    if (*src != 0x00)
      *dest++ = *src++;
    else {
      dest++;
      src++;
    }

  }

} // mymeminsertabove()
*/
