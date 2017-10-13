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
#include "utils/cpu_usage.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#define increase_0 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);
#define increase_1 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,GPIO_PIN_1);
#define keep_0 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
#define keep_1 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
#define decrease_0 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3,0x00);
#define decrease_1 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3,GPIO_PIN_3);
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

signed short x_t,y_t,x_d,y_d,angle_x,angle_y,servo1_angle=550,servo2_angle=512;
uint8_t x_f,y_f;
/* ----------------------- Internal Data ----------------------------------- */
static RC_Ctl_t RC_Ctl;

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
static uint8_t servocode[9];

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

void
UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    while(ui32Count--)
    {
        ROM_UARTCharPutNonBlocking(UART3_BASE, *pui8Buffer++);
    }
}
void writeaction(uint8_t ID,uint16_t angle)
{
	unsigned char data[11];
	data[0]=0xFF;
	data[1]=0xFF;
	data[2]=ID;
	data[3]=0x07;
	data[4]=0x04;
	data[5]=0x1E;
	data[6]=angle&0x00FF;
	data[7]=angle>>8;
	data[8]=0xff;
	data[9]=0x03;
	data[10]=~((data[2]+data[3]+data[4]+data[5]+data[6]+data[7]+data[8]+data[9])&0xff);
	UARTSend(data, 11);
}
void action(void)
{
	unsigned char data[6];
	data[0]=0xFF;
	data[1]=0xFF;
	data[2]=0xFE;
	data[3]=0x02;
	data[4]=0x05;
	data[5]=0xFA;
	UARTSend(data, 6);
}
void systeminit(void)
{
	ROM_FPUEnable();
	ROM_FPULazyStackingEnable();
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);
}

void control(void)
{
	//UARTprintf("%d\n", RC_Ctl.mouse.x);
	//UARTprintf("%d\n", RC_Ctl.mouse.y);
	//UARTprintf("%d\n\n", RC_Ctl.mouse.z);
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
			UARTprintf("X=%d\n", angle_x);
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
			UARTprintf("Y=%d\n", angle_y);
		}
	}
	if((angle_x>5)||(angle_x<-5))
	{
		servo1_angle=servo1_angle-angle_x;
		angle_x=0;
		if(servo1_angle>850)
		{
			servo1_angle=850;
		}
		if(servo1_angle<=240)
		{
			servo1_angle=240;
		}
		writeaction(0x01,servo1_angle);
	}
	SysCtlDelay(10000);
	if((angle_y>5)||(angle_y<-5))
	{
		servo2_angle+=angle_y;
		angle_y=0;
		if(servo2_angle>812)
		{
			servo2_angle=812;
		}
		if(servo2_angle<212)
		{
			servo2_angle=212;
		}
		writeaction(0x02,servo2_angle);
	}
	SysCtlDelay(10000);
	action();
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
		control();
		/*if((364<RC_Ctl.rc.ch0)&&(RC_Ctl.rc.ch0<1685)  //验错
		 &&(364<RC_Ctl.rc.ch1)&&(RC_Ctl.rc.ch1<1685)
		 &&(364<RC_Ctl.rc.ch2)&&(RC_Ctl.rc.ch2<1685)
		 &&(364<RC_Ctl.rc.ch3)&&(RC_Ctl.rc.ch3<1685))
		{
			right++;
		}
		else
		{
			wrong++;
		}
		if(g_ui32RxBufACount==10000)
		{
			rate=right/g_ui32RxBufACount;
			UARTprintf("rate = %d\n", (int)rate*10000);
		}*/
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
void
ConfigureUART(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}
UART3servoinit(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);
    ROM_GPIOPinConfigure(GPIO_PC6_U3RX);
    ROM_GPIOPinConfigure(GPIO_PC7_U3TX);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);
    ROM_UARTConfigSetExpClk(UART3_BASE, ROM_SysCtlClockGet(), 115200,
    	                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
    	                             UART_CONFIG_PAR_NONE));
}


int
main(void)
{
	systeminit();
    ROM_SysCtlPeripheralClockGating(true);

    ConfigureUART();
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
    ROM_IntEnable(INT_UDMAERR);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(ui8ControlTable);
    InitUART1Transfer();
    UART3servoinit();
    writeaction(0x01,servo1_angle);
    SysCtlDelay(50000);
    writeaction(0x02,servo2_angle);
    SysCtlDelay(50000);
    action();
    while(1)
    {
    }
}
