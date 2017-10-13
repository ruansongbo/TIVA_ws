/*
 * MS5611.c
 *
 *  Created on: 2015年6月11日
 *      Author: ideapad
 */

#include "MS5611.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "math.h"

uint16_t Cal_C[7];  //用于存放PROM中的8组数据
uint32_t D1_Pres,D2_Temp; // 存放数字压力和温度
float Pressure;             //温度补偿大气压
float dT,Temperature,Temperature2;//实际和参考温度之间的差异,实际温度,中间值
double OFF,SENS;  //实际温度抵消,实际温度灵敏度

void Delay_ms(uint16_t time)
{
    SysCtlDelay(SysCtlClockGet()*time/6000);
}

void MS561101BA_RESET(void)
{
    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
    I2CMasterDataPut(I2C1_BASE, MS561101BA_RST);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE));
}
void MS561101BA_PROM_READ(void)
{
    uint16_t d1,d2;
    uint8_t i;
    for(i=0;i<=6;i++)
    {
        I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
        I2CMasterDataPut(I2C1_BASE, (MS561101BA_PROM_RD+i*2));
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
        while(I2CMasterBusy(I2C1_BASE));

        I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, true);
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        while(I2CMasterBusy(I2C1_BASE));
        d1=I2CMasterDataGet (I2C1_BASE);
        I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        d2=I2CMasterDataGet (I2C1_BASE);
        I2CMasterControl (I2C1_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

        SysCtlDelay(20000);

        Cal_C[i]=(d1<<8)|d2;
    }//打印PROM读取出厂校准数据，检测数据传输是否正常
    UARTprintf("C1 =%d\n",Cal_C[1]);
    UARTprintf("C2 =%d\n",Cal_C[2]);
    UARTprintf("C3 =%d\n",Cal_C[3]);
    UARTprintf("C4 =%d\n",Cal_C[4]);
    UARTprintf("C5 =%d\n",Cal_C[5]);
    UARTprintf("C6 =%d\n",Cal_C[6]);
}
uint32_t MS561101BA_DO_CONVERSION(uint8_t command)
{
    uint32_t conversion=0;
    uint32_t conv1,conv2,conv3;
    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
    I2CMasterDataPut(I2C1_BASE, command);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE));

    Delay_ms(20);

    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, false);
    I2CMasterDataPut(I2C1_BASE, 0x00);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE));

    I2CMasterSlaveAddrSet(I2C1_BASE, MS561101BA_SlaveAddress, true);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C1_BASE));
    conv1=I2CMasterDataGet (I2C1_BASE);
    I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    conv2=I2CMasterDataGet (I2C1_BASE);
    I2CMasterControl (I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
    conv3=I2CMasterDataGet (I2C1_BASE);
    I2CMasterControl (I2C1_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    conversion= (conv1<<16) + (conv2<<8) + conv3;

    return conversion;
}

void MS561101BA_GetTemperature(uint8_t OSR_Temp)
{
    D2_Temp= MS561101BA_DO_CONVERSION(OSR_Temp);
    Delay_ms(10);

    dT=D2_Temp - (((uint32_t)Cal_C[5])<<8);
    Temperature=2000+dT*((uint32_t)Cal_C[6])/8388608.0;
}
void MS561101BA_GetPressure(uint8_t OSR_Pres)
{
    float Aux,OFF2,SENS2;  //温度校验值

    D1_Pres= MS561101BA_DO_CONVERSION(OSR_Pres);
    Delay_ms(10);

    OFF=(uint32_t)(Cal_C[2]<<16)+((uint32_t)Cal_C[4]*dT)/128.0;
    SENS=(uint32_t)(Cal_C[1]<<15)+((uint32_t)Cal_C[3]*dT)/256.0;
    //温度补偿
    if(Temperature < 2000)// second order temperature compensation when under 20 degrees C
    {
        Temperature2 = (dT*dT) / 0x80000000;
        Aux = (Temperature-2000)*(Temperature-2000);
        OFF2 = 2.5*Aux;
        SENS2 = 1.25*Aux;
        if(Temperature < -1500)
        {
            Aux = (Temperature+1500)*(Temperature+1500);
            OFF2 = OFF2 + 7*Aux;
            SENS2 = SENS + 5.5*Aux;
        }
    }else  //(Temperature > 2000)
    {
        Temperature2 = 0;
        OFF2 = 0;
        SENS2 = 0;
    }

    Temperature = Temperature - Temperature2;
    OFF = OFF - OFF2;
    SENS = SENS - SENS2;

    Pressure=(D1_Pres*SENS/2097152.0-OFF)/32768.0;
}
void MS561101BA_Init(void)
{
    MS561101BA_RESET();
    Delay_ms(100);
    MS561101BA_PROM_READ();
    Delay_ms(100);
}

