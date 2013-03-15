#ifndef MISC_H
#define MISC_H

//////////////////////////////////////////////////////////////////////////////////////////

// See gp2def.h for defines
extern char gp2exe_version;
extern char string_version;


//////////////////////////////////////////////////////////////////////////////////////////

// Button id's
#define ID_none					    -1L		// use for 2nd button to skip it
#define ID_empty					0x0000L	// empty text, do not use
#define ID_OK						0x0001L
#define ID_CANCEL					0x0002L
#define ID_YES					    0x0003L
#define ID_NO						0x0004L
#define ID_ALWAYS		            0x0005L
#define ID_NEVER	                0x0006L
#define ID_ASK						0x0007L
#define ID_CONTINUE					0x0088L
#define ID_RETRY					0x008aL
#define ID_ABORT					0x008bL
#define ID_ABANDON					0x008dL
#define ID_OVERWRITE				0x02b7L

// Text id's
#define ID_GAMESAVED				0x02a8L
#define ID_HOTLAPSAVED				0x02b4L
#define ID_PERFSAVED				0x032aL
#define ID_GAMELOADED				0x02a6L
#define ID_HOTLAPLOADED				0x0598L	// 0x02b2L is after being loaded for perf data extraction

#define ID_filetype					0x0238L	// add to file type for file type string (0-15)
#define ID_langversion				0x055aL	// add to gp2.exe version for version string (0-9)
#define ID_failure					0x056fL	// add to failure type for failure type string (0-9)
#define ID_circuit					0x059fL	// add to tracknr for circuit string (0-15)
#define ID_country					0x061fL	// add to tracknr for country string (0-15)
#define ID_country_prefix			0x065fL	// add to tracknr for country prefix string (0-15)
#define ID_country_postfix			0x067fL	// add to tracknr for country postfix string (0-15)

// Error id's
#define ID_errno					0x02baL	// add to file errno for error as gp2 message (0-7)
#define ID_ERR_SAVENOTPERMITTED		0x02c2L

// Some menu id's
#define ID_MENU_STARTUP				0x00a5L
#define ID_MENU_MAIN				0x00a7L
#define ID_MENU_DRIVE				0x00a9L
#define ID_MENU_CURCIRCUIT			0x057aL
#define ID_MENU_SAVEGAME			0x01fbL
#define ID_MENU_SAVEHOTLAP			0x01fcL
#define ID_MENU_SAVEPERF			0x0329L

// Unused id's used for gp2lap
#define ID_menu	0x0670
#define ID_msg	0x0671

// Custom gp2lap id's
#define ID_GP2LAP					2000L
#define ID_HOF25_TRACKNOTCOMPLIANT	ID_GP2LAP+0x00L
#define ID_HOF25_TRACKWRONGSLOT		ID_GP2LAP+0x01L
#define ID_HOF25_NOSLOWDOWN			ID_GP2LAP+0x02L
#define ID_HOF25_NOEXTLOAD			ID_GP2LAP+0x03L
#define ID_HOF25_DETAILEVELCHANGED	ID_GP2LAP+0x04L
#define ID_HOF25_F4USED				ID_GP2LAP+0x05L
#define NUM_GP2LAP_IDS							 6

//////////////////////////////////////////////////////////////////////////////////////////

// Messagebox function
long __near _cdecl _saveregs MsgBox(long msgID, long menuID, long b1ID, long b2ID);
#define MBTELL(msg,menu)	MsgBox(msg,menu,ID_OK,ID_none)
#define MBCONFIRM(msg,menu)	MsgBox(msg,menu,ID_OK,ID_CANCEL)
#define MBASK(msg,menu)		MsgBox(msg,menu,ID_YES,ID_NO)


//////////////////////////////////////////////////////////////////////////////////////////


#define GP2STRCOUNT 1695            // laut init routine
// then coming:
//   40 drivernames   // at GP2STRCOUNT
//   20 teamnames     // at GP2STRCOUNT+40
//   20 enginenames   //    etc.
//    6 crtlmeth names
//    1 logworkfield
//    2 unknowns

#ifdef TEST
void __near _cdecl dump_all_gp2strings(void);
#endif

void init_gp2_version(unsigned long);
void init_new_gp2strings(void);
void set_gp2string(unsigned long, char*);
void restore_gp2string(unsigned long);
#pragma aux AHFGetStrNrESI parm caller [ESI] value [ESI];
char* __near _saveregs AHFGetStrNrESI(unsigned long StrNr);

char *GetGP2Dir();

//////////////////////////////////////////////////////////////////////////////////////////

#endif	// MISC_H
