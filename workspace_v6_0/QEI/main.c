/*
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
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
#include "driverlib/qei.h"
#include "utils/uartstdio.h"

int qei_position;
int qei_velocity;



void InitSystem(void)
{
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                           SYSCTL_XTAL_16MHZ);
    SysTickPeriodSet(ROM_SysCtlClockGet()/1000);
    IntMasterEnable();
}

void
Timer0IntHandler(void)
{
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	qei_position = QEIVelocityGet(QEI0_BASE) * QEIDirectionGet(QEI0_BASE);
	qei_velocity = QEIVelocityGet(QEI1_BASE) * QEIDirectionGet(QEI1_BASE);
	UARTprintf("pos:%d    vel:%d \r \n",qei_position,qei_velocity);
}

void InitTimer0(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet()/100);
    TimerIntRegister(TIMER0_BASE, TIMER_A, &Timer0IntHandler);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
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
QEI1IntHandler(void)
{
	unsigned long status;
	status = QEIIntStatus(QEI1_BASE, false);
	if ((status & QEI_INTTIMER) == QEI_INTTIMER)
	{
		QEIIntClear(QEI1_BASE, QEI_INTTIMER);
		//qei_position = QEIPositionGet(QEI1_BASE);
		qei_position = QEIVelocityGet(QEI0_BASE) * QEIDirectionGet(QEI0_BASE);
		qei_velocity = QEIVelocityGet(QEI1_BASE) * QEIDirectionGet(QEI1_BASE);
		UARTprintf("pos:%d    vel:%d \r \n",qei_position,qei_velocity);
	}
}
void
QEI0IntHandler(void)
{
	unsigned long status;
	status = QEIIntStatus(QEI0_BASE, false);
	if ((status & QEI_INTTIMER) == QEI_INTTIMER)
	{
		QEIIntClear(QEI0_BASE, QEI_INTTIMER);
		//qei_position = QEIPositionGet(QEI1_BASE);
		qei_position = QEIVelocityGet(QEI0_BASE) * QEIDirectionGet(QEI0_BASE);
		qei_velocity = QEIVelocityGet(QEI1_BASE) * QEIDirectionGet(QEI1_BASE);
		UARTprintf("pos:%d    vel:%d \r \n",qei_position,qei_velocity);
	}
}
void GPIOPinUnlockGPIO(uint32_t ui32Port, uint8_t ui8Pins) {
    HWREG(ui32Port + GPIO_O_LOCK) = GPIO_LOCK_KEY;      // Unlock the port
    HWREG(ui32Port + GPIO_O_CR) |= ui8Pins;             // Unlock the Pin
    HWREG(ui32Port + GPIO_O_LOCK) = 0;                  // Lock the port
}
InitQEI(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	QEIConfigure(QEI0_BASE, QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET
		| QEI_CONFIG_QUADRATURE | QEI_CONFIG_SWAP, 0xFFFFFFFF);
	QEIConfigure(QEI1_BASE, QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET
	| QEI_CONFIG_QUADRATURE | QEI_CONFIG_SWAP, 0xFFFFFFFF);
	GPIOPinUnlockGPIO(GPIO_PORTD_BASE, GPIO_PIN_7);     // Unlock the PD7 GPIO
	GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);
	GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6);

	GPIOPinConfigure(GPIO_PD6_PHA0);
	GPIOPinConfigure(GPIO_PD7_PHB0);
	GPIOPinConfigure(GPIO_PC5_PHA1);
	GPIOPinConfigure(GPIO_PC6_PHB1);
	QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_2, SysCtlClockGet()/100); //10 ms period
	QEIVelocityConfigure(QEI1_BASE, QEI_VELDIV_2, SysCtlClockGet()/100); //10 ms period
	QEIVelocityEnable(QEI0_BASE);
	QEIVelocityEnable(QEI1_BASE);
	QEIEnable(QEI0_BASE);
	QEIEnable(QEI1_BASE);

	/*QEIIntEnable(QEI0_BASE, QEI_INTTIMER);
	QEIIntEnable(QEI1_BASE, QEI_INTTIMER);
	QEIIntRegister(QEI0_BASE, &QEI0IntHandler);
	QEIIntRegister(QEI1_BASE, &QEI1IntHandler);
	IntEnable(INT_QEI0);
	IntEnable(INT_QEI1);
	IntMasterEnable();*/

	GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD_WPU);
}

/*void
InitQEI(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	QEIConfigure(QEI1_BASE, QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET
	| QEI_CONFIG_QUADRATURE | QEI_CONFIG_SWAP, 0xFFFFFFFF);
	GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6);


	GPIOPinConfigure(GPIO_PC5_PHA1);
	GPIOPinConfigure(GPIO_PC6_PHB1);
	QEIVelocityConfigure(QEI1_BASE, QEI_VELDIV_2, SysCtlClockGet()/50); //20 ms period
	QEIVelocityEnable(QEI1_BASE);
	QEIEnable(QEI1_BASE);

	QEIIntEnable(QEI1_BASE, QEI_INTTIMER);
	QEIIntRegister(QEI1_BASE, &QEI1IntHandler);
	IntEnable(INT_QEI1);
	IntMasterEnable();

	GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD_WPU);
}*/

void
main(void) {
	InitSystem();
	InitConsole();
	InitQEI();
	InitTimer0();
	UARTprintf("start!!!!!!!!!!!!!!!!!");
	while(1)
	{

	}
}
