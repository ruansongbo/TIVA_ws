/*
 * ADXL345.c
 *
 *  Created on: 2015年4月24日
 *      Author: ideapad
 */
#include  <math.h>
#include "i2c.h"
#include "extern.h"
#include <stdbool.h>
#include <stdint.h>

#define ADXL_SlaveAddress   0x53

void Init_ADXL345()
{
   writeI2C1(ADXL_SlaveAddress,0x31,0x0B);   //测量范围,正负16g，13位模式
   writeI2C1(ADXL_SlaveAddress,0x2C,0x08);   //速率设定为12.5 参考pdf13页
   writeI2C1(ADXL_SlaveAddress,0x2D,0x08);   //选择电源模式   参考pdf24页
   writeI2C1(ADXL_SlaveAddress,0x2E,0x80);   //使能 DATA_READY 中断
   writeI2C1(ADXL_SlaveAddress,0x1E,0x00);   //X 偏移量 根据测试传感器的状态写入pdf29页
   writeI2C1(ADXL_SlaveAddress,0x1F,0x00);   //Y 偏移量 根据测试传感器的状态写入pdf29页
   writeI2C1(ADXL_SlaveAddress,0x20,0x05);   //Z 偏移量 根据测试传感器的状态写入pdf29页
}
void ADXL_getdate(axis_f *data)
{
    uint8_t buf[6];
    axis_d temm;
    IICreadBytes(ADXL_SlaveAddress,0x32,6,buf);
    temm.X = (buf[1]<<8)+buf[0];
    temm.Y = (buf[3]<<8)+buf[2];
    temm.Z = (buf[5]<<8)+buf[4];
    data->X = (temm.X*3.9-5.27)/1000;
    data->Y = (temm.Y*3.9+9.8748)/1000;
    data->Z = (temm.Z*3.9+75.5752)/1000;
}




