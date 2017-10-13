/*
 * main.c
 */

//*********************************************************************
//* MPU9255
//*********************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "SPI.h"
#include "MPU9255.h"

extern S_INT16_XYZ MPU9255_ACC_LAST;
extern S_INT16_XYZ MPU9255_GYRO_LAST;
extern S_INT16_XYZ MPU9255_MAG_LAST;

//*********************************************************************************
//UART³õÊ¼»¯
void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}




//***************************************************************************
int main(void)
{
	uint32_t ui32Loop;
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    InitConsole();
    UARTprintf("SSI ->\n");
    UARTprintf("  Mode: SPI\n");
    UARTprintf("  Data: 8-bit\n\n");
    uint8_t ctr;
    SPI_TM4C123_init();
    MPU9255_Init();
    while(1)
    {
    	//ctr = SPI_Read_register(PWR_MGMT_1);
    	//UARTprintf("who:%X\r\n",ctr);
    	if(MPU9255_Init())
    	{
    		MPU9255_ReadValue(1);
    		UARTprintf("X:%d Y:%d Z:%d \r\n",MPU9255_ACC_LAST.X,MPU9255_ACC_LAST.Y,MPU9255_ACC_LAST.Z);
    		UARTprintf("X:%d Y:%d Z:%d \r\n",MPU9255_GYRO_LAST.X,MPU9255_GYRO_LAST.Y,MPU9255_GYRO_LAST.Z);
    		//UARTprintf("who:%X\r\n",who);
    		for(ui32Loop = 0; ui32Loop < 80; ui32Loop++)
    		{
    		}
    	}
    }
}
