#ifndef TRACK_H
#define TRACK_H

#include "typedefs.h"


#define GP2_SEGINFO_VERSION	4
#define GP2LAP_SEG_NAME_LEN	38
#define GP2LAP_DATADIR		"gp2lpdat"

// Segment range info in Track editor file description tag
#define GP2LOGSIG_GLX			0x4c4c4740							   // @GLL
#define GP2_SEGINFO_SIG			0x47455340							   // @SEG


///////////////////////////////////////////////////////////////////////////////////////////////// //
// Own track segment info

typedef struct {
	WORD	unk;
	WORD	inner;
	WORD	outer;									 // width of inner + outer
	WORD	total;				  // width of inner + outer + part under grass
} CurbInfo;

//	Header in version 1:
//		DWORD sig;
//		DWORD version;
//		DWORD num_segs;
//		CurbInfo[4];	(hw, lw, hh, lh)
//	Added in version 2:
//		char name[40];
//	Added in version 3:
//		pit segments
//  Added in version 4:
//      track name length 38 (already halfway during v3 though)

#define SIF_CURB_LEFT		0x01
#define SIF_CURB_RIGHT		0x02
#define SIF_CURB_LOW		0x04
#define SIF_CURB_CCLINE_HI	0x08
#define SIF_CURB_CCLINE_LO	0x10

typedef struct GP2SEGINFO
{
	WORD		nr;					   // except high 5 bits (which are flags)
	short		angle;											// angle left?
	short		angleV;
	short		xPos;
	short		yPos;
	short		zPos;												 // height
	WORD		width;									 // width of track * 4
	short		xSide;				 // b0-5=width (= 2 * length side vector),
// b6-15=x part of side vector to edge
	short		ySide;	   // b0-5=unused, b6-15=y part of side vector to edge
	char		xExtraSide;										// white line?
	char		yExtraSide;
	short		bestLine;
	DWORD		flags;							   // see SIF_ #define's above
	char		hoCurbHeight;					   // high outer width of curb
	char		fill[3];
} GP2SegInfo;


typedef struct { WORD from, to; } SegRange;

typedef struct {
	char name[GP2LAP_SEG_NAME_LEN];
	GP2SegInfo *segs;
	int num_segs;
	int num_pitsegs;
	CurbInfo curb_info[4];
} TrackSegInfo;
   
void ProcessTrackSegRanges(char *txt, int len);
const SegRange *GetCurSegInfo(int t, BOOL sh, BOOL custom);

void DumpTrackSegData(int t, DWORD csum, char *name);
BOOL ReadTrackSegData(DWORD csum, TrackSegInfo *info);
void KillTrackSegInfo(TrackSegInfo *info);


///////////////////////////////////////////////////////////////////////////

#endif
