/*
 * L3G4200D.c
 *
 *  Created on: 2015年4月24日
 *      Author: ideapad
 */

#include <stdint.h>
#include "i2c.h"
#include "extern.h"
#include "math.h"
#include "L3G4200D.h"

//**********L3G4200D内部寄存器地址*********
#define WHO_AM_I 0x0F
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24
#define REFERENCE 0x25
#define OUT_TEMP 0x26
#define STATUS_REG 0x27
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define FIFO_CTRL_REG 0x2E
#define FIFO_SRC_REG 0x2F
#define INT1_CFG 0x30
#define INT1_SRC 0x31
#define INT1_TSH_XH 0x32
#define INT1_TSH_XL 0x33
#define INT1_TSH_YH 0x34
#define INT1_TSH_YL 0x35
#define INT1_TSH_ZH 0x36
#define INT1_TSH_ZL 0x37
#define INT1_DURATION 0x38
//****************************************
#define L3G4_SlaveAddress   0x69

void InitL3G4200D()
{
   writeI2C1(L3G4_SlaveAddress,CTRL_REG1, 0x0f);   //
   writeI2C1(L3G4_SlaveAddress,CTRL_REG2, 0x00);   //
   writeI2C1(L3G4_SlaveAddress,CTRL_REG3, 0x08);   //
   writeI2C1(L3G4_SlaveAddress,CTRL_REG4, 0x30);  //+-2000dps
   writeI2C1(L3G4_SlaveAddress,CTRL_REG5, 0x00);
}

void L3G4_getdata(axis_d *gyro)
{
    axis_f temp;
    gyro->X = (readI2C1(L3G4_SlaveAddress, OUT_X_H)<<8)+readI2C1(L3G4_SlaveAddress, OUT_X_L);
    gyro->Y = (readI2C1(L3G4_SlaveAddress, OUT_Y_H)<<8)+readI2C1(L3G4_SlaveAddress, OUT_Y_L);
    gyro->Z = (readI2C1(L3G4_SlaveAddress, OUT_Z_H)<<8)+readI2C1(L3G4_SlaveAddress, OUT_Z_L);
    temp.X = (float)gyro->X*0.07;
    temp.Y = (float)gyro->Y*0.07;
    temp.Z = (float)gyro->Z*0.07;
    gyro->X = (int)temp.X;
    gyro->Y = (int)temp.Y;
    gyro->Z = (int)temp.Z;
}


