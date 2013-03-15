//
// GP2Def.h
//
#ifndef GP2DEF_H
#define GP2DEF_H

#include "typedefs.h"
#include "f1def.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif


#define GP2_EXE_UK					0
#define GP2_EXE_FRE					1
#define GP2_EXE_GER					2
#define GP2_EXE_DUT					3
#define GP2_EXE_ITA					4
#define GP2_EXE_SPA					5
#define GP2_EXE_USA					6
#define GP2_EXE_NUMVERSIONS			7

#define GP2_EXE_UK_SIZE				5702937
#define GP2_EXE_UK_DATALEN			0x449f25

#define GP2_EXE_CODE_START			0x78254		// (492116) difference with IDA (or 0x88274?)
#define GP2_EXE_DATA_START			0x63254		// (406100) difference with IDA

// Car shape offsets
#define GP2_EXE_CAR_START			0x14C4A8
#define GP2_EXE_CAR_LENGTH			54536

#define GP2_NUM_TRACKS				F1_NUM_TRACKS
#define GP2_NUM_WHEELS				F1_NUM_WHEELS
#define GP2_NUM_GEARS				F1_NUM_GEARS
#define GP2_NUM_CARS_RACE			F1_NUM_CARS_RACE
#define GP2_NUM_OPPLEVELS			5

#define GP2_MAX_REVS				20000
#define GP2_MAX_POWER_REVS			14800	// at this rpm power is highest
#define GP2_MAX_NUMSEGS				2048
#define GP2_MAX_NUMCARS				40
#define GP2_MAX_NUMTEAMS			20
#define GP2_MAX_PERFDATALAPS		40
#define GP2_MAX_FRONTRH				51600
#define GP2_MAX_REARRH				86000
#define GP2_MAX_FRONTST				34400
#define GP2_MAX_REARST				68800

#define GP2_CAR_HPBASE				790
#define GP2_CAR_POWERBASE			(GP2_CAR_HPBASE*HP_FACTOR)

#define GP2_WHEELSTRUCT_SIZE		0x200

#define GP2_FILE_ID					0x853da1be	// first four bytes of data file
#define GP2_FILE_ID_SIZE			4
#define GP2_FILE_DESC_SIZE			18
#define GP2_FILE_CHECKSUM_SIZE		4
#define GP2_FILE_TYPE_MASK			0x0f
#define GP2_FILE_TYPE_CRUNCHED_MASK	0x80
#define GP2_FILE_TYPE(t)			((t) & GP2_FILE_TYPE_MASK)
#define GP2_FILE_ISCRUNCHED(t)		((t) & GP2_FILE_TYPE_CRUNCHED_MASK)
#define GP2_FILE_TYPE_ISHOTLAP(t)	((t) == GP2_FILETYPE_HOTLAP || (t) == GP2_FILETYPE_HOTLAP_L)
#define GP2_FILE_TYPE_ISGAME(t)		((t) == GP2_FILETYPE_QRACE ||		\
									 (t) == GP2_FILETYPE_NCGAME ||		\
									 (t) == GP2_FILETYPE_CHGAME ||		\
									 (t) == GP2_FILETYPE_QRACE_L ||		\
									 (t) == GP2_FILETYPE_NCGAME_L ||	\
									 (t) == GP2_FILETYPE_CHGAME_L ||	\
									 (t) == GP2_FILETYPE_ULPRAC)

#define GP2_FILETYPE_NAMES			0
#define GP2_FILETYPE_RECORDS		1
#define GP2_FILETYPE_CONFIG			2
#define GP2_FILETYPE_SETUP			3
#define GP2_FILETYPE_ALLSETUPS		4
#define GP2_FILETYPE_QRACE			5
#define GP2_FILETYPE_ULPRAC			6
#define GP2_FILETYPE_NCGAME			7
#define GP2_FILETYPE_CHGAME			8
#define GP2_FILETYPE_QRACE_L		9
#define GP2_FILETYPE_ULPRAC_L		10
#define GP2_FILETYPE_NCGAME_L		11
#define GP2_FILETYPE_CHGAME_L		12
#define GP2_FILETYPE_PERF			13
#define GP2_FILETYPE_HOTLAP			14
#define GP2_FILETYPE_HOTLAP_L		15

#define GP2_PERF_MAXLEN				75000
#define GP2_EXTPERF_MAXLEN			150000

#define GP2_TRACK_MAXLEN			62000

#define GP2_SURF_TYPE_TRACK			0
#define GP2_SURF_TYPE_CURBLOW		1
#define GP2_SURF_TYPE_CURBHIGH		2
#define GP2_SURF_TYPE_GRASS			3
#define GP2_SURF_TYPE_GRIND			4
#define GP2_SURF_TYPE_CURBGRASS		5
#define GP2_SURF_OFFTRACK(t)		((t) == GP2_SURF_TYPE_GRASS || (t) == GP2_SURF_TYPE_GRIND)
#define GP2_SURF_VERYOFFTRACK(t)	(GP2_SURF_OFFTRACK(t) || (t) == GP2_SURF_TYPE_CURBGRASS)

// Car numbers.
#define GP2_CAR_ID_MASK				0x3f
#define GP2_CAR_ID_PLAYER_MASK		0x80
#define GP2_CAR_ID_CCPLAYER_MASK	0x40
#define GP2_CAR_ID(id)				((id) & GP2_CAR_ID_MASK)
#define GP2_CAR_ISPLAYER(id)		((id) & (GP2_CAR_ID_PLAYER_MASK | GP2_CAR_ID_CCPLAYER_MASK))
#define GP2_CAR_ISCCPLAYER(id)		((id) & GP2_CAR_ID_CCPLAYER_MASK)

#define GP2_LONGNAME_LEN			24
#define GP2_SHORTNAME_LEN			13

   
// Time mask.
#define GP2_TIME_INV_MASK			0xf0000000UL
#define GP2_TIME_ISVALID(t)			(!((t) & GP2_TIME_INV_MASK))

// Failure types
#define GP2_FAIL_TYPE_SUSP			0
#define GP2_FAIL_TYPE_LOOSEWHEEL	1
#define GP2_FAIL_TYPE_PUNCTURE		2
#define GP2_FAIL_TYPE_ENGINE		3
#define GP2_FAIL_TYPE_TRANSMISSION	4
#define GP2_FAIL_TYPE_OILLEAK		5
#define GP2_FAIL_TYPE_THROTTLE		6
#define GP2_FAIL_TYPE_ELECTRICS		7
#define GP2_FAIL_TYPE_WATERLEAK		8
#define GP2_FAIL_TYPE_BRAKE			9
#define GP2_FAIL_TYPE_WORNTYRES		10	// never used?

// Car parts, e.g. used in GP2Car.invisibleParts bits
#define GP2_CAR_PART_COCKPIT		0	// including driver helmet
#define GP2_CAR_PART_NOSE			1	// excluding front wing
#define GP2_CAR_PART_RF_WING		2	// right horizontal part
#define GP2_CAR_PART_RF_WING_SIDE	3	// right vertical part
#define GP2_CAR_PART_LF_WING		4	// left horizontal part
#define GP2_CAR_PART_LF_WING_SIDE	5	// left vertical part
#define GP2_CAR_PART_RF_SUSP		6
#define GP2_CAR_PART_RF_WHEEL		7
#define GP2_CAR_PART_LF_SUSP		8	
#define GP2_CAR_PART_LF_WHEEL		9
#define GP2_CAR_PART_R_BODY			10	// right sidepot
#define GP2_CAR_PART_L_BODY			11	// left sidepot
#define GP2_CAR_PART_ENGINE			12	// behind driver
#define GP2_CAR_PART_RR_SUSP		13
#define GP2_CAR_PART_RR_WHEEL		14
#define GP2_CAR_PART_LR_SUSP		15
#define GP2_CAR_PART_LR_WHEEL		16
#define GP2_CAR_PART_MR_WING		17	// mid horizontal part
#define GP2_CAR_PART_RR_WING		18	// right vertical part
#define GP2_CAR_PART_LR_WING		19	// left vertical part
#define GP2_CAR_PART_TR_WING		20	// top horizontal part
#define GP2_NUM_CAR_PARTS			21

#define GP2_SEG_LENGTH				(16*FOOT_FACTOR)

#ifdef RGB
#define COLOR_LR RGB(255,0,0)
#define COLOR_RR RGB(0,0,255)
#define COLOR_LF RGB(0,255,0)
#define COLOR_RF RGB(255,255,0)
#endif


typedef struct GP2HEADER
{
	DWORD id;	// GP2_FILE_ID
	BYTE type;
	// Track field only for track specific file
	BYTE track;
	// Fields below only for saved games (session mode valid)
	BYTE num_players;
	BYTE num_linkplayers;	// linked game only
	BYTE unknown2;		// unused
	BYTE unknown3;		// unused
	BYTE session_mode_valid;
	BYTE session_mode;
	BYTE laps_completed;
	BYTE laps_total;
	char desc[GP2_FILE_DESC_SIZE];	// Race leader
} GP2Header;


// size 0x20 (32)
typedef struct GP2PAXINFO
{
	DWORD num_samples;	// 00 Number of sample blocks in the file 
	WORD track_length;	// 04 In feet
	BYTE track_nr;		// 06 0-15
	BYTE lap_nr;		// 07 Lap number 
	BYTE total_laps;	// 08 Total number of laps in session
	BYTE flags;			// 09 b5=name ok?, b6=not saved, b7=cur hdr
	WORD date;			// 0a days since 1978 (Amiga date format!)
	BYTE time_jifs;		// 0c Day time at which the lap was done
	BYTE time_secs;		// 0d
	BYTE time_mins;		// 0e
	BYTE time_hours;	// 0f
	DWORD lap_time;		// 10 Lap time in milliseconds 
	BYTE file_name[8];	// 14 null-padded string giving the filename of the hot lap from which the data was taken, or this file's name if it is saved directly
	DWORD data_start;	// 1c Pointer to first perf record in memory (in memory, the records are not stored behind this info block)
} GP2PAxInfo;

// size 0x19 (25)
typedef struct GP2PAXSAMPLE
{
	WORD distance;		// Distance past the start/finish line, in feet 
	short speed;		// Speed
	short steering;		// Steering
	WORD revs;			// RPM
	BYTE throttle;		// Throttle position, 0 through 64 (0 being none) 
	BYTE brake;			// Brake position, 0 through 64 (0 being none) 
	char gear;			// Gear, -1, 0, 1, ..., 6
	BYTE ride_height[GP2_NUM_WHEELS];	// Ride heights
	BYTE susp_travel[GP2_NUM_WHEELS];	// Remaining suspension travels
	BYTE wheelspin[GP2_NUM_WHEELS];		// Wheelspin values
	BYTE seg_x;			// HIBYTE of (x-coordinate of track segment + 0x8000), 0 through 255
	BYTE seg_y;	// HIBYTE of (y-coordinate of track segment + 0x8000), 0 through 255
} GP2PAxSample;


// In gp2, unit of speed is 1/64 foot/sec (for word, for dword it's 1/(64*65536))
// Note: unit of wheel speed in perf data is divided by 128
#define GP2_SPEED_FACTOR			(FOOT_FACTOR/64.0)	// = 0.0047625
#define GP2_RAWSPEED_FACTOR			(GP2_SPEED_FACTOR/65536.0)
#define GP2_RAWWHEELSPEED_FACTOR	(FOOT_FACTOR/16384.0)

#define GP2_XY_FACTOR			(FOOT_FACTOR/16384.0)
#define GP2_XYCAM_FACTOR		(FOOT_FACTOR/512.0)
#define GP2_SEG_FACTOR			(FOOT_FACTOR/8.0)
#define GP2_XYTOSEG_FACTOR		(GP2_XY_FACTOR/GP2_SEG_FACTOR)
#define GP2_SEGTOXY_FACTOR		(GP2_SEG_FACTOR/GP2_XY_FACTOR)

#define GP2_STEER_FACTOR		((-35.0*PI)/(180.0*6336.0))
#define GP2_ANGLE_FACTOR		(PI/32768.0)

#define GP2_NUM_PITSTOPS		F1_NUM_PITSTOPS

#define GP2_FILE_SETUP_SIZE		84
#define GP2_NUM_ARB_VALUES	11


typedef struct GP2CSX
{										// Unit				Range
	BYTE front_wing;					//					[1-20]
	BYTE rear_wing;						//					[1-20]
	BYTE gear_ratios[GP2_NUM_GEARS];	// 1:64				[16-80]
	BYTE compound;						// 0-3 = A-D		[2]
	BYTE brake_bias;					// (+50%) 0.125%	[0-200]
	BYTE num_pitstops;					//					[0-3]
	BYTE level;							// 0=1, 0x80=2
	BYTE pit_laps[GP2_NUM_PITSTOPS];	// % of total laps	[1-102]
	BYTE unused1;
	BYTE packers[GP2_NUM_WHEELS];		// mm				front: [0-40], rear: [0-80]
	BYTE fast_bump[GP2_NUM_WHEELS];		// ?				[0-8]
	BYTE fast_rebound[GP2_NUM_WHEELS];	// ?				[0-8]
	BYTE slow_bump[GP2_NUM_WHEELS];		// ?				[0-24]
	BYTE slow_rebound[GP2_NUM_WHEELS];	// ?				[0-24]
	BYTE spring[GP2_NUM_WHEELS];		// 10 lb/in			front: [80-160], rear: [60-140]
	BYTE ride_height[GP2_NUM_WHEELS];	// 0.5 mm			front: [30-100], rear: [40-160]
	BYTE rarb;							// lb/in			[0-10] index in RARB_VALUES
	BYTE unused2;
	BYTE farb;							// lb/in			[0-10] index in FARB_VALUES
	BYTE unused3;
} GP2CSx;



#define TC_A	0
#define TC_B	1
#define TC_C	2
#define TC_D	3


static const BYTE GP2_TRACK_TC[] =
{
	TC_B, TC_C, TC_C, TC_D,
	TC_C, TC_C, TC_B, TC_B,
	TC_A, TC_D, TC_B, TC_A,
	TC_B, TC_B, TC_B, TC_D
};


// File info

typedef struct GP2FILEINFO
{
	char		file_name[16];
	DWORD		menu_id;
	BYTE		*buf_ptr;
	DWORD		buf_len;
	DWORD		dec_len;
	DWORD		min_len;
	DWORD		max_len;
	DWORD		flags_28;
	BYTE		type;
	BYTE		unk_2d;
	BYTE		unk_2e;
	BYTE		has_header;
	BYTE		has_checksum;
	BYTE		jtbl_index;
	BYTE		do_crunch;
	BYTE		unk_33;
} GP2FileInfo;



// Track segment
typedef struct GP2SEG
{
	short		angle;			// angle left?
	short		angleV;
	short		xPos;
	short		height;
	short		yPos;
	short		angle_0a;		// angle right?
	short		xSide;			// b0-5=width (= 2 * length side vector),
								// b6-15=x part of side vector to edge
	short		ySide;			// b0-5=unused, b6-15=y part of side vector to edge
	BYTE		field_10;
	char		xExtraSide;		// white line?
	BYTE		flags_12;		// b6=ccline over max, b7=ccline below min
	char		yExtraSide;
	short		field_14;
	short		bestLine;
	char		field_18;
	char		field_19;
	WORD		nr;				// except high 5 bits (which are flags)
	char		field_1c;
	char		field_1d;
	BYTE		field_1e;
	char		flags_1f;
	WORD		field_20;
	char		field_22;
	char		field_23;
	char		field_24;
	char		field_25;
	WORD		field_26;
	char		field_28;
	char		field_29;
	WORD		field_2a;
	DWORD		curbs_2c;		// b2=right curb, b3=left curb
	BYTE		unk030[0x14];
	struct GP2SEG *equal_seg;		// segment along pit segment
	BYTE		flags_48;		// b2=low curb
	BYTE		unk049[0x03];
	short		xSide_4c;		// slightly different at very few places!
	short		ySide_4e;
	BYTE		unk050[0x07];
	BYTE		field_57;
	BYTE		unk058[0x08];
	WORD		width_60;		// width left * 2
	WORD		width;			// width right * 2
	char		fl_split;
	char		field_65;
	char		field_66;
	char		flags_67;
	char		field_68;
	char		field_69;
	char		curb_ho_height;	// high outer width of curb
	BYTE		unk06b;
} GP2Seg;


// Car data
typedef struct GP2CAR
{
	short		zAngle;			// normal angle of car, one turn is -32768 ... 32767
	BYTE		unk_002;
	BYTE		unk_003;
	DWORD		timeAlarm004;
	BYTE		splitNr;
	BYTE		anaClutch;		// analog clutch input
	WORD		segPosX;		// zero is middle, negative is left, unit same as best line
	WORD		segPosY;		//
	WORD		field_E;
	GP2Seg		*pSeg;			// pointer to segment block
	WORD		dwSpeed;		// low 16 bits of speed (not always used)
	WORD		speed;			// unit 1/64 FOOT/sec
	BYTE		flags_18;											   // b4=?
	BYTE		flags_19;
	BYTE		failLap;		// lap nr where you will get failure
	BYTE		failSeg;		// segment >> 3 where you will get failure
	WORD		segDist;		// inter-segment distance
	WORD		segDistFactor;	// 0-0x4000, used for interpolation
	WORD		field_20;
	BYTE		lapNr;			// starts with 0 for race, 1 for non-race?
	BYTE		flags_23;		// b1=checkpoint end of lap?, b5=out of race, b7=pit?
	char		gear;			// [-1,0,1,...,6 = R,N,1 or tg,2,...,6]
	BYTE		teamNr;			// starts with 1
	WORD		field_26;
	long		xPos;
	long		yPos;
	long		xSpeed;
	long		ySpeed;
	DWORD		field_38;
	BYTE		flagsFail2;
	BYTE		flags_3D;		// b4=use adv setup, b6=OH off, b7=SH off
	WORD		curbrel_3E;		// ? related to curbs and related to field_c8
	DWORD		timePrevBest;
	WORD		weight;			// in lb
	WORD		grip;
	WORD		convSteer;		// both ana and dig: steering after st.help, lsz, etc.
	WORD		field_4A;
	BYTE		lockedWheelRel;
	BYTE		flagsGrass_4D;	// ? b4-7=wheel on grass?
	WORD		field_4E;		// related to 16c	
	DWORD		field_50;
	DWORD		timeLapStart;
	WORD		rearWingRel2;	// ?
	WORD		field_5A;
	WORD		speed2;			// ?
	BYTE		flags_5E;		// b0=race over; b4=damage
	BYTE		anaBrake;		// analog brake input
	WORD		field_60;
	WORD		revs;
	WORD		field_64;
	BYTE		racePos_66;		// copy from racePos
	BYTE		field_67;
	DWORD		field_68;
	WORD		field_6C;
	WORD		field_6E;
	WORD		cc_speed;		// used for CC's, auto-brake and pit speed limit
	WORD		cc_acc;			// ?
	WORD		field_74;
	WORD		field_76;
	WORD		field_78;
	WORD		anaSteer;		// raw analog steering input; right = positive
	BYTE		flags_7C;		// b0=player, b2=cc, b4=turning gear
	BYTE		digCtrlInput;	// b0=thr, b1=brk, b2=stl, b3=str, b4=grshft, b5=?, b6=?, b7=uniq.gr
	WORD		field_7E;		// something with visibility of car towards other cars
	BYTE		field_80;
	BYTE		drvAids;
	BYTE		field_82;
	BYTE		flags_83;
	WORD		field_84;
	WORD		field_86;
	WORD		field_88;
	WORD		field_8A;
	DWORD		timeAlarm08C;
	BYTE		flags_90;		// b5=removing, b7=show fuel
	BYTE		flags_91;		// b7=marshall pushing, etc
	WORD		throttle;		// range = 0...engine power
	BYTE		flags_94;
	BYTE		anaThrottle;	// analog throttle input
	WORD		field_96;
	BYTE		flags_98;
	BYTE		flagsFail1;
	DWORD		timeAlarm09a;
	DWORD		field_9E;
	WORD		enginePower;	// hp factor
	BYTE		racePos;		// (race pos - 1) * 2
	BYTE		field_A5;
	BYTE		id;				// b7=player
	BYTE		csIndex;		// index into car struct offset table (0x020e bytes before first struct)
	DWORD		field_A8;
	BYTE		failType;		// see failure type defines
	BYTE		flags_AD;		// b0=inbox, b2=to pit, b3=pitlane
	WORD		field_AE;
	WORD		field_B0;
	WORD		gripFact2_0b2;	// ?
	WORD		field_B4;
	BYTE		flags_B6;
	BYTE		field_B7;
	WORD		gripFactor;		// always $4000 for player
	char		prevGear_0ba;	// ?
	BYTE		damageLeds;
	struct GP2CAR	*pFirstCar;
	struct GP2CAR	*pCar0c0;
	struct GP2CAR	*pCar0c4;
	WORD		field_C8;		// ? related to field_0A and seg+0x60
	WORD		field_CA;		// ? related to field_0A and seg+0x62
	BYTE		flags_CC;
	BYTE		unk0cd;
	WORD		flags_CE;		// copy of 18
	struct GP2CAR	*pCar0d0;
	WORD		field_D4;
	BYTE		field_D6;
	BYTE		numPitStopsDone;
	DWORD		timeAlarm0d8;	// black flag
	struct GP2CAR	*pCar0dc;
	struct GP2CAR	*pCar0e0;
	struct GP2CAR	*pCar0e4;
	BYTE		flags_E8;
	BYTE		flags_E9;
	WORD		field_EA;
	WORD		field_EC;
	WORD		field_EE;
	BYTE		unk0f0;
	BYTE		unk0f1;
	BYTE		field_F2;
	BYTE		field_F3;
	WORD		field_F4;
	WORD		field_F6;
	WORD		segLenMinDist_0f8;	// ?
	WORD		field_FA;			// ? 0-field_F4
	DWORD		field_FC;
	DWORD		field_100;
	DWORD		zSpeed;
	DWORD		zPos;
	DWORD		field_10C[GP2_NUM_WHEELS];
	DWORD		negST[GP2_NUM_WHEELS];	// ?
	DWORD		ST[GP2_NUM_WHEELS];	// ?
	DWORD		wheelSpeed[GP2_NUM_WHEELS];
	DWORD		invisibleParts;
	DWORD		xField_150;
	DWORD		zField_154;
	DWORD		yField_158;
	DWORD		field_15C;
	DWORD		field_160;
	DWORD		field_164;
	BYTE		dispFlags_168;		// b2=black flag, b3=? (set if out of race)
	BYTE		flags_169;			// b2=will get failure, b3=failure active, b4+5=checkpoints split2+1
	BYTE		flags_16A;			// b4=call pit?
	BYTE		flags_16B;
	DWORD		downforceRelRW;
	WORD		downforceRelFW;
	BYTE		field_172;
	BYTE		field_173;
	DWORD		rearWingRel;
	BYTE		unk178;
	BYTE		unk179;
	BYTE		unk17a;
	BYTE		unk17b;
	BYTE		unk17c;
	BYTE		unk17d;
	BYTE		unk17e;
	BYTE		unk17f;
	WORD		fuelUnit;	// unit value of fuel consumption
	BYTE		field_182;
	BYTE		flags_183;
	DWORD		field_184;
	DWORD		suSprings[GP2_NUM_WHEELS];
	DWORD		suRideheights[GP2_NUM_WHEELS];
	DWORD		suSlowRebounds[GP2_NUM_WHEELS];
	DWORD		suFastRebounds[GP2_NUM_WHEELS];
	DWORD		suSlowBumps[GP2_NUM_WHEELS];
	DWORD		sufastBumps[GP2_NUM_WHEELS];
	DWORD		field_1E8[GP2_NUM_WHEELS];	// sr related
	DWORD		field_1F8[GP2_NUM_WHEELS];	// fr related
	DWORD		field_208[GP2_NUM_WHEELS];
	DWORD		rideHeight[GP2_NUM_WHEELS];
	DWORD		damageRel;			// ? related to damage
	DWORD		field_22C;
	WORD		field_230;
	BYTE		ledInfo;			// b0+2=car in front
	BYTE		unk233;
	WORD		field_234;
	WORD		field_236;
	DWORD		tyreWear[GP2_NUM_WHEELS];
	DWORD		calc_248[GP2_NUM_WHEELS];
	DWORD		suspTravel[GP2_NUM_WHEELS];
	DWORD		rarb;
	DWORD		farb;
	WORD		brakes;
	BYTE		field_272;
	BYTE		field_273;
	BYTE		numPitStops;
	BYTE		pitStop1;			// %
	BYTE		pitStop2;			// %
	BYTE		pitStop3;			// %
	DWORD		calc_278[GP2_NUM_WHEELS];
	DWORD		notOnDamper_288[GP2_NUM_WHEELS];	// ?
	DWORD		fuelLoad;
	DWORD		field_29C;
	DWORD		field_2A0;
	DWORD		field_2A4;
	WORD		grip_2a8;			// ? grip from track?
	WORD		field_2AA;
	DWORD		wheel_2AC[GP2_NUM_WHEELS];
	long		xRoll;	// 'dive' (forwards/backwards)
	long		yRoll;	// (left/right)
	DWORD		pSeg2;
	WORD		field_2C8;
	BYTE		unk2ca;
	BYTE		unk2cb;
	DWORD		timeLastSpl1;
	DWORD		timeLastSpl2;
	DWORD		timeLast;			// last lap time
	DWORD		timeBestSpl1;
	DWORD		timeBestSpl2;
	DWORD		timeBest;			// best lap time
	DWORD		field_2E4;
	BYTE		gearRatios[GP2_NUM_GEARS];
	BYTE		field_2ee;
	BYTE		field_2ef;
	DWORD		packers[GP2_NUM_WHEELS];
	DWORD		field_300;
	BYTE		unk304;
	BYTE		unk405;
	WORD		fuelLoadLaps;		// 1 lap fuel = 0x40
	DWORD		time308;
	DWORD		field_30C;
	DWORD		timeAlarm310;
	BYTE		field_314;
	BYTE		field_315;
	WORD		field_316;
	DWORD		timeAlarm318;
	WORD		field_31C;
	BYTE		field_31E;
	BYTE		flags_31F;
	DWORD		timeAlarm320;
	DWORD		timeAlarm324;
	DWORD		timeAlarm328;
	BYTE		field_32C;
	BYTE		field_32D;
	BYTE		field_32E;
	BYTE		flags_32F;
} GP2Car;



typedef char	GP2LongName[GP2_LONGNAME_LEN];
typedef char	GP2ShortName[GP2_SHORTNAME_LEN];


#define GP2_YES				0xff
#define GP2_NO				0x00

//
// Offsets within gp2 saved game file, header exclusive
#define GP2_FPS					0x00001a	// byte
#define GP2_ISHOTLAP			0x002510	// bool
#define GP2_NOPERFDATA			0x002511	// bool
#define GP2_SNAPSHOT_1			0x002db3	// offset (see GP2SS_*)
#define GP2_SNAPSHOT_2			0x00aae7	//   "
#define GP2_SNAPSHOT_3			0x010f07	//   "

// Offsets within a snapshot
#define GP2SS_NUM_CARS			0x0000	// number of cars (word)
#define GP2SS_CAR_STRUCT_CARS	0x0004	// 40 bytes (index of car same as index in GP2SS_CAR_STRUCTS)
#define GP2SS_CAR_STRUCTS		0x1078	// 26 * GP2CS_SIZEOF bytes
#define GP2SS_PMIRES1			0x6a64	// dword
#define GP2SS_WHEEL_STRUCTS		0x6bd0	// 4 * 0x200 bytes


#ifdef _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif
