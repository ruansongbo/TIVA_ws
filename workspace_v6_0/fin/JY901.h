/*
 * JY901.h
 *
 *  Created on: 2015Äê8ÔÂ3ÈÕ
 *      Author: rsb
 */

#ifndef JY901_H_
#define JY901_H_

typedef struct
{
	short a[3];
	short T;
}SAcc;
typedef struct
{
	short w[3];
	short T;
}SGyro;
typedef struct
{
	short Angle[3];
	short T;
}SAngle;

typedef struct
{
	long lPressure;
	long lAltitude;
} SPress;


SAcc 	stcAcc;
SGyro   stcGyro;
SAngle 	stcAngle;
SPress 	stcPress;




#endif /* JY901_H_ */
