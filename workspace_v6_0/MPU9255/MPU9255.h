/*
 * MPU9255.h
 *
 *  Created on: 2016��11��19��
 *      Author: bb
 */

#ifndef MPU9255_H_
#define MPU9255_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct{
				int16_t X;
				int16_t Y;
				int16_t Z;}S_INT16_XYZ;
typedef struct{
				int32_t X;
				int32_t Y;
				int32_t Z;}S_INT32_XYZ;
typedef struct{
				float X;
				float Y;
				float Z;}S_FLOAT_XYZ;
S_INT16_XYZ MPU9255_ACC_LAST;
S_INT16_XYZ MPU9255_GYRO_LAST;
S_INT16_XYZ MPU9255_MAG_LAST;
int16_t MPU9255_TEMP_LAST;
float AngleGyro;
//MPU9255�豸��ַ
#define MPU9255_ADDR 0x68
//bypassģʽ
#define INT_PIN_CFG 0x37
//ָ�����豸��ַ
#define MAG_ADDR 0x0c
//ָ�����豸ID�洢��ַ
#define MAG_WIA 0x00
//ָ�����豸ID
#define MAG_ID 0x48
//�������δ���
#define MAG_CNTL1 0x0a

//��������ƼĴ���
#define MAG_XOUT_L		0x03
#define MAG_XOUT_H		0x04
#define MAG_YOUT_L		0x05
#define MAG_YOUT_H		0x06
#define MAG_ZOUT_L		0x07
#define MAG_ZOUT_H		0x08

//�Ĵ�������
#define SELF_TEST_X_GYRO		0X00
#define SELF_TEST_Y_GYRO		0X01
#define SELF_TEST_Z_GYRO		0X02

#define SELF_TEST_X_ACCEL		0X0D
#define SELF_TEST_Y_ACCEL		0X0E
#define SELF_TEST_Z_ACCEL		0X0F

#define XG_OFFSET_H					0X13
#define XG_OFFSET_L					0X14
#define YG_OFFSET_H					0X15
#define YG_OFFSET_L					0X16
#define ZG_OFFSET_H					0X17
#define ZG_OFFSET_L					0X18

#define SMPLRT_DIV					0X19 //�����ǲ�����	����ֵΪ0X07  1000/(1+7)=125HZ
#define CONFIG						0X1A //��ͨ�˲��� 	����ֵ0x06 5hz
#define GYRO_CONFIG					0X1B //�����ǲ�����Χ 0X18 ����2000��
#define ACCEL_CONFIG				0X1C //���ٶȼƲ�����Χ 0X18 ����16g
#define ACCEL_CONFIG2				0X1D //���ٶȼƵ�ͨ�˲���0x0E 5hz

#define LP_ACCEL_ODR				0X1E
#define WOM_THR						0X1F
#define FIFO_EN						0X23

#define ACCEL_XOUT_H				0X3B  //���ٶȼ��������
#define ACCEL_XOUT_L				0X3C
#define ACCEL_YOUT_H				0X3D
#define ACCEL_YOUT_L				0X3E
#define ACCEL_ZOUT_H				0X3F
#define ACCEL_ZOUT_L				0X40

#define TEMP_OUT_H					0X41  //�¶��������
#define TEMP_OUT_L					0X42

#define GYRO_XOUT_H					0X43  //�������������
#define GYRO_XOUT_L					0X44
#define GYRO_YOUT_H					0X45
#define GYRO_YOUT_L					0X46
#define GYRO_ZOUT_H					0X47
#define GYRO_ZOUT_L					0X48

//����������
#define GYRO_250DPS					0x00	//sensitivity 131LSB/dps
#define GYRO_500DPS					0x08	//sensitivity 65.5LSB/dps
#define GYRO_1000DPS				0x10	//sensitivity 32.8LSB/dps
#define GYRO_2000DPS				0x18	//sensitivity 16.4LSB/dps

//���ٶȼ�����
#define ACC_2G							0x00	//sensitivity 16384LSB/g
#define ACC_4G							0x08	//sensitivity 8192LSB/g
#define ACC_8G							0x10	//sensitivity 4096LSB/g
#define ACC_16G							0x18	//sensitivity 2048LSB/g

//���������������ͨ�˲�����
#define GYRO_BW_3600				0X07
#define GYRO_BW_250					0X00
#define GYRO_BW_184					0X01
#define GYRO_BW_92					0X02
#define GYRO_BW_41					0X03
#define GYRO_BW_20					0X04
#define GYRO_BW_10					0X05
#define GYRO_BW_5					0X06


//���ٶȼ����������ͨ�˲�����
#define ACC_BW_1130					0x00
#define ACC_BW_460					0x08
#define ACC_BW_184					0x09
#define ACC_BW_92						0x0A
#define ACC_BW_41						0x0B
#define ACC_BW_20						0x0C
#define ACC_BW_10						0x0D
#define ACC_BW_5						0x0E

#define PWR_MGMT_1					0X6B //��Դ����1	����ֵΪ0x00
#define PWR_MGMT_2					0X6C //��Դ����2	����ֵΪ0X00

#define WHO_AM_I						0X75 //����ID MPU9255morenIDΪ0X73
#define USER_CTRL						0X6A //�û����� ��Ϊ0X10ʱʹ��SPIģʽ


uint8_t MPU9255_Init(void);
uint8_t MPU9255_ReadValue(uint8_t status);
uint8_t MPU9255_DataBuffer[14];



#endif /* MPU9255_H_ */
