#ifndef GP2HOOK_H
#define GP2HOOK_H

#include "gp2def.h"


extern BYTE opt_hof25;
extern BYTE opt_spa98;

extern GP2Car *pCurrentCS;	// lammcall sets this to esi for each hook
extern DWORD dwTrackChecksum;		 // lammcall sets this in tfl hook

extern void ToggleFastMotion(void);


////////////////////////////////////////////////////////////////////////////////////

#endif
