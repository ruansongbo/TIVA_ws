#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
//*****************************************************************************
#define CS_0 GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,0x00);
#define CS_1 GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,GPIO_PIN_1);
#define Fs_0 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4,0x00);
#define Fs_1 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4,GPIO_PIN_4);
#define SCLK_0 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,0x00);
#define SCLK_1 GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5,GPIO_PIN_5);
#define com0sta GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4)
#define com1sta GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5)
#define com2sta GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6)
#define com3sta GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7)
unsigned char SCLK;
unsigned int sample;
unsigned int bits,i,j;
unsigned int conversion;
unsigned int com0,com1,com2,com3;
uint32_t clock;
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
//*****************************************************************************
void
Timer0IntHandler(void)
{
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	CS_1;
	SysCtlDelay(32000);
	for(i=0;i<8;i++)
	{
		SCLK_1;
		SysCtlDelay(8000);
		SCLK_0;

		SysCtlDelay(8000);
	}
	SysCtlDelay(32000);
	CS_0;
	SysCtlDelay(32000);
	com0=0;
	com1=0;
	com2=0;
	com3=0;
	for(i=12;i>0;i--)
	{
		SCLK_1;
		SysCtlDelay(8000);
		SCLK_0;
		com0+=com0sta<<i;
		com1+=com1sta<<i;
		com2+=com2sta<<i;
		com3+=com3sta<<i;
		SysCtlDelay(8000);
	}
	SCLK_1;
	SysCtlDelay(8000);
	SCLK_0;
	SysCtlDelay(8000);
	SysCtlDelay(320000);
}

//*****************************************************************************

void
ConfigureUART(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************

void
InitTLV2541(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
}

//*****************************************************************************

void
InitTimerA0(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    IntMasterEnable();
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}
int
main(void)
{
    ROM_FPULazyStackingEnable();
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    ConfigureUART();
    InitTLV2541();
    InitTimerA0();
    while(1)
    {
    }
}
