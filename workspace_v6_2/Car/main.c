
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "timer.h"
#include "utils.h"
#include "prcm.h"
#include "gpio.h"
#include "Motor.h"
#include "pin.h"
#include "uartdma.h"
#include "uart.h"
#include "string.h"
#include "udma.h"
#include "udma_if.h"
#include "Timer_if.h"
#include "encoder.h"
#include "uart_if.h"

#define APPLICATION_VERSION     "1.1.1"
#define DBG_PRINT               Report
#define car 2

extern char mark0;
extern char mark1;
extern char ucRxBuff[50];
extern char raRxBuff[50];

unsigned char groupcode[8]={
		0xff,0xa1,0xc0,0x00,0x00,0x00,0x00,0x00
}; //群组消息a1,来自c0车辆，收到后广播
unsigned char generalcode[8]={
		0xff,0xb1,0xc0,0x00,0x00,0x00,0x00,0x00
}; //一般消息b1，来自车辆c0
unsigned char rxdata[8]={
		0xca,0x00,0xcc,0x00,0x00,0x00,0x00,0xca
};
int mdata=0;
int wdata=0;
int mwdif=0;
int speedz=50;
int speedy=51;
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//****************************************************************************
//
//! Demonstrates the controlling of LED brightness using PWM
//!
//! \param none
//!
//! This function
//!    1. Pinmux the GPIOs that drive LEDs to PWM mode.
//!    2. Initializes the timer as PWM.
//!    3. Loops to continuously change the PWM value and hence brightness
//!       of LEDs.
//!
//! \return None.
//
//****************************************************************************
void main()
{
    int task=0;
    int connectpi=1;
    int spd1z=0;
    int spd2y=0;

    //
    // Board Initialisation
    //
    BoardInit();

    Zigbeeuartdmainit();
    Raspberryuartdmainit();
    motorinit();
    encoderinit();
    limitencoder(500);
	//move(0,50,0);
	//move(1,55,0);
   // PID_init();

    while(1)
    {
        //
        // RYB - Update the duty cycle of the corresponding timers.
        // This changes the brightness of the LEDs appropriately.
        // The timers used are as per LP schematics.
        //


//    	PID_realize(0,40,0,40);
//    	UARTCharPut(UARTA0_BASE,0xf1);

    	//temp=65535-g_ulSamples[0];
    	//rxdata[3]=temp>>8;
    	//rxdata[4]=temp*0xff;
    	//uart0dmatx(rxdata);
    	//MAP_UtilsDelay(80000000/5);
    	//Message("outputdata:");
    	//UARTCharPut(UARTA0_BASE,temp);
    	//UARTCharPut(UARTA0_BASE,ucRxBuff[0]);
		if(mark0==1)
		{
			mark0=0;
			if(ucRxBuff[0]==0xff)
			{
				   // rxdata[1]=car;
				    //uart0dmatx(rxdata);
				    if (ucRxBuff[1]==0xa1)	//得到的消息为群组消息
				    	{
				    	   if (1==car)		//当前车为1号协调器车，改为一般消息并转发
				    	   {
				    		   ucRxBuff[1]=0xb1;
				    		   uart0dmatx(ucRxBuff);
				    	   }
				    	}
				    if (ucRxBuff[1]==0xb1)
				    {
				    	switch (ucRxBuff[3])
						{
							case 0x00:
								task=0;
								break;
							case 0x01:
								task=1;
								break;
							case 0x02:
								task=2;
								break;
							case 0x03:
								task=3;
								break;
							case 0x04:
								task=4;
								initdelay(5000,TimerBaseIntHandler);
								break;
							case 0x05:
								task=5;
								break;
							case 0x06:
								task=6;
								break;
						}
				    }
			}
		uart0dmarx();
		}
		if(connectpi==1)
		{
			uart1dmatx(raTxBuff);
		}
		if(mark1==1)
				{
					mark1=0;
					connectpi=0;
				//	uart0dmatx(raRxBuff);
				 // len=strlen(raRxBuff);
				//	UARTCharPut(UARTA0_BASE,len);
					if(raRxBuff[0]==0xff)
					{

							switch (raRxBuff[1])
								{
									case 0xd1:
								//	UARTCharPut(UARTA0_BASE,mwdif);
									mdata=(raRxBuff[2]<<24)|(raRxBuff[3]<<16)|(raRxBuff[4]<<8)|raRxBuff[5];
									  break;
									case 0xd2:
							//		UARTCharPut(UARTA0_BASE,0xa2);
									wdata=(raRxBuff[2]<<24)|(raRxBuff[3]<<16)|(raRxBuff[4]<<8)|raRxBuff[5];
									  break;
									case 0x13:
							//		UARTCharPut(UARTA0_BASE,0xa3);;
									  break;
								}
					}
					else
					{
					}
				uart1dmarx();
				}
    	MAP_UtilsDelay(800000);
    	switch (task)
    	{
    	case 0:
    		break;
    	case 1:
//    		rotating(1);
    	{
			mwdif=(wdata/2)-(mdata+30);
		//	mwdif=(int)mwdif/10;
			if(mwdif>20) //前车偏右，需要向右移动，左轮加速，右轮减速
			{
				spd1z=20;
				spd2y=20;
				move(1,spd1z,1);
				move(0,spd2y,0);
			}
			else if(mwdif<-20)
			{
				spd1z=20;
				spd2y=20;
				move(1,spd1z,0);
				move(0,spd2y,1);
			}
			else
			{
				stop(2);
			}
    	}
    		break;
    	case 2:
			move(0,50,1);
			move(1,50,1);
			break;
    	case 3:
    		stop(2);
    		break;
    	case 4:
    		{//move(0,52,1);
    			/*
				mwdif=(wdata/2)-(mdata+30);
				if(mwdif>3) //前车偏左，需要向左移动，左轮减速，右轮加速
				{
					speedz=speedz-1;
					speedy=speedy+1;
				}
				else if(mwdif<-3)
				{
					speedz=speedz+1;
					speedy=speedy-1;
				}*/
    			if (car==1)
    			{
    				move(0,speedy,1);
    				move(1,speedz,1);
    			}
    			else
    			{
    				move(0,50,1);
    				move(1,51,1);
    			}
				wdata=0;
				mdata=0;
			}
    		break;
    	case 5:
			{
				stop(2);
				deinitdelay();
			}
			break;
    	case 6:
    		move(0,53,1);
    		move(1,50,1);
    		delayms(10000);
    		rotating(1);
    		break;

    	}
    }

    //
    // De-Init peripherals - will not reach here...
    //
    //DeInitPWMModules();
}

