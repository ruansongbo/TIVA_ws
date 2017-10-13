/*
 * ADXL345.c
 *
 *  Created on: 2015��4��24��
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
   writeI2C1(ADXL_SlaveAddress,0x31,0x0B);   //������Χ,����16g��13λģʽ
   writeI2C1(ADXL_SlaveAddress,0x2C,0x08);   //�����趨Ϊ12.5 �ο�pdf13ҳ
   writeI2C1(ADXL_SlaveAddress,0x2D,0x08);   //ѡ���Դģʽ   �ο�pdf24ҳ
   writeI2C1(ADXL_SlaveAddress,0x2E,0x80);   //ʹ�� DATA_READY �ж�
   writeI2C1(ADXL_SlaveAddress,0x1E,0x00);   //X ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
   writeI2C1(ADXL_SlaveAddress,0x1F,0x00);   //Y ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
   writeI2C1(ADXL_SlaveAddress,0x20,0x05);   //Z ƫ���� ���ݲ��Դ�������״̬д��pdf29ҳ
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




