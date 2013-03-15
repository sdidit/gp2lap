#ifndef F1DEF_H
#define F1DEF_H

/*
There are only 4 non-derived units in the metric system
meter, kilogram, second and Ampere
 1 Watt = 1Joule / second
 1 Joule = kg m² / s²
 1 Newton = kg m / s²
*/
// Multiply by these factors to convert from corresponding unit to metrics
#define INCH_FACTOR			0.0254
#define FOOT_FACTOR			0.3048	// gpx uses 16000/52493
#define LBS_FACTOR			0.45359
#define NAUTMILE_FACTOR		1852
#define KNOTMILE_FACTOR		NAUTMILE_FACTOR	// 1 knot = nautical mile / hour
#define MILE_FACTOR			1609
#define GRAVITY_FACTOR		9.81
#define HP_FACTOR			735.499	// 1 BHP = 735.499 Watt
#define KM_FACTOR			((MILE_FACTOR)/1000.0)	// TODO: better name

#define PI					3.1415926535897932384626433832795
#define RAD_TO_DEG(r)		((180.0*(r))/PI)
#define DEG_TO_RAD(d)		((PI*(d))/180.0)
#define RADPS_TO_RPM(r)		((30.0*(r))/PI)
#define RPM_TO_RADPS(d)		((PI*(d))/30.0)
#define MS_TO_KH(s)			((s)*3.6)
#define KH_TO_MS(s)			((s)/3.6)
#define LBSIN_TO_NM(x)		(((x)*LBS_FACTOR*GRAVITY_FACTOR)/INCH_FACTOR)

#define F1_NUM_TRACKS		16
#define F1_NUM_GEARS		6
#define F1_NUM_CARS_RACE	26

#define F1_NUM_PITSTOPS		3

#define F1_NUM_WHEELS		4

//--------- Actually for gp2 only: -----------

#define F1_WHEEL_BASE				3.1623
// TODO: check
#define F1_WHEEL_FRONT_TREAD		1.8669
#define F1_WHEEL_REAR_TREAD			1.8288
#define F1_WHEEL_FRONT_WIDTH		0.3810
#define F1_WHEEL_REAR_WIDTH			0.4191
#define F1_WHEEL_REAR_MASS			16.6
#define F1_TYRE_MASS				3.0
#define F1_CAR_WIDTH				2.2479
#define F1_CAR_LENGTH				4.572
#define F1_CAR_HEIGHT				1.00965
#define F1_CAR_FRONT_SUSP_WIDTH		1.4859
#define F1_CAR_REAR_SUSP_WIDTH		1.4097
#define	F1_CAR_MASS					(1313.0*LBS_FACTOR)
#define F1_DIFFRATIO				0.05812

//--------------------------------------------

/* Define wheel order */
#define LR	0
#define RR	1
#define LF	2
#define RF	3


#endif	/* F1DEF_H */
