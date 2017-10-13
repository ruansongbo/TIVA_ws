/*
 * BMP085.c
 *
 *  Created on: 2015年4月24日
 *      Author: ideapad
 */
#include <stdint.h>
#include "i2c.h"
#include "extern.h"
#include "math.h"
#include "BMP085.h"
#include <stdbool.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.h"
BMP BMPs;

#define BMP085_SlaveAddress   0x77
#define OSS 0   // Oversampling Setting (note: code is not set up to use other OSS values)

//***BMP085使用
short ac1;
short ac2;
short ac3;
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1;
short b2;
short mb;
short mc;
short md;

long  temperature;
long  pressure;
//********
void Delay_ms(uint16_t time);
//****************************
void  Init_BMP085(void)
{
    ac1 = readI2C1(BMP085_SlaveAddress,0xAA);
    ac1 = (ac1<<8)|readI2C1(BMP085_SlaveAddress,0xAB);

    ac2 = readI2C1(BMP085_SlaveAddress,0xAC);
    ac2 = (ac2<<8)| readI2C1(BMP085_SlaveAddress,0xAD);

    ac3 = readI2C1(BMP085_SlaveAddress,0xAE);
    ac3 = (ac3<<8)| readI2C1(BMP085_SlaveAddress,0xAF);

    ac4 = readI2C1(BMP085_SlaveAddress,0xB0);
    ac4 = (ac4<<8)| readI2C1(BMP085_SlaveAddress,0xB1);

    ac5 = readI2C1(BMP085_SlaveAddress,0xB2);
    ac5 = (ac5<<8)| readI2C1(BMP085_SlaveAddress,0xB3);

    ac6 = readI2C1(BMP085_SlaveAddress,0xB4);
    ac6 = (ac6<<8)| readI2C1(BMP085_SlaveAddress,0xB5);

    b1 = readI2C1(BMP085_SlaveAddress,0xB6);
    b1 = (b1<<8)| readI2C1(BMP085_SlaveAddress,0xB7);

    b2 = readI2C1(BMP085_SlaveAddress,0xB8);
    b2 = (b2<<8)| readI2C1(BMP085_SlaveAddress,0xB9);

    mb = readI2C1(BMP085_SlaveAddress,0xBA);
    mb = (mb<<8)| readI2C1(BMP085_SlaveAddress,0xBB);

    mc = readI2C1(BMP085_SlaveAddress,0xBC);
    mc = (mc<<8)| readI2C1(BMP085_SlaveAddress,0xBD);

    md = readI2C1(BMP085_SlaveAddress,0xBE);
    md = (md<<8)| readI2C1(BMP085_SlaveAddress,0xBF);

}
//*****************************************
void read_BMP085(void)
{

    long ut;
    long up;
    long x1, x2, b5, b6, x3, b3, p;
    unsigned long b4, b7;

    ut = bmp085ReadTemp();     // 读取温度
//  ut = bmp085ReadTemp();     // 读取温度
    up = bmp085ReadPressure();  // 读取压强
//  up = bmp085ReadPressure();  // 读取压强
    x1 = ((long)ut - ac6) * ac5 >> 15;
    x2 = ((long) mc << 11) / (x1 + md);
    b5 = x1 + x2;
    BMPs.temperature = (b5 + 8) >> 4;

     //****************

    b6 = b5 - 4000;
    x1 = (b2 * ((b6 * b6 )>> 12)) >> 11;
    x2 = ac2 * b6 >> 11;
    x3 = x1 + x2;
    b3 = (((long)ac1 * 4 + x3) + 2)/4;
    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;
    b7 = ((unsigned long) up - b3) * (50000 >> OSS);
    if( b7 < 0x80000000)
         p = (b7 * 2) / b4 ;
           else
            p = (b7 / b4) * 2;
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    BMPs.pressure = p + ((x1 + x2 + 3791) >> 4);
}

//********************************************************************
long bmp085ReadTemp(void)
{   short  temp_ut;
    writeI2C1(BMP085_SlaveAddress,0xF4,0x2E);
    SysCtlDelay(4); // max time is 4.5ms
    temp_ut = readI2C1(BMP085_SlaveAddress,0xF6);
    temp_ut = (temp_ut<<8)| readI2C1(BMP085_SlaveAddress,0xF7);
    return (long) temp_ut ;
}
//*************************************************************

long bmp085ReadPressure(void)
{
    long pressure = 0;
    writeI2C1(BMP085_SlaveAddress,0xF4,0x34);
    SysCtlDelay(4); // max time is 4.5ms
    pressure = readI2C1(BMP085_SlaveAddress,0xF6);
    pressure = (pressure<<8)| readI2C1(BMP085_SlaveAddress,0xF7);
    pressure &= 0x0000FFFF;
    return pressure;
}


