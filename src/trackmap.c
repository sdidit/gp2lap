#include "stdinc.h"
#include <math.h>
#include "gp2lap.h"
#include "gp2glob.h"
#include "cfgmain.h"
#include "basiclog.h"
#include "keys.h"
#include "svga/vesa.h"
#include "keyhand.h"
#include "dpmi.h"										 // for mymemcpy()
#include "miscahf.h"							 // for mypicinsert*****()
#include "svga/gp2color.h"
#include "trackmap.h"

#define MAP_MAXZOOM				5

static BOOL map_rotate =		TRUE;				  // map rotation enabled?

static int SCREEN_SIZE_X =		640;
static int SCREEN_SIZE_Y =		CPYLINESNUMDEFAULT;	 // defined by svga/vesa.h
static int MAP_SIZE_X =			0;
static int MAP_SIZE_Y =			192;
static int MAP_VIEWPORT_EXT_X =	0;
static int MAP_VIEWPORT_EXT_Y =	0;
static int MAP_VIEWPORT_ORG_X =	0;
static int MAP_VIEWPORT_ORG_Y =	0;
static int MAP_ALIGN_LEFT =		0;
static int MAP_ALIGN_CENTER =	0;
static int MAP_ALIGN_RIGHT =	0;
static int MAP_ALIGN_TOP =		0;
static int MAP_ALIGN_VCENTER =	0;
static int MAP_ALIGN_BOTTOM =	0;

static int map_align_x =		0;				// horizontal alignment offset
static int map_align_y =		0;				  // vertical alignment offset
static int map_x =				0;								   // x origin
static int map_y =				0;								   // y origin
static int map_zoom =			2;			  // zooming, set in int9 with TAB
static int pixel_w =			2;								 // pixel size
static int pixel_hw =			1;							// half pixel size
static int carpixel_w =			2;							 // car pixel size
static int carpixel_hw =		1;						// half car pixel size
static int clip_min_x =			0;							  // left clipping
static int clip_max_x =			0;							 // right clipping
static int segscale_base =		8;
static int carscale_base =		19;
static int map_pixel_adjust =	1;
static int map_max_y =			CPYLINESNUMDEFAULT;	// don't draw below this line

static BYTE svga_car_pixelsizes[MAP_MAXZOOM+1] =	{ 1,2,2,3,5,8 };
static BYTE vga_car_pixelsizes[MAP_MAXZOOM+1] =		{ 1,1,1,2,3,5 };
static BYTE *car_pixelsizes = svga_car_pixelsizes;

#define XY2LINEAR(x,y)			(((y))*SCREEN_SIZE_X+(x))
#define CLEARMAP()				memset(picbuf,0,SCREEN_SIZE_X*SCREEN_SIZE_Y)
#define XYROTATE(x,y)			{if(map_rotate){int rx=(int)(((double)x*map_angle_cos)-((double)y*map_angle_sin));	\
								y=(int)(((double)x*map_angle_sin)+((double)y*map_angle_cos));x=rx;}}
#define XY2MAP(x,y)				{x-=map_x;y-=map_y;x=x-(x>>2)+MAP_VIEWPORT_ORG_X+map_align_x;y=y-(y>>2)+MAP_VIEWPORT_ORG_Y+map_align_y;}
#define SETCARBIGPIXEL(l,c)		{if(carpixel_w<=1)picbuf[l]=c;else{int xp,yp;for(yp=-carpixel_hw;yp<carpixel_w-carpixel_hw;yp++)for(xp=-carpixel_hw;xp<carpixel_w-carpixel_hw;xp++)picbuf[l+SCREEN_SIZE_X*yp+xp]=c;}}
#define SETMAPBIGPIXEL(l,c)		{if(pixel_w<=1)picbuf[l]=c;else{int xp,yp;for(yp=-pixel_hw;yp<pixel_w-pixel_hw;yp++)for(xp=-pixel_hw;xp<pixel_w-pixel_hw;xp++)picbuf[l+SCREEN_SIZE_X*yp+xp]=c;}}

static BYTE map_cycle = 0;				 // changed on reselection of PAGE_MAP
static BOOL map_pressed_first = FALSE;

static double map_angle_cos = 1.0;
static double map_angle_sin = 0.0;


//--- Frank 12/99
unsigned char ReDrawBufTrackMap[0x4b000];


// (x, y) is a normalized vector, c is color
static void SETCARMAPPIXEL(int x, int y, BYTE c)
{
	// rotate
	XYROTATE(x, y);
	// map to screen
	XY2MAP(x, y);
	// clip and draw
	if (x >= clip_min_x+carpixel_hw && x < clip_max_x-carpixel_hw && y >= carpixel_hw && y < SCREEN_SIZE_Y-carpixel_hw) {
		int l = XY2LINEAR(x, y);
		SETCARBIGPIXEL(l, c);
	}
}

// (x, y) is a normalized vector, c is color
static void SETSEGMAPPIXEL(int x, int y, BYTE c)
{
	// rotate
	XYROTATE(x, y);
	// map to screen
	XY2MAP(x, y);
	// clip and draw
	if (x >= clip_min_x+pixel_hw && x < clip_max_x-pixel_hw && y >= pixel_hw && y < SCREEN_SIZE_Y-pixel_hw) {
		int l = XY2LINEAR(x, y);
		SETMAPBIGPIXEL(l, c);
	}
}


#define SCALESEGPOS(p,zoom)	((int)((p)>>(segscale_base-(zoom))))
#define SCALECARPOS(p,zoom)	((int)((p)>>(carscale_base-(zoom))))
#define SETSEGPIXEL(seg,c)	SETSEGMAPPIXEL(-SCALESEGPOS((seg)->xPos,map_zoom),SCALESEGPOS((seg)->yPos,map_zoom),(c))
#define SETCARPIXEL(car,c)	SETCARMAPPIXEL(-SCALECARPOS((car)->xPos,map_zoom),SCALECARPOS((car)->yPos,map_zoom),(c))

#define CAR_COLOR_RANGE			10					   // for highlighting car
#define CAR_COLOR_PLAYER		73								  // light red
#define CAR_COLOR_PLAYER_PIT	51									 // yellow
#define CAR_COLOR_CC			24								 // light gray
#define CAR_COLOR_CC_BACK		18								// medium gray
#define CAR_COLOR_CC_BACK2		12								  // dark gray
#define CAR_COLOR_CC_PIT		60									 // yellow
#define CAR_COLOR_CC_PIT_BACK	54							  // medium yellow
#define CAR_COLOR_CC_PIT_BACK2	48								// dark yellow
#define CAR_COLOR_CC_CURRENT	223								// light green
#define CAR_COLOR_RETIRED		40										// red

// Not retired if race already over
#define CARRETIRED(car)			(ison((car)->flags_90,32) && isoff((car)->flags_5E,1))
#define CARPITTING(car)			(ison((car)->flags_AD,4))

static BYTE color_offset = 0;
static BOOL color_offset_inc = TRUE;
static BYTE map_color = 1;											  // black

static int split1_seg = -1;
static int split2_seg = -1;

static BOOL map_cfgdone = FALSE;

static void InitMapAlign()
{
	switch (map_cycle % 3) {
		case 1:		map_align_x = MAP_ALIGN_CENTER; break;
		case 2:		map_align_x = MAP_ALIGN_RIGHT; break;
		default:	map_align_x = MAP_ALIGN_LEFT; break;
	}
	switch (map_cycle / 3) {
		case 1:		map_align_y = MAP_ALIGN_VCENTER; break;
		case 2:		map_align_y = MAP_ALIGN_BOTTOM; break;
		default:	map_align_y = MAP_ALIGN_TOP; break;
	}
	if (map_align_x == MAP_ALIGN_CENTER) {
		clip_min_x = 0;
		clip_max_x = SCREEN_SIZE_X;
	} else {
		clip_min_x = map_align_x;
		clip_max_x = map_align_x + MAP_VIEWPORT_EXT_X;
	}
}


void InitTrackMap(void)
{
	if (!map_cfgdone) {
		DWORD *pULongCfg = NULL;
		DWORD val;

		map_max_y = GetCopySvgaLinesNum();
		// set track map color (can be changed with ` too)
		pULongCfg = GetCfgULong("mapTrackColor");
		if (pULongCfg)
			map_color = *pULongCfg & 0xff;
		// set track alignment (can be changed with '8' too)
		pULongCfg = GetCfgULong("mapHAlign");
		if (pULongCfg) {
			switch (*pULongCfg) {
				case 1:		map_cycle = 1; break;
				case 2:		map_cycle = 2; break;
				default:	map_cycle = 0; break;
			}
		}
		pULongCfg = GetCfgULong("mapVAlign");
		if (pULongCfg) {
			switch (*pULongCfg) {
				case 1:		map_cycle += 3; break;
				case 2:		map_cycle += 6; break;
			}
		}
		pULongCfg = GetCfgULong("mapDotSize");
		if (pULongCfg) {
			val = *pULongCfg;
			if (val >= 111111 && val <= 999999) {
				int z;
				BYTE ds;
				for (z=MAP_MAXZOOM; z>=0; --z) {
					ds = (BYTE)(val % 10);
					svga_car_pixelsizes[z] = ds ? ds : 1;
					val /= 10;
				}
			}
		}
		pULongCfg = GetCfgULong("mapDotSizeVGA");
		if (pULongCfg) {
			val = *pULongCfg;
			if (val >= 111111 && val <= 999999) {
				int z;
				BYTE ds;
				for (z=MAP_MAXZOOM; z>=0; --z) {
					ds = (BYTE)(val % 10);
					vga_car_pixelsizes[z] = ds ? ds : 1;
					val /= 10;
				}
			}
		}
		pULongCfg = GetCfgULong("mapZoom");
		if (pULongCfg)
			map_zoom = *pULongCfg;
		if (map_zoom < 0)
			map_zoom = 0;
		else if (map_zoom > MAP_MAXZOOM)
			map_zoom = MAP_MAXZOOM;
		pULongCfg = GetCfgULong("mapRotate");
		if (pULongCfg)
			map_rotate = *pULongCfg != 0;
		pULongCfg = GetCfgULong("mapMaxY");
		if (pULongCfg)
			map_max_y = (int)*pULongCfg;
		if (map_max_y < 0)
			map_max_y = 0;
		else if (map_max_y > GetCopySvgaLinesNum())
			map_max_y = GetCopySvgaLinesNum();

		map_cfgdone = TRUE;
	}
	if (*pUseSVGA) {
		SCREEN_SIZE_X =		640;
		SCREEN_SIZE_Y =		map_max_y;
		MAP_SIZE_Y =		192;								  // 3/4 * 256
		segscale_base =		8;
		carscale_base =		19;
		map_pixel_adjust =	1;
		car_pixelsizes =	svga_car_pixelsizes;
	} else {
		SCREEN_SIZE_X =		320;
		SCREEN_SIZE_Y =		map_max_y>>1;
		MAP_SIZE_Y =		96;									// 3/4 * 256/2
		segscale_base =		9;
		carscale_base =		20;
		map_pixel_adjust =	0;
		car_pixelsizes =	vga_car_pixelsizes;
	}
	MAP_SIZE_X =			SCREEN_SIZE_X;
	MAP_VIEWPORT_EXT_X =	MAP_SIZE_Y;							 // isotropic!
	MAP_VIEWPORT_EXT_Y =	MAP_SIZE_Y;
	MAP_VIEWPORT_ORG_X =	MAP_VIEWPORT_EXT_X>>1;
	MAP_VIEWPORT_ORG_Y =	MAP_VIEWPORT_EXT_Y>>1;
	MAP_ALIGN_CENTER =		(SCREEN_SIZE_X>>1)-(MAP_VIEWPORT_EXT_X>>1);
	MAP_ALIGN_RIGHT =		SCREEN_SIZE_X-MAP_VIEWPORT_EXT_X;
	MAP_ALIGN_VCENTER =		(SCREEN_SIZE_Y>>1)-(MAP_VIEWPORT_EXT_Y>>1);
	MAP_ALIGN_BOTTOM =		SCREEN_SIZE_Y-MAP_VIEWPORT_EXT_Y;
	InitMapAlign();
}


void OnTrackChanged(void)
{
	split1_seg = -1;
	split2_seg = -1;
}


void DrawTrackMap(void)
{
	GP2Car *pCar, *pSelCar;
	BYTE first_pl_car, last_pl_car;
	int c, num_segs, num_pitsegs;
	double map_angle;
	BYTE color, pit_color;

	pSelCar = *ppSelectedCS;
	first_pl_car = GP2_MAX_NUMCARS;
	last_pl_car = 0;
	num_segs = pNumTrackSegs[0];
	num_pitsegs = pNumTrackSegs[1];
	if (map_pressed_first) {
		map_pressed_first = FALSE;
		sprintf(strbuf, "Enabling track map\n  color = %d, halign = %d, valign = %d, zoom = %d)\n",
				map_color, map_cycle%3, map_cycle/3, map_zoom);
		VisLogLine(strbuf);
	}

	// update color blink
	if (color_offset_inc) {
		if (color_offset < CAR_COLOR_RANGE) color_offset++; else color_offset_inc = FALSE;
	} else {
		if (color_offset > 0) color_offset--; else color_offset_inc = TRUE;
	}

	CLEARMAP();

	// precalculate some values
	if (map_rotate) {
		map_angle = pSelCar ? (((double)(0x8000-pSelCar->zAngle) * PI) / 32768.0) : 0.0;
		map_angle_cos = cos(map_angle);
		map_angle_sin = sin(map_angle);
	}
	if (map_zoom && pSelCar) {
		map_x = -SCALECARPOS(pSelCar->xPos,map_zoom);
		map_y = SCALECARPOS(pSelCar->yPos,map_zoom);
		XYROTATE(map_x, map_y);
	} else
		map_x = map_y = 0;
	pixel_w = (1<<map_zoom)+map_pixel_adjust;
	pixel_hw = pixel_w>>1;
	carpixel_w = car_pixelsizes[map_zoom];
	carpixel_hw = carpixel_w>>1;

	// draw track
	pit_color = (map_color+10)&0xff;
	for (c=1; c<num_segs; c++) {
		if (split1_seg == -1 && ison(pTrackSegs[c].fl_split, 1))
			split1_seg = c;
		if (split2_seg == -1 && ison(pTrackSegs[c].fl_split, 2))
			split2_seg = c;
		SETSEGPIXEL(&pTrackSegs[c], map_color);
	}
	if (split1_seg != -1 && split1_seg < num_segs)
		SETSEGPIXEL(&pTrackSegs[split1_seg], pit_color);			 // split1
	if (split2_seg != -1 && split2_seg < num_segs)
		SETSEGPIXEL(&pTrackSegs[split2_seg], pit_color);			 // split2
	SETSEGPIXEL(&pTrackSegs[0], pit_color);						// finish line
	
	// draw pit lane at half width
	pixel_w >>= 1;
	pixel_hw >>= 1;
	for (c=0; c<num_pitsegs; c++)
		SETSEGPIXEL(&pTrackSegs[num_segs+3+c], pit_color);
	// draw cc cars
	for (c=0; c<*pNumCars; c++) {
		pCar = &pCarStructs[c];
		if (GP2_CAR_ISPLAYER(pCar->id)) {  // draw player car(s) after cc cars
			if (c < first_pl_car)
				first_pl_car = c;
			last_pl_car = c;
		} else if (pCar != pSelCar) { // draw selected cc car after other ones
			if (CARRETIRED(pCar)) {
				if (*pSessionMode & 0x80)
					color = CAR_COLOR_RETIRED;
				else
					continue;
			} else if (CARPITTING(pCar)) {
				color = CAR_COLOR_CC_PIT;
				if (pCar->lapNr == *pNumLapsDone)
					color = CAR_COLOR_CC_PIT_BACK;
				else if (pCar->lapNr < *pNumLapsDone)
					color = CAR_COLOR_CC_PIT_BACK2;
			} else if (pCar->lapNr == *pNumLapsDone)
				color = CAR_COLOR_CC_BACK;
			else if (pCar->lapNr < *pNumLapsDone)
				color = CAR_COLOR_CC_BACK2;
			else
				color = CAR_COLOR_CC;
			SETCARPIXEL(pCar, color);
		}
	}
	// draw player cars
	for (c=first_pl_car; c<=last_pl_car; c++) {
		pCar = &pCarStructs[c];
		if (GP2_CAR_ISPLAYER(pCar->id)) {
			if (CARRETIRED(pCar)) {
				if (*pSessionMode & 0x80)
					color = CAR_COLOR_RETIRED;
				else
					continue;
			} else
				color = color_offset + (CARPITTING(pCar) ? CAR_COLOR_PLAYER_PIT : CAR_COLOR_PLAYER);
			SETCARPIXEL(pCar, color);
		}
	}
	// draw selected cc car
	if (pSelCar && !GP2_CAR_ISPLAYER(pSelCar->id)) {
		color = color_offset+CAR_COLOR_CC_CURRENT;
		if (map_zoom) {
			// always at origin, so optimize by drawing directly at logical coordinate (0, 0)
			SETCARBIGPIXEL(XY2LINEAR(MAP_VIEWPORT_ORG_X+map_align_x,MAP_VIEWPORT_ORG_Y+map_align_y), color);
		} else
			SETCARPIXEL(pSelCar, color);
	}

    //--- Frank 12/99
    // this is being made for every frame. Wohhooooo. Maybe optimzing somehow

    SaveThisPage(&ReDrawBufTrackMap, &picbuf, GetCopySvgaLinesNum());
    ReDrawAllPages(&OnTrackMapReDraw);
}

//--- Frank 12/99
void OnTrackMapReDraw(int below)
{
	if (below)
		mypicinsertbelow(&picbuf, &ReDrawBufTrackMap, GetCopySvgaLinesNum());
	else
		mypicinsertabove(&picbuf, &ReDrawBufTrackMap, GetCopySvgaLinesNum());
}


// Called when track map is activated
void OnActivateTrackMap()
{
	map_pressed_first = TRUE;
}

// Key hits while track map is active
void OnModuleTrackMapKeyHit(KeyEvent *mks)
{
	switch (mks->scancode) {
		case KEY_8:
			if (!mks->lctrlactive) {
				map_cycle = (map_cycle + (mks->lshiftactive ? 8 : 1)) % 9;
				InitMapAlign();
			}
			break;
		case KEY_TAB:
			map_zoom = (map_zoom + (mks->lshiftactive ? MAP_MAXZOOM : 1))%(MAP_MAXZOOM+1);
			break;
		case KEY_TICK:
			if (mks->lshiftactive)
				--map_color;
			else
				++map_color;
			break;
		case KEY_M:
			map_rotate = !map_rotate;
			break;
	}
}




////////////////////////////////////////////////////////////////////////////////// //

#define CANVAS_SIZE_X	440
#define CANVAS_SIZE_Y	330
#define CANVAS_ORG_X	179
#define CANVAS_ORG_Y	72

void DrawTrackMenuMap(char *canvas, TrackSegInfo *info)
{
#define SETCANVASPIXEL(l,c)	\
						for (yp = -phw; yp<pw-phw; yp++)	\
							for (xp = -phw; xp<pw-phw; xp++)	\
								canvas[l+640*yp+xp] = c;
#define XY2CANVAS(x,y)	(((y)+CANVAS_ORG_Y)*640+((x)+CANVAS_ORG_X))
#define XSEGPOS2POS(p)	(phw+((32767-(int)p)*(CANVAS_SIZE_X-pw))/65536)
#define YSEGPOS2POS(p)	(phw+(((int)p+32768)*(CANVAS_SIZE_Y-pw))/65536)

	int x, y, xp, yp, s, num = info->num_segs + info->num_pitsegs;
	char c;

	// clear canvas
	int pw = 1, phw = pw/2;
	for (y=0; y<CANVAS_SIZE_Y; y++)
		for (x=0; x<CANVAS_SIZE_X; x++)
			SETCANVASPIXEL(XY2CANVAS(x,y), GP2MENUBLUE);

	// draw segments
	pw = 4; phw = pw/2;
	c = GP2MENUWHITE;
	for (s=0; s<num; s++) {
		GP2SegInfo *seg = &info->segs[s];
		if (s == info->num_segs) {
			// pit segments start now
			c = GP2MENUYELLOW;
			pw /= 2;
			phw /= 2;
		}
		x = XSEGPOS2POS(seg->xPos);
		y = YSEGPOS2POS(seg->yPos);
		SETCANVASPIXEL(XY2CANVAS(x,y), c);
	}
}

