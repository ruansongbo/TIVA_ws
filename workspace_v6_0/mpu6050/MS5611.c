/*
 * MS5611.c
 *
 *  Created on: 2015年5月3日
 *      Author: ideapad
 */



/**********MS5611模块*************/
/**用途：MS5611模块IIC测试程序****/
/*****串口波特率设置为9600*******
#include <REG52.H>
#include <math.h> //Keil library
#include <stdlib.h> //Keil library
#include <stdio.h> //Keil library
#include <INTRINS.H> //Keil library
#define uint8_t unsigned char
#define uint unsigned int
#define uint32_t unsigned long
sbit SCL=P0^1; //IIC时钟引脚定义
sbit SDA=P0^2; //IIC数据引脚定义
#define MS561101BA_SlaveAddress 0x77 //定义器件在IIC总线中的从地址
#define MS561101BA_D1 0x40
#define MS561101BA_D2 0x50
#define MS561101BA_RST 0x1E
//#define MS561101BA_D1_OSR_256 0x40
//#define MS561101BA_D1_OSR_512 0x42
//#define MS561101BA_D1_OSR_1024 0x44
//#define MS561101BA_D1_OSR_2048 0x46
#define MS561101BA_D1_OSR_4096 0x48
//#define MS561101BA_D2_OSR_256 0x50
//#define MS561101BA_D2_OSR_512 0x52
//#define MS561101BA_D2_OSR_1024 0x54
//#define MS561101BA_D2_OSR_2048 0x56
#define MS561101BA_D2_OSR_4096 0x58
#define MS561101BA_ADC_RD 0x00
#define MS561101BA_PROM_RD 0xA0
#define MS561101BA_PROM_CRC 0xAE
////////////////////////////////////////////////////////////////////////////////////
unsigned int Cal_C[7]; //用于存放PROM中的8组数据
unsigned long D1_Pres,D2_Temp; // 存放压力和温度
float dT,TEMP;
double OFF_,SENS;
float Pressure; //大气压
float TEMP2,Aux,OFF2,SENS2; //温度校验值

//-----------子函数声明-------------

void MS561101BA_RESET();
void MS561101BA_PROM_READ();
uint32_t MS561101BA_DO_CONVERSION(uint8_t command);
void MS561101BA_getTemperature(uint8_t OSR_Temp);
void MS561101BA_getPressure(uint8_t OSR_Pres);
void MS561101BA_Init();
void init_uart();
void SeriPushSend(uint8_t send_data);
void Exchange_Number();
void Delay_ms(uint16_t time);
//-----------------------------------

//=========================================================
//******MS561101BA程序********
//=========================================================
void Delay_ms(uint16_t time)
{
    SysCtlDelay(SysCtlClockGet()*time/1000);
}
void MS561101BA_RESET()
{
    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
    I2CMasterDataPut(I2C1_BASE, MS561101BA_RST);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE));
}
void MS561101BA_PROM_READ()
{
    uint8_t d1,d2,i;
    for(i=0;i<=6;i++)
    {
        I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
        I2CMasterDataPut(I2C1_BASE, (MS561101BA_PROM_RD+i*2));
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, true);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        d1 = I2CMasterDataGet(I2C1_BASE);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        while(I2CMasterBusy(I2C1_BASE))
        {
        }
        d2 = I2CMasterDataGet(I2C1_BASE);
        Delay_ms(5);
        Cal_C[i]=((uint)d1<<8)|d2;
    }
}

uint32_t MS561101BA_DO_CONVERSION(uint8_t command)
{
    uint32_t conversion=0;
    uint32_t conv1,conv2,conv3;
    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
    I2CMasterDataPut(I2C1_BASE, command);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE));

    Delay_ms(1);

    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
    I2CMasterDataPut(I2C1_BASE, 0x00);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }

    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, true);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    conv1 = I2CMasterDataGet(I2C1_BASE);

    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    conv2 = I2CMasterDataGet(I2C1_BASE);

    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C1_BASE))
    {
    }
    conv3 = I2CMasterDataGet(I2C1_BASE);

    conversion=conv1*65535+conv2*256+conv3;
    return conversion;
}
void MS561101BA_getTemperature(uint8_t OSR_Temp)
{
    D2_Temp= MS561101BA_DO_CONVERSION(OSR_Temp);
    Delay_ms(1);
    dT=D2_Temp - (((uint32_t)Cal_C[5])<<8);
    TEMP=2000+dT*((uint32_t)Cal_C[6])/8388608;
}
void MS561101BA_getPressure(uint8_t OSR_Pres)
{
    D1_Pres= MS561101BA_DO_CONVERSION(OSR_Pres);
    Delay_ms(1);
    OFF_=(uint32_t)Cal_C[2]*65536+((uint32_t)Cal_C[4]*dT)/128;
    SENS=(uint32_t)Cal_C[1]*32768+((uint32_t)Cal_C[3]*dT)/256;
    if(TEMP<2000)
    {
        // second order temperature compensation when under 20 degrees C
        T2 = (dT*dT) / 0x80000000;
        Aux = TEMP*TEMP;
        OFF2 = 2.5*Aux;
        SENS2 = 1.25*Aux;
        TEMP = TEMP - TEMP2;
        OFF_ = OFF_ - OFF2;
        SENS = SENS - SENS2;
    }
    Pressure=(D1_Pres*SENS/2097152-OFF_)/32768;
}
void MS561101BA_Init()
{
    MS561101BA_RESET();
    Delay_ms(100);
    MS561101BA_PROM_READ();
    Delay_ms(100);
}

void main()
{
uint8_t i;
delay(50); //上电延时
init_uart(); //串口初始化，波特率9600
MS561101BA_Init(); //MS561101BA初始化
while(1) //循环
{
MS561101BA_getTemperature(MS561101BA_D2_OSR_4096);
MS561101BA_getPressure(MS561101BA_D1_OSR_4096);
Exchange_Number();
for(i=0;i<=7;i++)
{
SeriPushSend(exchange_num[i]);
}
SeriPushSend('m');
SeriPushSend('b');
SeriPushSend('a');
SeriPushSend('r');
SeriPushSend(0x0d);
SeriPushSend(0x0a);//换行，回车
}
}*/
