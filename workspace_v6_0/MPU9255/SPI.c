/*
 * SPI.c
 *
 *  Created on: 2016年11月19日
 *      Author: bb
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "SPI.h"

//*********************************************************************
//SPI initialization,配置TM4C123GH6PM的SPI硬件
void SPI_TM4C123_init(void)
{

    uint32_t ui32Data;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,SSI_MODE_MASTER,SSI_BitRate,8);
    SSIEnable(SSI0_BASE);
    while(SSIDataGetNonBlocking(SSI0_BASE, &ui32Data)){}
}


//**********************************************************************
uint8_t SPI_RW(uint8_t value)
{
    uint32_t ui32Data;
    uint8_t ui8Data;
    SSIDataPut(SSI0_BASE,value);
    while (SSIBusy(SSI0_BASE)){}
    SSIDataGet(SSI0_BASE, &ui32Data);
    ui8Data = ui32Data & 0xff;
    return(ui8Data);
}
//********************************************************************
//用SPI配置的寄存器函数
uint8_t SPI_Write_register(uint8_t WriteReg,uint8_t Value)//WriteReg要写的寄存器地址，Value要写的寄存器值
{
    uint8_t status;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    status=SPI_RW(WriteReg);
    SPI_RW(Value);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);
    return(status);
}

//**********************************************************************
uint8_t SPI_Write_Buf(uint8_t WriteReg, uint8_t *pBuf, uint8_t bytes)
{
    uint8_t i,status;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    status=SPI_RW(WriteReg);
    for(i=0; i<bytes; i++) // then write all byte in buffer(*pBuf)
    {
        SPI_RW(*pBuf++);
    }
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);
    return(status);
}

//读寄存器
uint8_t SPI_Read_register(uint8_t reg)
{
    uint8_t reg_val;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);
    reg|=0x80;
    SPI_RW(reg);
    reg_val = SPI_RW(0);      /* 读取该寄存器返回数据 */
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);
    return  reg_val;
}

//读缓冲区
uint8_t SPI_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t uchars)
{
    uint8_t i;
    uint8_t status;
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0);                         /* 选通器件 */
    reg|=0x80;
    status = SPI_RW(reg);   /* 写寄存器地址 */
    for(i=0; i<uchars; i++)
    {
        pBuf[i] = SPI_RW(0); /* 读取返回数据 */
    }
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,GPIO_PIN_3);                        /* 禁止该器件 */
    return  status;
}
