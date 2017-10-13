/*
 * MPU9255.c
 *
 *  Created on: 2016��11��19��
 *      Author: bb
 */

#include "MPU9255.h"
#include "SPI.h"

#define t 2000

S_INT16_XYZ MPU9255_ACC_LAST;
S_INT16_XYZ MPU9255_GYRO_LAST;
S_INT16_XYZ MPU9255_MAG_LAST;
S_INT32_XYZ MPU9255_ACC_OFFSET;
S_INT32_XYZ MPU9255_GYRO_OFFSET;
S_INT32_XYZ MPU9255_MAG_OFFSET;

uint8_t MPU9255_Init(void)
{
	if(SPI_Read_register(WHO_AM_I)==0x73)
	{
		SPI_Write_register(USER_CTRL,0X10); //ʹ��MPU9255SPI
    	SPI_Write_register(PWR_MGMT_1,0x80);	//��Դ������λMPU9255
    	SPI_Write_register(SMPLRT_DIV,0x07);	//������1000/(1+7)=125Hz
    	SPI_Write_register(CONFIG,GYRO_BW_5);	//���������¶ȼƵ�Ͳ�˲��� 5Hz
    	SPI_Write_register(GYRO_CONFIG,GYRO_1000DPS);	//�����ǲ�����Χ 0x18 +100dps
    	SPI_Write_register(ACCEL_CONFIG,ACC_2G);	//���ٶȼƲ�����Χ	+-2g
    	SPI_Write_register(ACCEL_CONFIG2,ACC_BW_5);	//���ٶȼƵ�ͨ�˲��� 5Hz
    	//SPI_Write_register(INT_PIN_CFG,0x02);//����Bypassģʽ�����ڿ��Ƶ���ָ����


		MPU9255_ACC_OFFSET.X=0;
		MPU9255_ACC_OFFSET.Y=0;
		MPU9255_ACC_OFFSET.Z=0;

    	MPU9255_ACC_LAST.X=0;
    	MPU9255_ACC_LAST.Y=0;
    	MPU9255_ACC_LAST.Z=0;
    	uint16_t i;

    	for(i=0;i<t;i++)
    	{
    		MPU9255_ReadValue(0);
    		if(i==6)
    		{
    			MPU9255_ACC_OFFSET.X=0;
    			MPU9255_ACC_OFFSET.Y=0;
    			MPU9255_ACC_OFFSET.Z=0;
    		}
		}
    	MPU9255_ACC_OFFSET.X/=t;
    	MPU9255_ACC_OFFSET.Y/=t;
    	MPU9255_ACC_OFFSET.Z=MPU9255_ACC_OFFSET.Z/t-16384;
    	MPU9255_GYRO_OFFSET.X/=t;
    	MPU9255_GYRO_OFFSET.Y/=t;
    	MPU9255_GYRO_OFFSET.Z/=t;
    	return 1;
	}
	return 0;
}
uint8_t MPU9255_ReadValue(uint8_t status)
{
	SPI_Read_Buf(ACCEL_XOUT_H, MPU9255_DataBuffer, 14); //��ȡ14�ֽ�����

	//init status
	if(status == 0)
	{
		MPU9255_ACC_OFFSET.X += ((int16_t)(MPU9255_DataBuffer[0]<<8)) | (MPU9255_DataBuffer)[1];
		MPU9255_ACC_OFFSET.Y += ((int16_t)(MPU9255_DataBuffer[2]<<8)) | (MPU9255_DataBuffer)[3];
		MPU9255_ACC_OFFSET.Z += ((int16_t)(MPU9255_DataBuffer[4]<<8)) | (MPU9255_DataBuffer)[5];
		MPU9255_GYRO_OFFSET.X += ((int16_t)(MPU9255_DataBuffer[8]<<8)) | (MPU9255_DataBuffer)[9];
		MPU9255_GYRO_OFFSET.Y += ((int16_t)(MPU9255_DataBuffer[10]<<8)) | (MPU9255_DataBuffer)[11];
		MPU9255_GYRO_OFFSET.Z += ((int16_t)(MPU9255_DataBuffer[12]<<8)) | (MPU9255_DataBuffer)[13];
	}
	//measure status
	else if(status == 1)
	{
		//���ټ�

		MPU9255_ACC_LAST.X = (((int16_t)(MPU9255_DataBuffer[0]<<8)) | (MPU9255_DataBuffer[1])) - (int16_t)MPU9255_ACC_OFFSET.X;
		MPU9255_ACC_LAST.Y = (((int16_t)(MPU9255_DataBuffer[2]<<8)) | (MPU9255_DataBuffer[3])) - (int16_t)MPU9255_ACC_OFFSET.Y;
		MPU9255_ACC_LAST.Z = (((int16_t)(MPU9255_DataBuffer[4]<<8)) | (MPU9255_DataBuffer[5])) - (int16_t)MPU9255_ACC_OFFSET.Z;

		//�¶�
		MPU9255_TEMP_LAST =  ((int16_t)(MPU9255_DataBuffer[6]<<8)) | (MPU9255_DataBuffer)[7];
		//SPI_Write_register(PWR_MGMT_2,0X00); //ʹ��MPU9255SPI
		//SPI_Read_Buf(GYRO_XOUT_H, MPU9255_DataBuffer, 14);
		//���ٶ�
		MPU9255_GYRO_LAST.X = (((int16_t)(MPU9255_DataBuffer[8]<<8)) | (MPU9255_DataBuffer[9])) - (int16_t)MPU9255_GYRO_OFFSET.X;
		MPU9255_GYRO_LAST.Y = (((int16_t)(MPU9255_DataBuffer[10]<<8)) | (MPU9255_DataBuffer[11])) - (int16_t)MPU9255_GYRO_OFFSET.Y;
		MPU9255_GYRO_LAST.Z = (((int16_t)(MPU9255_DataBuffer[12]<<8)) | (MPU9255_DataBuffer[13])) - (int16_t)MPU9255_GYRO_OFFSET.Z;


/*		MPU9255_ACC_LAST.X /= (float)16384.0;
		MPU9255_ACC_LAST.Y /= (float)16384.0;
		MPU9255_ACC_LAST.Z /= (float)16384.0;

		MPU9255_GYRO_LAST.X /=32.8;
		MPU9255_GYRO_LAST.Y /=32.8;
		MPU9255_GYRO_LAST.Z /=32.8;


		MPU9255_ACC_LAST.Z+=16384;*/
	}
	return 0;
}


