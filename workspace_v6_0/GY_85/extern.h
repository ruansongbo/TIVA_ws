/*
 * extern.h
 *
 *  Created on: 2015Äê4ÔÂ24ÈÕ
 *      Author: ideapad
 */

#ifndef EXTERN_H_
#define EXTERN_H_


#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int16_t X;
    int16_t Y;
    int16_t Z;
}axis_d;

typedef struct
{
    float X;
    float Y;
    float Z;
}axis_f;

typedef struct
{
    int16_t temperature;
    int16_t pressure;
}BMP;

#endif /* EXTERN_H_ */
