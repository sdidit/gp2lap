#include "stdinc.h"
#include <direct.h>
#include "gp2lap.h"
#include "gp2glob.h"
#include "basiclog.h"
#include "misc.h"
#include "track.h"

/*
**	Segment range notes:
**	-----------------
**	- For each track there are MAX_NUM_SEGRANGES segment ranges.
**	- Some action can be taken if the car is within one of these ranges.
**	- Default tracks have hardcoded segment ranges.
**	- Custom tracks can have a special @SEG tag in the #GP2INFO
**	  description tag (Desc), which includes ranges to be used for
**	  that particular track (see below).
**	- There are separate ranges for steering help on or off.
**
*/

#define BEGIN_TRACK(n)	static const SegRange tr_##n##_act_segs[]={
#define SEG_RANGE(f,t)	{f,t},
#define END_TRACK()		{0,0}};

BEGIN_TRACK(0d)	// Interlagos
END_TRACK()

BEGIN_TRACK(1d)	// Aida
END_TRACK()

BEGIN_TRACK(2d)	// Imola
SEG_RANGE(500, 550)		// Aqcue Minerale
SEG_RANGE(918, 978)		// Variante Bassa + Traguardo
END_TRACK()

BEGIN_TRACK(3d)	// Monaco
END_TRACK()

BEGIN_TRACK(4d)	// Barcelona
END_TRACK()

BEGIN_TRACK(5d)	// Montreal
END_TRACK()

BEGIN_TRACK(6d)	// Magny-Cours
END_TRACK()

BEGIN_TRACK(7d)	// Silverstone
END_TRACK()

BEGIN_TRACK(8d)	// Hockenheim
END_TRACK()

BEGIN_TRACK(9d)	// Hungaroring
SEG_RANGE(417, 436)		// First chicane
SEG_RANGE(614, 633)		// Second chicane
END_TRACK()

BEGIN_TRACK(ad)	// Spa
SEG_RANGE(1308, 1338)	// Bus-stop first chicane
END_TRACK()

BEGIN_TRACK(bd)	// Monza
SEG_RANGE(375, 400)		// Curva Della Roggia
END_TRACK()

BEGIN_TRACK(cd)	// Estoril
END_TRACK()

BEGIN_TRACK(dd)	// Jerez
END_TRACK()

BEGIN_TRACK(ed)	// Suzuka
SEG_RANGE(1070, 1085)	// Casio chicane
END_TRACK()

BEGIN_TRACK(fd)	// Aidelaide
SEG_RANGE(38, 70)		// Wakefield Road
END_TRACK()


// -----------------------------------------------------

BEGIN_TRACK(0a)	// Interlagos
END_TRACK()

BEGIN_TRACK(1a)	// Aida
END_TRACK()

BEGIN_TRACK(2a)	// Imola
SEG_RANGE(495, 521)		// JB, Aqcue Minerale
SEG_RANGE(918, 978)		// Variante Bassa + Traguardo
END_TRACK()

BEGIN_TRACK(3a)	// Monaco
END_TRACK()

BEGIN_TRACK(4a)	// Barcelona
END_TRACK()

BEGIN_TRACK(5a)	// Montreal
END_TRACK()

BEGIN_TRACK(6a)	// Magny-Cours
END_TRACK()

BEGIN_TRACK(7a)	// Silverstone
END_TRACK()

BEGIN_TRACK(8a)	// Hockenheim
SEG_RANGE(377, 407)		// JB
END_TRACK()

BEGIN_TRACK(9a)	// Hungaroring
SEG_RANGE(417, 436)		// First chicane
SEG_RANGE(614, 633)		// Second chicane
END_TRACK()

BEGIN_TRACK(aa)	// Spa
SEG_RANGE(1300, 1349)	// JB Bus-stop first chicane
END_TRACK()

BEGIN_TRACK(ba)	// Monza
SEG_RANGE(120, 164)		// JB
SEG_RANGE(378, 394)		// JB Curva Della Roggia
SEG_RANGE(745, 792)		// JB
END_TRACK()

BEGIN_TRACK(ca)	// Estoril
END_TRACK()

BEGIN_TRACK(da)	// Jerez
END_TRACK()

BEGIN_TRACK(ea)	// Suzuka
SEG_RANGE(1070, 1085)	// Casio chicane
END_TRACK()

BEGIN_TRACK(fa)	// Aidelaide
SEG_RANGE(43, 65)		// JB Wakefield Road
END_TRACK()



static const SegRange *dig_seg_ranges[F1_NUM_TRACKS] = {
	tr_0d_act_segs, tr_1d_act_segs, tr_2d_act_segs, tr_3d_act_segs,
	tr_4d_act_segs, tr_5d_act_segs, tr_6d_act_segs, tr_7d_act_segs,
	tr_8d_act_segs, tr_9d_act_segs, tr_ad_act_segs, tr_bd_act_segs,
	tr_cd_act_segs, tr_dd_act_segs, tr_ed_act_segs, tr_fd_act_segs
};

static const SegRange *ana_seg_ranges[F1_NUM_TRACKS] = {
	tr_0a_act_segs, tr_1a_act_segs, tr_2a_act_segs, tr_3a_act_segs,
	tr_4a_act_segs, tr_5a_act_segs, tr_6a_act_segs, tr_7a_act_segs,
	tr_8a_act_segs, tr_9a_act_segs, tr_aa_act_segs, tr_ba_act_segs,
	tr_ca_act_segs, tr_da_act_segs, tr_ea_act_segs, tr_fa_act_segs
};

#define MAX_NUM_SEGRANGES	16
static SegRange tr_cur_dig_segs[MAX_NUM_SEGRANGES+1] = { 0 };
static SegRange tr_cur_ana_segs[MAX_NUM_SEGRANGES+1] = { 0 };



const SegRange *GetCurSegInfo(int t, BOOL sh, BOOL custom)
{
	return sh ?
			(custom ? tr_cur_dig_segs : dig_seg_ranges[t]) :
			(custom ? tr_cur_ana_segs : ana_seg_ranges[t]);
}


// Simple scanner for ranges in the format:
//
// Range:	[S/N]%04d-%04d
// Ranges:	Range[,Range,...]
//
// A range may not start or end at segment 0.
//
void ProcessTrackSegRanges(char *txt, int len)
{
	memset(tr_cur_dig_segs, 0, sizeof(tr_cur_dig_segs));
	memset(tr_cur_ana_segs, 0, sizeof(tr_cur_ana_segs));
	if (len > 8) {
		int di = 0, ai=0;
		int from = 0, to = 0;
		do {
			BOOL ana = TRUE, dig = TRUE;
			if (len > 9) {
				if (txt[0] == 'S') {
					ana = FALSE;
					txt++;
					len--;
				} else if (txt[0] == 'N') {
					dig = FALSE;
					txt++;
					len--;
				}
			}
			if (txt[4] != '-')
				break;
			from = atoi(txt);
			to = atoi(txt+5);
			if (from && to) {
				if (dig) {
					tr_cur_dig_segs[di].from = (WORD) from;
					tr_cur_dig_segs[di].to = (WORD) to;
					di++;
				}
				if (ana) {
					tr_cur_ana_segs[ai].from = (WORD) from;
					tr_cur_ana_segs[ai].to = (WORD) to;
					ai++;
				}
#ifdef TEST
				sprintf(strbuf, "      found segment check: range %d-%d", from, to);
				if (dig && !ana)
					strcat(strbuf, " (SH only)");
				else if (!dig && ana)
					strcat(strbuf, " (NSH only)");
				strcat(strbuf, "\n");
				LogLine(strbuf);
#endif
			} else
				break;
			if (len<10 || txt[9] != ',')
				break;
			txt += 10;
			len -= 10;
		} while (di<MAX_NUM_SEGRANGES && ai<MAX_NUM_SEGRANGES && len>8);
	}
}




/*
**	Track segment data notes:
**	-------------------------
**	- File should contain header with curb width/height info, and
**	  header should be expandable.
**	- File name or header must be unique for all tracks, including
**	  custom tracks.
**	- Perf data should indicate which track data file it needs for
**	  its track position data. Checksum seems to be the best way.
**
*/

static void FillSegInfo(const GP2Seg *pSeg, GP2SegInfo *pSI)
{
	pSI->nr = pSeg->nr;
	pSI->angle = pSeg->angle;
	pSI->angleV = pSeg->angleV;
	pSI->xPos = pSeg->xPos;
	pSI->yPos = pSeg->yPos;
	pSI->zPos = pSeg->height;
	pSI->width = pSeg->width;
	pSI->xSide = pSeg->xSide;
	pSI->ySide = pSeg->ySide;
	pSI->xExtraSide = pSeg->xExtraSide;
	pSI->yExtraSide = pSeg->yExtraSide;
	pSI->bestLine = pSeg->bestLine;
	pSI->flags = 0;
	setbits(pSI->flags, SIF_CURB_LEFT, ison(pSeg->curbs_2c, 1<<3));
	setbits(pSI->flags, SIF_CURB_RIGHT, ison(pSeg->curbs_2c, 1<<2));
	setbits(pSI->flags, SIF_CURB_LOW, ison(pSeg->flags_48, 1<<2));
	setbits(pSI->flags, SIF_CURB_CCLINE_HI, ison(pSeg->flags_12, 1<<6));
	setbits(pSI->flags, SIF_CURB_CCLINE_LO, ison(pSeg->flags_12, 1<<7));
	pSI->hoCurbHeight = pSeg->curb_ho_height;
	pSI->fill[0] = pSI->fill[1] = pSI->fill[2] = 0;
}


void DumpTrackSegData(int t, DWORD csum, char *name)
{
	char tpath[_MAX_PATH];
	FILE *tfh = NULL;
	int num_segs = pNumTrackSegs[0], num_pitsegs = pNumTrackSegs[1];
	DWORD file_ver = 0;
	DWORD sig = 0;
	char *base = GetGP2Dir();
	char *datadir = GP2LAP_DATADIR;

	if (!base)
		return;

//	if (!_access(datadir, F_OK))
//		rmdir(datadir); // bug in v1.04: was incorrectly created in current directory.
	sprintf(tpath, "%s\\%s", base, datadir);
	if (_access(tpath, F_OK))
		mkdir(tpath);	// put all files in here
	sprintf(tpath, "%s\\%s\\%08x.seg", base, datadir, csum);

	if (!_access(tpath, F_OK)) {
		tfh = fopen(tpath, "rb");
		if (tfh) {
			// Don't create new one if it already exists and it has same version
			fread(&sig, 1, 4, tfh);
			fread(&file_ver, 1, 4, tfh);
			fclose(tfh);
			tfh = NULL;
			if (sig == GP2_SEGINFO_SIG && file_ver >= GP2_SEGINFO_VERSION) {
#ifdef TEST
				sprintf(strbuf, "Segment data file %s already exists\n", tpath);
				LogLine(strbuf);
#else
				return;
#endif
			}
			// In case it's read-only...
			if (_access(tpath, W_OK))
				chmod(tpath, S_IRUSR | S_IWUSR);
		}
	}
	
	tfh = fopen(tpath, "wb");
	if (tfh) {
		int c, s;
		GP2Seg *pSeg = pTrackSegs;
		// Write sig and version
		sig = GP2_SEGINFO_SIG;
		file_ver = GP2_SEGINFO_VERSION;
		fwrite(&sig, 1, 4, tfh);
		fwrite(&file_ver, 1, 4, tfh);
		fwrite(&num_segs, 1, 4, tfh);
		fwrite(&num_pitsegs, 1, 4, tfh);	// added in v3
		for (c=0; c<16; c++)
			// Write only the low word (assuming little endian)
			fwrite(&pCurbData[c], 1, 2, tfh);
		// Added in segment data version 2: track checksum and name
		fwrite(&csum, 1, 4, tfh);
		fwrite(name, 1, GP2LAP_SEG_NAME_LEN, tfh);	// has been 38 by accident in v3 for a while
		// Write data
		for (s=0; s<num_segs; s++) {
			GP2SegInfo si;
			FillSegInfo(pSeg, &si);
			fwrite(&si, 1, sizeof(GP2SegInfo), tfh);
			pSeg++;
		}
		pSeg += 3;
		// pit segments added in v3
		for (s=0; s<num_pitsegs; s++) {
			GP2SegInfo si;
			FillSegInfo(pSeg, &si);
			fwrite(&si, 1, sizeof(GP2SegInfo), tfh);
			pSeg++;
		}
		fclose(tfh);
		tfh = NULL;
	}
	if (errno)
		sprintf(strbuf, "Error saving segment data to %s: %s\n", tpath, strerror(errno));
	else
		sprintf(strbuf, "Segment data saved to %s\n", tpath);
	LogLine(strbuf);
}



static char segbuf[100000];

BOOL ReadTrackSegData(DWORD csum, TrackSegInfo *info)
{
	char path[_MAX_PATH];
	FILE *file = NULL;
	char *base = GetGP2Dir();
	BOOL ok = FALSE;

	if (!base)
		return FALSE;

	memset(info, 0, sizeof(TrackSegInfo));
	sprintf(path, "%s\\%s\\%08x.seg", base, GP2LAP_DATADIR, csum);
	file = fopen(path, "rb");
	if (file) {
		// Read sig and version
		DWORD sig = 0;
		fread(&sig, 1, 4, file);
		if (sig == GP2_SEGINFO_SIG) {
			DWORD ver = 0;
			fread(&ver, 1, 4, file);
			if (ver >= 1) {
				// Read header
				BOOL ok2 = TRUE;
				fread(&info->num_segs, 1, 4, file);
				if (ver >= 3)
					fread(&info->num_pitsegs, 1, 4, file);
				fread(info->curb_info, 1, 32, file);
				if (ver >= 2) {
					DWORD tcsum = 0;
					fread(&tcsum, 1, 4, file);
					if (tcsum != csum)
						ok2 = FALSE;
					if (ok2) {
						fread(info->name, 1, GP2LAP_SEG_NAME_LEN, file);
						info->name[GP2LAP_SEG_NAME_LEN-1] = 0;
					}
				}
				if (ok2) {
					// Read data
					int num = info->num_segs + info->num_pitsegs;
					size_t sz = num * sizeof(GP2SegInfo);
					info->segs = (GP2SegInfo*) segbuf;
					if (info->segs) {
						ok = fread(info->segs, 1, sz, file) == sz;
//						if (!ok) {
//							free(info->segs);
//							info->segs = NULL;
//						}
					}
				}
			}
		}
		fclose(file);
	} else {
		sprintf(strbuf, "Cannot open segment data file '%s' (error %d)\n", path);
		LogLine(strbuf);
	}
	return ok;
}


void KillTrackSegInfo(TrackSegInfo *info)
{
//	if (info->segs) {
//		free(info->segs);
//		info->segs = NULL;
//	}
}

