#ifndef _GP2STRUC_H
#define _GP2STRUC_H


#ifndef BYTE
#define BYTE    unsigned char
#endif
#ifndef WORD
#define WORD    unsigned short
#endif
#ifndef DWORD
#define DWORD   unsigned long
#endif


#pragma pack (push, 1)

#define lastlap_split1 lastlap[0]
#define lastlap_split2 lastlap[1]
#define lastlap_time   lastlap[2]

// Track segment
struct gp2seg {
  short           angle;                  // angle left?
  short           angleV;
  short           xPos;
  short           height;
  short           yPos;
  short           angle_0a;               // angle right?
  short           xSide;                  // b0-5=width (= 2 * length side vector),
                                                          // b6-15=x part of side vector to edge
  short           ySide;                  // b0-5=unused, b6-15=y part of side vector to edge
  BYTE            field_10;
  char            field_11;
  BYTE            flags_12;               // b6=ccline over max, b7=ccline below min
  char            field_13;               // very small (-10...10)
  short           field_14;
  short           bestLine;
  char            field_18;
  char            field_19;
  WORD            nr;                             // except high 5 bits (which are flags)
  short           f5Line;                 // +1Ch
  BYTE            field_1e;
  char            flags_1f;
  WORD            field_20;
  char            field_22;
  char            field_23;
  char            field_24;
  char            field_25;
  WORD            field_26;
  char            field_28;
  char            field_29;
  WORD            field_2a;

  DWORD           curbs_2c;               // b2=right curb, b3=left curb
  DWORD           dword_30;
  DWORD           dword_34;
  DWORD           dword_38;
  DWORD           dword_3C;
  DWORD           dword_40;

  struct gp2seg   *equalseg;

  WORD            flags_48;               // b2=low curb
  WORD            flags_4A;
  short           xSide_4c;               // slightly different at very few places!
  short           ySide_4e;
  BYTE            unk050[0x07];
  BYTE            field_57;
  BYTE            unk058[0x0a];
  WORD            width;                  // width of track * 4
  BYTE            unk064[0x08];
};


struct singlecarstruct {
  unsigned short vAngle;
  unsigned char unk_002;
  unsigned char unk_003;
  unsigned long timeAlarm004;
  unsigned char splitNr;
  unsigned char anaClutch;
  unsigned short field_A;
  unsigned short SegDist2_00b;
  unsigned short field_E;

  struct gp2seg *curSeg;          // at +10h

  unsigned short field_14;

  unsigned short curr_speed;            // at +16h
  unsigned char unk_18;
  unsigned char unk_19;

  unsigned char carfailurelap;          // at +1Ah
  unsigned char carfailuresection;      // at +1Bh
  unsigned short unk_1C1D;
  unsigned short unk_1E1F;
  unsigned short unk_2021;
  unsigned char lapnumber;              // at +22h
  unsigned char unk_23;    // oft benutzt
  unsigned char curr_gear;              // at +24h
  unsigned char teamnumber;             // at +25h
  unsigned short unk_2627;
  unsigned long Xcoord;                 // at +28h
  unsigned long Ycoord;                 // at +2Ch
  unsigned long Xspeed;                 // at +30h
  unsigned long Yspeed;                 // at +34h

  unsigned short unk_3839;
  unsigned short unk_3A3B;
  unsigned char unk_3C;    // oft benutzt
  unsigned char bitfield3D;       // Bit4 == adv. carsetup
  unsigned char unk_3E;
  unsigned char unk_3F;

  //unsigned long prevlaptime;            // at +40h
  unsigned long last_bestlap_time;            // at +40h
  unsigned short carweight;             // at +44h
  unsigned short unk_46;
  unsigned short unk_48;

  unsigned char unk_4A;
  unsigned char unk_4B;
  unsigned char unk_4C;
  unsigned char unk_4D;
  unsigned char unk_4E;
  unsigned char unk_4F;
  unsigned char unk_50;
  unsigned char unk_51;
  unsigned char unk_52;
  unsigned char unk_53;
  //unsigned long timeLapStart;              // at +54h
  unsigned char unk_54;
  unsigned char unk_55;
  unsigned char unk_56;
  unsigned char unk_57;
  unsigned char unk_58;
  unsigned char unk_59;
  unsigned char unk_5A;
  unsigned char unk_5B;
  unsigned char unk_5C;
  unsigned char unk_5D;
  unsigned char unk_5E;
  unsigned char unk_5F;
  unsigned char unk_60;
  unsigned char unk_61;

  unsigned short curr_rpm;              // at +62h
  unsigned short unk_64;
  unsigned char racepos_66;             // at +66h
  unsigned char unk_67_7B[21];

  unsigned char player_or_comp;         // at +7Ch
  unsigned char unk_7D;
  unsigned short visibility;            // at +7Eh   wozu?????
  unsigned char unk_80;
  unsigned char driving_aids;           // at +81h
  unsigned char unk_82_8B[10];
  unsigned long unk_8C;

  unsigned char remove_from_track;      // at +90h
  unsigned char marshallpushsyou;       // at +91h
  unsigned short engpower_acc_gas;      // at +92h
  unsigned char unk_94_A1[14];

  unsigned short enginepower;           // at +A2h
  unsigned char racepos;                // at +A4h
  unsigned char unk_A5;
  unsigned char car_id;                 // at +A6h    bit7 set if player
  unsigned char unk_A7_AB[5];

  unsigned char failurebits;            // at +ACh
  unsigned char info;                   // at +ADh    oft benutzt (char on track status?)
  unsigned char unk_AE_B1[4];
  unsigned short unk_B2;  //mit grip was
  unsigned short unk_B4;                // am Anfg. auf 4000h gesetzt
  unsigned char unk_B6;
  unsigned char unk_B7;

  unsigned short grip;                  // at +B8h
  unsigned char gear_BA;                // at +BAh
  //unsigned char unk_BB_14B[145];
  unsigned char unk_BB;
  unsigned long unk_BC;                 // pointer to carstruct table??

  unsigned long unk_C0[4];
  unsigned long unk_D0[4];
  unsigned long unk_E0[4];
  unsigned long unk_F0[4];
  unsigned long unk_100[4];
  unsigned long unk_110[4];
  unsigned long unk_120[4];
  unsigned long unk_130;
  unsigned long unk_134;
  unsigned long unk_138;

  unsigned long unk_13C;                // geschw. related
  unsigned long unk_140;                // geschw. related
  unsigned long unk_144;                // geschw. related (neg. bei rÅckw rollen)
  unsigned long unk_148;                // geschw. related (neg. bei rÅckw rollen)

  unsigned char damageinfo[16];         // at +14Ch
  unsigned long unk_15C;
  unsigned long unk_160;                // !=0 zÑhlt vor sich hin
  unsigned long unk_164;                // !=0 zÑhlt vor sich hin

  unsigned char unk_168;                // wird auf Zielgerade zu 0x40 sonst 0
  unsigned char failureplan;            // at +169h
  unsigned char unk_16A;                // Bit6 immer?,
                                        // Bit0 == aktiv, wenn car in box "hochgehoben",
                                        // Bit3 == car haelt sich zw. boxenein- und ausfahrtsstrichen auf; in jeder session; auch wenn falsch rum in box fahre
  unsigned char unk_16B;                // wechselt in zwei Streckenabschnitten

  unsigned long unk_16C;
  unsigned long unk_170;
  unsigned long unk_174;
  unsigned long unk_178;
  unsigned long unk_17C;

  unsigned short fuel_consumption;      // at +180h

  //unsigned char unk_182_231[176];

  unsigned short unk_182;
  unsigned short unk_184;
  unsigned short unk_186;
  unsigned long springrates[4];         // at +188h
  unsigned long rideheight[4];          // at +198h
  unsigned long slow_rebound[4];        // at +1A8h
  unsigned long fast_rebound[4];        // at +1B8h
  unsigned long slow_bump[4];           // at +1C8h
  unsigned long fast_bump[4];           // at +1D8h
  unsigned long sr_related[4];          // at +1E8h
  unsigned long fr_related[4];          // at +1F8h

  unsigned char unk_208_231[42];
  unsigned char led_info;               // at +232h

  //unsigned char unk_233_297[101];
  unsigned char unk_233;
  unsigned char unk_234;
  unsigned char unk_235;
  unsigned char unk_236;
  unsigned char unk_237;

  unsigned long tirewear[4];            // at +238h


  unsigned long calc248[4];
  unsigned long calc258[4];
  unsigned long ranti;                  // at +268h
  unsigned long fanti;                  // at +26Ch
  unsigned short brakes;                // at +270h
  unsigned short unk_272;

//---------- pitstop related ----------------
  unsigned char num_pitstops;           // at +274h
  unsigned char stop_perc1;             // at +275h
  unsigned char stop_perc2;             // at +276h
  unsigned char stop_perc3;             // at +277h
  unsigned long calc278[4];             // at +278h

  unsigned char unk_288;
  unsigned char unk_289;
  unsigned char unk_28A;
  unsigned char unk_28B;
  unsigned char unk_28C;
  unsigned char unk_28D;
  unsigned char unk_28E;
  unsigned char unk_28F;
  unsigned char unk_290;
  unsigned char unk_291;
  unsigned char unk_292;
  unsigned char unk_293;
  unsigned char unk_294;
  unsigned char unk_295;
  unsigned char unk_296;
  unsigned char unk_297;

  unsigned long fuel_load;              // at +298h

  //unsigned char unk_29C_2CB[48];
  unsigned long unk_29C;
  unsigned long unk_2A0;
  unsigned long unk_2A4;
  unsigned short unk_2A8;   // mit trackspez. grip was
  unsigned short unk_2AA;
  unsigned long unk_2AC;
  unsigned long unk_2B0;
  unsigned long unk_2B4;
  unsigned long unk_2B8;
  unsigned long unk_2BC;
  unsigned long unk_2C0;
  unsigned long unk_2C4;
  unsigned long unk_2C8;

//------------- timing -------------------------
  unsigned long lastlap[3];             // at +2CCh
  //unsigned long lastlap_split1;         // at +2CCh
  //unsigned long lastlap_split2;         // at +2D0h
  //unsigned long lastlap_time;           // at +2D4h

  //unsigned long bestlap[3];             // at +2D8h
  unsigned long bestlap_split1;         // at +2D8h
  unsigned long bestlap_split2;         // at +2DCh
  unsigned long bestlap_time;           // at +2E0h

  unsigned long unk_2E4;

  unsigned char gear_ratios[6];         // at +2E8h
  //unsigned char unk_2EE_305[24];
  unsigned short unk_2EE;               // immer 3232h ???

  unsigned long packers[4];             // at +2F0h

  unsigned short unk_300;
  unsigned short unk_302;
  unsigned short unk_304;
  unsigned short fuel_laps;             // at +306h  1 lap fuel = $40

  //unsigned char unk_308_32F[40];
  unsigned char unk_308;
  unsigned char unk_309;
  unsigned char unk_30A;
  unsigned char unk_30B;
  unsigned long unk_30C;
  unsigned long unk_310;
  unsigned char unk_314;
  unsigned char unk_315;
  unsigned char unk_316;
  unsigned char unk_317;
  unsigned char unk_318;
  unsigned char unk_319;
  unsigned char unk_31A;
  unsigned char unk_31B;
  unsigned char unk_31C;
  unsigned char unk_31D;
  unsigned char unk_31E;
  unsigned char unk_31F;
  unsigned char unk_320;
  unsigned char unk_321;
  unsigned char unk_322;
  unsigned char unk_323;
  unsigned char unk_324;
  unsigned char unk_325;
  unsigned char unk_326;
  unsigned char unk_327;
  unsigned char unk_328;
  unsigned char unk_329;
  unsigned char unk_32A;
  unsigned char unk_32B;
  unsigned char unk_32C;
  unsigned char unk_32D;
  unsigned char unk_32E;
  unsigned char unk_32F;

};

struct gp2cartable {
  struct singlecarstruct car[26];          // 26 autos
};

struct gp2timetable {
  unsigned long car[40];          // 40 autos, indiziert Åber cardid
};

/*
typedef struct {
  unsigned char name[24]}
} namedef;
*/
typedef unsigned char namedef[24];


struct gp2drivernames {
       // Ausnahme fÅr evtl. Pit Bit
  namedef driver[256/*40*/];     // da ja max. 40 autos
};


struct gp2order {
  unsigned char car[40];          // 40 autos, indiziert Åber cardid
};

struct gp2array40 {     // eingefuehrt wegen CaridTeamTab
  unsigned char elem[40];          // 40 elems
};


struct ErwRundzahl {     // siehe IDA 0x179dd7 t_ErwRundzahl
  unsigned char  unk_0[2];
  unsigned short length;
  unsigned char  laps;
  unsigned char  unk_5;
  unsigned char  thu_qual;
};


#pragma pack (pop)

struct rmcodestruc {
  unsigned char keytable[0x10];
  //unsigned char _dummy[0x4C3];
  unsigned char _dummy[0x4C3];
  unsigned char rmstring[12];
/*
  unsigned long rmstring;                  // ich krieg sonst die ptr-anweisg nicht hin...
  unsigned char rmstr_rest[8];
*/
  // ....
};


#endif
