#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/timer.h"
#include "utils/cpu_usage.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#define min_fs 40
#define max_fs 80
/* ----------------------- RC Channel Definition---------------------------- */
#define RC_CH_VALUE_MIN ((uint16_t)364 )
#define RC_CH_VALUE_OFFSET ((uint16_t)1024)
#define RC_CH_VALUE_MAX ((uint16_t)1684)
/* ----------------------- RC Switch Definition----------------------------- */
#define RC_SW_UP ((uint16_t)1)
#define RC_SW_MID ((uint16_t)3)
#define RC_SW_DOWN ((uint16_t)2)
/* ----------------------- PC Key Definition-------------------------------- */
#define KEY_PRESSED_OFFSET_W ((uint16_t)0x01<<0)
#define KEY_PRESSED_OFFSET_S ((uint16_t)0x01<<1)
#define KEY_PRESSED_OFFSET_A ((uint16_t)0x01<<2)
#define KEY_PRESSED_OFFSET_D ((uint16_t)0x01<<3)
#define KEY_PRESSED_OFFSET_Q ((uint16_t)0x01<<4)
#define KEY_PRESSED_OFFSET_E ((uint16_t)0x01<<5)
#define KEY_PRESSED_OFFSET_SHIFT ((uint16_t)0x01<<6)
#define KEY_PRESSED_OFFSET_CTRL ((uint16_t)0x01<<7)
/* ----------------------- Data Struct ------------------------------------- */
typedef struct
{
	struct
	{
		uint16_t ch0;
		uint16_t ch1;
		uint16_t ch2;
		uint16_t ch3;
		uint8_t s1;
		uint8_t s2;
	}rc;
	struct
	{
		int16_t x;
		int16_t y;
		int16_t z;
		uint8_t press_l;
		uint8_t press_r;
	}mouse;
	struct
	{
		uint16_t v;
	}key;
}RC_Ctl_t;

signed short x_t,y_t,x_d,y_d,angle_x,angle_y,servo1_angle=4688,servo2_angle=4500;
uint8_t x_f,y_f;
/* ----------------------- Internal Data ----------------------------------- */
static RC_Ctl_t RC_Ctl;
int16_t pros=0,proa=0;
int16_t sd=1,fs=min_fs;

//*****************************************************************************
//
// The size of the UART transmit and receive buffers.  They do not need to be
// the same size.
//
//*****************************************************************************
#define UART_RXBUF_SIZE         18

//*****************************************************************************
//
// The transmit and receive buffers used for the UART transfers.  There is one
// transmit buffer and a pair of recieve ping-pong buffers.
//
//*****************************************************************************
static uint8_t g_ui8RxBuf[UART_RXBUF_SIZE];

//*****************************************************************************
//
// The count of uDMA errors.  This value is incremented by the uDMA error
// handler.
//
//*****************************************************************************
static uint32_t g_ui32uDMAErrCount = 0;



//*****************************************************************************
//
// The count of UART buffers filled, one for each ping-pong buffer.
//
//*****************************************************************************


//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
uint8_t ui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(ui8ControlTable, 1024)
uint8_t ui8ControlTable[1024];
#else
uint8_t ui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1)
    {
    }
}
#endif
//**************************motor*********************


void systeminit(void)
{
	ROM_FPUEnable();
	ROM_FPULazyStackingEnable();
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);
}

void control(void)
{
	if(RC_Ctl.mouse.x!=0)
	{
		x_f=1;
		x_t++;
		x_d+=RC_Ctl.mouse.x;
	}
	if(x_f)
	{
		if(RC_Ctl.mouse.x==0)
		{
			angle_x=x_d/x_t;
			x_f=0;
			x_t=0;
			x_d=0;
		}
	}
	if(RC_Ctl.mouse.y!=0)
	{
		y_f=1;
		y_t++;
		y_d+=RC_Ctl.mouse.y;
	}
	if(y_f)
	{
		if(RC_Ctl.mouse.y==0)
		{
			angle_y=y_d/y_t;
			y_f=0;
			y_t=0;
			y_d=0;
		}
	}
	if((angle_x>5)||(angle_x<-5))
	{
		servo1_angle = servo1_angle + (angle_x * 4);
		angle_x=0;
		if(servo1_angle>6250)
		{
			servo1_angle=6250;
		}
		if(servo1_angle<3125)
		{
			servo1_angle=3125;
		}
		PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,servo1_angle);
	}
	if((angle_y>5)||(angle_y<-5))
	{
		servo2_angle = servo2_angle + (angle_y*2);
		angle_y=0;
		if(servo2_angle>4996)
		{
			servo2_angle=4966;
		}
		if(servo2_angle<4498)
		{
			servo2_angle=4498;
		}
		PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,servo2_angle);
	}
	switch(RC_Ctl.rc.s1)
	{
		case 1:
		    proa=0;
		break;
		case 2:
			proa=1;
		break;
		case 3:
			proa=0;
		break;
	}
	switch(RC_Ctl.rc.s2)
	{
		case 1:
			pros = 0;
		break;
		case 2:
			pros = 1;
		break;
		case 3:
			pros = 0;
		break;
	}


}
//*****************************************************************************
//*****************************************************************************
void
uDMAErrorHandler(void)
{
    uint32_t ui32Status;
    ui32Status = ROM_uDMAErrorStatusGet();
    if(ui32Status)
    {
        ROM_uDMAErrorStatusClear();
        g_ui32uDMAErrCount++;
    }
}

//long float right,wrong;
//float rate;
void
UART1IntHandler(void)
{
    uint32_t ui32Status;
    uint32_t ui32Mode;
    ui32Status = ROM_UARTIntStatus(UART1_BASE, 1);
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    ui32Mode = ROM_uDMAChannelModeGet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT);   //主结构体

    if(ui32Mode == UDMA_MODE_STOP)
    {
        ROM_uDMAChannelTransferSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
        							UDMA_MODE_BASIC,
                                   (void *)(UART1_BASE + UART_O_DR),
                                   g_ui8RxBuf, sizeof(g_ui8RxBuf));
		RC_Ctl.rc.ch0 = (g_ui8RxBuf[0]| (g_ui8RxBuf[1] << 8)) & 0x07ff; //!< Channel 0
		RC_Ctl.rc.ch1 = ((g_ui8RxBuf[1] >> 3) | (g_ui8RxBuf[2] << 5)) & 0x07ff; //!< Channel 1
		RC_Ctl.rc.ch2 = ((g_ui8RxBuf[2] >> 6) | (g_ui8RxBuf[3] << 2) | (g_ui8RxBuf[4] << 10)) & 0x07ff;   //!< Channel 2
		RC_Ctl.rc.ch3 = ((g_ui8RxBuf[4] >> 1) | (g_ui8RxBuf[5] << 7)) & 0x07ff; //!< Channel 3
		RC_Ctl.rc.s1 = ((g_ui8RxBuf[5] >> 4)& 0x000C) >> 2; //!< Switch left
		RC_Ctl.rc.s2 = ((g_ui8RxBuf[5] >> 4)& 0x0003); //!< Switch right
		RC_Ctl.mouse.x = g_ui8RxBuf[6] | (g_ui8RxBuf[7] << 8); //!< Mouse X axis
		RC_Ctl.mouse.y = g_ui8RxBuf[8] | (g_ui8RxBuf[9] << 8); //!< Mouse Y axis
		RC_Ctl.mouse.z = g_ui8RxBuf[10] | (g_ui8RxBuf[11] << 8); //!< Mouse Z axis
		RC_Ctl.mouse.press_l = g_ui8RxBuf[12]; //!< Mouse Left Is Press ?
		RC_Ctl.mouse.press_r = g_ui8RxBuf[13]; //!< Mouse Right Is Press ?
		RC_Ctl.key.v = g_ui8RxBuf[14] | (g_ui8RxBuf[15] << 8); //!< KeyBoard value
		if((364<RC_Ctl.rc.ch0)&&(RC_Ctl.rc.ch0<1685)  //验错
		 &&(364<RC_Ctl.rc.ch1)&&(RC_Ctl.rc.ch1<1685)
		 &&(364<RC_Ctl.rc.ch2)&&(RC_Ctl.rc.ch2<1685)
		 &&(364<RC_Ctl.rc.ch3)&&(RC_Ctl.rc.ch3<1685))
		{
			control();
		}
    }
    ROM_uDMAChannelEnable(UDMA_CHANNEL_UART1RX);
}

void
InitUART1Transfer(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 100000,
                            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_EVEN);
    /*ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 115200,
                                UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                UART_CONFIG_PAR_NONE);*/
    ROM_UARTFIFOLevelSet(UART1_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);
    ROM_UARTEnable(UART1_BASE);
    ROM_UARTDMAEnable(UART1_BASE, UART_DMA_RX);
    ROM_IntEnable(INT_UART1);
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_UART1RX,
                                    UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);
    ROM_uDMAChannelControlSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
                              UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 |
                              UDMA_ARB_1024);
    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_UART1RX | UDMA_PRI_SELECT,
    							UDMA_MODE_BASIC,
                               (void *)(UART1_BASE + UART_O_DR),
                               g_ui8RxBuf, sizeof(g_ui8RxBuf));
    ROM_uDMAChannelEnable(UDMA_CHANNEL_UART1RX);
}

void Initmotors(void)
{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

	    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
	    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);
	    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_4);
	    GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_4);
	    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
	    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

	    GPIOPinConfigure(GPIO_PB6_M0PWM0);
	    GPIOPinConfigure(GPIO_PB4_M0PWM2);
	    GPIOPinConfigure(GPIO_PE4_M0PWM4);
	    GPIOPinConfigure(GPIO_PC4_M0PWM6);
	    GPIOPinConfigure(GPIO_PF2_M1PWM6);
	    GPIOPinConfigure(GPIO_PF1_M1PWM5);

	    SysCtlPWMClockSet(SYSCTL_PWMDIV_16);

	    PWMGenConfigure(PWM0_BASE , PWM_GEN_0, PWM_GEN_MODE_UP_DOWN|
	                                PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN|
	                        		PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN|
	                            	PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN|
	                            	PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN|
	    	                                PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN|
	    	                            	PWM_GEN_MODE_NO_SYNC);

	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 62500);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 62500);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, 6250);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, 6250);
	    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 6250);
	    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 6250);

	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,servo1_angle);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,servo2_angle);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,0);
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,0);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,0);
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,0);            //!!!!!!!!!!!!!!!!!!!!!!!!!!!赋初值

	    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT|PWM_OUT_2_BIT|PWM_OUT_4_BIT|PWM_OUT_6_BIT, true);
	    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT|PWM_OUT_5_BIT, true);

	    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
	    PWMGenEnable(PWM1_BASE, PWM_GEN_3);
	    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
}

void TIMERA1init(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, ROM_SysCtlClockGet()/10);
	ROM_IntEnable(INT_TIMER1A);
	ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	ROM_TimerEnable(TIMER1_BASE, TIMER_A);
}

void
Timer1IntHandler(void)
{
	ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	if(pros)
	{
		if((sd<20)&&(sd>=0))
		{
			PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,2000);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,0);
			sd++;
		}
		if((sd<25)&&(sd>=20))
		{
			PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,0);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,2000);
			sd++;
		}
		if(sd==25)
		{
			sd=0;
		}
	}
	else
	{
		PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,0);
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,0);
	}
	if(proa)
	{
		fs++;
		if(fs>max_fs)
		{
			fs=max_fs;
		}
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,fs*43);
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,fs*43);
	}
	else
	{
		fs--;
		if(fs<min_fs)
		{
			fs=min_fs;
		}
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,fs*43);
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,fs*43);
	}
}

int
main(void)
{
	systeminit();
	Initmotors();
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,fs*43);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,fs*43);
	SysCtlDelay(2*SysCtlClockGet());
    ROM_SysCtlPeripheralClockGating(true);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
    ROM_IntEnable(INT_UDMAERR);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(ui8ControlTable);
    InitUART1Transfer();
    TIMERA1init();
    while(1)
    {
    }
}
