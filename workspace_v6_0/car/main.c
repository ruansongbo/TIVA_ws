/*
 * main.c
 *
 *  Created on: 2016年11月9日
 *      Author: bb
 */
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
#include "moto.h"

uint32_t count;

void systeminit(void)
{
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                           SYSCTL_XTAL_16MHZ);
    SysTickPeriodSet(ROM_SysCtlClockGet()/1000);
    IntMasterEnable();
}
void TIMERA0init(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet());
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}
void
TIMER3IntHandler(void)
{
	TimerIntClear(TIMER3_BASE, TIMER_CAPA_MATCH);
	//ROM_TimerLoadSet(TIMER3_BASE, TIMER_A,100);
}
void
Timer0IntHandler(void)
{
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    count = ROM_TimerValueGet(TIMER3_BASE, TIMER_A);
    UARTprintf("Count:%d\r\n",count);
}
void
InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}
void
InitCCP(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);
	ROM_GPIOPinConfigure(GPIO_PB2_T3CCP0);
	ROM_GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2,
                     GPIO_STRENGTH_6MA, GPIO_PIN_TYPE_STD_WPU);
	ROM_TimerConfigure(TIMER3_BASE,TIMER_CFG_A_CAP_COUNT|TIMER_CFG_SPLIT_PAIR);//计时捕获模式，上升沿捕获
	ROM_TimerControlEvent(TIMER3_BASE,TIMER_A,TIMER_EVENT_POS_EDGE);
    ROM_TimerLoadSet(TIMER3_BASE, TIMER_A,100);
    ROM_TimerMatchSet(TIMER3_BASE, TIMER_A, 0);
    ROM_IntEnable(INT_TIMER3A);
    ROM_IntMasterEnable();
    ROM_TimerIntEnable(TIMER3_BASE, TIMER_CAPA_MATCH);
    //ROM_TimerIntEnable(TIMER3_BASE, TIMER_CAPA_EVENT);
    ROM_TimerEnable(TIMER3_BASE, TIMER_A);
}
void
main(void)
{
	systeminit();
	InitConsole();
	TIMERA0init();
	MotorInit();
	InitCCP();
	UARTprintf("hello");
	while(1)
	{

	}
}

