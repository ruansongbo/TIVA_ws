/*
 * extern.h
 *
 *  Created on: 2015年6月23日
 *      Author: rsb
 */

#ifndef EXTERN_H_
#define EXTERN_H_


#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#define ACCEL_1G 9.81f

int16_t MOTO1_PWM;
int16_t MOTO2_PWM;
int16_t MOTO3_PWM;
int16_t MOTO4_PWM;

typedef struct
{
    int16_t X;
    int16_t Y;
    int16_t Z;
}axis_short;

typedef struct
{
    int32_t X;
    int32_t Y;
    int32_t Z;
}axis_long;

typedef struct
{
    float X;
    float Y;
    float Z;
}axis_f;

typedef struct
{
    float temperature;
    float pressure;
    float altitude;
}bmp;

typedef struct
{
    float roll;
    float pitch;
    float yaw;
}EULER;

typedef struct {
    int16_t ROLL;
    int16_t PITCH;
    int16_t THROTTLE;
    int16_t YAW;
    int16_t AUX1;
    int16_t AUX2;
    int16_t AUX3;
    int16_t AUX4;
    int16_t AUX5;
    int16_t AUX6;
    uint8_t S1;
    uint8_t S2;
}T_RC_Data;

T_RC_Data Rc_D;       //遥控通道数据
float ACCZbuff[5];
float inityaw;

bmp BMP;
axis_f Accel;
axis_f Gyro;
EULER Euler;
uint8_t FLY_ENABLE,FLY_mode;
axis_f EXP_ANGLE;	//期望角度
uint16_t Hight;
extern float g_Alt_Hight;
uint16_t EXP_Alt;
axis_f Acc_offset;
axis_f GY_offset;
axis_f Euler_offset;
#endif /* EXTERN_H_ */
