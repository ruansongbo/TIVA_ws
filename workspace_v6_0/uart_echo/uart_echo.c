/*
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "utils/uartstdio.h"
#define CE(x)   x ? GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,GPIO_PIN_6) : GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,0x00)
#define CSN(x)  x ? GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,GPIO_PIN_3) : GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,0x00)
#define IRQ     GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7)>>7

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
/*void InitGPIO(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, 0xFF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, 0x7C);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, 0x3e);
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
}*/
void InitGPIO(void)
{
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1);
     GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, 0xFC);
     GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, 0xFF);
     GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, 0x60);
     GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, 0x3e);
     GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_7);
     GPIOPinWrite(GPIO_PORTA_BASE,0xFC,0xFC);
     GPIOPinWrite(GPIO_PORTB_BASE,0xFF,0xFF);
     GPIOPinWrite(GPIO_PORTC_BASE,0x60,0x60);
     GPIOPinWrite(GPIO_PORTE_BASE,0x3e,0x3e);

}
uint16_t ss;
uint32_t sysy;
void main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    sysy = ROM_SysCtlClockGet();
    InitConsole();
    InitGPIO();
    UARTprintf("SSI ->\n");
    UARTprintf("  Mode: SPI\n");
    UARTprintf("  Data: 8-bit\n\n");
    //进入一直发射状态
    while(1)
    {
        if(IRQ)

        {
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);
            //M_SysCtlDelay(SysCtlClockGet());
        }
        else
        {
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0x00);
            //ROM_SysCtlDelay(SysCtlClockGet());
        }
    }
}
