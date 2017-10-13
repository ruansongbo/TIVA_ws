/*
 * mpu9255.c
 *
 *  Created on: 2016年10月23日
 *      Author: bb
 */

#include "mpu9255.h"



uint8_t MPU9255_Read_Reg(uint8_t device_addr, uint8_t reg)
{
	uint8_t data;
	I2C_IF_Write(device_addr, &reg, 1, 0);
	I2C_IF_Read(device_addr, &data, 1);
	return data;
}
uint8_t MPU9255_Write_Reg(uint8_t device_addr, uint8_t reg,uint8_t value)
{
	uint8_t status;
	status = I2C_IF_Write(device_addr, &reg, 1, 0);
	status = I2C_IF_Write(device_addr, &value, 1, 1);
	return status;
}

uint8_t i2c_dev = 0;
uint8_t MPU9255_Init(void)
{
    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
    //
    // Configure PIN_01 for I2C0 I2C_SCL
    //
    MAP_PinTypeI2C(PIN_01, PIN_MODE_1);

    //
    // Configure PIN_02 for I2C0 I2C_SDA
    //
    MAP_PinTypeI2C(PIN_02, PIN_MODE_1);

    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    MPU9255_Write_Reg(MPU9255_ADDR,PWR_MGMT_1,0x00);

    i2c_dev=MPU9255_Read_Reg(MPU9255_ADDR,WHO_AM_I);

    if(i2c_dev == 0x73)
    {
    	MPU9255_Write_Reg(MPU9255_ADDR,PWR_MGMT_1,0x80);	//电源管理，复位MPU9255
    	MPU9255_Write_Reg(MPU9255_ADDR,SMPLRT_DIV,0x07);	//采样率1000/(1+7)=125Hz
    	MPU9255_Write_Reg(MPU9255_ADDR,CONFIG,GYRO_BW_5);	//陀螺仪与温度计低筒滤波器 5Hz
    	MPU9255_Write_Reg(MPU9255_ADDR,GYRO_CONFIG,GYRO_1000DPS);	//陀螺仪测量范围 0x18 +100dps
    	MPU9255_Write_Reg(MPU9255_ADDR,ACCEL_CONFIG,ACC_2G);	//加速度计测量范围	+-2g
    	MPU9255_Write_Reg(MPU9255_ADDR,ACCEL_CONFIG2,ACC_BW_5);	//加速度计低通滤波器 5Hz
    	MPU9255_Write_Reg(MPU9255_ADDR,INT_PIN_CFG,0x02);//
    	MAP_UtilsDelay(80000);
    	i2c_dev = MPU9255_Read_Reg(MAG_ADDR,MAG_WIA);
    	if(i2c_dev == 0x02)
    	{
    		MPU9255_Write_Reg(MAG_ADDR,MAG_CNTL1,0x01);
    		return 0;
    	}

    	return 2;
    }
    else
    	return 1;
}

uint8_t MPU9255_ReadValue(uint8_t status)
{
	uint8_t reg = ACCEL_XOUT_H;
	I2C_IF_Write(MPU9255_ADDR, &reg, 1, 0);
	I2C_IF_Read(MPU9255_ADDR, MPU9255_DataBuffer, 14);
	reg = MAG_XOUT_L;
	I2C_IF_Write(MAG_ADDR, &reg, 1, 0);
	I2C_IF_Read(MAG_ADDR, MagTable, 6);
	MPU9255_Write_Reg(MAG_ADDR,MAG_CNTL1,0x01);
	if(status)
	{
		MPU9255_ACC_LAST.X = (((int16_t)(MPU9255_DataBuffer[0]<<8)) | (MPU9255_DataBuffer[1]));
		MPU9255_ACC_LAST.Y = (((int16_t)(MPU9255_DataBuffer[2]<<8)) | (MPU9255_DataBuffer[3]));
		MPU9255_ACC_LAST.Z = (((int16_t)(MPU9255_DataBuffer[4]<<8)) | (MPU9255_DataBuffer[5]));

		MPU9255_TEMP_LAST =  ((int16_t)(MPU9255_DataBuffer[6]<<8)) | (MPU9255_DataBuffer)[7];

		MPU9255_GYRO_LAST.X = (((int16_t)(MPU9255_DataBuffer[8]<<8)) | (MPU9255_DataBuffer[9]));
		MPU9255_GYRO_LAST.Y = (((int16_t)(MPU9255_DataBuffer[10]<<8)) | (MPU9255_DataBuffer[11]));
		MPU9255_GYRO_LAST.Z = (((int16_t)(MPU9255_DataBuffer[12]<<8)) | (MPU9255_DataBuffer[13]));

		MPU9255_MAG_LAST.X = ((uint16_t)MagTable[1])<<8 | MagTable[0];
		MPU9255_MAG_LAST.Y = ((uint16_t)MagTable[3])<<8 | MagTable[2];
		MPU9255_MAG_LAST.Z = ((uint16_t)MagTable[5])<<8 | MagTable[4];

	}
	else
	{

	}
	return 0;

}
