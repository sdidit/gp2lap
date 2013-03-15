#ifndef TRACKMAP_H
#define TRACKMAP_H

#include "track.h"

extern void OnTrackMapReDraw(int);
extern void OnActivateTrackMap();
extern void OnModuleTrackMapKeyHit(KeyEvent *mks);
extern void InitTrackMap(void);
extern void OnTrackChanged(void);
extern void DrawTrackMap(void);
extern void DrawTrackMenuMap(char *canvas, TrackSegInfo *info);


//////////////////////////////////////////////////////////////////////////////

#endif
