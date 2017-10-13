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
#define key_w       0x0001
#define key_s       0x0002
#define key_a       0x0004
#define key_d       0x0008
#define key_q       0x0010
#define key_e       0x0020
#define key_Shift   0x0040
#define key_Ctrl    0x0080



#define motor1A1_0 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,0x00);
#define motor1A1_1 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,GPIO_PIN_5);
#define motor1A2_0 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0x00);
#define motor1A2_1 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,GPIO_PIN_5);
#define motor1B1_0 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,0x00);
#define motor1B1_1 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,GPIO_PIN_6);
#define motor1B2_0 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7,0x00);
#define motor1B2_1 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_7,GPIO_PIN_7);

#define motor2A1_0 GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7,0x00);
#define motor2A1_1 GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7,GPIO_PIN_7);
#define motor2A2_0 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,0x00);
#define motor2A2_1 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,GPIO_PIN_4);
#define motor2B1_0 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,0x00);
#define motor2B1_1 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,GPIO_PIN_3);
#define motor2B2_0 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,0x00);
#define motor2B2_1 GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_PIN_2);

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
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void systeminit(void)
{
	ROM_FPUEnable();
	ROM_FPULazyStackingEnable();
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	                       SYSCTL_XTAL_16MHZ);
}

void
InitGPIO(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_7);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);
	motor1A1_0;
	motor2A1_0;
	motor1A2_0;
	motor2A2_0;
	motor1B1_0;
	motor2B1_0;
	motor1B2_0;
	motor2B2_0;
}

void Front(void)
{
	motor1A1_1;
	motor1A2_0;
	motor1B1_1;
	motor1B2_0;
	motor2A1_1;
	motor2A2_0;
	motor2B1_1;
	motor2B2_0;
}

void Rear(void)
{
	motor1A1_0;
	motor1A2_1;
	motor1B1_0;
	motor1B2_1;
	motor2A1_0;
	motor2A2_1;
	motor2B1_0;
	motor2B2_1;
}

void Left(void)
{
	motor1A1_0;
	motor1A2_1;
	motor1B1_1;
	motor1B2_0;
	motor2A1_1;
	motor2A2_0;
	motor2B1_0;
	motor2B2_1;
}

void Right(void)
{
	motor1A1_1;
	motor1A2_0;
	motor1B1_0;
	motor1B2_1;
	motor2A1_0;
	motor2A2_1;
	motor2B1_1;
	motor2B2_0;
}

void LF(void)
{
	motor1A1_0;
	motor1A2_0;
	motor1B1_1;
	motor1B2_0;
	motor2A1_1;
	motor2A2_0;
	motor2B1_0;
	motor2B2_0;
}

void RF(void)
{
	motor1A1_1;
	motor1A2_0;
	motor1B1_0;
	motor1B2_0;
	motor2A1_0;
	motor2A2_0;
	motor2B1_1;
	motor2B2_0;
}

void LR(void)
{
	motor1A1_0;
	motor1A2_1;
	motor1B1_0;
	motor1B2_0;
	motor2A1_0;
	motor2A2_0;
	motor2B1_0;
	motor2B2_1;
}

void RR(void)
{
	motor1A1_0;
	motor1A2_0;
	motor1B1_0;
	motor1B2_1;
	motor2A1_0;
	motor2A2_1;
	motor2B1_0;
	motor2B2_0;
}

void LT(void)
{
	motor1A1_0;
	motor1A2_1;
	motor1B1_0;
	motor1B2_1;
	motor2A1_1;
	motor2A2_0;
	motor2B1_1;
	motor2B2_0;
}

void RT(void)
{
	motor1A1_1;
	motor1A2_0;
	motor1B1_1;
	motor1B2_0;
	motor2A1_0;
	motor2A2_1;
	motor2B1_0;
	motor2B2_1;
}

void Stop(void)
{
	motor1A1_0;
	motor1A2_0;
	motor1B1_0;
	motor1B2_0;
	motor2A1_0;
	motor2A2_0;
	motor2B1_0;
	motor2B2_0;
}

uint16_t speed=60;
void Initmotors(void)
{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

	    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
	    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);
	    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_4);
	    GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_4);

	    GPIOPinConfigure(GPIO_PB6_M0PWM0);
	    GPIOPinConfigure(GPIO_PB4_M0PWM2);
	    GPIOPinConfigure(GPIO_PE4_M0PWM4);
	    GPIOPinConfigure(GPIO_PC4_M0PWM6);

	    SysCtlPWMClockSet(SYSCTL_PWMDIV_16);

	    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN|
	                                PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN|
	                        		PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN|
	                            	PWM_GEN_MODE_NO_SYNC);
	    PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN|
	                            	PWM_GEN_MODE_NO_SYNC);

	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 6250);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 6250);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, 6250);
	    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, 6250);

	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,5000);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,5000);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,5000);
	    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,5000);

	    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT|PWM_OUT_2_BIT|PWM_OUT_4_BIT|PWM_OUT_6_BIT, true);

	    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
	    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}

void control(void)
{
	if((RC_Ctl.rc.ch2<1244)&&(RC_Ctl.rc.ch2>804))
	{
		if((RC_Ctl.rc.ch3<1244)&&(RC_Ctl.rc.ch3>804))
		{
			Stop();
		}
		else if(RC_Ctl.rc.ch3<=804)
		{
			Rear();
		}
		else if(RC_Ctl.rc.ch3>=1244)
		{
			Front();
		}
	}
	else if(RC_Ctl.rc.ch2<=804)
	{
		if((RC_Ctl.rc.ch3<1244)&&(RC_Ctl.rc.ch3>804))
		{
			Right();
		}
		else if(RC_Ctl.rc.ch3<=804)
		{
			RR();
		}
		else if(RC_Ctl.rc.ch3>=1244)
		{
			RF();
		}
	}
	else if(RC_Ctl.rc.ch2>=1244)
	{
		if((RC_Ctl.rc.ch3<1244)&&(RC_Ctl.rc.ch3>804))
		{
			Left();
		}
		else if(RC_Ctl.rc.ch3<=804)
		{
			LR();
		}
		else if(RC_Ctl.rc.ch3>=1244)
		{
			LF();
		}
	}
	if(RC_Ctl.rc.ch0<=804)
	{
		LT();
	}
	else if(RC_Ctl.rc.ch0>=1244)
	{
		RT();
	}
}

/*void control(void)
{
    if(RC_Ctl.key.v & key_w)
    {
        Front();
    }
    if(RC_Ctl.key.v & key_s)
    {
        Rear();
    }
    if(RC_Ctl.key.v & key_a)
    {
        Left();
    }
    if(RC_Ctl.key.v & key_d)
    {
        Right();
    }
    if(RC_Ctl.key.v & key_q)
    {
        LT();
    }
    if(RC_Ctl.key.v & key_e)
    {
        RT();
    }
    if(RC_Ctl.key.v & key_Shift)
    {
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,160);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,160);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,160);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,160);
    }
    if(RC_Ctl.key.v & key_Crtl)
    {
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,60);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,60);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,60);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,60);
    }
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,110);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,110);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,110);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6,110);
    Stop();
}*/

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
void main(void)
{
	systeminit();
	InitGPIO();
	Initmotors();
	SysCtlPeripheralClockGating(true);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
	IntEnable(INT_UDMAERR);
	uDMAEnable();
	uDMAControlBaseSet(ui8ControlTable);
	InitUART1Transfer();
	while(1)
	{
	}
}
