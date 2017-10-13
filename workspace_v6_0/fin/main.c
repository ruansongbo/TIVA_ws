#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "inc/hw_timer.h"
#include "driverlib/timer.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "utils/uartstdio.h"
#include "stdio.h"
#include "DataUpload.h"
#include "extern.h"
#include "control.h"
#include "moto.h"
#include "time.h"
#include "string.h"
#include "AHRS.h"

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

void systeminit(void)
{
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                           SYSCTL_XTAL_16MHZ);
    SysTickPeriodSet(ROM_SysCtlClockGet()/1000);
    IntMasterEnable();
    SysTickIntEnable();
    SysTickEnable();
}
void TIMERA0init(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet()/100);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}
uint8_t HC_time=0,fall = 0,rise = 0;
void
Timer0IntHandler(void)
{
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    PID_Calculate(&EXP_ANGLE);
    ROM_IntEnable(INT_UART0);
    HC_time++;
}

void InitLED(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);
}

void InitGPIO(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_BOTH_EDGES);
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_7);
    IntEnable(INT_GPIOA);
}

uint32_t start,end,Ult;
void GPIOIntA(void)
{
	GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_7);
	if(fall)
	{
		end = micros();
		Ult = end - start;
		Hight = Ult*0.17;
    	fall = 0;
    	rise = 0;
	}
	if(rise)
	{
		start = micros();
		rise = 0;
		fall = 1;
	}
}
uint8_t launch=0,hover=0,land=0,lock=1,prep = 0;
uint32_t starttime,nowtime;
void
main(void)
{
	systeminit();
 /*   Initoffset();
    InitGPIO();
    InitLED();
    InitPID();
    MotorInit();
    InitPID();
    InitUARTuDMA();
    InitUART1Transfer();
    InitUART0Transfer();
    InitUART2();*/
    TIMERA0init();

    EXP_Alt = 38;
    flyInit();
    EXP_ANGLE.Z = inityaw;
    MotorPwmFlash(5000,5000,5000,5000);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    SysCtlDelay(6000000);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    SysCtlDelay(6000000);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    SysCtlDelay(6000000);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    SysCtlDelay(6000000);
    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    while(1)
    {
    	if(lock)
    	{
    		if(Rc_D.S1 == 3)
    		{
    			FLY_ENABLE = 1;
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    			prep = 1;
    			starttime = millis();
    			lock = 0;
    		}
    	}
    	if(Rc_D.S1 == 2)
    	{
			FLY_ENABLE = 0;
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
			MotorPwmFlash(5000,5000,5000,5000);
			lock = 1;
			InitPID();
    	}
    	if(prep)
    	{
    		nowtime = millis() - starttime;
    		if(nowtime == 1)//×¼±¸
    		{
    		    EXP_ANGLE.X = -2.5;
    		    EXP_ANGLE.Y = 0.8;
    			EXP_Alt = 38;
    		}
    		if(nowtime == 1300)//Æð·É
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    			EXP_Alt =360;
    		    EXP_ANGLE.X = -1.5;
    		    EXP_ANGLE.Y = 0.8;
    		}

    		if(nowtime == 2300)//×ó¼Ó
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    			EXP_Alt =360;
    		    EXP_ANGLE.X = 0.5;
    		    EXP_ANGLE.Y = 2.3;
    		}
    		if(nowtime == 3600)//×ó¼õ
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    			EXP_Alt =360;
    		    EXP_ANGLE.X = -2.5;
    		    EXP_ANGLE.Y = -3.3;
    		}

    		if(nowtime == 4300)//Í£1
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    			EXP_Alt =360;
    		    EXP_ANGLE.X = -2.5;
    		    EXP_ANGLE.Y = -0.8;
    		}
    		if(nowtime == 4800)//R-
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -5.5;
    			EXP_ANGLE.Y = 0.8;
    		}
    		if(nowtime == 6300) //R+
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = 2.5;
    			EXP_ANGLE.Y = -2.8;
    		}
    		if(nowtime == 7000)//Í£2
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -2.5;
    			EXP_ANGLE.Y = 0.8;
    		}

    		if(nowtime == 7500)//P+
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = 0;
    			EXP_ANGLE.Y = -3.8;
    		}
    		if(nowtime == 10000)//P-
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = 0;
    			EXP_ANGLE.Y = 3.8;
    		}

    		if(nowtime == 11000)//Í£3
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -2.5;
    			EXP_ANGLE.Y = -0.8;
    		}



    		if(nowtime == 11500)//R-
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = 4.5;
    			EXP_ANGLE.Y = -0.8;
    		}
    		if(nowtime == 13000)//R+
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -4.5;
    			EXP_ANGLE.Y = -0.8;
    		}

    		if(nowtime == 14000)//Í£4
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -2.5;
    			EXP_ANGLE.Y = 0.8;
    		}

    		if(nowtime == 14500)//P+
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -2.5;
    			EXP_ANGLE.Y = 2.8;
    		}
    		if(nowtime == 15500)//P-
    		{
    			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
    			EXP_Alt =360;
    			EXP_ANGLE.X = -2.5;
    			EXP_ANGLE.Y = -2.8;
    		}
    		if(nowtime == 16500)
    		{
    			EXP_Alt =230;
    		    EXP_ANGLE.X = -2.5;
    		    EXP_ANGLE.Y = 0.8;
    		}

    		if(nowtime == 16700)
    		{
    			EXP_Alt =100;
    		    EXP_ANGLE.X = -2.5;
    		    EXP_ANGLE.Y = 0.8;
    		    MotorPwmFlash(5000,5000,5000,5000);
    		    FLY_ENABLE = 0;
    		}

    	}
    	update();
        if(HC_time==5)
        {
        	HC_time = 0;
        	Atl_PID(EXP_Alt);
        	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);
        	SysCtlDelay(270);
        	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);
        	fall = 0;
        	rise = 1;
        }
    }

}

