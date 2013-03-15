#include "stdinc.h"
#include "typedefs.h"
#include "gp2str.h"
#include "gp2misc.h"
#include "gp2log.h"
#include "misc.h"
#include "basiclog.h"

static char *gp2lap_strs[GP2_EXE_NUMVERSIONS-1][NUM_GP2LAP_IDS] = {
	{
// English
		"\x11""This custom track is not HOF2.5 compliant!\n"
			"\x11""The resulting hot lap will not be accepted in HOF2.5.\n"
			"\x11""Please download custom HOF2.5 tracks from the HOF2.5 home page.",
		"\x11""Custom track is placed in wrong slot!\n"
			"\x11""Please consult the HOF2.5 home page to get the correct slot.\n",
		"\x11""[HOF25] DQ: No slowdown before start of lap.",
		"\x11""[HOF25] DQ: Session not loaded from a GP2Lap extended file with HOF2.5 mode.",
		"\x11""[HOF25] DQ: Detail level changed during lap.",
		"\x11""[HOF25] DQ: Indestructability was used during lap.",
	}, {
// French
		"\x11""Le circuit n'est pas HOF2.5 conforme!\n"
			"\x11""Le meilleur tour r‚sultant ne sera pas re‡u dans HOF2.5.\n"
			"\x11""Veuillez t‚l‚charger les circuits HOF2.5 faites\n"
			"\x11""sur commande du Home Page HOF2.5.\n",
		"\x11""Le circuit est plac‚e dans fausse la fente!\n"
			"\x11""Veuillez consulter le Home Page HOF2.5\n"
			"\x11""pour obtenir correcte la fente.",
		"\x11""[HOF25] DQ: Aucun d‚filement ralenti avant d‚but de recouvrement.",
		"\x11""[HOF25] DQ: Session non charg‚e … partir ‚tendu par GP2Lap d'un fichier.",
		"\x11""[HOF25] DQ: Le niveau de d‚tail a chang‚ pendant le recouvrement.",
		"\x11""[HOF25] DQ: Indestructability a ‚t‚ utilis‚ pendant le recouvrement.",
	}, {
// German
		"\x11""Der gew„hlte Kurs ist nicht HOF2.5 kompatibel!\n"
			"\x11""Die erzielten \"heiáen\" Runden werden in HOF2.5 nicht akzeptiert.\n"
			"\x11""Bitte besorgen Sie sich HOF2.5 kompatible Kurse\n"
			"\x11""von der HOF2.5 Homepage.\n",
		"\x11""Der Kurs befindet sich im falschen Slot!\n"
			"\x11""Informieren Sie sich bitte auf der HOF2.5 Homepage,\n"
			"\x11""um den korrekten Slot zu ermitteln.",
		"\x11""[HOF25] DQ: Keine Verlangsamung vor Anfang der Runde.",
		"\x11""[HOF25] DQ: Session nicht von einer GP2Lap erweiterten Datei geladen.",
		"\x11""[HOF25] DQ: Žnderung der Detailstufe w„hrend der Runde.",
		"\x11""[HOF25] DQ: Unkaputtbar-Modus w„hrend Runde aktiviert.",
	}, {
// Dutch
		"\x11""Dit circuit is niet HOF2.5 compatible!\n"
			"\x11""De resulterende snelle ronde zal niet in HOF2.5 worden geaccepteerd.\n"
			"\x11""Download HOF2.5 compatible circuits van de HOF2.5 homepage, A.U.B.",
		"\x11""Circuit zit in verkeerde slot!\n"
			"\x11""Raadpleeg de HOF2.5 homepage voor het juiste slot.",
		"\x11""[HOF25] DQ: Geen vertraging voor begin van de ronde.",
		"\x11""[HOF25] DQ: Sessie niet geladen van een GP2Lap-extended bestand.\n"
			"\x11""met HOF2.5 mode.",
		"\x11""[HOF25] DQ: Detail-niveau veranderd tijdens de ronde.",
		"\x11""[HOF25] DQ: Indestructability gebruikt tijdens de ronde.",
	}, {
// Italian
		"\x11""Questo circuito su ordinazione non Š HOF2.5 compliant!\n"
			"\x11""Il giro veloce risultante non sar… accettato in HOF2.5.\n"
			"\x11""Trasferire prego le piste dal sistema centrale verso\n"
			"\x11""i satelliti su ordinazione HOF2.5 dal Home Page."
		"\x11""Le circuito su ordinazione Š disposta errata in scanalatura!\n"
			"\x11""Consultare prego il Home Page HOF2.5\n"
			"\x11""per ottenere corretta la scanalatura.",
		"\x11""[HOF25] DQ: Nessun rallentamento prima dell' inizio del giro.",
		"\x11""[HOF25] DQ: Sessione non caricata da un archivio di GP2Lap-extended.",
		"\x11""[HOF25] DQ: Il livello del particolare Š cambiato durante il giro.",
		"\x11""[HOF25] DQ: Indestructability Š stato usato durante il giro.",
	}, {
// Spanish
		"\x11""Esta pista de encargo no es HOF2.5 obediente!\n"
			"\x11""El gran vuelta que resulta no ser  validado en HOF2.5.\n"
			"\x11""Descargue por favor las pistas de encargo HOF2.5\n"
			"\x11""del Home Page HOF2.5.",
		"\x11""La pista de encargo se coloca en incorrecta ranura!\n"
			"\x11""Consulte por favor el Home Page HOF2.5\n"
			"\x11""para conseguir correcta la ranura.",
		"\x11""[HOF25] DQ: Ning£n mecanismo de frenado de la velocidad de\n"
			"\x11""visualizaci¢n en pantella antes del comienzo del regazo.",
		"\x11""[HOF25] DQ: Sesi¢n no cargada de un fichero de GP2Lap-extended.",
		"\x11""[HOF25] DQ: El nivel del detalle cambi¢ durante regazo.",
		"\x11""[HOF25] DQ: Indestructability fue utilizado durante regazo.",
	}
};

static char *ver_tst_strings[GP2_EXE_NUMVERSIONS] = {
	"UK English version",
	"Version fran‡aise",
	"Deutsche Ausgabe",
	"Nederlandse versie",
	"Versione Italiana",
	"Versi¢n en espa¤ol",
	"American"	// ???
};

char gp2exe_version = GP2_EXE_UK;
char string_version = GP2_EXE_UK;

static char *NewGp2Strings[GP2STRCOUNT];    // about 7KB
char **pAllMnuStrPtrs = NULL;	// set from lammcall

typedef struct { long b1ID, b2ID, menuID, msgID; } MsgInfo;

#if 1	// method using typedef
	typedef long GP2MSGBOXFUNC(MsgInfo*);
	#pragma aux GP2MSGBOXFUNC parm[ESI] value[EAX];
	GP2MSGBOXFUNC *fpMsgBox = NULL; // set from lammcall
#else	// Frank's method
	long (*fpMsgBox)(void*) = NULL; // set from lammcall
	long CallGp2MsgBox(void*);
	#pragma aux CallGp2MsgBox = "call dword ptr fpMsgBox" parm[ESI] value[EAX];
#endif


static MsgInfo msg_info = { ID_OK, ID_none, ID_menu, ID_msg };


long __near _cdecl _saveregs MsgBox(long msgID, long menuID, long b1ID, long b2ID)
{
	long res = 0;
	msg_info.b1ID = b1ID;
	msg_info.b2ID = b2ID;
	if (menuID >= ID_GP2LAP) {
		char *tstr = gp2lap_strs[string_version][menuID-ID_GP2LAP];
		if (!tstr || !*tstr)
			tstr = gp2lap_strs[GP2_EXE_UK][menuID-ID_GP2LAP];
		set_gp2string(ID_menu, tstr);
		msg_info.menuID = ID_menu;
	} else
		msg_info.menuID = menuID;
	if (msgID >= ID_GP2LAP) {
		char *tstr = gp2lap_strs[string_version][msgID-ID_GP2LAP];
		if (!tstr || !*tstr)
			tstr = gp2lap_strs[GP2_EXE_UK][msgID-ID_GP2LAP];
		set_gp2string(ID_msg, tstr);
		msg_info.msgID = ID_msg;
	} else
		msg_info.msgID = msgID;
	res = fpMsgBox(&msg_info);
	if (msgID >= ID_GP2LAP)
		restore_gp2string(ID_msg);
	if (menuID >= ID_GP2LAP)
		restore_gp2string(ID_menu);
	return res;
}


// Call after pAllMnuStrPtrs is assigned!
void init_gp2_version(unsigned long local_strings)
{
	GP2LapRecInit *rec = NULL;
	char *test_str;
	char *ver_str;

	test_str = pAllMnuStrPtrs[0x3e7] + 4;
#ifdef TEST
	LogLine("  Test String: ");
	strncpy(strbuf, test_str, 40);
	strbuf[40] = 13;
	strbuf[41] = 10;
	strbuf[42] = 0;
	LogLine(strbuf);
#endif
	gp2exe_version = 0;
	do {
		ver_str = ver_tst_strings[gp2exe_version];
	} while (strnicmp(ver_str+2, test_str+2, strlen(ver_str)-2) && ++gp2exe_version < GP2_EXE_NUMVERSIONS);
	sprintf(strbuf, "  GP2 version: %s\n", GP2_ExeVerString(gp2exe_version));
	LogLine(strbuf);
	if (gp2exe_version == GP2_EXE_NUMVERSIONS)
		gp2exe_version = GP2_EXE_UK;
	if (local_strings) {
		string_version = gp2exe_version;
		if (string_version == GP2_EXE_USA)
			string_version = GP2_EXE_UK;
	}
	rec = (GP2LapRecInit *) Log_GetRecBuf(GP2LRTYPE_INIT);
	if (rec)
		rec->gp2lap_version |= (DWORD)gp2exe_version << 24;
}


//--------------------------------------------


#ifdef TEST
void __near _cdecl dump_all_gp2strings(void)
{
	unsigned long i, base;
	if (*pAllMnuStrPtrs) {
	// not dumping the strings beyond: i.e. teamnames, engines, etc...
		sprintf(strbuf, "- Now dumping all original/normal %u gp2 strings: \n", GP2STRCOUNT);
		LogLine(strbuf);
		for (i = 0; i < GP2STRCOUNT; i++) {
			sprintf(strbuf, "%04X==\"%s\"\n", i, pAllMnuStrPtrs[i]);
			LogLine(strbuf);
		}

//   40 drivernames   // at GP2STRCOUNT 	// 69f
//   20 teamnames     // at GP2STRCOUNT+40	// 6c7
//   20 enginenames   //    etc.		// 6db
//    6 crtlmeth names				// 6ef
//    1 logworkfield				// 6f5
//    2 unknowns				// 6f6

		base = GP2STRCOUNT;
		LogLine("- Now dumping driver names: \n");
		for (i = 0; i < 40; i++) {
			sprintf(strbuf, "%04X==\"%s\"\n", i+base, pAllMnuStrPtrs[i+base]);
			LogLine(strbuf);
		}

		base += 40;
		LogLine("- Now dumping team names: \n");
		for (i = 0; i < 20; i++) {
			sprintf(strbuf, "%04X==\"%s\"\n", i+base, pAllMnuStrPtrs[i+base]);
			LogLine(strbuf);
		}

		base += 20;
		LogLine("- Now dumping engine names: \n");
		for (i = 0; i < 20; i++) {
			sprintf(strbuf, "%04X==\"%s\"\n", i+base, pAllMnuStrPtrs[i+base]);
			LogLine(strbuf);
		}

		base += 20;
		LogLine("- Now dumping ctrlmeth names: \n");
		for (i = 0; i < 6; i++) {
			sprintf(strbuf, "%04X==\"%s\"\n", i+base, pAllMnuStrPtrs[i+base]);
			LogLine(strbuf);
		}

		base += 6;
		LogLine("- Now dumping logworkfield: \n");
		sprintf(strbuf, "%04X==\"%s\"\n", base, pAllMnuStrPtrs[base]);
		LogLine(strbuf);

		base += 1;
		LogLine("- Now dumping unknowns: \n");
		for (i = 0; i < 2; i++) {
			sprintf(strbuf, "%04X==\"%s\"\n", i+base, pAllMnuStrPtrs[i+base]);
			LogLine(strbuf);
		}

	}
}
#endif

//--------------------------------------------

void init_new_gp2strings(void)
{
	memset(&NewGp2Strings, 0, sizeof(NewGp2Strings));
}


//--------------------------------------------

void set_gp2string(unsigned long strid, char *newstr)
// šbergabe: strid == gp2 id des strings
//	     newstr == ptr auf neuen String
{
  if (strid < GP2STRCOUNT) NewGp2Strings[strid] = newstr;
}

//--------------------------------------------

void restore_gp2string(unsigned long strid)
// šbergabe: strid == gp2 id des strings
{
  if (strid < GP2STRCOUNT) NewGp2Strings[strid] = NULL;
}

//--------------------------------------------

//#pragma aux AHFGetStrNrESI parm caller [ESI] value [ESI];

char* __near _saveregs AHFGetStrNrESI(unsigned long StrNr)
{
  static char MyNullStr[] = { "_" };
  if (*pAllMnuStrPtrs) {
    if ( (StrNr < GP2STRCOUNT) && (NewGp2Strings[StrNr] != NULL) )
      // only then!!!!!!!!
      return NewGp2Strings[StrNr];
    else
      return pAllMnuStrPtrs[StrNr];
  }
  return &MyNullStr;
} // AHFGetStrNrESI()


char *(__near _saveregs *fpAHFGetStrNrESI)(unsigned long) = AHFGetStrNrESI;


//-----------------------------------------------

// Warning: cannot use yet, needs a temporary buffer!
typedef long GP2CRUNCHFUNC(void *src, void *dst, long len);
#pragma aux GP2CRUNCHFUNC parm[ESI EDI ECX] value[ECX] modify[EAX ESI EDI ECX];
GP2CRUNCHFUNC *fpCrunch = NULL; 						  // set from lammcall


//-----------------------------------------------

char	*pGP2Dir =	NULL;

char *GetGP2Dir()
{
	return pGP2Dir;
}
