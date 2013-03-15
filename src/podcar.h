#ifndef PODCAR_H
#define PODCAR_H

// TODO: rotational velocity around each axis
// TODO: car acceleration along each axis
// TODO: rotational acceleration around each axis

// car position & orientation
// car velocity along each axis
// time stamp
// car speed
// engine speed
// wheelspin values
// steering angle
// current gear

#pragma pack(push, 1)

typedef struct
{
	double pos[3];			// x, y, z; in m
	double angle[3];		// x, y, z; in rad
	double speed[3];		// x, y, z; in m/s
	double carspeed;		// in m/s
	double wheelSpeed[4];	// lr, rr, lf, rf in m/s
	double steer;			// in rad
	unsigned long time;		// in ms
	unsigned short revs;	// in rpm
	char gear;				// -1,0,1,2,3,4,5,6
} PodCarStruct;

#pragma pack(pop)


#endif
