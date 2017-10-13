/*
 * AHRS.c
 *
 *  Created on: 2015Äê8ÔÂ3ÈÕ
 *      Author: rsb
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "stdio.h"
#include "DataUpload.h"
#include "extern.h"
#include "JY901.h"
#include "string.h"
#include "time.h"

uint8_t date_update_status;
uint8_t gyroindex;
axis_f gyrosum,gyrotemp;

axis_f gyrobuff[5];

void update(void)
{
	uint8_t accindex;
	if(date_update_status&0x01)
	{
		Accel.X = (float)stcAcc.a[0]/32768.0*16*9.81f+Acc_offset.X;
		Accel.Y = (float)stcAcc.a[1]/32768.0*16*9.81f+Acc_offset.Y;
		Accel.Z = (float)stcAcc.a[2]/32768.0*16*9.81f+Acc_offset.Z;
		for(accindex = 1; accindex < 5; accindex ++)
		{
			ACCZbuff[accindex-1] = ACCZbuff[accindex];
		}
		ACCZbuff[4] = Accel.Z;
		date_update_status = date_update_status&0xFE;
		Data_Send_Senser();
	}
	if(date_update_status&0x02)
	{
		gyrotemp.X = (float)stcGyro.w[0]/32768.0*2000+GY_offset.X;
		gyrotemp.Y = (float)stcGyro.w[1]/32768.0*2000+GY_offset.Y;
		gyrotemp.Z = (float)stcGyro.w[2]/32768.0*2000+GY_offset.Z;
		for(gyroindex=1;gyroindex<5;gyroindex++)
		{
			gyrobuff[gyroindex-1].X= gyrobuff[gyroindex].X;
			gyrobuff[gyroindex-1].Y= gyrobuff[gyroindex].Y;
			gyrobuff[gyroindex-1].Z = gyrobuff[gyroindex].Z;
			gyrosum.X += gyrobuff[gyroindex-1].X;
			gyrosum.Y += gyrobuff[gyroindex-1].Y;
			gyrosum.Z += gyrobuff[gyroindex-1].Z;
		}
		gyrobuff[4].X = gyrotemp.X;
		gyrobuff[4].Y = gyrotemp.Y;
		gyrobuff[4].Z = gyrotemp.Z;
		Gyro.X = (gyrosum.X + gyrotemp.X)/5;
		Gyro.Y = (gyrosum.Y + gyrotemp.Y)/5;
		Gyro.Z = (gyrosum.Z + gyrotemp.Z)/5;
		gyrosum.X = 0;
		gyrosum.Y = 0;
		gyrosum.Z = 0;
		date_update_status = date_update_status&0xFD;
		Data_Send_Senser();
	}
	if(date_update_status&0x04)
	{
		Euler.roll = (float)stcAngle.Angle[0]/32768.0*180+Euler_offset.Y;
		Euler.pitch = (float)stcAngle.Angle[1]/32768.0*180+Euler_offset.X;
		Euler.yaw = (float)stcAngle.Angle[2]/32768.0*180+Euler_offset.Z;
		date_update_status = date_update_status&0xFB;
		Data_Send_Status();
	}
	if(date_update_status&0x08)
	{
		BMP.pressure = stcPress.lPressure;
		BMP.altitude = (float)stcPress.lAltitude/100;
		date_update_status = date_update_status&0xF7;
		Data_Send_Status();
	}
}

void InitUART2(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinConfigure(GPIO_PD6_U2RX);
	GPIOPinConfigure(GPIO_PD7_U2TX);
	ROM_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
	ROM_UARTConfigSetExpClk(UART2_BASE, ROM_SysCtlClockGet(), 230400,
	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
	                             UART_CONFIG_PAR_NONE));
	IntPrioritySet(INT_UART2, 0x00);
	ROM_IntEnable(INT_UART2);
	ROM_UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);
}

void
UARTIntHandler(void)
{
    uint32_t ui32Status;
	static unsigned char ucRxBuffer[11];
	static unsigned char ucRxCnt = 0;
    ui32Status = ROM_UARTIntStatus(UART2_BASE, true);
    ROM_UARTIntClear(UART2_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART2_BASE))
    {
    	ucRxBuffer[ucRxCnt++]=ROM_UARTCharGetNonBlocking(UART2_BASE);
    	if (ucRxBuffer[0]!=0x55)
    	{
    		ucRxCnt=0;
    		return;
    	}
    	if (ucRxCnt<11) {return;}
    	else
    	{
    		switch(ucRxBuffer[1])
    		{
    			case 0x51:
    				memcpy(&stcAcc,&ucRxBuffer[2],8);
    				date_update_status = date_update_status|0x01;
    				break;
    			case 0x52:
    				memcpy(&stcGyro,&ucRxBuffer[2],8);
    				date_update_status = date_update_status|0x02;
    				break;
    			case 0x53:
    				memcpy(&stcAngle,&ucRxBuffer[2],8);
    				date_update_status = date_update_status|0x04;
    				break;
    			case 0x56:
    				memcpy(&stcPress,&ucRxBuffer[2],8);
    				date_update_status = date_update_status|0x08;
    				break;
    		}
    		ucRxCnt=0;
    	}
    }
}

void flyInit(void)
{
	while(!(Accel.Z < 9.82 && Accel.Z>9.79))
	{
		update();
	};
	inityaw = Euler.yaw;
}


//333333333
void Initoffset(void)
{
	Acc_offset.X = -0.02;
	Acc_offset.Y = -0.29;
	Acc_offset.Z = -0.90;
	GY_offset.X = -0.29;
	GY_offset.Y = 0.02;
	GY_offset.Z = 0.08;
	Euler_offset.X = 0.01;
	Euler_offset.Y = -1.77;//roll
	Euler_offset.Z = 0;
}
//222222222222222222
/*void Initoffset(void)
{
	Acc_offset.X = -0.01;
	Acc_offset.Y = 0.10;
	Acc_offset.Z = -0.96;
	GY_offset.X = 0.13;
	GY_offset.Y = -0.02;
	GY_offset.Z = -0.09;
	Euler_offset.X = 0.09;
	Euler_offset.Y = 0.58;//roll
	Euler_offset.Z = 0;
}*/

//1111111111
/*void Initoffset(void)
{
	Acc_offset.X = -0.25;
	Acc_offset.Y = 0.09;
	Acc_offset.Z = 0.08;
	GY_offset.X = 0.05;
	GY_offset.Y = 0.17;
	GY_offset.Z = 0.06;
	Euler_offset.X = 1.67;
	Euler_offset.Y = 0.6;//roll
	Euler_offset.Z = 0;
}*/
