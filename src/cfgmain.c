#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfgmain.h"


//---- private functions ------
char *LocateCfgFile(const char *filename, char *paths_to_check[]);
void InitCfgItem(FILE *, itemdef *);
itemdef *GetCfgItem(const char *);


// pointer to actual item list (set by InitCfgValues())
itemdef *actitems = NULL;


//---- function implementations --------------------------

char *LocateCfgFile(const char *filename, char *paths_to_check[])
// šbergabe: filename == string im 8.3 format
// Rckgabe: vollst. Pfad zur gefundenen Datei (current dir first)
{
  int i = 0;
  static char fullpath[_MAX_PATH];

  while (paths_to_check[i] != NULL) {
    _searchenv(filename, paths_to_check[i], fullpath);
    if (fullpath[0]) break;
    i++;
  }

  if (fullpath[0])
    return &fullpath;
  else
    return NULL;

} // LocateCfgFile()

//--------------------------------------------------------

char *strpbrkn(char *s1, const char *ign)
// returns a pointer to the 1st char not within ign
{
  char *srchs2;
  unsigned char leave;

  while (*s1) {   // for all chars in the sourcestring

    leave = 1;
    for (srchs2 = (char *)ign; *srchs2; srchs2++) {  // for all in charset
      if (*s1 == *srchs2) {
        leave = 0;
        break;
      }
    }
    if (leave) return((char *)s1);

    s1++;
  }
  return NULL;

} // strpbrkn()

//--------------------------------------------------------

void InitCfgItem(FILE *fin, itemdef *item)
// šbergabe: fin == offenes cfg file, item == ptr auf def. item
// Rckgabe: - (success to be determind by item->value)
{
  char *mystr;
  unsigned long tmpuval;
  static unsigned char buf[256];
  static char iname[200], ival[200], ieqsign[16];

  if (fin == NULL || item == NULL)
    return;

  fseek(fin, 0L, SEEK_SET);

  //---- for all lines -----------
  while (fgets(buf, sizeof(buf) - 1, fin) != NULL) {

    //---- get first token -----
    if ((mystr = strpbrkn((char *)&buf, " \t")) == NULL) continue;
    //---- commentline? -----   RENE: added two more comment characters
    if (*mystr == ';' || *mystr == '[' || *mystr == '#') continue;            // skip to next line
    sscanf(mystr, "%190[^ =\t\n]", &iname);
    mystr += strlen(iname);

    //---- get second token -----
    if ((mystr = strpbrkn(mystr, " \t")) == NULL) continue; // FF to "="
    sscanf(mystr, "%14[= \t]", &ieqsign);
    mystr += strlen(ieqsign);

    //---- get third token -----
    if ((mystr = strpbrkn(mystr, " \t")) == NULL) continue;
    sscanf(mystr, "%190[^\n]", &ival);

    //printf("iname:   [%s]\n", iname);
    //printf("ieqsign: [%s]\n", ieqsign);
    //printf("ival:    [%s]\n", ival);

    if ( (strlen(iname) > 0) && (strlen(ieqsign) > 0) && (strlen(ival) > 0)) {

      //--- check for single '=' ------
      if (strchr(&ieqsign, '=') == NULL)
        continue;

      //--- only if 2 args were found ----
      if (stricmp(item->name, &iname) == 0) {
        //--- wow, iname found ----
        switch (item->type) {
          case LONGTYPE :
          case ULONGTYPE : tmpuval = strtoul((char *)&ival, NULL, 0);
                          if (errno == 0) {   // EZERO
                            if ((item->value = malloc(8)) != NULL)
                              *(unsigned long *)item->value = tmpuval;
                          }
                          else
                            errno = 0;
                          break;

          case STRTYPE : if ((mystr = strrchr(&ival, '"')) != NULL) {
                           *mystr = '\0';
                           if ((mystr = strchr(&ival, '"')) != NULL) {
                             mystr++;
                             if (strlen(mystr) > 0)
                               if ((item->value = malloc(strlen(mystr)+2)) != NULL)
                                 strcpy(item->value, mystr);
                           }
                         }
                         break;
        }

#ifndef CFGQUIET
        //---- output recog. item ---------
        if (item->value == NULL)
          printf("[%s]=<NULL>\n", item->name);
        else
          switch (item->type) {
            case LONGTYPE :  printf("[%s]=@%p: [%d, 0x%X]\n", item->name, item->value, *(long int *)item->value, *(long int *)item->value); break;
            case ULONGTYPE : printf("[%s]=@%p: [%u, 0x%X]\n", item->name, item->value, *(unsigned long *)item->value, *(unsigned long *)item->value); break;
            case STRTYPE :   printf("[%s]=@%p: [%s]\n", item->name, item->value, (char *)item->value);  break;
          }
#endif
        break;  // item found (first one wins)
      }
    }
  }

} // InitCfgItem()

//--------------------------------------------------------

long InitCfgValues(const char *cfgname, char *paths_to_search[],
                    itemdef *defined_items, char **fullname)
// šbergabe: -
// Rckgabe: 0 == ok, fullname to be filled with parsed filename
//           1 == can't open fullname
//           2 == can't locate fullname
{
  long i, retval = 0;
  FILE *cfgin;

  DoneCfgValues();  // wer weiss...
  actitems = defined_items;   // save new list of items

  if ((*fullname = LocateCfgFile(cfgname, paths_to_search)) != NULL) {
    //---- cfg is found ---------------
    if ((cfgin = fopen(*fullname, "rt")) != NULL) {
      for (i = 0; actitems[i].type != TERMITEM; i++) {
        //---- for every defined item ----
        InitCfgItem(cfgin, &actitems[i]);
      }
      fclose(cfgin);
      retval = 0;
    } else retval = 1; // can't open
  }
  else {
    *fullname = (char *)cfgname;  // should work
    retval = 2; // can't locate
  }

  atexit( DoneCfgValues );

  return retval;

} // InitCfgValues()

//--------------------------------------------------------

void DoneCfgValues(void)
// šbergabe: -
// Rckgabe: -
{
  long i;

  for (i = 0; actitems[i].type != TERMITEM; i++) {
    //---- for every defined item ----
    if (actitems[i].value != NULL) free(actitems[i].value);
  }
  actitems = NULL;

} // DoneCfgValues()

//--------------------------------------------------------

itemdef *GetCfgItem(const char *itemname)
// šbergabe: name of cfgitem (case insensitive)
// Rckgabe: ptr auf den itemvalue falls vorhanden, sonst NULL
{
  long i;

  for (i = 0; actitems[i].type != TERMITEM; i++) {
    //---- for every defined item ----
    if (stricmp(actitems[i].name, itemname) == 0)
      return &actitems[i];
  }
  return NULL;

} // GetCfgItem()

//--------------------------------------------------------

long *GetCfgLong(const char *itemname)
// šbergabe: name of cfgitem (case insensitive)
// Rckgabe: ptr auf den itemvalue falls vorhanden, sonst NULL
//           Achtung: ebenfalls NULL, falls type falsch
{
  itemdef *myitem = GetCfgItem(itemname);
  if (myitem == NULL || myitem->type != LONGTYPE)
    return NULL;   // it's a string, NOT an ordinal type
  return (long *)myitem->value;

} // GetCfgLong()

//--------------------------------------------------------

unsigned long *GetCfgULong(const char *itemname)
// šbergabe: name of cfgitem (case insensitive)
// Rckgabe: ptr auf den itemvalue falls vorhanden, sonst NULL
//           Achtung: ebenfalls NULL, falls type falsch
{
  itemdef *myitem = GetCfgItem(itemname);
  if (myitem == NULL || myitem->type != ULONGTYPE)
    return NULL;   // it's a string, NOT an ordinal type
  return (unsigned long *)myitem->value;

} // GetCfgULong()

//--------------------------------------------------------

char *GetCfgString(const char *itemname)
// šbergabe: name of cfgitem (case insensitive)
// Rckgabe: ptr auf den itemvalue falls vorhanden, sonst NULL
//           Achtung: ebenfalls NULL, falls type falsch
{
  itemdef *myitem = GetCfgItem(itemname);
  if (myitem == NULL || myitem->type != STRTYPE)
    return NULL;   // it's a string, NOT an ordinal type
  return (char *)myitem->value;

} // GetCfgString()


