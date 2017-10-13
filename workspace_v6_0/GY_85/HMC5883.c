/*
 * HMC5883.c
 *
 *  Created on: 2015Äê4ÔÂ24ÈÕ
 *      Author: ideapad
 */

#include "HMC5883.h"
#include <stdbool.h>
#include <stdint.h>
#include "i2c.h"
#include "extern.h"
#include "math.h"

void Delay_ms(uint16_t time);

void HMC5883_Init(void)
{
    writeI2C1(hmc_SlaveAddress,0x02, 0x00);
}


//******************************************************
int HMC5883_getdata(axis_d *magnet)
{
    uint8_t BUF[6];
    double Angle;
    writeI2C1(hmc_SlaveAddress,0x00, 0x14);
    writeI2C1(hmc_SlaveAddress,0x02, 0x00);
    Delay_ms(10);

    BUF[0]=readI2C1(hmc_SlaveAddress,0x03);//OUT_X_L_A
    BUF[1]=readI2C1(hmc_SlaveAddress,0x04);//OUT_X_H_A

    BUF[2]=readI2C1(hmc_SlaveAddress,0x07);//OUT_Y_L_A
    BUF[3]=readI2C1(hmc_SlaveAddress,0x08);//OUT_Y_H_A

    BUF[4]=readI2C1(hmc_SlaveAddress,0x05);//OUT_Z_L_A
    BUF[5]=readI2C1(hmc_SlaveAddress,0x06);//OUT_Z_H_A

    magnet->X=(BUF[0] << 8) | BUF[1]; //Combine MSB and LSB of X Data output register
    magnet->Y=(BUF[2] << 8) | BUF[3]; //Combine MSB and LSB of Y Data output register
    magnet->Z=(BUF[4] << 8) | BUF[5]; //Combine MSB and LSB of Z Data output register

    if(magnet->X>0x7fff)magnet->X-=0xffff;
    if(magnet->Y>0x7fff)magnet->Y-=0xffff;
    Angle= atan2(magnet->Y,magnet->X) * (180 / 3.14159265) + 180; // angle in degrees

    return Angle;
}

