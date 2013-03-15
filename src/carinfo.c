#include "stdinc.h"
#include <math.h>
#include "gp2lap.h"
#include "gp2glob.h"
#include "cfgmain.h"
#include "keys.h"
#include "keyhand.h"
#include "svga/vesa.h"
#include "svga/gp2color.h"
#include "dpmi.h"										 // for mymemcpy()
#include "miscahf.h"							 // for mypicinsert*****()
#include "fonts/myfntlib.h"
#include "carinfo.h"


static int align = 0;


static unsigned char ReDrawBuf[0x4b000];


/*
	             push    esi
                 movzx   eax, [esi+car.carId] ; EAX := carid
                 dec     eax
                 and     eax, 3Fh
                 cmp     eax, 39
                 jle     short loc_6D659
                 mov     eax, 39

 loc_6D659:                              ; rHandleGearRpm+F?j
                 shl     eax, 4          ; EAX := carid * 16
                 movsx   edx, [esi+car.gear] ; EDX := Gear
                 test    edx, 0FFFFFFFFh
                 jns     short loc_6D66A
                 neg     edx

 loc_6D66A:                              ; rHandleGearRpm+23?j
                 cmp     edx, 6
                 jle     short loc_6D674
                 mov     edx, 6

 loc_6D674:                              ; rHandleGearRpm+2A?j
                 movsx   eax, t_rpmGearTab[eax+edx*2]
                 movsx   edx, word_D6024
                 add     eax, edx
                 sub     eax, 6154
                 mov     dword_D6700, eax
                 movzx   esi, [esi+car.revs] ; ESI := curr. rpm
                 sub     esi, 6154
                 mov     edi, dA0000_3rd
                 mov     ebx, 3
                 mov     ebp, 3
                 mov     edx, 0

 loc_6D6AE:                              ; rHandleGearRpm+89?j
                 mov     eax, dword_D66E0[edx*4]
                 cmp     esi, dword_D6700[edx*4]
                 jge     short loc_6D6C3
                 mov     eax, 0

 loc_6D6C3:                              ; rHandleGearRpm+79?j
                 call    CalcCPitLEDs
                 inc     edx
                 cmp     edx, 4
                 jnz     short loc_6D6AE
                 pop     esi
                 retn
*/


void InitCarInfo(void)
{
/*	DWORD *pULongCfg = NULL;
	
	pULongCfg = GetCfgULong("carinfo");
	if (pULongCfg)
		;
*/
}

void OnActivateCarInfo()
{
}

void DrawCarInfo(void)
{
	char info[32];
	unsigned char color = GP2WHITE;
	double speed;
	char gear;
	WORD revs, revs1;
	BYTE id;
	int r, ri;
	GP2Car *pCar = *ppSelectedCS;
	if (pCar == NULL)
		return;

	memset(picbuf, 0, 640*GetCopySvgaLinesNum());

	id = (BYTE) ((pCar->id-1) & 0x3f);
	gear = pCar->gear;
	switch (gear) {
		case (char)-1:
			info[0] = 'R';
			color = 83;	// light red
			break;
		case 0:
			info[0] = 'N';
			color = 143;	// light blue
			break;
		case 1:
			if (ison(pCar->flags_7C, 16)) {	// is it the turning gear?
				info[0] = '@';	// has a special char in fntarial20
				color = 63;	// light yellow
				break;
			} // else fall through
		default:
			info[0] = (char) ('0'+pCar->gear);
			color = GP2WHITE;
			break;
	}
	info[1] = 0;
	if (align == 0)
		WRITEARIAL20(info, picbuf, 10, 10, color);
	else
		WRITEARIAL20R(info, picbuf, 10, *pUseSVGA ? 630 : 310, color);

	revs = pCar->revs;
	if (gear == (char)-1)
		gear = 1;
	revs1 = arGearRevsTable[(id<<3)+gear] + 800;
	ri = 0;
	for (r=0; r<4; r++) {
		if (revs >= revs1)
			ri = r;
		if (r < 3)
			revs1 = arLightRevsTable[r+1]+6154;
	}
	if (revs >= revs1)
		ri = r;
	switch (ri) {
		case 1:		color = 233; break;	// light green
		case 2:		color = 63; break;	// light yellow
		case 3:		color = 83; break;	// pink
		case 4:		color = 47; break;	// light red
		default:	color = GP2WHITE; break;

	}
	sprintf(info, "%5d", pCar->revs);
	if (align == 0)
		WRITEARIAL20(info, picbuf, 40, 10, color);
	else
		WRITEARIAL20R(info, picbuf, 40, *pUseSVGA ? 630 : 310, color);

	color = GP2WHITE;
	speed = MS_TO_KH((double)(pCar->speed) * GP2_SPEED_FACTOR);
	if (!*pIsKPH)
		speed /= KM_FACTOR;
	sprintf(info, "%.1f", speed);
	if (align == 0)
		WRITEARIAL20(info, picbuf, 70, 10, color);
	else
		WRITEARIAL20R(info, picbuf, 70, *pUseSVGA ? 630 : 310, color);

	SaveThisPage(&ReDrawBuf, &picbuf, GetCopySvgaLinesNum());
	ReDrawAllPages(OnCarInfoReDraw);
}

void OnCarInfoReDraw(int below)
{
	if (below)
		mypicinsertbelow(picbuf, ReDrawBuf, GetCopySvgaLinesNum());
	else
		mypicinsertabove(picbuf, ReDrawBuf, GetCopySvgaLinesNum());
}

void OnModuleCarInfoKeyHit(KeyEvent *mks)
{
	switch (mks->scancode) {
		case KEY_1:
			if (!mks->lctrlactive) {
				align = (align + 1) % 2;
			}
			break;
	}
}

